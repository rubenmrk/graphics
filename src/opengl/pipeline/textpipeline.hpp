#pragma once

#include "pipelinebase.hpp"
#include "opengl/vertexinput.hpp"
#include "opengl/text/text.hpp"
#include "opengl/shader.hpp"
#include "font/manager.hpp"

namespace game::opengl
{
	template<class UBO, class VIO>
	class textPipeline : public font::manager, pipelineBase
	{
	public:

		using idtype = uint32_t;

		textPipeline(std::string_view name, float width, float height)
			: _width(width), _height(height), _program(name)
		{
			loadFont(std::string(dir).append("SourceSansPro-Regular.otf"), 12);
		}

		textPipeline(const textPipeline& rhs) = delete;

		textPipeline(textPipeline&& rhs) noexcept
			: _width(rhs._width), _height(rhs._height), 
			_program(std::move(rhs._program)), _vao(std::move(rhs._vao)), 
			_vio(std::move(rhs._vio)), _buffer(rhs._buffer), _texts(std::move(rhs._texts)),
			_idgen(rhs._idgen)
		{
			rhs._width = 0;
			rhs._height = 0;
			rhs._buffer = nullptr;
			rhs._idgen = 0;
		}

		~textPipeline() noexcept
		{
			delete _buffer;
		}

		void render(const glm::mat4& proj, const glm::mat4& view) override
		{
			// (Re)create the buffer
			if (_updateBuffer) {
				if (_buffer)
					delete _buffer;
				_buffer = new buffer(_vio.data(), _vio.size()*sizeof(_vio[0]));
				_vao.bind(*_buffer);
				_updateBuffer = false;
			}

			glUseProgram(_program);
			glBindVertexArray(_vao);
			
			for (auto& [id, pair] : _texts) {
				UBO::update(_program, 0, pair.t._color);
				glBindTextureUnit(0, *pair.t._texture);
				glDrawArrays(GL_TRIANGLES, pair.i, 6);
			}
		}

		idtype loadText(std::string_view txt, float xpos, float ypos, text::anchor attachPos = text::anchor::bottomLeft)
		{
			// Create the text object
			auto id = _idgen++;
			auto txtObject = text(static_cast<manager&>(*this), _width, _height, xpos, ypos, -1.0f, -1.0f, txt, attachPos);
			uint32_t index = _vio.size();

			// Append the indices to the vio
			const auto& vertices = txtObject._vertices;
			_vio.push_back(vertices[0]);
			_vio.push_back(vertices[1]);
			_vio.push_back(vertices[2]);
			_vio.push_back(vertices[3]);
			_vio.push_back(vertices[0]);
			_vio.push_back(vertices[2]);

			// Register the text object
			_texts.emplace(std::pair<idtype, textIndex>(id, textIndex({{std::move(txtObject)}, index})));
		
			_updateBuffer = true;
			return id;
		}

		void replaceText(idtype id, std::string_view txt, float xpos, float ypos, text::anchor attachPos = text::anchor::bottomLeft)
		{
			// Create the new text object
			auto txtObject = text(static_cast<manager&>(*this), _width, _height, xpos, ypos, -1.0f, -1.0f, txt, attachPos);
			auto it = _texts.find(id);
			auto index = it->second.i;

			// Replace the indices
			const auto& vertices = txtObject._vertices;
			_vio[index+0] = vertices[0];
			_vio[index+1] = vertices[1];
			_vio[index+2] = vertices[2];
			_vio[index+3] = vertices[3];
			_vio[index+4] = vertices[0];
			_vio[index+5] = vertices[2];

			// Remove the old txt object and add the new one
			it->second.t = std::move(std::move(txtObject));

			_updateBuffer = true;
		}

		void shiftText(idtype id, float deltax, float deltay)
		{
			auto it = _texts.find(id);
			auto index = it->second.i;

			// Shift the indices
			auto& vertices = it->second.t._vertices;
			vertices[0].inputTexCoord.x += deltax;
			vertices[0].inputTexCoord.y += deltay;
			vertices[1].inputTexCoord.x += deltax;
			vertices[1].inputTexCoord.y += deltay;
			vertices[2].inputTexCoord.x += deltax;
			vertices[2].inputTexCoord.y += deltay;
			vertices[3].inputTexCoord.x += deltax;
			vertices[3].inputTexCoord.y += deltay;

			// Replace the indices
			_vio[index+0] = vertices[0];
			_vio[index+1] = vertices[1];
			_vio[index+2] = vertices[2];
			_vio[index+3] = vertices[3];
			_vio[index+4] = vertices[0];
			_vio[index+5] = vertices[2];

			_updateBuffer = true;
		}

		void removeText(idtype id)
		{
			auto it = _texts.find(id);
			_vio.erase(_vio.begin()+it->second.i, _vio.begin()+(it->second.i+6));
			_texts.erase(id);
			_updateBuffer = true;
		}

		text * getInternalObjectPtr(idtype id)
		{
			auto it = _texts.find(id);
			return &it->second.t;
		}

		const text * getInternalObjectPtr(idtype id) const
		{
			auto it = _texts.find(id);
			return &it->second.t;
		}

	private:

		bool _updateBuffer = false;

		float _width, _height;

		program<UBO, true> _program;

		VAO<VIO> _vao;

		std::vector<VIO> _vio;

		buffer *_buffer = nullptr;

		struct textIndex
		{
			text t;
			uint32_t i;
		};

		std::map<uint32_t, textIndex> _texts;

		idtype _idgen = 0;
	};
}
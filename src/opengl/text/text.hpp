#pragma once

#include "font/manager.hpp"
#include "opengl/vertexinput.hpp"
#include "opengl/texture.hpp"
#include <array>

namespace game::opengl
{
    class text
    {
		template<class UBO, class VIO>
		friend class textPipeline;
		
    public:

		enum class anchor { bottomLeft, bottomRight, topLeft, topRigth, center };

		text(font::manager& fm, float windowWidth, float windowHeight, float xpos, float ypos, float xmax, float ymax, std::string_view str,
			anchor attachPoint = anchor::bottomLeft)
		{
			font::bitmap bmap;
			if (xmax > 0.0f)
				bmap = fm.renderString(str, std::ceil((xmax/2.0)*windowHeight), false);
			else
				bmap = fm.renderString(str, false);

			float xlen = (xmax > 0.0f) ? std::max(static_cast<double>(2*bmap.width()) / windowWidth, static_cast<double>(xmax))
				: static_cast<double>(2*bmap.width()) / windowWidth;
			float ylen = (ymax > 0.0f) ? std::max(static_cast<double>(2*bmap.height()) / windowHeight, static_cast<double>(ymax))
				: static_cast<double>(2*bmap.height()) / windowHeight;

			switch (attachPoint)
			{
			case anchor::bottomLeft:
				break;
			case anchor::bottomRight:
				xpos -= xlen;
				break;
			case anchor::topLeft:
				ypos -= ylen;
				break;
			case anchor::topRigth:
				xpos -= xlen;
				ypos -= ylen;
				break;
			case anchor::center:
				xpos -= (xlen/2);
				ypos -= (ylen/2);
				break;
			}

			_vertices[0] ={{glm::vec3(xpos, ypos, 0.0f)}, {}, {glm::vec2(0.0f, 1.0f)}};
			_vertices[1] ={{glm::vec3(xpos, ypos+ylen, 0.0f)}, {}, {glm::vec2(0.0f, 0.0f)}};
			_vertices[2] ={{glm::vec3(xpos+xlen, ypos+ylen, 0.0f)}, {}, {glm::vec2(1.0f, 0.0f)}};
			_vertices[3] ={{glm::vec3(xpos+xlen, ypos, 0.0f)}, {}, {glm::vec2(1.0f, 1.0f)}};

			_texture = new texture(bmap.data(), bmap.width(), bmap.height());
		}

        text(const text& rhs) = delete;

        text(text&& rhs) noexcept
			: _pos(std::move(rhs._pos)), _length(std::move(rhs._length)), _vertices(std::move(rhs._vertices))
        {
			if (_texture)
				delete _texture;

            _texture = rhs._texture;
            rhs._texture = nullptr;
        }

        ~text()
        {
			delete _texture;
        }

		text& operator=(text&& rhs) noexcept
		{
			delete _texture;
			_pos = std::move(rhs._pos);
			_length = std::move(rhs._length);
			_vertices = std::move(rhs._vertices);
			_texture = rhs._texture;
			rhs._texture = nullptr;
			return *this;
		}

		text& color(const glm::vec3& color)
		{
			_color = color;
			return *this;
		}

		const glm::vec3& color() const
		{
			return _color;
		}

		const glm::vec2& pos() const
		{
			return _pos;
		}

		const glm::vec2& length() const
		{
			return _length;
		}

    private:

		glm::vec3 _color = glm::vec3(1.0f, 1.0f, 1.0f);

		glm::vec2 _pos, _length;

        texture *_texture = nullptr;
		
		std::array<vertexInputObject<true, false, true, false>, 4> _vertices;
    };
}
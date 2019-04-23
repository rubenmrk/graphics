#pragma once

#include "proxy.hpp"
#include "opengl/shadertypes.hpp"

namespace game
{
    class graphics
    {
		using planeModelInfo = typename opengl::ShaderInfo<opengl::ShaderType::PLANE_TEXTURED>;

		using modelInfo = typename opengl::ShaderInfo<opengl::ShaderType::PHONG_TEXTURED>;

		using textInfo = typename opengl::ShaderInfo<opengl::ShaderType::TEXT>;

    public:

		using model = modelProxy<typename modelInfo::pipeline, typename modelInfo::model>;

		using planeModel = modelProxy<typename planeModelInfo::pipeline, typename planeModelInfo::model>;

		using text = textProxy<typename textInfo::pipeline, typename textInfo::text>;

		using anchor = typename textInfo::text::anchor;

        graphics(float width, float height)
			: _modelPipeline(modelInfo::shaderName), _pmodelPipeline(planeModelInfo::shaderName), _textPipeline(textInfo::shaderName, width, height)
		{
			// Setup OpenGL
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_CULL_FACE);

			// Wayland will otherwise render a completely transparent window
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

			// Initialize the projection and view matrices
			proj = glm::perspective(glm::radians(45.0f), width/height, 0.1f, 100.0f);
            view = glm::lookAt(glm::vec3(-2.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		}

		graphics(const graphics& rhs) = delete;

		graphics(graphics&& rhs) = delete;

		~graphics()
		{
		}

		void render()
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			_modelPipeline.render(proj, view);
			_pmodelPipeline.render(proj, view);
			glDisable(GL_DEPTH_TEST);
			_textPipeline.render(proj, view);
			glEnable(GL_DEPTH_TEST);
		}

		model loadModel(std::string_view name)
		{
			return model(&_modelPipeline, _modelPipeline.loadModel(name));
		}

		planeModel loadPlaneModel(std::string_view name)
		{
			return planeModel(&_pmodelPipeline, _pmodelPipeline.loadModel(name));
		}

		text loadText(std::string_view txt, float xpos, float ypos, anchor attachPos = anchor::bottomLeft)
		{
			return text(&_textPipeline, _textPipeline.loadText(txt, xpos, ypos, attachPos));
		}

		void setFontSize(unsigned int number = 12)
		{
			_textPipeline.selectFont(number);
		}

		void setLightPos(const glm::vec3& lightPos)
		{
			_modelPipeline.light.position = lightPos;
		}

		void setAmbientColor(const glm::vec3& ambientColor)
		{
			_modelPipeline.ambient = ambientColor;
		}

		void setDiffuseColor(const glm::vec3& diffuseColor)
		{
			_modelPipeline.light.diffuse = diffuseColor;
		}

		void setspecularColor(const glm::vec3& specularColor)
		{
			_modelPipeline.light.specular = specularColor;
		}

		glm::mat4 proj, view;

    private:

		typename modelInfo::pipeline _modelPipeline;

		typename planeModelInfo::pipeline _pmodelPipeline;

		typename textInfo::pipeline _textPipeline;
    };
}
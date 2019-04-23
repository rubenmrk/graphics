#pragma once

#include "pipelinebase.hpp"
#include "opengl/model/complexmodel.hpp"
#include "opengl/shader.hpp"

namespace game::opengl
{
    template<class UBO, class VIO, class VI>
    class complexModelPipeline : public modelPipelineBase<complexModel<VIO, VI>>
    {
        using base = modelPipelineBase<complexModel<VIO, VI>>;

    public:

        complexModelPipeline(std::string_view name)
            : base(), _program(name)
        {
        }

        complexModelPipeline(const complexModelPipeline& rhs) = delete;

        complexModelPipeline(complexModelPipeline&& rhs)
            : base(std::move(rhs)), _program(std::move(rhs._program))
        {
        }

        ~complexModelPipeline()
        {
        }

        void render(const glm::mat4& proj, const glm::mat4& view) override
        {
            glUseProgram(_program);

			for (auto& [id, m] : _models) {
                auto viewSpace = view * m.modelMatrix;
                auto mvp = proj * viewSpace;
                _program.updateUbo(mvp, viewSpace, ambient, m.material, light);
                m.render();
            }
        }

        glm::vec3 ambient = glm::vec3(0.3f, 0.3f, 0.3f);

        struct Light
        {
            glm::vec3 position;
            glm::vec3 diffuse;
            glm::vec3 specular;
        } light;

    protected:

        using base::_models;

        program<UBO, true> _program;
    };
}
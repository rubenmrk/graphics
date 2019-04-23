#pragma once

#include "pipelinebase.hpp"
#include "opengl/model/basicmodel.hpp"
#include "opengl/shader.hpp"

namespace game::opengl
{
    template<class UBO, class VIO, class VI, bool indexed>
    class basicModelPipeline : public modelPipelineBase<basicModel<VIO, VI, indexed>>
    {
        using base = modelPipelineBase<basicModel<VIO, VI, indexed>>;

    public:

        basicModelPipeline(std::string_view name)
            : base(), _program(name)
        {
        }

        basicModelPipeline(const basicModelPipeline& rhs) = delete;

        basicModelPipeline(basicModelPipeline&& rhs)
            : base(std::move(rhs)), _program(std::move(rhs._program))
        {
        }

        ~basicModelPipeline()
        {
        }

        void render(const glm::mat4& proj, const glm::mat4& view) override
        {
            glUseProgram(_program);

			for (auto&[id, m] : _models) {
                auto mvp = proj * view * m.modelMatrix;
                _program.updateUbo(mvp, 0);
                m.render();
            }
        }

    protected:

        using base::_models;

        program<UBO, true> _program;
    };
}
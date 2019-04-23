#pragma once

#include "modelbase.hpp"

namespace game::opengl
{
    template<class VIO, typename VI, bool indexed>
    class basicModel : modelBase<VIO, VI, indexed>
    {
    public:

        basicModel(std::string_view name)
            : modelBase<VIO, VI, indexed>(name), modelMatrix(1.0f), _diffuse(std::string(_dir).append(name).append(".jpg"))
        {
        }

        basicModel(const basicModel& rhs) = delete;

        basicModel(basicModel&& rhs) noexcept
            : modelBase<VIO, VI, indexed>(std::move(rhs)), modelMatrix(std::move(rhs.modelMatrix)), _diffuse(std::move(rhs._diffuse))
        {
        }

        ~basicModel()
        {
        }

        void render()
        {
            glBindTextureUnit(0, _diffuse);
            modelBase<VIO, VI, indexed>::render();
        }

        glm::mat4 modelMatrix;

    protected:

        using modelBase<VIO, VI, indexed>::_dir;

        texture _diffuse;
    };
}
#pragma once

#include "modelbase.hpp"
#include "basicmodel.hpp"

namespace game::opengl
{
    template<class VIO, typename VI>
    class complexModel : public basicModel<VIO, VI, true>
    {
    public:

        complexModel(std::string_view name)
            : basicModel<VIO, VI, true>(name), _specular(std::string(_dir).append(name).append("_spec.jpg"))
        {
        }

        complexModel(const complexModel& rhs) = delete;

        complexModel(complexModel&& rhs) noexcept
            : basicModel<VIO, VI, true>(std::move(rhs)), material(std::move(rhs.material)), _specular(std::move(rhs._specular))
        {
        }

        ~complexModel()
        {
        }

        void render()
        {
            glBindTextureUnit(1, _specular);
            basicModel<VIO, VI, true>::render();
        }

        struct Material
        {
            const GLint diffuse   = 0;
            const GLint specular  = 1;
            GLfloat shininess = 32.0f;

            Material()
            {
            }

            Material(const Material& rhs)
                : shininess(rhs.shininess)
            {
            }

            Material(Material&& rhs) noexcept
            {
                shininess = rhs.shininess;
                rhs.shininess = 32.0f;
            }

            Material& operator=(const Material& rhs)
            {
                shininess = rhs.shininess;
                return *this;
            }

            Material& operator=(Material&& rhs) noexcept
            {
                shininess = rhs.shininess;
                rhs.shininess = 32.0f;
                return *this;
            }
        } material;

    protected:

        using basicModel<VIO, VI, true>::_dir;

        texture _specular;
    };
}
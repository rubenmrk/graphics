#pragma once

#include "graphics.hpp"
#include "physics.hpp"
#include "base/input_base.hpp"

namespace game
{
    class camera
    {
    public:

        void update(native::input_base& input, graphics& gfx, double dt)
        {
            glm::vec3 Vdot(0.0f, 0.0f, 0.0f);
            // If you add the commented code you get camera controls in 0 gravity (I think)
            if (input.isPressed(native::kbutton::w)) {
                Vdot.x += cosf(_yawpitchroll.y)*cosf(_yawpitchroll.x);
                Vdot.y += sinf(_yawpitchroll.y);
                Vdot.z += sinf(_yawpitchroll.x);
            }
            if (input.isPressed(native::kbutton::s)) {
                Vdot.x -= cosf(_yawpitchroll.y)*cosf(_yawpitchroll.x);
                Vdot.y -= sinf(_yawpitchroll.y);
                Vdot.z -= sinf(_yawpitchroll.x);
            }
            if (input.isPressed(native::kbutton::a)) {
                Vdot.x += cosf(_yawpitchroll.y+glm::half_pi<float>()); // * cosf(_yawpitchroll.x)
                Vdot.y += sinf(_yawpitchroll.y+glm::half_pi<float>());
                //Vdot.z += sinf(_yawpitchroll.x);
            }
            if (input.isPressed(native::kbutton::d)) {
                Vdot.x -= cosf(_yawpitchroll.y+glm::half_pi<float>()); // * cosf(_yawpitchroll.x)
                Vdot.y -= sinf(_yawpitchroll.y+glm::half_pi<float>());
                //Vdot.z += sinf(_yawpitchroll.x);
            }
            if (input.isPressed(native::kbutton::space)) {
                Vdot.z += 1.0f;
            }
            if (input.isPressed(native::kbutton::lshift)) {
                Vdot.z -= 1.0f;
            }

            if (Vdot.x != 0.0f || Vdot.y != 0.0f || Vdot.z != 0.0f)
                _pos += physics::linear::dV(glm::normalize(Vdot)*_mspeed, dt);

            float dx, dy;
            input.getPointerDelta(dx, dy);
            _yawpitchroll.y -= dx * _lspeed.y * cosf(_yawpitchroll.x);
            while (_yawpitchroll.y > glm::two_pi<float>()) _yawpitchroll.y -= glm::two_pi<float>();
            _yawpitchroll.x -= dy * _lspeed.x;
            if (_yawpitchroll.x > glm::half_pi<float>()-0.1f) 
                _yawpitchroll.x = glm::half_pi<float>()-0.1f;
            else if (_yawpitchroll.x < -glm::half_pi<float>()+0.1f)
                _yawpitchroll.x = -glm::half_pi<float>()+0.1f;

            auto direction = glm::vec3(cosf(_yawpitchroll.y)*cosf(_yawpitchroll.x), sinf(_yawpitchroll.y), sinf(_yawpitchroll.x));
            gfx.view = glm::lookAt(_pos, _pos+direction, glm::vec3(0.0f, 0.0f, 1.0f));
        }

    private:

        glm::vec3 _pos = glm::vec3(-2.0f, 0.0f, 1.0f);

        glm::vec3 _yawpitchroll = glm::vec3(0.0f, 0.0f, 0.0f);

#ifdef USE_WIN32 // MS compiler fails using GLM 0.9.9.3 and C++17
		inline static const glm::vec3 _lspeed = glm::vec3(0.0015f, 0.0015f, 0.0015f);

		inline static const glm::vec3 _mspeed = glm::vec3(4*1.5f, 4*1.5f, 4*1.5f);
#elif USE_WAYLAND // GCC and Clang actually compile this fine using GLM 0.9.8.5 and C++17
        static constexpr glm::vec3 _lspeed = glm::vec3(0.0015f, 0.0015f, 0.0015f);

        static constexpr glm::vec3 _mspeed = glm::vec3(4*1.5f, 4*1.5f, 4*1.5f);
#endif
    };
}
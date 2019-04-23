#pragma once

namespace game::physics
{
    namespace linear
    {
        // dV = dV̇ x dt
        template<class VecUnit, class TimeUnit>
        VecUnit dV(const VecUnit& vdot, const TimeUnit& dt)
        {
            // I am doing this until I throw out glm
            return vdot * (float)dt;
        }
    }
}
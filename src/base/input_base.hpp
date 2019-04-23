#pragma once

#include "platform.hpp"
#include <algorithm>

namespace game::native
{
    class input_base
    {
		friend class win32_input;
		friend class wayland_input;

    public:

        input_base()
        {
            std::fill(_keys, _keys+sizeof(_keys), false);
            std::fill(_mbuttons, _mbuttons+sizeof(_mbuttons), false);
            _mdx = _mdy = _mdz = 0;
            _mx = _my = 0;
        }

		input_base(const input_base& rhs)
		{
			*this = rhs;
		}

        input_base(input_base&& rhs) = delete;

		~input_base()
		{
		}

		input_base& operator=(const input_base& rhs)
		{
			std::copy(rhs._keys, rhs._keys+sizeof(_keys), _keys);
			std::copy(rhs._mbuttons, rhs._mbuttons+sizeof(_mbuttons), _mbuttons);
			_mdx = rhs._mdx; _mdy = rhs._mdy; _mdz = rhs._mdz;
			_mx = rhs._mx; _my = rhs._my;
			return *this;
		}

        bool isPressed(mbutton mb)
        {
            return _mbuttons[std::underlying_type_t<mbutton>(mb)];
        }

        bool isPressed(kbutton kb)
        {
            return _keys[std::underlying_type_t<kbutton>(kb)];
        }

        void getPointerPos(float& x, float& y)
        {
            x = _mx;
            y = _my;
        }

        void getPointerDelta(float& dx, float& dy)
        {
            dx = _mdx;
            dy = _mdy;
        }

        void getPointerDelta(float& dx, float& dy, float& dz)
        {
            dx = _mdx;
            dy = _mdy;
            dz = _mdz;
        }

    protected:

        bool _keys[256];

        bool _mbuttons[3];

        float _mdx, _mdy;

        float _mdz;

        float _mx, _my;
    };
}
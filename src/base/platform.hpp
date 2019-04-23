#pragma once

// Include OpenGL (this has to go first because otherwise GL/glx.h will pull in GL/gl.h)

#include <GL/gl3w.h>

// Automatically determine the platform if no platform is specified

#if !defined(USE_WAYLAND) && !defined(USE_XLIB) && !defined(USE_WIN32)
#ifdef __linux__
#define USE_WAYLAND
#elif defined(_WIN32)
#define USE_WIN32
#endif

#endif

// Platform specific includes and definitions

#ifdef USE_WAYLAND

#include <wayland-client.h>
#include "wayland-protocol/xdg-shell.h"
#include "wayland-protocol/pointer-constraints-v1.h"
#include "wayland-protocol/relative-pointer-v1.h"
#include <wayland-egl.h>
#include <EGL/egl.h>
#include <linux/input.h>
#include <time.h>

namespace game::native
{
    enum class kbutton { a = KEY_A, b = KEY_B, c = KEY_C, d = KEY_D, e = KEY_E, f = KEY_F, g = KEY_G, h = KEY_H, i = KEY_I, j = KEY_J, 
        k = KEY_K, l = KEY_L, m = KEY_M, n = KEY_N, o = KEY_O, p = KEY_P, q = KEY_Q, r = KEY_R, s = KEY_S, t = KEY_T, u = KEY_U, v = KEY_V, 
        w = KEY_W, x = KEY_X, y = KEY_Y, z = KEY_Z, zero = KEY_0, one = KEY_1, two = KEY_2, three = KEY_3, four = KEY_4, five = KEY_5, six = KEY_6, 
        seven = KEY_7, eight = KEY_8, nine = KEY_9, lctrl = KEY_LEFTCTRL, lshift = KEY_LEFTSHIFT, escape = KEY_ESC, space = KEY_SPACE 
    };

    enum class mbutton { left = BTN_LEFT, right = BTN_RIGHT, middle = BTN_MIDDLE };
}

#elif defined(USE_WIN32)

#define WINDOWS_LEAN_AND_MEAN
#include <Windows.h>
#include <GL/GL.h>
#include <GL/wglext.h>

namespace game::native
{
	enum class kbutton {
		a = 'A', b = 'B', c = 'C', d = 'D', e = 'E', f = 'F', g = 'G', h = 'H', i = 'I', j = 'J', k = 'K', l = 'L', m = 'M', n = 'N', o = 'O', p = 'P',
		q = 'Q', r = 'R', s = 'S', t = 'T', u = 'U', v = 'V', w = 'W', x = 'X', y = 'Y', z = 'Z', zero = '0', one = '1', two = '2', three = '3', four = '4',
		five = '5', six = '6', seven = '7', eight = '8', nine = '9', lctrl = VK_LCONTROL, lshift = VK_LSHIFT, escape = VK_ESCAPE, space = VK_SPACE
	};

	enum class mbutton { left, right, middle };
}

#endif
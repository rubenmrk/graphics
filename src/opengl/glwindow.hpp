#pragma once

#include "base/platform.hpp"
#include "base/time.hpp"

#ifdef USE_WAYLAND

#include "base/wayland_opengl.hpp"

namespace game::opengl
{
    template<class T>
    using window = native::wayland_opengl<T>;
}

#elif defined(USE_WIN32)

#include "base/win32_opengl.hpp"

namespace game::opengl
{
	template<class T>
	using window = native::win32_opengl<T>;
}

#endif
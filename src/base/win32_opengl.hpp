#pragma once

#include "win32_window.hpp"
#include "application/graphics.hpp"
#include <memory>

namespace game::native
{
	template<class Derived>
	class win32_opengl : public win32_window<Derived>
	{
		friend window_base<Derived>;

		using window_base<Derived>::_displayType;
		using window_base<Derived>::_title;
		using window_base<Derived>::_width;
		using window_base<Derived>::_height;
		using window_base<Derived>::_vsync;
		using window_base<Derived>::_running;

		using window_base<Derived>::createRenderThread;
		using window_base<Derived>::waitForRenderStart;
		using window_base<Derived>::signalWindowThread;
		using window_base<Derived>::signalRenderFinished;
		using window_base<Derived>::isWindowRunning;

		using win32_window<Derived>::createWindow;
		using win32_window<Derived>::WndProc;
		using win32_window<Derived>::_hwnd;
		using win32_window<Derived>::_input;

	public:

		win32_opengl(bool fullscreen, bool maximized, bool vsync, int width, int height, std::string_view title)
			: win32_window<Derived>(fullscreen, maximized, vsync, width, height, title)
		{
			// Create the fake window to load the WGL extensions
			createWindow(GetModuleHandle(nullptr), "loadwgl", "loadwglClass", FakeWndProc);
			MSG msg; INT ret;
			while ((ret = GetMessage(&msg, nullptr, 0, 0))) {
				if (ret == -1)
					throw exception(except_e::OPENGL_BASE, "GetMessage");
				DispatchMessage(&msg);
			}

			// Create the actual window
			_hwnd = createWindow(GetModuleHandle(nullptr), _title.c_str(), (_title + "Class").c_str(), WndProc);
			if (!_hwnd)
				throw exception(except_e::OPENGL_BASE, "createWindow");

			ret = SetWindowLongPtr(_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
			if (GetLastError() != 0)
				throw exception(except_e::OPENGL_BASE, "SetWindowLongPtr");

			createRenderThread();

			// Show the window and initialize the input
			ShowWindow(_hwnd, TRUE);
			_input = new win32_input(_hwnd);
		}

		win32_opengl(const win32_opengl& rhs) = delete;

		win32_opengl(win32_opengl&& rhs) = delete;

		~win32_opengl()
		{
			if (_input)
				delete _input;
			if (_hwnd)
				DestroyWindow(_hwnd);
		}

	protected:

		input_base& getInput()
		{
			return *_input;
		}

		graphics& getGraphics()
		{
			return *_rt->_graphics;
		}

	private:

		class renderThread
		{
		public:

			renderThread(win32_opengl& ref)
			{
				_hwnd = ref._hwnd;

				_hdc = GetDC(_hwnd);
				if (_hdc == NULL)
					throw exception(except_e::OPENGL_BASE, "GetDC");

				const int config_attribs[] ={
					WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
					WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
					WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
					WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
					WGL_SWAP_METHOD_ARB, WGL_SWAP_EXCHANGE_ARB,
					WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
					WGL_COLOR_BITS_ARB, 32,
					WGL_DEPTH_BITS_ARB, 24,
					WGL_STENCIL_BITS_ARB, 8,
					WGL_SAMPLE_BUFFERS_ARB, 1,
					0
				};

				int num;
				int attr = WGL_NUMBER_PIXEL_FORMATS_ARB;
				if (wglGetPixelFormatAttribivARB(_hdc, 0, 0, 1, &attr, &num) == FALSE)
					throw exception(except_e::OPENGL_BASE, "wglGetPixelFormatAttribivARB");

				int *configs = new int[num];
				if (wglChoosePixelFormatARB(_hdc, config_attribs, nullptr, num, configs, (UINT*)&num) == FALSE) {
					delete[] configs;
					throw exception(except_e::OPENGL_BASE, "wglChoosePixelFormatARB");
				}

				// Select the config with the most samples
				attr = WGL_SAMPLES_ARB;
				int best = 0, bestSamples = 0;
				for (int i = 0; i < num; ++i) {
					int samples;
					if (wglGetPixelFormatAttribivARB(_hdc, configs[i], 0, 1, &attr, &samples) == FALSE) {
						delete[] configs;
						throw exception(except_e::OPENGL_BASE, "wglChoosePixelFormatARB");
					}
					if (samples > bestSamples) {
						best = i;
						bestSamples = samples;
					}
				}
				auto config = configs[0];
				delete[] configs;

				PIXELFORMATDESCRIPTOR pfd;
				if (DescribePixelFormat(_hdc, config, sizeof(pfd), &pfd) == 0)
					throw exception(except_e::OPENGL_BASE, "DescribePixelFormat");
				if (SetPixelFormat(_hdc, config, &pfd) == FALSE)
					throw exception(except_e::OPENGL_BASE, "SetPixelFormat");

				int context_attribs[] ={
					WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
					WGL_CONTEXT_MINOR_VERSION_ARB, 5,
					0
				};

				_context = wglCreateContextAttribsARB(_hdc, 0, context_attribs);
				if (_context == NULL)
					throw exception(except_e::OPENGL_BASE, "wglCreateContextAttribsARB");

				if (wglMakeCurrent(_hdc, _context) == FALSE)
					throw exception(except_e::OPENGL_BASE, "wglMakeCurrent");

				if (wglSwapIntervalEXT(ref._vsync) == FALSE)
					throw exception(except_e::OPENGL_BASE, "wglSwapIntervalEXT");

				if (gl3wInit2(customProc))
					throw exception(except_e::OPENGL_BASE, "gl3wInit2");

				_graphics = new graphics(ref._width, ref._height);
			}

			renderThread(const renderThread& rhs) = delete;

			renderThread(renderThread&& rhs) = delete;

			~renderThread()
			{
				delete _graphics;
				if (_context) {
					wglMakeCurrent(nullptr, nullptr);
					wglDeleteContext(_context);
				}
				if (_hdc)
					ReleaseDC(_hwnd, _hdc);
			}

			void renderLoop(win32_opengl<Derived> *ptr)
			{
				while (ptr->isWindowRunning()) {
					static_cast<Derived*>(ptr)->updateLogic();
					ptr->_input->update();
					_graphics->render();

					if (SwapBuffers(_hdc) == FALSE)
						throw exception(except_e::OPENGL_BASE, "SwapBuffers");
				}
			}

			graphics *_graphics = nullptr;

		private:

			HWND _hwnd;

			HDC _hdc = nullptr;

			HGLRC _context = nullptr;
		};

		void initOpenGL()
		{
			std::unique_ptr<renderThread> gfx;

			try {
				gfx = std::unique_ptr<renderThread>(new renderThread(*this));
				_rt = gfx.get();
			}
			catch (...) {
				signalWindowThread("renderThread");
				return;
			}
			signalWindowThread();

			if (waitForRenderStart()) {
				try {
					gfx->renderLoop(this);
				}
				catch (...) {
					signalRenderFinished("renderLoop");
					return;
				}
			}
			signalRenderFinished();

			static_cast<Derived*>(this)->onExit();
		}

		renderThread *_rt;

		static LRESULT CALLBACK FakeWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
		{
			switch (msg) {
			case WM_CREATE:
			{
				PIXELFORMATDESCRIPTOR pfd ={
					sizeof(PIXELFORMATDESCRIPTOR),
					1,
					PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
					PFD_TYPE_RGBA,
					32, // Color bits
					0, 0, 0, 0, 0, 0,
					0,
					0,
					0,
					0, 0, 0, 0,
					24, // Depth
					8, // Stencil
					0, // Aux
					PFD_MAIN_PLANE,
					0,
					0, 0, 0
				};

				auto hdc = GetDC(hwnd);
				if (hdc == NULL)
					throw exception(except_e::OPENGL_BASE, "GetDC");

				auto format = ChoosePixelFormat(hdc, &pfd);
				if (format == 0)
					throw exception(except_e::OPENGL_BASE, "ChoosePixelFormat");
				if (SetPixelFormat(hdc, format, &pfd) == FALSE)
					throw exception(except_e::OPENGL_BASE, "SetPixelFormat");

				auto context = wglCreateContext(hdc);
				if (context == NULL)
					throw exception(except_e::OPENGL_BASE, "wglCreateContext");
				if (wglMakeCurrent(hdc, context) == FALSE)
					throw exception(except_e::OPENGL_BASE, "wglMakeCurrent");

				// Load the WGL extensions we need
				wglGetPixelFormatAttribivARB = reinterpret_cast<PFNWGLGETPIXELFORMATATTRIBIVARBPROC>(wglGetProcAddress("wglGetPixelFormatAttribivARB"));
				wglChoosePixelFormatARB = reinterpret_cast<PFNWGLCHOOSEPIXELFORMATARBPROC>(wglGetProcAddress("wglChoosePixelFormatARB"));
				wglCreateContextAttribsARB = reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC>(wglGetProcAddress("wglCreateContextAttribsARB"));
				wglSwapIntervalEXT = reinterpret_cast<PFNWGLSWAPINTERVALEXTPROC>(wglGetProcAddress("wglSwapIntervalEXT"));

				wglMakeCurrent(nullptr, nullptr);
				wglDeleteContext(context);
				ReleaseDC(hwnd, hdc);
				DestroyWindow(hwnd);

				return 0;
			}

			case WM_DESTROY:
				PostQuitMessage(0);
				return 0;

			default:
				return DefWindowProc(hwnd, msg, wparam, lparam);
			}
		}

		static GL3WglProc customProc(const char *proc)
		{
			static auto libgl = GetModuleHandle("opengl32.dll");
			GL3WglProc res;

			res = (GL3WglProc)wglGetProcAddress(proc);
			if (!res)
				res = (GL3WglProc)GetProcAddress(libgl, proc);
			return res;
		}

		inline static PFNWGLGETPIXELFORMATATTRIBIVARBPROC wglGetPixelFormatAttribivARB = nullptr;

		inline static PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = nullptr;

		inline static PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;

		inline static PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = nullptr;
	};
}
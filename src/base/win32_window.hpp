#pragma once

#include "window_base.hpp"
#include "win32_input.hpp"

namespace game::native
{
	/*
	 *  Destroys the Derived object by calling delete
	 */

	template<class Derived>
	class win32_window : public window_base<Derived>
	{
		using window_base<Derived>::signalRenderStart;
		using window_base<Derived>::signalWindowFinished;
		using window_base<Derived>::isRenderRunning;

	public:

		win32_window(bool fullscreen, bool maximized, bool vsync, int width, int height, std::string_view title)
			: window_base<Derived>(fullscreen, maximized, vsync, width, height, title)
		{
		}

		win32_window(const win32_window& rhs) = delete;

		win32_window(win32_window&& rhs) = delete;

		~win32_window()
		{
			if (_hwnd) {
				DestroyWindow(_hwnd);
				MSG msg ={};
				while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
					if (msg.message == WM_QUIT)
						break;
					else
						DispatchMessage(&msg);
				}
			}
		}

	protected:

		using window_base<Derived>::signalRenderFinished;

		HWND createWindow(HINSTANCE hinst, LPCSTR wtitle, LPCSTR ctitle, WNDPROC WndProc)
		{
			WNDCLASS wc;
			wc.style = CS_OWNDC;
			wc.lpfnWndProc = WndProc;
			wc.cbClsExtra = 0;
			wc.cbWndExtra = 0;
			wc.hInstance = hinst;
			wc.hIcon = nullptr;
			wc.hCursor = nullptr;
			wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
			wc.lpszMenuName = nullptr;
			wc.lpszClassName = ctitle;

			if (!RegisterClass(&wc))
				throw exception(except_e::NATIVE_WINDOW, "RegisterClass");
			
			if (this->_displayType == window_base<Derived>::displayType::Fullscreen) {
				this->_width = GetSystemMetrics(SM_CXSCREEN);
				this->_height = GetSystemMetrics(SM_CYSCREEN);
			}
			else if (this->_displayType == window_base<Derived>::displayType::Maximized) {
				RECT tmp;
				SystemParametersInfo(SPI_GETWORKAREA, 0, &tmp, 0);
				this->_width = tmp.right - tmp.left;
				this->_height = tmp.bottom - tmp.top;
			}

			return CreateWindow(wc.lpszClassName, wtitle, WS_POPUP, 0, 0, this->_width, this->_height, nullptr, nullptr, hinst, nullptr);
		}

		void runWindow()
		{
			signalRenderStart();

			bool first = true;
			MSG msg;
			while (true) {
				if (GetMessage(&msg, nullptr, 0, 0) == -1) {
					signalWindowFinished("GetMessage");
					break;
				}
				DispatchMessage(&msg);
				
				if (!isRenderRunning()) {
					signalWindowFinished();
					break;
				}
			}

			// Object is useless now, time for kamikaze
			delete static_cast<Derived*>(this);
		}

		void quit()
		{
			signalRenderFinished();
		}

		HWND _hwnd = nullptr;

		win32_input *_input = nullptr;

		static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
		{
			switch (msg) {
			case WM_INPUT:
			{
				auto ptr = reinterpret_cast<win32_window<Derived>*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

				UINT dwSize;
				if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lparam), RID_INPUT, nullptr, &dwSize, sizeof(RAWINPUTHEADER)) == -1)
					throw exception(except_e::NATIVE_WINDOW, "GetRawInputData");
				LPBYTE lpb = new BYTE[dwSize];
				RAWINPUT *raw = reinterpret_cast<RAWINPUT*>(lpb);

				if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lparam), RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize) {
					delete[] lpb;
					throw exception(except_e::NATIVE_WINDOW, "GetRawInputData");
				}

				if (raw->header.dwType == RIM_TYPEMOUSE)
					ptr->_input->updateMouse(raw->data.mouse);
				else if (raw->header.dwType == RIM_TYPEKEYBOARD)
					ptr->_input->updateKeyboard(raw->data.keyboard);

				delete[] lpb;
				return 0;
			}

			case WM_CLOSE:
			{
				auto ptr = reinterpret_cast<Derived*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
				ptr->signalWindowFinished();
				return 0;
			}

			case WM_DESTROY:
				PostQuitMessage(0);
				return 0;

			default:
				return DefWindowProc(hwnd, msg, wparam, lparam);
			}
		}
	};
}
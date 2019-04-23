#pragma once

#include "input_base.hpp"
#include "exception.hpp"
#include <mutex>

namespace game::native
{
	class win32_input : public input_base
	{
	public:

		win32_input(HWND hwnd)
			: _hwnd(hwnd)
		{
			capture();
		}

		win32_input(const win32_input& rhs) = delete;

		win32_input(win32_input&& rhs) = delete;

		~win32_input()
		{
			if (_captured) {
				release();
			}
		}

		void update()
		{
			std::lock_guard<std::mutex> lk(_mtx);
			input_base::operator=(_copy);
			_copy._mdx = 0;
			_copy._mdy = 0;
			_copy._mdz = 0;
		}

		void updateMouse(const RAWMOUSE& mouse)
		{
			std::lock_guard<std::mutex> lk(_mtx);
			if (mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN)
				_copy._mbuttons[0] = true;
			else if (mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP)
				_copy._mbuttons[0] = false;
			if (mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN)
				_copy._mbuttons[2] = true;
			else if (mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP)
				_copy._mbuttons[2] = false;
			if (mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN)
				_copy._mbuttons[1] = true;
			else if (mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP)
				_copy._mbuttons[1] = false;

			if (mouse.usFlags == MOUSE_MOVE_RELATIVE) {
				_copy._mdx += mouse.lLastX;
				_copy._mdy += mouse.lLastY;
			}
			else if (mouse.usFlags == MOUSE_MOVE_ABSOLUTE) {
				_copy._mx = mouse.lLastX;
				_copy._my = mouse.lLastY;
			}
		}

		void updateKeyboard(const RAWKEYBOARD& keyboard)
		{
			std::lock_guard<std::mutex> lk(_mtx);
			if (keyboard.Flags & RI_KEY_BREAK) {
				if (keyboard.VKey < sizeof(_keys)) {
					if (keyboard.VKey == VK_CONTROL)
						_copy._keys[keyboard.Flags == RI_KEY_E0 ? VK_LCONTROL : VK_RCONTROL] = false;
					else if (keyboard.VKey == VK_SHIFT)
						_copy._keys[keyboard.MakeCode == 0x2A ? VK_LSHIFT : VK_RSHIFT] = false;
					else if (keyboard.VKey == VK_MENU)
						_copy._keys[keyboard.Flags == RI_KEY_E0 ? VK_LMENU : VK_RMENU] = false;
					else
						_copy._keys[keyboard.VKey] = false;
				}
			}
			else if (keyboard.Flags == RI_KEY_MAKE) {
				if (keyboard.VKey < sizeof(_keys)) {
					if (keyboard.VKey == VK_CONTROL)
						_copy._keys[keyboard.Flags == RI_KEY_E0 ? VK_LCONTROL : VK_RCONTROL] = true;
					else if (keyboard.VKey == VK_SHIFT)
						_copy._keys[keyboard.MakeCode == 0x2A ? VK_LSHIFT : VK_RSHIFT] = true;
					else if (keyboard.VKey == VK_MENU)
						_copy._keys[keyboard.Flags == RI_KEY_E0 ? VK_LMENU : VK_RMENU] = true;
					else
						_copy._keys[keyboard.VKey] = true;
				}
			}
		}

		void capture()
		{
			RAWINPUTDEVICE rid[2];

			// HID mouse
			rid[0].usUsagePage = 0x01;
			rid[0].usUsage = 0x02;
			rid[0].dwFlags = RIDEV_CAPTUREMOUSE | RIDEV_NOLEGACY;
			rid[0].hwndTarget = _hwnd;

			// HID keyboard
			rid[1].usUsagePage = 0x01;
			rid[1].usUsage = 0x06;
			rid[1].dwFlags = RIDEV_NOLEGACY;
			rid[1].hwndTarget = _hwnd;

			if (RegisterRawInputDevices(rid, 2, sizeof(rid[0])) == FALSE)
				throw exception(except_e::NATIVE_INPUT, "RegisterRawInputDevices");

			_captured = true;
			ShowCursor(FALSE);
		}

		void release()
		{
			RAWINPUTDEVICE rid[2];

			// HID mouse
			rid[0].usUsagePage = 0x01;
			rid[0].usUsage = 0x02;
			rid[0].dwFlags = RIDEV_REMOVE | RIDEV_NOLEGACY;
			rid[0].hwndTarget = nullptr;

			// HID keyboard
			rid[1].usUsagePage = 0x01;
			rid[1].usUsage = 0x06;
			rid[1].dwFlags = RIDEV_REMOVE | RIDEV_NOLEGACY;
			rid[1].hwndTarget = nullptr;

			RegisterRawInputDevices(rid, 2, sizeof(rid[0]));

			_captured = false;
			ShowCursor(TRUE);
		}

	private:

		HWND _hwnd;

		bool _captured = false;

		input_base _copy;

		std::mutex _mtx;
	};
}
#pragma once

#include <stdexcept>
#include <string_view>
#include <algorithm>

namespace game
{
    enum class except_e { NATIVE_CLOCK, NATIVE_INPUT, NATIVE_WINDOW, OPENGL_BASE, GRAPHICS_BASE, FONT_BASE, APPLICATION };

    class exception : public std::exception
    {
    public:

        exception(except_e ecode, std::string_view msg = nullptr)
            : ecode(ecode)
        {
            const char cstr[] = "Error in ui::native_clock: ";
            const char istr[] = "Error in ui::native_input: ";
            const char wstr[] = "Error in ui::native_window: ";
            const char gstr[] = "Error in ui::opengl_base: ";
			const char gfxstr[] = "Error in ui::graphics_base: ";
			const char fstr[] = "Error in ui::font_base: ";
            const char estr[] = "Error in ui::application: ";
            unsigned int len = 0;

            switch (ecode) {
            case except_e::NATIVE_CLOCK:
                len = sizeof(cstr)-1;
                std::copy(cstr, cstr+len, _buffer);
                break;
            case except_e::NATIVE_INPUT:
                len = sizeof(istr)-1;
                std::copy(istr, istr+len, _buffer);
                break;
            case except_e::NATIVE_WINDOW:
                len = sizeof(wstr)-1;
                std::copy(wstr, wstr+len, _buffer);
                break;
            case except_e::OPENGL_BASE:
                len = sizeof(gstr)-1;
                std::copy(gstr, gstr+len, _buffer);
                break;
            case except_e::GRAPHICS_BASE:
                len = sizeof(gfxstr)-1;
                std::copy(gfxstr, gfxstr+len, _buffer);
                break;
			case except_e::FONT_BASE:
				len = sizeof(fstr)-1;
				std::copy(fstr, fstr+len, _buffer);
				break;
            case except_e::APPLICATION:
                len = sizeof(estr)-1;
                std::copy(estr, estr+len, _buffer);
                break;
            }

            std::copy(msg.begin(), msg.end(), _buffer+len);
            len += msg.length();
            _buffer[len] = '\0';
        }

        exception(const exception& rhs)
            : ecode(rhs.ecode)
        {
            std::copy(rhs._buffer, rhs._buffer+sizeof(_buffer), _buffer);
        }

        const char * what() const noexcept override
        {
            return _buffer;
        }

        const except_e ecode;

    private:

        char _buffer[256];
    };
}
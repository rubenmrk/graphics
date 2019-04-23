#pragma once

#include "input_base.hpp"
#include <mutex>

namespace game::native
{
    class wayland_input : public input_base
    {
    public:

        wayland_input(wl_display *disp, wl_seat *seat, wl_surface *surf, zwp_pointer_constraints_v1 *pconstraints, zwp_relative_pointer_manager_v1 *rpman)
        {
            _parent = surf;
            _constraints = pconstraints;

            /*
             * Grab the pointer and mouse from the main wayland protocol
             */

            static const wl_seat_listener listener {
                seatCapabilities, seatName
            };
            wl_seat_add_listener(seat, &listener, this);
            wl_display_roundtrip(disp);

            if (!_pointer || !_keyboard)
                throw exception(except_e::NATIVE_INPUT, "wl_seat_listener");

            /*
             * Confine the mouse to the window and get relative mouse movement from unstable wayland protocols
             */

            _rpointer = zwp_relative_pointer_manager_v1_get_relative_pointer(rpman, _pointer);
            if (!_rpointer)
                throw exception(except_e::NATIVE_INPUT, "zwp_relative_pointer_copy._manager_v1_get_relative_pointer");
            static const zwp_relative_pointer_v1_listener listener2 {
                relativePointerMotion
            };
            zwp_relative_pointer_v1_add_listener(_rpointer, &listener2, this);

            _confined = zwp_pointer_constraints_v1_confine_pointer(_constraints, _parent, _pointer, nullptr, ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT);
            if (!_confined)
                throw exception(except_e::NATIVE_INPUT, "zwp_pointer_constraints_v1_confine_pointer");
        }

        ~wayland_input()
        {
            if (_rpointer)
                zwp_relative_pointer_v1_destroy(_rpointer);
            if (_confined)
                zwp_confined_pointer_v1_destroy(_confined);
            if (_keyboard)
                wl_keyboard_destroy(_keyboard);
            if (_pointer)
                wl_pointer_destroy(_pointer);
        }

        void update()
        {
            std::lock_guard<std::mutex> lk(_mtx);
            input_base::operator=(_copy);
            _copy._mdx = 0;
            _copy._mdy = 0;
            _copy._mdz = 0;
        }

    private:

        input_base _copy;

        std::mutex _mtx;

        wl_pointer *_pointer = nullptr;

        wl_keyboard *_keyboard = nullptr;

        zwp_confined_pointer_v1 *_confined = nullptr;

        zwp_relative_pointer_v1 *_rpointer = nullptr;

        wl_surface *_parent;

        zwp_pointer_constraints_v1 *_constraints;

        static void seatCapabilities(void *data, wl_seat *wl_seat,  uint32_t capabilities)
        {
            auto ptr = static_cast<wayland_input*>(data);
            if (capabilities & WL_SEAT_CAPABILITY_POINTER) {
                ptr->_pointer = wl_seat_get_pointer(wl_seat);
                static const wl_pointer_listener listener {
                    pointerEnter, pointerLeave, pointerMotion, pointerButton, pointerAxis, pointerFrame, pointerAxisSource, 
                    pointerAxisStop, pointerAxisDiscrete
                };
                wl_pointer_add_listener(ptr->_pointer, &listener, data);
            }
            if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD) {
                ptr->_keyboard = wl_seat_get_keyboard(wl_seat);
                static const wl_keyboard_listener listener {
                    keyboardKeyMap, keyboardEnter, keyboardLeave, keyboardKey, keyboardModifiers, keyboardRepeatInfo
                };
                wl_keyboard_add_listener(ptr->_keyboard, &listener, data);
            }
        }

        static void seatName(void *data, wl_seat *wl_seat, const char *name)
        {
        }

        static void pointerEnter(void *data, wl_pointer *wl_pointer, uint32_t serial, wl_surface *surface, wl_fixed_t surface_x, wl_fixed_t surface_y)
        {
            // Hide the cursor
            auto ptr = static_cast<wayland_input*>(data);
            wl_pointer_set_cursor(ptr->_pointer, serial, nullptr, 0, 0);
        }

        static void pointerLeave(void *data, wl_pointer *wl_pointer, uint32_t serial, wl_surface *surface)
        {
        }

        static void pointerMotion(void *data, wl_pointer *wl_pointer, uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y)
        {
            auto ptr = static_cast<wayland_input*>(data);
            std::lock_guard<std::mutex> lk(ptr->_mtx);

            ptr->_copy._mx = wl_fixed_to_double(surface_x);
            ptr->_copy._my = wl_fixed_to_double(surface_y);
        }

        static void relativePointerMotion(void *data, zwp_relative_pointer_v1 *zwp_relative_pointer_v1, uint32_t utime_hi, uint32_t utime_lo,
			wl_fixed_t dx, wl_fixed_t dy, wl_fixed_t dx_unaccel, wl_fixed_t dy_unaccel)
        {
            auto ptr = static_cast<wayland_input*>(data);
            std::lock_guard<std::mutex> lk(ptr->_mtx);

            ptr->_copy._mdx += wl_fixed_to_double(dx_unaccel);
            ptr->_copy._mdy += wl_fixed_to_double(dy_unaccel);
        }

        static void pointerButton(void *data, wl_pointer *wl_pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state)
        {
            auto ptr = static_cast<wayland_input*>(data);
            std::lock_guard<std::mutex> lk(ptr->_mtx);

            if (button == BTN_LEFT)
                ptr->_copy._mbuttons[0] = static_cast<bool>(state);
            else if (button == BTN_RIGHT)
                ptr->_copy._mbuttons[1] = static_cast<bool>(state);
            else if (button == BTN_MIDDLE)
                ptr->_copy._mbuttons[2] = static_cast<bool>(state);
        }

        static void pointerAxis(void *data, wl_pointer *wl_pointer, uint32_t time, uint32_t axis, wl_fixed_t value)
        {
            auto ptr = static_cast<wayland_input*>(data);
            std::lock_guard<std::mutex> lk(ptr->_mtx);

            ptr->_copy._mdz += wl_fixed_to_double(value);
        }

        static void pointerFrame(void *data, wl_pointer *wl_pointer)
        {
        }

        static void pointerAxisSource(void *data, wl_pointer *wl_pointer, uint32_t axis_source)
        {
        }

        static void pointerAxisStop(void *data, wl_pointer *wl_pointer, uint32_t time, uint32_t axis)
        {
        }

        static void pointerAxisDiscrete(void *data, wl_pointer *wl_pointer, uint32_t axis, int32_t discrete)
        {
        }

        static void keyboardKeyMap(void *data, wl_keyboard *wl_keyboard, uint32_t format, int32_t fd, uint32_t size)
        {
        }

        static void keyboardEnter(void *data, wl_keyboard *wl_keyboard, uint32_t serial, wl_surface *surface, wl_array *keys)
        {
        }

        static void keyboardLeave(void *data, wl_keyboard *wl_keyboard, uint32_t serial, wl_surface *surface)
        {
        }

        static void keyboardKey(void *data, wl_keyboard *wl_keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
        {
            auto ptr = static_cast<wayland_input*>(data);
            std::lock_guard<std::mutex> lk(ptr->_mtx);

            if (key < sizeof(ptr->_keys))
                ptr->_copy._keys[key] = static_cast<bool>(state);
        }

        static void keyboardModifiers(void *data, wl_keyboard *wl_keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched,
			uint32_t mods_locked, uint32_t group)
        {
        }

        static void keyboardRepeatInfo(void *data, wl_keyboard *wl_keyboard, int32_t rate, int32_t delay)
        {
        }
    };
}
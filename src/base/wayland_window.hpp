#pragma once

#include "window_base.hpp"
#include "wayland_input.hpp"
#include <cstring>

namespace game::native
{
    /*
     *  Destroys the Derived object by calling delete
     */

    template<class Derived>
    class wayland_window : public window_base<Derived>
    {
        using window_base<Derived>::signalRenderStart;
        using window_base<Derived>::signalWindowFinished;
        using window_base<Derived>::isRenderRunning;

    public:

        wayland_window(bool fullscreen, bool maximized, bool vsync, int width, int height, std::string_view title) 
            : window_base<Derived>(fullscreen, maximized, vsync, width, height, title)
        {
            /*
             * Initialize all global wayland objects
             */

            _display = wl_display_connect(nullptr);
            if (!_display)
                throw exception(except_e::NATIVE_WINDOW, "wl_display_connect");
            
            _registry = wl_display_get_registry(_display);
            if (!_registry)
                throw exception(except_e::NATIVE_WINDOW, "wl_display_get_registry");
            
            static const wl_registry_listener listen = {
                registryGlobal, registryGlobalRemove
            };
            wl_registry_add_listener(_registry, &listen, this);
            wl_display_roundtrip(_display);

            if (!_compositor || !_seat || !_output || !_xbase || !_pconstrain)
                throw exception(except_e::NATIVE_WINDOW, "wl_registry_add_listener");
            
            /*
             * Create window surface to draw on
             */

            _surf = wl_compositor_create_surface(_compositor);
            if (!_surf)
                throw exception(except_e::NATIVE_WINDOW, "wl_compositor_create_surface");

            static const xdg_wm_base_listener listener {
                wmBasePing
            };
            xdg_wm_base_add_listener(_xbase, &listener, this);
            
            _xsurf = xdg_wm_base_get_xdg_surface(_xbase, _surf);
            if (!_xsurf)
                throw exception(except_e::NATIVE_WINDOW, "xdg_wm_base_get_xdg_surface");
            static const xdg_surface_listener listener2 {
                xdgSurfaceConfigure
            };
            xdg_surface_add_listener(_xsurf, &listener2, this);
            
            _xtlevel = xdg_surface_get_toplevel(_xsurf);
            if (!_xtlevel)
                throw exception(except_e::NATIVE_WINDOW, "xdg_surface_get_toplevel");
            static const xdg_toplevel_listener listener3 = {
                xdgTopLevelConfigure, xdgTopLevelClose
            };
            xdg_toplevel_add_listener(_xtlevel, &listener3, this);
            xdg_toplevel_set_title(_xtlevel, title.data());
            if (maximized)
                xdg_toplevel_set_maximized(_xtlevel);
            else if (fullscreen)
                xdg_toplevel_set_fullscreen(_xtlevel, _output);

            /*
             * Initialize the window input
             */

            _input = new wayland_input(_display, _seat, _surf, _pconstrain, _rpman);

            wl_surface_commit(_surf);
            wl_display_roundtrip(_display);
        }

        wayland_window(const wayland_window& rhs) = delete;

        wayland_window(wayland_window&& rhs) = delete;

        ~wayland_window()
        {
            delete _input;
            if (_rpman)
                zwp_relative_pointer_manager_v1_destroy(_rpman);
            if (_pconstrain)
                zwp_pointer_constraints_v1_destroy(_pconstrain);
            if (_xtlevel)
                xdg_toplevel_destroy(_xtlevel);
            if (_xsurf)
                xdg_surface_destroy(_xsurf);
            if (_surf)
                wl_surface_destroy(_surf);
            if (_xbase)
                xdg_wm_base_destroy(_xbase);
            if (_output)
                wl_output_destroy(_output);
            if (_seat)
                wl_seat_destroy(_seat);
            if (_compositor)
                wl_compositor_destroy(_compositor);
            if (_registry)
                wl_registry_destroy(_registry);
            if (_display)
                wl_display_disconnect(_display);
        }

    protected:

        using window_base<Derived>::signalRenderFinished;

        void runWindow()
        {
            signalRenderStart();

            while (true) {
				if (wl_display_dispatch(_display) == -1) {
                    signalWindowFinished("wl_display_dispatch");
                    break;
                }

				if (!isRenderRunning()) {
					signalWindowFinished();
					break;
				}
            }

            // Object is useless now, time for kamikaze
            delete static_cast<Derived*>(this);
        }

        input_base& getInput()
        {
            return *_input;
        }

        void quit()
        {
            signalRenderFinished();
        }

        wayland_input *_input = nullptr;

        wl_display *_display = nullptr;

        wl_surface *_surf = nullptr;

    private:

        wl_registry *_registry = nullptr;

        wl_compositor *_compositor = nullptr;

        wl_seat *_seat = nullptr;

        wl_output *_output = nullptr;

        // Basically wl_shell
        xdg_wm_base *_xbase = nullptr;

        // Basically wl_shell_surface
        xdg_surface *_xsurf = nullptr;

        // Like calling wl_shell_surface::set_toplevel
        xdg_toplevel *_xtlevel = nullptr;

        // To keep the pointer inside the window
        zwp_pointer_constraints_v1 *_pconstrain = nullptr;

        // To grab relative pointer changes (even when trapped inside the window)
        zwp_relative_pointer_manager_v1 *_rpman = nullptr;

        static void registryGlobal(void *data, wl_registry *wl_registry, uint32_t name, const char *interface, uint32_t version)
        {
            auto ptr = static_cast<wayland_window<Derived>*>(data);
            if (std::strcmp(interface, "wl_compositor") == 0)
                ptr->_compositor = (wl_compositor*)wl_registry_bind(ptr->_registry, name, &wl_compositor_interface, version);
            else if (std::strcmp(interface, "wl_seat") == 0)
                ptr->_seat = (wl_seat*)wl_registry_bind(ptr->_registry, name, &wl_seat_interface, version);
            else if (std::strcmp(interface, "wl_output") == 0)
                ptr->_output = (wl_output*)wl_registry_bind(ptr->_registry, name, &wl_output_interface, version);
            else if (std::strcmp(interface, "xdg_wm_base") == 0)
                ptr->_xbase = (xdg_wm_base*)wl_registry_bind(ptr->_registry, name, &xdg_wm_base_interface, version);
            else if (std::strcmp(interface, "zwp_pointer_constraints_v1") == 0)
                ptr->_pconstrain = (zwp_pointer_constraints_v1*)wl_registry_bind(ptr->_registry, name, &zwp_pointer_constraints_v1_interface, version);
            else if (std::strcmp(interface, "zwp_relative_pointer_manager_v1") == 0)
                ptr->_rpman = (zwp_relative_pointer_manager_v1*)wl_registry_bind(ptr->_registry, name, &zwp_relative_pointer_manager_v1_interface, version);
        }

        static void registryGlobalRemove(void *data, wl_registry *wl_registry, uint32_t name)
        {
        }

        static void wmBasePing(void *data, xdg_wm_base *xdg_wm_base, uint32_t serial)
        {
            auto ptr = static_cast<wayland_window<Derived>*>(data);
            xdg_wm_base_pong(ptr->_xbase, serial);
        }

        static void xdgSurfaceConfigure(void *data, xdg_surface *xdg_surface, uint32_t serial)
        {
            xdg_surface_ack_configure(xdg_surface, serial);
        }

        static void xdgTopLevelConfigure(void *data, xdg_toplevel *xdg_toplevel, int32_t width, int32_t height, wl_array *states)
        {
            auto ptr = static_cast<wayland_window<Derived>*>(data);
            auto maximized = (ptr->_displayType == window_base<Derived>::displayType::Maximized), 
                fullscreen = (ptr->_displayType == window_base<Derived>::displayType::Fullscreen);

            for (auto state = static_cast<const uint32_t*>(states->data); 
                state != reinterpret_cast<const uint32_t*>(static_cast<const char*>(states->data)+states->size); 
                ++state)
            {
                switch (*state) {
                case XDG_TOPLEVEL_STATE_MAXIMIZED:
                    maximized = true;
                    break;
                case XDG_TOPLEVEL_STATE_FULLSCREEN:
                    fullscreen = true;
                    break;
                }
            }

            if (maximized != (ptr->_displayType == window_base<Derived>::displayType::Maximized))
                ptr->_displayType = window_base<Derived>::displayType::Maximized;
            if (fullscreen != (ptr->_displayType == window_base<Derived>::displayType::Fullscreen))
                ptr->_displayType = window_base<Derived>::displayType::Fullscreen;
            if (width != 0 && height != 0 && (width != ptr->_width || height != ptr->_height)) {
                ptr->_width = width;
                ptr->_height = height;
            }
        }

        static void xdgTopLevelClose(void *data, xdg_toplevel *xdg_toplevel)
        {
            auto ptr = static_cast<wayland_window<Derived>*>(data);
            ptr->quit();
        }
    };
}
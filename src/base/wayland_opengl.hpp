#pragma once

#include "wayland_window.hpp"
#include "application/graphics.hpp"
#include <memory>

namespace game::native
{
    /*
     *  Calls Derived::updateLogic
     */

    template<class Derived>
    class wayland_opengl : public wayland_window<Derived>
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

        using wayland_window<Derived>::_input;
        using wayland_window<Derived>::_display;
        using wayland_window<Derived>::_surf;

    public:

        wayland_opengl(bool fullscreen, bool maximized, bool vsync, int width, int height, std::string_view title)
            : wayland_window<Derived>(fullscreen, maximized, vsync, width, height, title)
        {
            createRenderThread();
        }

        wayland_opengl(const wayland_opengl& rhs) = delete;

        wayland_opengl(wayland_opengl&& rhs) = delete;

        ~wayland_opengl()
        {
        }

    protected:

        graphics& getGraphics()
        {
            return *_rt->_graphics;
        }

    private:

        class renderThread
        {
        public:

            renderThread(wayland_opengl& ref)
            {
                _window = wl_egl_window_create(ref._surf, ref._width, ref._height);
                if (!_window) 
                    throw exception(except_e::OPENGL_BASE, "wl_egl_window_create");
                
                _egldisp = eglGetDisplay(ref._display);
                if (_egldisp == EGL_NO_DISPLAY)
                    throw exception(except_e::OPENGL_BASE, "eglGetDisplay");
                
                EGLint major, minor;
                if (eglInitialize(_egldisp, &major, &minor) == EGL_FALSE)
                    throw exception(except_e::OPENGL_BASE, "eglInitialize");

                if (!((major == 1 && minor >= 4) || major >= 2)) 
                    throw exception(except_e::OPENGL_BASE, "EGL version too old");
                
                if (eglBindAPI(EGL_OPENGL_API) == EGL_FALSE)
                    throw exception(except_e::OPENGL_BASE, "eglBindAPI");
                
                EGLint config_attribs[] = {
                    EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
                    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                    EGL_RED_SIZE, 8,
                    EGL_GREEN_SIZE, 8,
                    EGL_BLUE_SIZE, 8,
                    EGL_ALPHA_SIZE, 8,
                    EGL_DEPTH_SIZE, 24,
                    EGL_STENCIL_SIZE, 8,
                    EGL_SAMPLE_BUFFERS, 1,
                    EGL_NONE
                };

                EGLint num;
                if (eglGetConfigs(_egldisp, nullptr, 0, &num) == EGL_FALSE)
                    throw exception(except_e::OPENGL_BASE, "eglGetConfigs");
                
                auto *configs = new EGLConfig[num];
                if (eglChooseConfig(_egldisp, config_attribs, configs, num, &num) == EGL_FALSE || num == 0) {
                    delete[] configs;
                    throw exception(except_e::OPENGL_BASE, "eglChooseConfig");
                }

                // Select the config with the most samples
                EGLint best = 0, bestSamples = 0;
                for (EGLint i = 0; i < num; ++i) {
                    EGLint samples;
                    if (eglGetConfigAttrib(_egldisp, configs[i], EGL_SAMPLES, &samples) == GL_FALSE) {
                        delete[] configs;
                        throw exception(except_e::OPENGL_BASE, "eglGetConfigAttrib");
                    }
                    if (samples > bestSamples) {
                        best = i;
                        bestSamples = samples;
                    }
                }
                auto config = configs[best];
                delete[] configs;

                EGLint context_attribs[] = {
                    EGL_CONTEXT_CLIENT_VERSION, 2,
                    EGL_CONTEXT_MAJOR_VERSION, 4,
                    EGL_CONTEXT_MINOR_VERSION, 5,
                    EGL_NONE
                };

                _eglcontext = eglCreateContext(_egldisp, config, EGL_NO_CONTEXT, context_attribs);
                if (_eglcontext == EGL_NO_CONTEXT)
                    throw exception(except_e::OPENGL_BASE, "eglCreateContext");

                _eglsurf = eglCreateWindowSurface(_egldisp, config, _window, NULL);
                if (_eglsurf == EGL_NO_SURFACE) {
                    eglDestroyContext(_egldisp, _eglcontext);
                    _egldisp = EGL_NO_CONTEXT;
                    throw exception(except_e::OPENGL_BASE, "eglCreateWindowSurface");
                }

                if (eglMakeCurrent(_egldisp, _eglsurf, _eglsurf, _eglcontext) == EGL_FALSE)
                    throw exception(except_e::OPENGL_BASE, "eglMakeCurrent");

                if (!ref._vsync) {
                    if (eglSwapInterval(_egldisp, 0) == EGL_FALSE)
                        throw exception(except_e::OPENGL_BASE, "eglSwapInterval");
                }

                if (gl3wInit2(eglGetProcAddress))
                    throw exception(except_e::OPENGL_BASE, "gl3wInit2");
                
                _graphics = new graphics(ref._width, ref._height);
            }

            renderThread(const renderThread& rhs) = delete;

            renderThread(renderThread&& rhs) = delete;

            ~renderThread()
            {
                delete _graphics;
                if (_eglcontext != EGL_NO_CONTEXT) {
                    eglMakeCurrent(_egldisp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
                    eglDestroyContext(_egldisp, _eglcontext);
                }
                if (_eglsurf != EGL_NO_SURFACE)
                    eglDestroySurface(_egldisp, _eglsurf);
                if (_egldisp != EGL_NO_DISPLAY)
                    eglTerminate(_egldisp);
                if (_window)
                    wl_egl_window_destroy(_window);
            }

            void renderLoop(wayland_opengl<Derived> *ptr)
            {
                while (ptr->isWindowRunning()) {
                    static_cast<Derived*>(ptr)->updateLogic();
					ptr->_input->update();
                    _graphics->render();

                    if (eglSwapBuffers(_egldisp, _eglsurf) == EGL_FALSE)
                        throw exception(except_e::OPENGL_BASE, "eglSwapBuffers");
                }
            }

            graphics *_graphics = nullptr;

        private:

            wl_egl_window *_window = nullptr;

            EGLDisplay _egldisp = EGL_NO_DISPLAY;

            EGLSurface _eglsurf = EGL_NO_SURFACE;

            EGLContext _eglcontext = EGL_NO_CONTEXT;
        };

        void initOpenGL()
        {
            std::unique_ptr<renderThread> gfx;

            try {
                gfx = std::unique_ptr<renderThread>(new renderThread(*this));
                _rt = gfx.get();
            } catch (...) {
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
    };
}
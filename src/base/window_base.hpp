#pragma once

#include "platform.hpp"
#include "exception.hpp"
#include <string>
#include <mutex>
#include <condition_variable>
#include <thread>

namespace game::native
{
    /* 
     *  Calls Derived::initOpenGL
     */

    template<class Derived>
    class window_base
    {
    public:

        window_base(bool fullscreen, bool maximized, bool vsync, int width, int height, std::string_view title)
        {
            if (fullscreen && maximized)
                throw exception(except_e::NATIVE_WINDOW);
            
            if (maximized)
                _displayType = displayType::Maximized;
            else if (fullscreen)
                _displayType = displayType::Fullscreen;
            else
                _displayType = displayType::Windowed;
            
            _vsync = vsync;
            _width = width;
            _height = height;
            _title = title;
        }

        window_base(const window_base& rhs) = delete;

        window_base(window_base&& rhs) = delete;

        ~window_base()
        {
            delete _rendere;
        }

        enum class displayType { Windowed, Maximized, Fullscreen };

        displayType _displayType;

        std::string _title;

        int _width, _height;

        bool _vsync, _running = false;

    protected:

        /*
         * Functions called from the window thread
         */

        void createRenderThread()
        {
            std::unique_lock<std::mutex> lk(_rendermtx);
            _rendert = std::thread(&Derived::initOpenGL, static_cast<Derived*>(this));
            _rendercv.wait(lk);
            if (_rendere) {
                _rendert.join();
                throw *_rendere;
            }
        }

        void signalRenderStart()
        {
            std::lock_guard<std::mutex> lk(_rendermtx);
            _running = true;
            _rendercv.notify_one();
        }

        void signalWindowFinished()
        {
            {
                std::lock_guard<std::mutex> lk(_rendermtx);
                _running = false;
            }
			if (_rendert.joinable())
				_rendert.join();
        }

        void signalWindowFinished(std::string_view msg)
        {
            {
                std::lock_guard<std::mutex> lk(_rendermtx);
                _running = false;
            }
			if (_rendert.joinable())
				_rendert.join();
            throw exception(except_e::NATIVE_WINDOW, msg);
        }

        bool isRenderRunning()
        {
            {
                std::lock_guard<std::mutex> lk(_rendermtx);
                if (_running)
                    return true;
            }

			if (_rendert.joinable())
				_rendert.join();
            if (_rendere)
                throw *_rendere;
            return false;
        }

        /*
         * Functions called from the render thread
         */

        bool waitForRenderStart()
        {
            std::unique_lock<std::mutex> lk(_rendermtx);
            _rendercv.wait(lk);
            if (!_running) {
                _rendere = new exception(except_e::OPENGL_BASE, "invalid state");
                return false;
            }
            return true;
        }

        void signalWindowThread()
        {
            std::lock_guard<std::mutex> lk(_rendermtx);
            _rendercv.notify_one();
        }

        void signalWindowThread(std::string_view errmsg)
        {
            std::lock_guard<std::mutex> lk(_rendermtx);
            _rendere = new exception(except_e::OPENGL_BASE, errmsg);
            _rendercv.notify_one();
        }

        void signalRenderFinished()
        {
            std::lock_guard<std::mutex> lk(_rendermtx);
            _running = false;
        }

        void signalRenderFinished(std::string_view errmsg)
        {
            std::lock_guard<std::mutex> lk(_rendermtx);
            _rendere = new exception(except_e::OPENGL_BASE, errmsg);
            _running = false;
        }

        bool isWindowRunning()
        {
            std::lock_guard<std::mutex> lk(_rendermtx);
			return _running;
        }

    private:

        std::thread _rendert;

        std::mutex _rendermtx;

        std::condition_variable _rendercv;

        exception *_rendere = nullptr;
    };
}
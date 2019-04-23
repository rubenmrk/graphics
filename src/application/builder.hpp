#pragma once

#include "application.hpp"

namespace game
{
	class applicationHandle
	{
	public:

		applicationHandle(application *app)
			: _app(app)
		{
		}

		applicationHandle(const applicationHandle& rhs) = delete;

		applicationHandle(applicationHandle&& rhs) noexcept
			: _app(rhs._app)
		{
			rhs._app = nullptr;
		}

		~applicationHandle()
		{
			delete _app;
		}

		void run()
		{
			if (_app) {
				_app->run();
				_app = nullptr;
			}
			else {
				throw std::runtime_error("trying to run deleted application");
			}
		}

	private:

		application *_app;
	};

    class applicationBuilder
    {
    public:

        applicationHandle build() const
        {
            return new application(_fullscreen, _maximized, _vsync, _width, _height, _title);
        }

        bool vsync() const
        {
            return _vsync;
        }

        applicationBuilder& vsync(bool vsync)
        {
            _vsync = vsync;
            return *this;
        }

        bool fullscreen() const
        {
            return _fullscreen;
        }

        applicationBuilder& fullscreen(bool fullscreen)
        {
            _fullscreen = fullscreen;
            _maximized = false;
            return *this;
        }

        bool maximized() const
        {
            return _maximized;
        }

        applicationBuilder& maximized(bool maximized)
        {
            _maximized = maximized;
            _fullscreen = false;
            return *this;
        }

        int width() const
        {
            return _width;
        }

        applicationBuilder& width(int width)
        {
            _width = width;
            _maximized = false;
            _fullscreen = false;
            return *this;
        }

        int height() const
        {
            return _height;
        }

        applicationBuilder& height(int height)
        {
            _height = height;
            _maximized = false;
            _fullscreen = false;
            return *this;
        }

        applicationBuilder& dimensions(int width, int height)
        {
            _width = width;
            _height = height;
            _maximized = false;
            _fullscreen = false;
            return *this;
        }

        std::string_view title() const
        {
            return _title;
        }

        applicationBuilder& title(std::string_view title)
        {
            _title = title;
            return *this;
        }

    private:

        bool _maximized = false;

        bool _fullscreen = false;

        bool _vsync = true;

        int _width = 1280;

        int _height = 720;

        std::string _title = "my app";
    };
}
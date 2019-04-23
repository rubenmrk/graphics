#pragma once

#include "opengl/glwindow.hpp"
#include "physics.hpp"
#include "camera.hpp"

namespace game
{
    class application : public opengl::window<application>
    {
        friend opengl::window<application>;
        friend class applicationBuilder;

        using opengl::window<application>::runWindow;
        using opengl::window<application>::getInput;
        using opengl::window<application>::getGraphics;
        using opengl::window<application>::quit;

		using model = typename graphics::model;
		using planeModel = typename graphics::planeModel;
		using text = typename graphics::text;
		using anchor = opengl::text::anchor;

    private:

        application(bool fullscreen, bool maximized, bool vsync, int width, int height, std::string_view title);

    public:

        application(const application& rhs) = delete;

        application(application&& rhs) = delete;

        ~application();

        void run();

    private:

        void updateLogic();

		void onExit();

        camera _cam;

        native::timeproxy _now;

        native::timeproxy _prev;

		std::vector<model> _models;			// Models to render with lighting enabled

		std::vector<planeModel> _pmodels;	// Models to render with lighting disabled

		std::vector<text> _texts;			// Texts to render
    };
}
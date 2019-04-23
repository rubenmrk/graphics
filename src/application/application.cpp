#include "application.hpp"
#include <sstream>
#include <iomanip>

using namespace game;

application::application(bool fullscreen, bool maximized, bool vsync, int width, int height, std::string_view title)
    : opengl::window<application>(fullscreen, maximized, vsync, width, height, title)
{
    if (_now.resolution() < 1'000'000)
        throw exception(except_e::APPLICATION, "inadequate timer resolution");
    _now.update();
}

application::~application()
{
	for (auto& m : _models)
		m.invalidate();
	for (auto& pm : _pmodels)
		pm.invalidate();
	for (auto& t : _texts)
		t.invalidate();
}

void application::run()
{
	runWindow();
}

void application::updateLogic()
{
    // Update time
    _prev = _now;
    _now.update();

    // Grab base class data
    double delta = (_now-_prev).toseconds();
    auto& input = getInput();
    auto& gfx = getGraphics();

	// Main logic
	static int state = 0;
	// Show a loading screen
	if (state == 0) {
		gfx.setFontSize(64);
		_texts.emplace_back(gfx.loadText("LOADING", 0.0f, 0.0f, anchor::center));
		gfx.setFontSize();
		state = 1;
	}
	// Load the models
	else if (state == 1) {
		_models.emplace_back(gfx.loadModel("chair"));
		_pmodels.emplace_back(gfx.loadPlaneModel("cube"));

		glm::vec3 lightPos = { 10.f, 10.f, 10.f };
		_pmodels.back()->modelMatrix = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(0.3f, 0.3f, 0.3f)), glm::vec3(lightPos));
		_models.back()->material.shininess = 64.0f;
		gfx.setLightPos(lightPos);
		gfx.setDiffuseColor(glm::vec3(1.0f, 1.0f, 1.0f));
		gfx.setspecularColor(glm::vec3(1.0f, 1.0f, 1.0f));

		state = 2;
	}
	// Make the light cube orbit around the regular cube and display the fps
	else {
		std::stringstream frametime;
		frametime << "frame: " << std::setprecision(delta >= 0.1 ? 3 : delta >= 0.01 ? 2 : 1) << (delta) << " s";
		_texts.back().reload(frametime.str(), 1.0f, 1.0f, anchor::topRigth);
	}

    // Always update the camera
    _cam.update(input, gfx, delta);

	// Always quit on escape
    if (input.isPressed(native::kbutton::escape)) {
        quit();
    }
}

void application::onExit()
{
	// Nothing needs to be cleaned up because the engine takes care of all resources.
	// Leaving this in because I might want to autosave here later.
}

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#ifdef _WIN32
extern "C"
{
	__declspec(dllexport) DWORD NvOptimusEnablement = 1;

	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif
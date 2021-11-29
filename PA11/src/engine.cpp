#include "engine.h"
#include "light.h"
#include "window.h"
#include "graphics.h"
#include "physics.h"
#include "camera.h"
#include "sound.h"

Engine::Engine(std::string name, int width, int height) {
	WINDOW_NAME = name;
	WINDOW_WIDTH = width;
	WINDOW_HEIGHT = height;
	FULLSCREEN = false;
}

Engine::Engine(std::string name) {
	WINDOW_NAME = name;
	WINDOW_HEIGHT = 0;
	WINDOW_WIDTH = 0;
	FULLSCREEN = true;
}

Engine::~Engine() {
	delete window;
	sceneRoot.reset((Object*) nullptr);
	delete graphics;
	delete sound;
	window = nullptr;
	graphics = nullptr;
}

bool Engine::initialize(const Arguments& args) {
	// Start a window
	window = new Window();
	if(!window->initialize(WINDOW_NAME, &WINDOW_WIDTH, &WINDOW_HEIGHT)) {
		printf("The window failed to initialize.\n");
		return false;
	}

	// Start the graphics
	graphics = new Graphics(sceneRoot);
	if(!graphics->initialize(WINDOW_WIDTH, WINDOW_HEIGHT, this, args)) {
		printf("The graphics failed to initialize.\n");
		return false;
	}

	// Start the physics
	physics = new Physics(sceneRoot);
	if(!physics->initialize(this, args)) {
		printf("The physics failed to initialize.\n");
		return false;
	}

	// Create the scene tree
	sceneRoot = std::make_shared<Object>();

	// Set the time
	frameStartTime = std::chrono::high_resolution_clock::now();

	sound = new Sound();

	// No errors
	return true;
}

void Engine::run() {
	running = true;

	while(running) {
		// Update the DT
		DT = getDT();

		// Process events
		GUI* gui = graphics->getGUI();
		while(SDL_PollEvent(&event) != 0) {
			auto shouldProcess = gui->processEvent(event);
			// Quit Events
			if(event.type == SDL_QUIT)
				running = false;
			// Key Events
			else if (shouldProcess.keyboard && (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)) {
				// Escape is quit
				if(event.key.keysym.sym == SDLK_ESCAPE)
					running = false;
				// Forward other events
				else
					keyboardEvent(event.key);
			}
			// Mouse Motion events
			else if(shouldProcess.mouse && event.type == SDL_MOUSEMOTION)
				mouseMotionEvent(event.motion);
			// Mouse Button events
			else if (shouldProcess.mouse && (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP))
				mouseButtonEvent(event.button);
			// Mouse Wheel events
			else if (shouldProcess.mouse && (event.type == SDL_MOUSEWHEEL))
				mouseWheelEvent(event.wheel);
		}

		// Take a sample of the current FPS
		fpsMeasurements.push_back(1.0 / DT);

		// Run application specific code
		update(DT);

		// Update the physics simulation (at a constant rate of 60 times per second)
		physicsAccumulator += DT;
		if(physicsAccumulator > 1.0/60) {
			physics->update(physicsAccumulator);
			physicsAccumulator = 0;
		}

		// Update the scene tree
		sceneRoot->update(DT);
		// Update and render the graphics
		graphics->update(DT);
		graphics->render();

		// Physic debug display
#ifdef PHYSICS_DEBUG
		physics->render(graphics->getCamera());
#endif

		// Swap the framebuffer
		window->swap();
	}
}

float Engine::getDT() {
	auto now = std::chrono::high_resolution_clock::now();
	auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(now - frameStartTime).count();
	frameStartTime = now;

	// Return elpased time in seconds
	return microseconds * 0.000001;
}

float Engine::getAverageFPS() {
	// Sum all of the recently measure framerates
	float accumulator = 0;
	for(float f: fpsMeasurements)
		accumulator += f;

	// Return the sum divided by the number of measurements taken
	return accumulator / fpsMeasurements.size();
}

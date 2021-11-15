#include "engine.h"
#include "light.h"
#include "window.h"
#include "graphics.h"
#include "physics.h"
#include "camera.h"
#include "sound.h"

Engine::Engine(string name, int width, int height) {
	m_WINDOW_NAME = name;
	m_WINDOW_WIDTH = width;
	m_WINDOW_HEIGHT = height;
	m_FULLSCREEN = false;
}

Engine::Engine(string name) {
	m_WINDOW_NAME = name;
	m_WINDOW_HEIGHT = 0;
	m_WINDOW_WIDTH = 0;
	m_FULLSCREEN = true;
}

Engine::~Engine() {
	delete m_window;
	delete sceneRoot;
	delete m_graphics;
	delete m_sound;
	m_window = nullptr;
	sceneRoot = nullptr;
	m_graphics = nullptr;
}

bool Engine::Initialize(const Arguments& args) {
	// Start a window
	m_window = new Window();
	if(!m_window->Initialize(m_WINDOW_NAME, &m_WINDOW_WIDTH, &m_WINDOW_HEIGHT)) {
		printf("The window failed to initialize.\n");
		return false;
	}

	// Start the graphics
	m_graphics = new Graphics(sceneRoot);
	if(!m_graphics->Initialize(m_WINDOW_WIDTH, m_WINDOW_HEIGHT, this, args)) {
		printf("The graphics failed to initialize.\n");
		return false;
	}

	// Start the physics
	m_physics = new Physics(sceneRoot);
	if(!m_physics->Initialize(this, args)) {
		printf("The physics failed to initialize.\n");
		return false;
	}

	// Create the scene tree
	sceneRoot = new Object();

	// Set the time
	m_currentTimeMillis = GetCurrentTimeMillis();

	m_sound = new Sound();

	// No errors
	return true;
}

void Engine::Run() {
	m_running = true;

	while(m_running) {
		// Update the DT
		m_DT = getDT();

		// Process events
		GUI* gui = m_graphics->getGUI();
		while(SDL_PollEvent(&m_event) != 0) {
			auto shouldProcess = gui->ProcessEvent(m_event);

			// bool w_down = false, a_down = false, s_down = false, d_down = false;

			// Quit Events
			if(m_event.type == SDL_QUIT)
				m_running = false;
			// Key Events
			else if (shouldProcess.keyboard && (m_event.type == SDL_KEYDOWN || m_event.type == SDL_KEYUP)){
				
				// Escape is quit
				if(m_event.key.keysym.sym == SDLK_ESCAPE)
					m_running = false;
				// Forward other events
				else
					keyboardEvent(m_event.key);
			}
			// Mouse Motion events
			else if(shouldProcess.mouse && m_event.type == SDL_MOUSEMOTION)
				mouseMotionEvent(m_event.motion);
			// Mouse Button events
			else if (shouldProcess.mouse && (m_event.type == SDL_MOUSEBUTTONDOWN || m_event.type == SDL_MOUSEBUTTONUP))
				mouseButtonEvent(m_event.button);
			// Mouse Wheel events
			else if (shouldProcess.mouse && (m_event.type == SDL_MOUSEWHEEL))
				mouseWheelEvent(m_event.wheel);
		}

		// Take a sample of the current FPS
		fpsMeasurements.push_back(1000.0 / (m_DT > 0 ? m_DT : 0.001));

		// Run application specific code
		Update(getDTMilli());

		// Update the physics simulation (at a constant rate of 60 times per second)
		if(m_DT > 0) m_physics->Update(m_DT);

		// Update the scene tree
		sceneRoot->Update(m_DT);
		// Update and render the graphics
		m_graphics->Update(m_DT);
		m_graphics->Render();

		// Physic debug display
#ifdef PHYSICS_DEBUG
		m_physics->Render(m_graphics->getCamera());
#endif

		// Swap to the Window
		m_window->Swap();
	}
}

unsigned int Engine::getDT() {
	// Get the current time
	long long TimeNowMillis = GetCurrentTimeMillis();
	assert(TimeNowMillis >= m_currentTimeMillis);
	// Get the elapsed time
	unsigned int DeltaTimeMillis = (unsigned int)(TimeNowMillis - m_currentTimeMillis);
	// Mark the current time as the previous time
	m_currentTimeMillis = TimeNowMillis;
	return DeltaTimeMillis;
}

long long Engine::GetCurrentTimeMillis() {
	// Get the current time
	timeval t;
	gettimeofday(&t, NULL);
	// Convert it to milliseconds and return
	return t.tv_sec * 1000 + t.tv_usec / 1000;
}

float Engine::getAverageFPS(){
	// Sum all of the recently measure framerates
	float accumulator = 0;
	for(float f: fpsMeasurements)
		accumulator += f;

	// Return the sum divided by the number of measurements taken
	return accumulator / fpsMeasurements.size();
}

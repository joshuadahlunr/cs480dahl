#include "engine.h"

#include "window.h"
#include "graphics.h"
#include "physics.h"

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

	// Start the physics
	m_physics = new Physics(sceneRoot);
	if(!m_physics->Initialize(this, args)) {
		printf("The physics failed to initialize.\n");
		return false;
	}

	// Start the graphics
	m_graphics = new Graphics(sceneRoot);
	if(!m_graphics->Initialize(m_WINDOW_WIDTH, m_WINDOW_HEIGHT, this, args)) {
		printf("The graphics failed to initialize.\n");
		return false;
	}

	// Create the scene tree
	sceneRoot = new Object();

	// Create a cylinder
	Object* cylinder = new Object();
	sceneRoot->addChild(cylinder);
	cylinder->InitializeGraphics(args, "cylinder.obj");
	cylinder->setPosition(glm::vec3(0,5,0));
	cylinder->InitializePhysics(args, *m_physics);

	// Create a sphere
	Object* sphere = new Object();
	sceneRoot->addChild(sphere);
	sphere->InitializeGraphics(args, "unitsphere.obj");
	sphere->setPosition(glm::vec3(4,5,0));
	sphere->InitializePhysics(args, *m_physics);

	// Create a cube
	Object* cube = new Object();
	sceneRoot->addChild(cube);
	cube->InitializeGraphics(args, "cube.obj");
	cube->setPosition(glm::vec3(-4,5,0));
	cube->InitializePhysics(args, *m_physics);

	// Create the board with walls
	Object* board = new Object();
	sceneRoot->addChild(board);
	board->InitializeGraphics(args, "fourwallboard.obj");
	//board->InitializePhysics(args, *m_physics); // TODO make static convex mesh physics object

	// Set the time
	m_currentTimeMillis = GetCurrentTimeMillis();

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

			// Quit Events
			if(m_event.type == SDL_QUIT)
				m_running = false;
			// Key Events
			else if (shouldProcess.keyboard && (m_event.type == SDL_KEYDOWN || m_event.type == SDL_KEYUP)){
				// Escape is quit
				if(m_event.key.keysym.sym == SDLK_ESCAPE)
					m_running = false;
				else
					keyboardEvent(m_event.key);
			// Mouse Motion events
			} else if(shouldProcess.mouse && m_event.type == SDL_MOUSEMOTION)
				mouseMotionEvent(m_event.motion);
			// Mouse Button events
			else if (shouldProcess.mouse && (m_event.type == SDL_MOUSEBUTTONDOWN || m_event.type == SDL_MOUSEBUTTONUP))
				mouseButtonEvent(m_event.button);
			// Mouse Wheel events
			else if (shouldProcess.mouse && (m_event.type == SDL_MOUSEWHEEL))
				mouseWheelEvent(m_event.wheel);
		}

		// Update the physics simulation
		m_physics->Update(m_DT);
		// Update the scene tree
		sceneRoot->Update(m_DT);
		// Update and render the graphics
		m_graphics->Update(m_DT);
		m_graphics->Render();

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

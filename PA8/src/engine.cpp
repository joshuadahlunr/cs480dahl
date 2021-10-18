#include "engine.h"

#include "window.h"
#include "graphics.h"
#include "physics.h"

#include "rings.h"
#include "celestial.h"

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

	// Create the celestials
	sceneRoot = CelestialFromJson(args, args.getConfig()["Scene"]);

	// Set the time
	m_currentTimeMillis = GetCurrentTimeMillis();

	// No errors
	return true;
}

// Helper function which converts a json array into a glm::vec3
// Provides an optional default value in case the provided json is null
glm::vec3 jsonToVec3(json j, glm::vec3 _default = glm::vec3(0)){
	if(j.is_null()) return _default;

	glm::vec3 out;
	out.x = j[0];
	out.y = j[1];
	out.z = j[2];
	return out;
}

// Helper function which converts a json array into a glm::vec3
// Provides an optional default value in case the provided json is null
glm::vec2 jsonToVec2(json j, glm::vec2 _default = glm::vec2(0)){
	if(j.is_null()) return _default;

	glm::vec2 out;
	out.x = j[0];
	out.y = j[1];
	return out;
}

// Helper function which converts a json array into a float
// Provides an optional default value in case the provided json is null
float jsonToFloat(json j, float _default = 0) {
	if(j.is_null()) return _default;

	return j;
}

// Recursively initializes a scene tree from the provided json data
Celestial* Engine::CelestialFromJson(const Arguments& args, json j, uint depth) {
	// Create a new celestial object
	Celestial* celestial = new Celestial();
	celestial->sceneDepth = depth;
	celestial->celestialRadius = j.value("Mean Radius (km)", 1);

	// Distance can be provided as a single number or a pair
	auto od = j["Mean Orbit Radius (km)"];
	if(od.is_number()) celestial->orbitDistance = glm::vec2((float) od);
	else if(od.is_array()) celestial->orbitDistance = jsonToVec2(od);
	else celestial->orbitDistance = glm::vec2(0);

	// Set the properties of the newly created object
	celestial->orbitSpeed = jsonToFloat(j["Orbit Period (d)"]);
	celestial->orbitSpeed = 360.0 / (((celestial->orbitSpeed)/365.0) * 60.0);
	if(isinf(celestial->orbitSpeed)) celestial->orbitSpeed = 0;

	//celestial->orbitSpeed = celestial->orbitSpeed/(60 * 60 * 360);
	celestial->orbitInitialOffset = jsonToFloat(j["Orbit Initial Offset"]);
	celestial->eclipticInclination = jsonToFloat(j["Ecliptic Inclination"]);
	celestial->orbitalTiltNormal = jsonToVec3(j["Orbital Tilt Normal"], glm::vec3(0, 1, 0));
	celestial->rotationSpeed = jsonToFloat(j["Sidereal Rotation Period (d)"]);
	celestial->rotationSpeed = 360.0 / (((celestial->rotationSpeed)/365.0) * 60.0);
	if(isinf(celestial->rotationSpeed)) celestial->rotationSpeed = 0;
	celestial->axialTiltNormal = jsonToVec3(j["Axial Tilt Normal"], glm::vec3(0, 1, 0));
	std::cout << celestial->rotationSpeed << std::endl;

	// Initialize the celestial and set its texture
	std::string texturePath = j.value("Texture Path", "textures/invalid.png");
	celestial->InitializeGraphics(args, args.getResourcePath() + texturePath);

	// If there is a ring defined
	if(j.contains("Ring")){
		// Create a child ring and set its properties
		Ring* ring = (Ring*) celestial->addChild(new Ring());
		ring->innerRadius = jsonToFloat(j["Ring"]["Inner Radius (km)"], 1);
		ring->outerRadius = jsonToFloat(j["Ring"]["Outer Radius (km)"], 2);
		ring->resolution = j["Ring"].value("Resolution", 128);
		ring->tilt = jsonToVec3(j["Ring"]["Tilt"], glm::vec3(0, 1, 0));

		// Initialize the ring and set its texture
		std::string texturePath = j["Ring"].value("Texture Path", "textures/invalid.png");
		ring->InitializeGraphics(args, args.getResourcePath() + texturePath);
	}

	// Recursively initialize the celestial's children
	for (auto child: j["Children"])
		celestial->addChild(CelestialFromJson(args, child, depth + 1));

	return celestial;
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

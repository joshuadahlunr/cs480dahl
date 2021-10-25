#include "engine.h"

#include "window.h"
#include "graphics.h"
#include "physics.h"
#include "camera.h"

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

	// Create a cylinder
	Object* cylinder = new Object();
	sceneRoot->addChild(cylinder);
	cylinder->InitializeGraphics(args, "cylinder.obj");
	cylinder->setPosition(glm::vec3(0,-1,0));
	cylinder->InitializePhysics(args, *m_physics, true);
	cylinder->addCapsuleCollider(1, 2);

	// Create a sphere
	Object* sphere = new Object();
	sceneRoot->addChild(sphere);
	sphere->InitializeGraphics(args, "unitsphere.obj");
	sphere->setPosition(glm::vec3(4,5,0));
	sphere->InitializePhysics(args, *m_physics, false);
	sphere->addSphereCollider(1);
	// Set inital values for the physics material
	{
		rp3d::Material& material = sphere->getCollider().getMaterial();
		material.setBounciness(1);
		material.setFrictionCoefficient(0);
		material.setRollingResistance(.01);
	}

	// Create a cube
	Object* cube = new Object();
	sceneRoot->addChild(cube);
	cube->InitializeGraphics(args, "cube.obj");
	cube->setPosition(glm::vec3(-4, -1,0));
	cube->InitializePhysics(args, *m_physics, true);
	cube->addBoxCollider(glm::vec3(1, 1, 1));
	cube->getRigidBody().setType(rp3d::BodyType::KINEMATIC);
	controlledObject = cube;


	// Create the board with walls
	Object* board = new Object();
	sceneRoot->addChild(board);
	board->InitializeGraphics(args, "fourwallboard.obj");
	board->InitializePhysics(args, *m_physics, true); // TODO make static convex mesh physics object
	board->addMeshCollider(false); // Concave mesh
	// Set inital values for the physics material
	{
		rp3d::Material& material = board->getCollider().getMaterial();
		material.setBounciness(.2);
		material.setFrictionCoefficient(0);
	}

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

			bool w_down = false, a_down = false, s_down = false, d_down = false;

			// Quit Events
			if(m_event.type == SDL_QUIT)
				m_running = false;
			// Key Events
			else if (shouldProcess.keyboard && (m_event.type == SDL_KEYDOWN || m_event.type == SDL_KEYUP)){
				// Force
				rp3d::Vector3 force(0.0,0.0,0.0);
				// Escape is quit
				if(m_event.key.keysym.sym == SDLK_ESCAPE)
					m_running = false;
				else {
					// Cube Controls
					if(m_event.key.keysym.sym == SDLK_w)
						w_down = (m_event.type == SDL_KEYDOWN) ? true : false;
					if(m_event.key.keysym.sym == SDLK_a)
						a_down = (m_event.type == SDL_KEYDOWN) ? true : false;
					if(m_event.key.keysym.sym == SDLK_s)
						s_down = (m_event.type == SDL_KEYDOWN) ? true : false;
					if(m_event.key.keysym.sym == SDLK_d)
						d_down = (m_event.type == SDL_KEYDOWN) ? true : false;

					keyboardEvent(m_event.key);
				}
					
					
				rp3d::Vector3 delta(0.0,0.0,0.0);
				if(w_down)
					delta = rp3d::Vector3(0.0, 0.0, 20.0 * m_DT / 1000.0); 
					
				if(s_down)
					delta = rp3d::Vector3(0.0, 0.0, -20.0 * m_DT / 1000.0); 
					
				if(a_down)
					delta = rp3d::Vector3(20.0 * m_DT / 1000.0, 0.0, 0); 
					
				if(d_down)
					delta = rp3d::Vector3(-20.0 * m_DT / 1000.0, 0.0, 0); 

				

				rp3d::Transform t = controlledObject->getPhysicsTransform();
				t.setPosition(t.getPosition() + delta);
				//controlledObject->.applyForceToCenterOfMass(force);
				controlledObject->setPhysicsTransform(t);
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

		// Update the physics simulation (at a constant rate of 60 times per second)
		physicsAccumulator += m_DT;
		if(physicsAccumulator > 16){
			m_physics->Update(m_DT);
			physicsAccumulator = 0;
		}

		// Update the scene tree
		sceneRoot->Update(m_DT);
		// Update and render the graphics
		m_graphics->Update(m_DT);
		m_graphics->Render();
		
		// // Physic debug display
		//m_physics->Render(m_graphics->getCamera());

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

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

	// Create global light
	Object* light = new GlobalLight();
	sceneRoot->addChild(light);

	// Create a spotlight
	light = new Spotlight();
	sceneRoot->addChild(light);
	light->setPosition(glm::vec3(-7,0.5,6));

	// Create a pointLight
	light = new PointLight();
	sceneRoot->addChild(light);
	light->setPosition(glm::vec3(7,0.5,6));

	// Create a cylinder
	// Object* cylinder = new Object();
	// sceneRoot->addChild(cylinder);
	// cylinder->InitializeGraphics(args, "cylinder.obj");
	// cylinder->setPosition(glm::vec3(0,-1,0));
	// cylinder->InitializePhysics(args, *m_physics, true);
	// cylinder->addCapsuleCollider(1, 2);

	// Create a sphere
	Object* sphere = new Object();
	sceneRoot->addChild(sphere);
	sphere->InitializeGraphics(args, "unitsphere.obj");
	sphere->setPosition(glm::vec3(0.75,0.5,6));
	sphere->InitializePhysics(args, *m_physics, false);
	sphere->addSphereCollider(.5);
	//m_physics->ballID = sphere->getCollider().getEntity().id; // Save the ball's ID
	// m_physics->ball = &sphere->getRigidBody();
	// Set initial values for the physics material
	{
		rp3d::Material& material = sphere->getCollider().getMaterial();
		material.setBounciness(0.5);
		material.setFrictionCoefficient(0);
		material.setRollingResistance(.01);
	}

	// Create the board with walls
	Object* board = new Object();
	sceneRoot->addChild(board);
	board->InitializeGraphics(args, "fourwallboard.obj");
	board->InitializePhysics(args, *m_physics, true); // TODO make static convex mesh physics object
	board->addMeshCollider(false); // Concave mesh
	// Set initial values for the physics material
	{
		rp3d::Material& material = board->getCollider().getMaterial();
		material.setBounciness(.2);
		material.setFrictionCoefficient(0);
	}

	// Lambda which bounces a ball off an object
	Physics::ContactEvent bounceBall = [sphere, ballID = sphere->getCollider().getEntity().id](const rp3d::CollisionCallback::ContactPair& contact){

		// Only bother if we are interacting with a ball
		if(contact.getCollider1()->getEntity().id == ballID || contact.getCollider2()->getEntity().id == ballID){
			// Get where we are contacting the ball
			rp3d::Vector3 contactPoint;
			if(contact.getCollider1()->getEntity().id == ballID)
				contactPoint = contact.getContactPoint(0).getLocalPointOnCollider1();
			else contactPoint = contact.getContactPoint(0).getLocalPointOnCollider2();

			// Apply a bounce force to the ball
			sphere->getRigidBody().applyForceAtLocalPosition(-contact.getContactPoint(0).getWorldNormal() * 1000.0, contactPoint);
		}
	};

	// Create left paddle
	leftPaddle = new Object();
	sceneRoot->addChild(leftPaddle);
	leftPaddle->InitializeGraphics(args, "paddle.obj");
	leftPaddle->setPosition(glm::vec3(2, -2,-2));
	leftPaddle->InitializePhysics(args, *m_physics, true); // TODO make static convex mesh physics object
	leftPaddle->addMeshCollider(false); // Concave mesh
	//leftPaddle->addBoxCollider(glm::vec3(1, 1, .3), rp3d::Transform(rp3d::Vector3(1, 0, 0), rp3d::Quaternion::identity()));
	leftPaddle->getRigidBody().setType(rp3d::BodyType::KINEMATIC);
	m_physics->addContactCallback(leftPaddle, bounceBall);
	// Set initial values for the physics material
	{
		rp3d::Material& material = leftPaddle->getCollider().getMaterial();
		material.setBounciness(.2);
		// material.setFrictionCoefficient(0);
	}

	// Create right paddle
	rightPaddle = new Object();
	sceneRoot->addChild(rightPaddle);
	rightPaddle->InitializeGraphics(args, "paddle.obj");
	rightPaddle->setPosition(glm::vec3(-2, -2, -2));
	rightPaddle->InitializePhysics(args, *m_physics, true); // TODO make static convex mesh physics object
	rightPaddle->addMeshCollider(false);
	//rightPaddle->addBoxCollider(glm::vec3(1, 1, .3), rp3d::Transform(rp3d::Vector3(1, 0, 0), rp3d::Quaternion::identity()));
	rightPaddle->getRigidBody().setType(rp3d::BodyType::KINEMATIC);
	m_physics->addContactCallback(rightPaddle, bounceBall);
	// Set initial values for the physics material
	{
		rp3d::Material& material = rightPaddle->getCollider().getMaterial();
		material.setBounciness(.2);
		// material.setFrictionCoefficient(0);
	}

	// Object* rightPaddleCollider = new Object();
	// rightPaddle->addChild(rightPaddleCollider);
	// rightPaddleCollider->InitializeGraphics(args, "paddlecollider.obj");
	// rightPaddleCollider->addMeshCollider(false);

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

			// bool w_down = false, a_down = false, s_down = false, d_down = false;

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
				// Space toggles between fragment and vertex shader
				else if(m_event.key.keysym.sym == SDLK_SPACE && m_event.type == SDL_KEYUP)
					m_graphics->useFragShader = !m_graphics->useFragShader;
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

		// Get what keys are pressed
		const Uint8* keyState = SDL_GetKeyboardState(NULL);

		// If A or Left is pressed flip the left paddle
		if(keyState[SDL_SCANCODE_A] || keyState[SDL_SCANCODE_LEFT])
			leftPaddleAngle += paddleSpeed * m_DT / 1000.0;
		else
			leftPaddleAngle -= paddleSpeed * m_DT / 1000.0;

		// If D or Right is pressed flip the right paddle
		if(keyState[SDL_SCANCODE_D] || keyState[SDL_SCANCODE_RIGHT])
			rightPaddleAngle -= paddleSpeed * m_DT / 1000.0;
		else
			rightPaddleAngle += paddleSpeed * m_DT / 1000.0;

		// Clamp the paddle angles
		if(leftPaddleAngle > 270) leftPaddleAngle = 270;
		if(leftPaddleAngle < 180) leftPaddleAngle = 180;
		if(rightPaddleAngle < -90) rightPaddleAngle = -90;
		if(rightPaddleAngle > 0) rightPaddleAngle = 0;

		// Apply the paddle angles to the physics simulation
		rp3d::Transform t = leftPaddle->getPhysicsTransform();
		t.setOrientation(rp3d::Quaternion::fromEulerAngles(0, glm::radians(leftPaddleAngle), 0));
		leftPaddle->setPhysicsTransform(t);
		t = rightPaddle->getPhysicsTransform();
		t.setOrientation(rp3d::Quaternion::fromEulerAngles(0, glm::radians(rightPaddleAngle), 0));
		rightPaddle->setPhysicsTransform(t);


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

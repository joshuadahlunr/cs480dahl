#include "engine.h"
#include "light.h"
#include "window.h"
#include "graphics.h"
#include "physics.h"
#include "camera.h"

int Engine::score = 0;

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
	// Object* light = new DirectionalLight();
	// sceneRoot->addChild(light);

	// Create an ambient light
	Light* light = new AmbientLight();
	sceneRoot->addChild(light);
	light->setAmbient(glm::vec4(0.6392156863, 0.7254901961, 0.8980392157, 1));

	// Create a spotlight
	light = new SpotLight();
	sceneRoot->addChild(light);
	light->setPosition(glm::vec3(0,24,0));
	light->setCutoffAngle(glm::radians(180.0));

	// Create another spotlight
	light = new SpotLight();
	sceneRoot->addChild(light);
	light->setPosition(glm::vec3(0,10,-10));
	light->setCutoffAngle(glm::radians(30.0));

	// // Create a pointLight
	// light = new PointLight();
	// sceneRoot->addChild(light);
	// light->setPosition(glm::vec3(7,0.5,6));

	// Create a cylinder
	// Object* cylinder = new Object();
	// sceneRoot->addChild(cylinder);
	// cylinder->InitializeGraphics(args, "cylinder.obj");
	// cylinder->setPosition(glm::vec3(0,-1,0));
	// cylinder->InitializePhysics(args, *m_physics, true);
	// cylinder->addCapsuleCollider(1, 2);

	// Create a sphere
	Object* sphere = new Object();
	ball = sphere;
	sceneRoot->addChild(sphere);
	sphere->InitializeGraphics(args, "unitsphere.obj");
	sphere->setPosition(glm::vec3(-8.75, 0.75, -12.5));
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
	board->InitializeGraphics(args, "pinballV3.obj");
	board->InitializePhysics(args, *m_physics, true); // TODO make static convex mesh physics object
	board->addMeshCollider(args, false);  // rp3d::Transform(), "board.obj" // Concave custom mesh
	// Set initial values for the physics material
	{
		rp3d::Material& material = board->getCollider().getMaterial();
		material.setBounciness(.2);
		material.setFrictionCoefficient(0);
	}

	// Lambda which bounces a ball off bumpers and increase score
	Physics::ContactEvent bounceBallWithPoints = [sphere, ballID = ball->getCollider().getEntity().id](const rp3d::CollisionCallback::ContactPair& contact){

		// Only bother if we are interacting with a ball
		if(contact.getCollider1()->getEntity().id == ballID || contact.getCollider2()->getEntity().id == ballID){
			// Get where we are contacting the ball
			rp3d::Vector3 contactPoint;
			if(contact.getCollider1()->getEntity().id == ballID)
				contactPoint = contact.getContactPoint(0).getLocalPointOnCollider1();
			else contactPoint = contact.getContactPoint(0).getLocalPointOnCollider2();

			// Apply a bounce force to the ball
			sphere->getRigidBody().applyForceAtLocalPosition(-contact.getContactPoint(0).getWorldNormal() * 1000.0, contactPoint);
			Engine::score += 10;
		}
	};

	// Lambda which bounces a ball off paddles
	Physics::ContactEvent bounceBall = [sphere, ballID = ball->getCollider().getEntity().id](const rp3d::CollisionCallback::ContactPair& contact){

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
	leftPaddle->setPosition(glm::vec3(4.4, 0, -13));
	leftPaddle->InitializePhysics(args, *m_physics, true); // TODO make static convex mesh physics object
	leftPaddle->addMeshCollider(args, false, rp3d::Transform(), "paddleCollider.obj"); // Concave mesh
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
	rightPaddle->setPosition(glm::vec3(-2.4, 0, -13));
	rightPaddle->InitializePhysics(args, *m_physics, true); // TODO make static convex mesh physics object
	rightPaddle->addMeshCollider(args, false, rp3d::Transform(), "paddleCollider.obj"); // Concave mesh
	//rightPaddle->addBoxCollider(glm::vec3(1, 1, .3), rp3d::Transform(rp3d::Vector3(1, 0, 0), rp3d::Quaternion::identity()));
	rightPaddle->getRigidBody().setType(rp3d::BodyType::KINEMATIC);
	m_physics->addContactCallback(rightPaddle, bounceBall);
	// Set initial values for the physics material
	{
		rp3d::Material& material = rightPaddle->getCollider().getMaterial();
		material.setBounciness(.2);
		// material.setFrictionCoefficient(0);
	}

	//CREATE COLLIDERS FOR BOARD COLLISIONS

	Object* floor = new Object();
	sceneRoot->addChild(floor);
	floor->setPosition(glm::vec3(0, -1, 0));
	floor->InitializePhysics(args, *m_physics, true); // TODO make static convex mesh physics object
	floor->addBoxCollider(glm::vec3(11.5, 1, 19));  // rp3d::Transform(), "board.obj" // Concave custom mesh
	// Set initial values for the physics material
	{
		rp3d::Material& material = floor->getCollider().getMaterial();
		material.setBounciness(.2);
		material.setFrictionCoefficient(0);
	}

	Object* leftWall = new Object();
	sceneRoot->addChild(leftWall);
	leftWall->setPosition(glm::vec3(10.5, 1, -4));
	leftWall->InitializePhysics(args, *m_physics, true); // TODO make static convex mesh physics object
	leftWall->addBoxCollider(glm::vec3(1, 2, 13.5));  // rp3d::Transform(), "board.obj" // Concave custom mesh
	// Set initial values for the physics material
	{
		rp3d::Material& material = leftWall->getCollider().getMaterial();
		material.setBounciness(.2);
		material.setFrictionCoefficient(.2);
	}

	Object* rightWall = new Object();
	sceneRoot->addChild(rightWall);
	rightWall->setPosition(glm::vec3(-10.5, 1, -4));
	rightWall->InitializePhysics(args, *m_physics, true); // TODO make static convex mesh physics object
	rightWall->addBoxCollider(glm::vec3(1, 2, 13.5));  // rp3d::Transform(), "board.obj" // Concave custom mesh
	// Set initial values for the physics material
	{
		rp3d::Material& material = rightWall->getCollider().getMaterial();
		material.setBounciness(.2);
		material.setFrictionCoefficient(0);
	}

	Object* bottomWallLeft = new Object();
	sceneRoot->addChild(bottomWallLeft);
	bottomWallLeft->setPosition(glm::vec3(6.101, 1, -15.571));
	bottomWallLeft->rotate(glm::radians(-20.0), glm::vec3(0, 1, 0));
	bottomWallLeft->InitializePhysics(args, *m_physics, true); // TODO make static convex mesh physics object
	bottomWallLeft->addBoxCollider(glm::vec3(4, 2, 1));  // rp3d::Transform(), "board.obj" // Concave custom mesh
	// Set initial values for the physics material
	{
		rp3d::Material& material = bottomWallLeft->getCollider().getMaterial();
		material.setBounciness(.2);
		material.setFrictionCoefficient(0);
	}

	Object* bottomWallRight = new Object();
	sceneRoot->addChild(bottomWallRight);
	bottomWallRight->setPosition(glm::vec3(-4.101, 1, -15.571));
	bottomWallRight->rotate(glm::radians(20.0), glm::vec3(0, 1, 0));
	bottomWallRight->InitializePhysics(args, *m_physics, true); // TODO make static convex mesh physics object
	bottomWallRight->addBoxCollider(glm::vec3(4, 2, 1));  // rp3d::Transform(), "board.obj" // Concave custom mesh
	// Set initial values for the physics material
	{
		rp3d::Material& material = bottomWallRight->getCollider().getMaterial();
		material.setBounciness(.2);
		material.setFrictionCoefficient(0);
	}

	Object* bottomWallCenter = new Object();
	sceneRoot->addChild(bottomWallCenter);
	bottomWallCenter->setPosition(glm::vec3(1, 1, -18));
	bottomWallCenter->InitializePhysics(args, *m_physics, true); // TODO make static convex mesh physics object
	bottomWallCenter->addBoxCollider(glm::vec3(2, 1, 1));  // rp3d::Transform(), "board.obj" // Concave custom mesh
	// Set initial values for the physics material
	{
		rp3d::Material& material = bottomWallCenter->getCollider().getMaterial();
		material.setBounciness(0.2);
		material.setFrictionCoefficient(0);
	}

	Object* dividerWall = new Object();
	sceneRoot->addChild(dividerWall);
	dividerWall->setPosition(glm::vec3(-7.75, 1, -3.75));
	dividerWall->InitializePhysics(args, *m_physics, true); // TODO make static convex mesh physics object
	dividerWall->addBoxCollider(glm::vec3(.25, 2, 11.25));  // rp3d::Transform(), "board.obj" // Concave custom mesh
	// Set initial values for the physics material
	{
		rp3d::Material& material = dividerWall->getCollider().getMaterial();
		material.setBounciness(0.2);
		material.setFrictionCoefficient(0);
	}

	Object* plungerCollider = new Object();
	sceneRoot->addChild(plungerCollider);
	plungerCollider->setPosition(glm::vec3(-8.75, 0.75, -15.25));
	plungerCollider->InitializePhysics(args, *m_physics, true); // TODO make static convex mesh physics object
	plungerCollider->addBoxCollider(glm::vec3(1, 1, 1));  // rp3d::Transform(), "board.obj" // Concave custom mesh
	// Set initial values for the physics material
	{
		rp3d::Material& material = plungerCollider->getCollider().getMaterial();
		material.setBounciness(0.2);
		material.setFrictionCoefficient(0);
	}

	Object* leftBumper = new Object();
	sceneRoot->addChild(leftBumper);
	leftBumper->setPosition(glm::vec3(6.5, 1, -1));
	leftBumper->InitializePhysics(args, *m_physics, true); // TODO make static convex mesh physics object
	leftBumper->addCapsuleCollider(0.625, 2);  // rp3d::Transform(), "board.obj" // Concave custom mesh
	m_physics->addContactCallback(leftBumper, bounceBallWithPoints);
	// Set initial values for the physics material
	{
		rp3d::Material& material = leftBumper->getCollider().getMaterial();
		material.setBounciness(0.2);
		material.setFrictionCoefficient(0);
	}

	Object* rightBumper = new Object();
	sceneRoot->addChild(rightBumper);
	rightBumper->setPosition(glm::vec3(-4.5, 1, -1));
	rightBumper->InitializePhysics(args, *m_physics, true); // TODO make static convex mesh physics object
	rightBumper->addCapsuleCollider(0.625, 2);  // rp3d::Transform(), "board.obj" // Concave custom mesh
	m_physics->addContactCallback(rightBumper, bounceBallWithPoints);
	// Set initial values for the physics material
	{
		rp3d::Material& material = rightBumper->getCollider().getMaterial();
		material.setBounciness(0.2);
		material.setFrictionCoefficient(0);
	}


	Object* topBumper_1 = new Object();
	sceneRoot->addChild(topBumper_1);
	topBumper_1->setPosition(glm::vec3(-4.5, 1, 5));
	topBumper_1->InitializePhysics(args, *m_physics, true); // TODO make static convex mesh physics object
	topBumper_1->addCapsuleCollider(0.5, 2);  // rp3d::Transform(), "board.obj" // Concave custom mesh
	m_physics->addContactCallback(topBumper_1, bounceBallWithPoints);
	// Set initial values for the physics material
	{
		rp3d::Material& material = topBumper_1->getCollider().getMaterial();
		material.setBounciness(0.2);
		material.setFrictionCoefficient(0);
	}

	Object* topBumper_2 = new Object();
	sceneRoot->addChild(topBumper_2);
	topBumper_2->setPosition(glm::vec3(-5, 1, 7.084));
	topBumper_2->InitializePhysics(args, *m_physics, true); // TODO make static convex mesh physics object
	topBumper_2->addCapsuleCollider(0.5, 2);  // rp3d::Transform(), "board.obj" // Concave custom mesh
	m_physics->addContactCallback(topBumper_2, bounceBallWithPoints);
	// Set initial values for the physics material
	{
		rp3d::Material& material = topBumper_2->getCollider().getMaterial();
		material.setBounciness(0.2);
		material.setFrictionCoefficient(0);
	}

	Object* topBumper_3 = new Object();
	sceneRoot->addChild(topBumper_3);
	topBumper_3->setPosition(glm::vec3(-4.7, 1, 9.156));
	topBumper_3->InitializePhysics(args, *m_physics, true); // TODO make static convex mesh physics object
	topBumper_3->addCapsuleCollider(0.5, 2);  // rp3d::Transform(), "board.obj" // Concave custom mesh
	m_physics->addContactCallback(topBumper_3, bounceBallWithPoints);
	// Set initial values for the physics material
	{
		rp3d::Material& material = topBumper_3->getCollider().getMaterial();
		material.setBounciness(0.2);
		material.setFrictionCoefficient(0);
	}

	Object* topBumper_4 = new Object();
	sceneRoot->addChild(topBumper_4);
	topBumper_4->setPosition(glm::vec3(-3.598, 1, 10.91));
	topBumper_4->InitializePhysics(args, *m_physics, true); // TODO make static convex mesh physics object
	topBumper_4->addCapsuleCollider(0.5, 2);  // rp3d::Transform(), "board.obj" // Concave custom mesh
	m_physics->addContactCallback(topBumper_4, bounceBallWithPoints);
	// Set initial values for the physics material
	{
		rp3d::Material& material = topBumper_4->getCollider().getMaterial();
		material.setBounciness(0.2);
		material.setFrictionCoefficient(0);
	}

	Object* topBumper_5 = new Object();
	sceneRoot->addChild(topBumper_5);
	topBumper_5->setPosition(glm::vec3(-1.927, 1, 12.081));
	topBumper_5->InitializePhysics(args, *m_physics, true); // TODO make static convex mesh physics object
	topBumper_5->addCapsuleCollider(0.5, 2);  // rp3d::Transform(), "board.obj" // Concave custom mesh
	m_physics->addContactCallback(topBumper_5, bounceBallWithPoints);
	// Set initial values for the physics material
	{
		rp3d::Material& material = topBumper_5->getCollider().getMaterial();
		material.setBounciness(0.2);
		material.setFrictionCoefficient(0);
	}

	Object* wallBumper_1 = new Object();
	sceneRoot->addChild(wallBumper_1);
	wallBumper_1->setPosition(glm::vec3(8.6865, 1, 8.3663));
	wallBumper_1->rotate(glm::radians(-5.21), glm::vec3(0, 1, 0));
	wallBumper_1->InitializePhysics(args, *m_physics, true); // TODO make static convex mesh physics object
	wallBumper_1->addBoxCollider(glm::vec3(1, 2, 0.5));  // rp3d::Transform(), "board.obj" // Concave custom mesh
	m_physics->addContactCallback(wallBumper_1, bounceBallWithPoints);
	// Set initial values for the physics material
	{
		rp3d::Material& material = wallBumper_1->getCollider().getMaterial();
		material.setBounciness(0.2);
		material.setFrictionCoefficient(0);
	}

	Object* wallBumper_2 = new Object();
	sceneRoot->addChild(wallBumper_2);
	wallBumper_2->setPosition(glm::vec3(8.1991, 1, 10.34));
	wallBumper_2->rotate(glm::radians(-19.4), glm::vec3(0, 1, 0));
	wallBumper_2->InitializePhysics(args, *m_physics, true); // TODO make static convex mesh physics object
	wallBumper_2->addBoxCollider(glm::vec3(1, 2, 1.1));  // rp3d::Transform(), "board.obj" // Concave custom mesh
	m_physics->addContactCallback(wallBumper_2, bounceBallWithPoints);
	// Set initial values for the physics material
	{
		rp3d::Material& material = wallBumper_2->getCollider().getMaterial();
		material.setBounciness(0.2);
		material.setFrictionCoefficient(0);
	}

	Object* wallBumper_3 = new Object();
	sceneRoot->addChild(wallBumper_3);
	wallBumper_3->setPosition(glm::vec3(7.3927, 1, 12.183));
	wallBumper_3->rotate(glm::radians(-32.6), glm::vec3(0, 1, 0));
	wallBumper_3->InitializePhysics(args, *m_physics, true); // TODO make static convex mesh physics object
	wallBumper_3->addBoxCollider(glm::vec3(1, 2, 0.5));  // rp3d::Transform(), "board.obj" // Concave custom mesh
	m_physics->addContactCallback(wallBumper_3, bounceBallWithPoints);
	// Set initial values for the physics material
	{
		rp3d::Material& material = wallBumper_3->getCollider().getMaterial();
		material.setBounciness(0.2);
		material.setFrictionCoefficient(0);
	}

	Object* wallBumper_4 = new Object();
	sceneRoot->addChild(wallBumper_4);
	wallBumper_4->setPosition(glm::vec3(6.0526, 1, 13.747));
	wallBumper_4->rotate(glm::radians(-45.3), glm::vec3(0, 1, 0));
	wallBumper_4->InitializePhysics(args, *m_physics, true); // TODO make static convex mesh physics object
	wallBumper_4->addBoxCollider(glm::vec3(1, 2, 1.1));  // rp3d::Transform(), "board.obj" // Concave custom mesh
	m_physics->addContactCallback(wallBumper_4, bounceBallWithPoints);
	// Set initial values for the physics material
	{
		rp3d::Material& material = wallBumper_4->getCollider().getMaterial();
		material.setBounciness(0.2);
		material.setFrictionCoefficient(0);
	}

	Object* wallBumper_5 = new Object();
	sceneRoot->addChild(wallBumper_5);
	wallBumper_5->setPosition(glm::vec3(4.4989, 1, 15.002));
	wallBumper_5->rotate(glm::radians(-58.1), glm::vec3(0, 1, 0));
	wallBumper_5->InitializePhysics(args, *m_physics, true); // TODO make static convex mesh physics object
	wallBumper_5->addBoxCollider(glm::vec3(1, 2, 0.5));  // rp3d::Transform(), "board.obj" // Concave custom mesh
	m_physics->addContactCallback(wallBumper_5, bounceBallWithPoints);
	// Set initial values for the physics material
	{
		rp3d::Material& material = wallBumper_4->getCollider().getMaterial();
		material.setBounciness(0.2);
		material.setFrictionCoefficient(0);
	}

	Object* wallGuard = new Object();
	sceneRoot->addChild(wallGuard);
	wallGuard->setPosition(glm::vec3(8.9938, 1, 8.5004));
	wallGuard->InitializePhysics(args, *m_physics, true); // TODO make static convex mesh physics object
	wallGuard->addBoxCollider(glm::vec3(1, 2, 1));  // rp3d::Transform(), "board.obj" // Concave custom mesh
	// Set initial values for the physics material
	{
		rp3d::Material& material = wallGuard->getCollider().getMaterial();
		material.setBounciness(0.2);
		material.setFrictionCoefficient(0);
	}

	Object* guideRailLeft = new Object();
	sceneRoot->addChild(guideRailLeft);
	guideRailLeft->InitializePhysics(args, *m_physics, true); // TODO make static convex mesh physics object
	guideRailLeft->addMeshCollider(args, false, rp3d::Transform(), "guideRailLeft.obj");  // rp3d::Transform(), "board.obj" // Concave custom mesh
	// Set initial values for the physics material
	{
		rp3d::Material& material = guideRailLeft->getCollider().getMaterial();
		material.setBounciness(0.2);
		material.setFrictionCoefficient(0);
	}

	Object* guideRailRight = new Object();
	sceneRoot->addChild(guideRailRight);
	guideRailRight->InitializePhysics(args, *m_physics, true); // TODO make static convex mesh physics object
	guideRailRight->addMeshCollider(args, false, rp3d::Transform(), "guideRailRight.obj");  // rp3d::Transform(), "board.obj" // Concave custom mesh
	// Set initial values for the physics material
	{
		rp3d::Material& material = guideRailRight->getCollider().getMaterial();
		material.setBounciness(0.2);
		material.setFrictionCoefficient(0);
	}

	Object* topWall = new Object();
	sceneRoot->addChild(topWall);
	topWall->InitializePhysics(args, *m_physics, true); // TODO make static convex mesh physics object
	topWall->addMeshCollider(args, false, rp3d::Transform(), "topWall.obj");  // rp3d::Transform(), "board.obj" // Concave custom mesh
	// Set initial values for the physics material
	{
		rp3d::Material& material = topWall->getCollider().getMaterial();
		material.setBounciness(0.2);
		material.setFrictionCoefficient(0);
	}

	Object* sCurves = new Object();
	sceneRoot->addChild(sCurves);
	sCurves->InitializePhysics(args, *m_physics, true); // TODO make static convex mesh physics object
	sCurves->addMeshCollider(args, false, rp3d::Transform(), "sCurves.obj");  // rp3d::Transform(), "board.obj" // Concave custom mesh
	// Set initial values for the physics material
	{
		rp3d::Material& material = sCurves->getCollider().getMaterial();
		material.setBounciness(0.2);
		material.setFrictionCoefficient(0);
	}

	Object* guards = new Object();
	sceneRoot->addChild(guards);
	guards->InitializePhysics(args, *m_physics, true); // TODO make static convex mesh physics object
	guards->addMeshCollider(args, false, rp3d::Transform(), "guards.obj");  // rp3d::Transform(), "board.obj" // Concave custom mesh
	// Set initial values for the physics material
	{
		rp3d::Material& material = guards->getCollider().getMaterial();
		material.setBounciness(0.2);
		material.setFrictionCoefficient(0);
	}

	Object* guardBumpers = new Object();
	sceneRoot->addChild(guardBumpers);
	guardBumpers->InitializePhysics(args, *m_physics, true); // TODO make static convex mesh physics object
	guardBumpers->addMeshCollider(args, false, rp3d::Transform(), "guardBumpers.obj");  // rp3d::Transform(), "board.obj" // Concave custom mesh
	m_physics->addContactCallback(guardBumpers, bounceBallWithPoints);
	// Set initial values for the physics material
	{
		rp3d::Material& material = guardBumpers->getCollider().getMaterial();
		material.setBounciness(0.2);
		material.setFrictionCoefficient(0);
	}

	Object* topArc = new Object();
	sceneRoot->addChild(topArc);
	topArc->InitializePhysics(args, *m_physics, true); // TODO make static convex mesh physics object
	topArc->addMeshCollider(args, false, rp3d::Transform(), "topArc.obj");  // rp3d::Transform(), "board.obj" // Concave custom mesh
	// Set initial values for the physics material
	{
		rp3d::Material& material = topArc->getCollider().getMaterial();
		material.setBounciness(0.2);
		material.setFrictionCoefficient(0);
	}

	Object* sideCurves = new Object();
	sceneRoot->addChild(sideCurves);
	sideCurves->InitializePhysics(args, *m_physics, true); // TODO make static convex mesh physics object
	sideCurves->addMeshCollider(args, false, rp3d::Transform(), "sideCurves.obj");  // rp3d::Transform(), "board.obj" // Concave custom mesh
	// Set initial values for the physics material
	{
		rp3d::Material& material = sideCurves->getCollider().getMaterial();
		material.setBounciness(0.2);
		material.setFrictionCoefficient(0);
	}

	Object* entryGuide = new Object();
	sceneRoot->addChild(entryGuide);
	entryGuide->InitializePhysics(args, *m_physics, true); // TODO make static convex mesh physics object
	entryGuide->addMeshCollider(args, false, rp3d::Transform(), "entryGuide.obj");  // rp3d::Transform(), "board.obj" // Concave custom mesh
	// Set initial values for the physics material
	{
		rp3d::Material& material = entryGuide->getCollider().getMaterial();
		material.setBounciness(0.2);
		material.setFrictionCoefficient(0);
	}

	Object* roof = new Object();
	sceneRoot->addChild(roof);
	roof->setPosition(glm::vec3(0, 4, 0));
	roof->InitializePhysics(args, *m_physics, true); // TODO make static convex mesh physics object
	roof->addBoxCollider(glm::vec3(11.5, 1, 19));  // rp3d::Transform(), "board.obj" // Concave custom mesh
	// Set initial values for the physics material
	{
		rp3d::Material& material = roof->getCollider().getMaterial();
		material.setBounciness(0.2);
		material.setFrictionCoefficient(0);
	}
	// Object* rightPaddleCollider = new Object();
	// rightPaddle->addChild(rightPaddleCollider);
	// a->InitializeGraphics(args, "paddlecollider.obj");
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
		std::cout << (1000 / (m_DT > 0 ? m_DT : 0.001)) << " fps" << std::endl;
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
				else if(m_event.key.keysym.sym == SDLK_DOWN && m_event.type == SDL_KEYUP)
				{
					ball->getRigidBody().applyForceAtLocalPosition(rp3d::Vector3(0, 0, 1) * 2000.0, rp3d::Vector3(0, 0, 0));
				}
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
		if(leftPaddleAngle > 250) leftPaddleAngle = 250;
		if(leftPaddleAngle < 160) leftPaddleAngle = 160;
		if(rightPaddleAngle < -70) rightPaddleAngle = -70;
		if(rightPaddleAngle > 20) rightPaddleAngle = 20;

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

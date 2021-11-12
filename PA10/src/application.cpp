#include "application.h"
#include "light.h"
#include "window.h"
#include "graphics.h"
#include "physics.h"
#include "camera.h"
#include "threadTimer.h"

Application::~Application() {
	// Save the leaderstats when shutting down
	if (leaderboard) {
		leaderboard->Save();
		leaderboard = nullptr;
	}
}

bool Application::Initialize(const Arguments& args){
	bool ret = Engine::Initialize(args);

	// Create a leaderboard
	leaderboard = new Leaderboard();
	leaderboard->Initialize(args, "leaderstats.csv");

	lights = std::vector<Light*>();
	lights.reserve(3);

	// Create an ambient light
	lights.push_back(new AmbientLight());
	std::cout << getSceneRoot() << std::endl;
	getSceneRoot()->addChild(lights[0]);
	lights[0]->setAmbient(glm::vec4(0.6392156863, 0.7254901961, 0.8980392157, 1));

	// Create a spotlight
	lights.push_back(new SpotLight());
	getSceneRoot()->addChild(lights[1]);
	lights[1]->setPosition(glm::vec3(0,24,0));
	lights[1]->setCutoffAngle(glm::radians(180.0));

	// Create a ball
	ball = new Object();
	getSceneRoot()->addChild(ball);
	ball->InitializeGraphics(args, "unitsphere.obj");
	ball->setPosition(glm::vec3(-8.75, 0.75, -12.5));
	ball->InitializePhysics(args, *getPhysics(), false);
	ball->addSphereCollider(.5);
	ball->setPhysicsTransform(ball->getGraphicsTransform());
	// Set initial values for the physics material
	{
		rp3d::Material& material = ball->getCollider().getMaterial();
		material.setBounciness(0.5);
		material.setFrictionCoefficient(0);
		material.setRollingResistance(.01);
	}

	// Lambda which bounces a ball off bumpers and increase score
	auto bounceBallWithPoints = [ballID = ball->getCollider().getEntity().id, this](const rp3d::CollisionCallback::ContactPair& contact, Light* lightup = nullptr){
		// Only bother if we are interacting with a ball
		if(contact.getCollider1()->getEntity().id == ballID || contact.getCollider2()->getEntity().id == ballID){
			// Get where we are contacting the ball
			rp3d::Vector3 contactPoint;
			if(contact.getCollider1()->getEntity().id == ballID)
				contactPoint = contact.getContactPoint(0).getLocalPointOnCollider1();
			else contactPoint = contact.getContactPoint(0).getLocalPointOnCollider2();

			// Apply a bounce force to the ball
			ball->getRigidBody().applyForceAtLocalPosition(-contact.getContactPoint(0).getWorldNormal() * 1000.0, contactPoint);
			score += 10;

			// Enable our light and start a timer to turn it off
			if(lightup){
				lightup->enable();
				threadTimer t;
				t.setTimeout([lightup](){ lightup->disable(); }, std::chrono::milliseconds(250));
			}
		}
	};

	// Lambda which bounces a ball off paddles
	Physics::ContactEvent bounceBall = [ballID = ball->getCollider().getEntity().id, this](const rp3d::CollisionCallback::ContactPair& contact){
		// Only bother if we are interacting with a ball
		if(contact.getCollider1()->getEntity().id == ballID || contact.getCollider2()->getEntity().id == ballID){
			// Get where we are contacting the ball
			rp3d::Vector3 contactPoint;
			if(contact.getCollider1()->getEntity().id == ballID)
				contactPoint = contact.getContactPoint(0).getLocalPointOnCollider1();
			else contactPoint = contact.getContactPoint(0).getLocalPointOnCollider2();

            uint32_t otherId = contact.getCollider1()->getEntity().id == ballID ? contact.getCollider2()->getEntity().id : contact.getCollider1()->getEntity().id;

			// Apply a bounce force to the ball
            if(otherId == leftPaddle->getCollider().getEntity().id && leftPaddleMoving || otherId == rightPaddle->getCollider().getEntity().id && rightPaddleMoving) {
			    ball->getRigidBody().applyForceAtLocalPosition(-contact.getContactPoint(0).getWorldNormal() * 1000.0, contactPoint);
            }
		}
	};

	// Lambda which detects when ball is at bottom of board
	Physics::ContactEvent ballInDeadZone = [ballID = ball->getCollider().getEntity().id, this](const rp3d::CollisionCallback::ContactPair& contact){

		// Only bother if we are interacting with a ball
		if(contact.getCollider1()->getEntity().id == ballID || contact.getCollider2()->getEntity().id == ballID){
			// Game over logic
			if (--ballsRemaining <= 0) {
				gameState = GameState::GameOver;
				std::cout << "Game Over! You are out of balls!!" << std::endl;
				std::cout << "Your final score was: " << score << std::endl;
			}

			resetBall();
		}
	};

	// Lambda which resets if we can push when the ball collides with the plunger
	Physics::ContactEvent plungerPushReset = [ballID = ball->getCollider().getEntity().id, this](const rp3d::CollisionCallback::ContactPair& contact){

		// Only bother if we are interacting with a ball
		if(contact.getCollider1()->getEntity().id == ballID || contact.getCollider2()->getEntity().id == ballID){
			hasLaunched = false;
		}
	};

	// Create another spotlight
	lights.push_back(new SpotLight());
	ball->addChild(lights[2]);
	lights[2]->setPosition(glm::vec3(0,10,0));
	lights[2]->setCutoffAngle(glm::radians(30.0));

	// Create the board with walls
	Object* board = new Object();
	getSceneRoot()->addChild(board);
	board->InitializeGraphics(args, "pinballV4.obj");
	board->InitializePhysics(args, *getPhysics(), true);
	// board->addMeshCollider(args, false);
	board->LoadTextureFile(args, "../textures/pinball-texture.png");


	// Create left paddle
	leftPaddle = new Object();
	getSceneRoot()->addChild(leftPaddle);
	leftPaddle->InitializeGraphics(args, "paddle.obj");
	leftPaddle->setPosition(glm::vec3(4.4, 0, -13.25));
	leftPaddle->InitializePhysics(args, *getPhysics(), true);
	leftPaddle->addBoxCollider(glm::vec3(1.53, 1, 0.333), rp3d::Transform(rp3d::Vector3(1.125, 0, 0), rp3d::Quaternion::identity()));
	leftPaddle->getRigidBody().setType(rp3d::BodyType::KINEMATIC);
	getPhysics()->addContactCallback(leftPaddle, bounceBall);

	// Create right paddle
	rightPaddle = new Object();
	getSceneRoot()->addChild(rightPaddle);
	rightPaddle->InitializeGraphics(args, "paddle.obj");
	rightPaddle->setPosition(glm::vec3(-2.4, 0, -13.25));
	rightPaddle->InitializePhysics(args, *getPhysics(), true);
	rightPaddle->addBoxCollider(glm::vec3(1.53, 1, 0.333), rp3d::Transform(rp3d::Vector3(1.125, 0, 0), rp3d::Quaternion::identity()));
	rightPaddle->getRigidBody().setType(rp3d::BodyType::KINEMATIC);
	getPhysics()->addContactCallback(rightPaddle, bounceBall);

	// Create left paddle
	plunger = new Object();
	getSceneRoot()->addChild(plunger);
	plunger->InitializeGraphics(args, "plunger.obj");
	plunger->setPosition(glm::vec3(-8.75, 0.75, -14.638));
	plunger->InitializePhysics(args, *getPhysics(), true);
	plunger->addBoxCollider(glm::vec3(1, 1, 1.3875));
	plunger->getRigidBody().setType(rp3d::BodyType::KINEMATIC);
	getPhysics()->addContactCallback(plunger, plungerPushReset);


	//CREATE COLLIDERS FOR BOARD COLLISIONS
	// Sorry not sorry :)


	Object* floor = new Object();
	getSceneRoot()->addChild(floor);
	floor->setPosition(glm::vec3(0, -1, 0));
	floor->InitializePhysics(args, *getPhysics(), true);
	floor->addBoxCollider(glm::vec3(11.5, 1, 19));

	Object* roof = new Object();
	getSceneRoot()->addChild(roof);
	roof->setPosition(glm::vec3(0, 1, 0));
	roof->InitializePhysics(args, *getPhysics(), true);
	roof->addBoxCollider(glm::vec3(11.5, 1, 19));

	Object* leftWall = new Object();
	getSceneRoot()->addChild(leftWall);
	leftWall->setPosition(glm::vec3(10.5, 1, -4));
	leftWall->InitializePhysics(args, *getPhysics(), true);
	leftWall->addBoxCollider(glm::vec3(1, 2, 13.5));

	Object* rightWall = new Object();
	getSceneRoot()->addChild(rightWall);
	rightWall->setPosition(glm::vec3(-10.5, 1, -4));
	rightWall->InitializePhysics(args, *getPhysics(), true);
	rightWall->addBoxCollider(glm::vec3(1, 2, 13.5));

	Object* bottomWallLeft = new Object();
	getSceneRoot()->addChild(bottomWallLeft);
	bottomWallLeft->setPosition(glm::vec3(6.101, 1, -15.571));
	bottomWallLeft->rotate(glm::radians(-20.0), glm::vec3(0, 1, 0));
	bottomWallLeft->InitializePhysics(args, *getPhysics(), true);
	bottomWallLeft->addBoxCollider(glm::vec3(4, 2, 1));

	Object* bottomWallRight = new Object();
	getSceneRoot()->addChild(bottomWallRight);
	bottomWallRight->setPosition(glm::vec3(-4.101, 1, -15.571));
	bottomWallRight->rotate(glm::radians(20.0), glm::vec3(0, 1, 0));
	bottomWallRight->InitializePhysics(args, *getPhysics(), true);
	bottomWallRight->addBoxCollider(glm::vec3(4, 2, 1));

	bottomWallCenter = new Object();
	getSceneRoot()->addChild(bottomWallCenter);
	bottomWallCenter->setPosition(glm::vec3(1, 1, -18));
	bottomWallCenter->InitializePhysics(args, *getPhysics(), true);
	bottomWallCenter->addBoxCollider(glm::vec3(2, 1, 1));
	getPhysics()->addContactCallback(bottomWallCenter, ballInDeadZone);

	Object* dividerWall = new Object();
	getSceneRoot()->addChild(dividerWall);
	dividerWall->setPosition(glm::vec3(-7.75, 1, -3.75));
	dividerWall->InitializePhysics(args, *getPhysics(), true);
	dividerWall->addBoxCollider(glm::vec3(.25, 2, 11.25));

	Light* bumperLight = new PointLight();
	getSceneRoot()->addChild(bumperLight);
	bumperLight->setPosition(glm::vec3(0, 5, 0));
	bumperLight->setDiffuse(glm::vec4(1, 0, 0, 1));
	bumperLight->setSpecular(glm::vec4(1, 0, 0, 1));
	bumperLight->setCutoffAngle(glm::radians(40.0));
	bumperLight->disable();

	Object* leftBumper = new Object();
	getSceneRoot()->addChild(leftBumper);
	leftBumper->setPosition(glm::vec3(6.5, 1, -1));
	leftBumper->InitializePhysics(args, *getPhysics(), true);
	leftBumper->addCapsuleCollider(0.625, 2);
	getPhysics()->addContactCallback(leftBumper, std::bind(bounceBallWithPoints, std::placeholders::_1, bumperLight));

	Object* rightBumper = new Object();
	getSceneRoot()->addChild(rightBumper);
	rightBumper->setPosition(glm::vec3(-4.5, 1, -1));
	rightBumper->InitializePhysics(args, *getPhysics(), true);
	rightBumper->addCapsuleCollider(0.625, 2);
	getPhysics()->addContactCallback(rightBumper, std::bind(bounceBallWithPoints, std::placeholders::_1, bumperLight));

	Object* topBumper_1 = new Object();
	getSceneRoot()->addChild(topBumper_1);
	topBumper_1->setPosition(glm::vec3(-4.5, 1, 5));
	topBumper_1->InitializePhysics(args, *getPhysics(), true);
	topBumper_1->addCapsuleCollider(0.5, 2);
	getPhysics()->addContactCallback(topBumper_1, std::bind(bounceBallWithPoints, std::placeholders::_1, bumperLight));

	Object* topBumper_2 = new Object();
	getSceneRoot()->addChild(topBumper_2);
	topBumper_2->setPosition(glm::vec3(-5, 1, 7.084));
	topBumper_2->InitializePhysics(args, *getPhysics(), true);
	topBumper_2->addCapsuleCollider(0.5, 2);
	getPhysics()->addContactCallback(topBumper_2, std::bind(bounceBallWithPoints, std::placeholders::_1, bumperLight));

	Object* topBumper_3 = new Object();
	getSceneRoot()->addChild(topBumper_3);
	topBumper_3->setPosition(glm::vec3(-4.7, 1, 9.156));
	topBumper_3->InitializePhysics(args, *getPhysics(), true);
	topBumper_3->addCapsuleCollider(0.5, 2);
	getPhysics()->addContactCallback(topBumper_3, std::bind(bounceBallWithPoints, std::placeholders::_1, bumperLight));

	Object* topBumper_4 = new Object();
	getSceneRoot()->addChild(topBumper_4);
	topBumper_4->setPosition(glm::vec3(-3.598, 1, 10.91));
	topBumper_4->InitializePhysics(args, *getPhysics(), true);
	topBumper_4->addCapsuleCollider(0.5, 2);
	getPhysics()->addContactCallback(topBumper_4, std::bind(bounceBallWithPoints, std::placeholders::_1, bumperLight));

	Object* topBumper_5 = new Object();
	getSceneRoot()->addChild(topBumper_5);
	topBumper_5->setPosition(glm::vec3(-1.927, 1, 12.081));
	topBumper_5->InitializePhysics(args, *getPhysics(), true);
	topBumper_5->addCapsuleCollider(0.5, 2);
	getPhysics()->addContactCallback(topBumper_5, std::bind(bounceBallWithPoints, std::placeholders::_1, bumperLight));

	Object* wallBumper_1 = new Object();
	getSceneRoot()->addChild(wallBumper_1);
	wallBumper_1->setPosition(glm::vec3(8.6865, 1, 8.3663));
	wallBumper_1->rotate(glm::radians(-5.21), glm::vec3(0, 1, 0));
	wallBumper_1->InitializePhysics(args, *getPhysics(), true);
	wallBumper_1->addBoxCollider(glm::vec3(1, 2, 0.5));
	getPhysics()->addContactCallback(wallBumper_1, std::bind(bounceBallWithPoints, std::placeholders::_1, bumperLight));

	Object* wallBumper_2 = new Object();
	getSceneRoot()->addChild(wallBumper_2);
	wallBumper_2->setPosition(glm::vec3(8.1991, 1, 10.34));
	wallBumper_2->rotate(glm::radians(-19.4), glm::vec3(0, 1, 0));
	wallBumper_2->InitializePhysics(args, *getPhysics(), true);
	wallBumper_2->addBoxCollider(glm::vec3(1, 2, 1.1));
	getPhysics()->addContactCallback(wallBumper_2, std::bind(bounceBallWithPoints, std::placeholders::_1, bumperLight));

	Object* wallBumper_3 = new Object();
	getSceneRoot()->addChild(wallBumper_3);
	wallBumper_3->setPosition(glm::vec3(7.3927, 1, 12.183));
	wallBumper_3->rotate(glm::radians(-32.6), glm::vec3(0, 1, 0));
	wallBumper_3->InitializePhysics(args, *getPhysics(), true);
	wallBumper_3->addBoxCollider(glm::vec3(1, 2, 0.5));
	getPhysics()->addContactCallback(wallBumper_3, std::bind(bounceBallWithPoints, std::placeholders::_1, bumperLight));

	Object* wallBumper_4 = new Object();
	getSceneRoot()->addChild(wallBumper_4);
	wallBumper_4->setPosition(glm::vec3(6.0526, 1, 13.747));
	wallBumper_4->rotate(glm::radians(-45.3), glm::vec3(0, 1, 0));
	wallBumper_4->InitializePhysics(args, *getPhysics(), true);
	wallBumper_4->addBoxCollider(glm::vec3(1, 2, 1.1));
	getPhysics()->addContactCallback(wallBumper_4, std::bind(bounceBallWithPoints, std::placeholders::_1, bumperLight));

	Object* wallBumper_5 = new Object();
	getSceneRoot()->addChild(wallBumper_5);
	wallBumper_5->setPosition(glm::vec3(4.4989, 1, 15.002));
	wallBumper_5->rotate(glm::radians(-58.1), glm::vec3(0, 1, 0));
	wallBumper_5->InitializePhysics(args, *getPhysics(), true);
	wallBumper_5->addBoxCollider(glm::vec3(1, 2, 0.5));
	getPhysics()->addContactCallback(wallBumper_5, std::bind(bounceBallWithPoints, std::placeholders::_1, bumperLight));

	Object* wallGuard = new Object();
	getSceneRoot()->addChild(wallGuard);
	wallGuard->setPosition(glm::vec3(8.9938, 1, 8.5004));
	wallGuard->InitializePhysics(args, *getPhysics(), true);
	wallGuard->addBoxCollider(glm::vec3(1, 2, 1));

	Object* guideRailLeft = new Object();
	getSceneRoot()->addChild(guideRailLeft);
	guideRailLeft->InitializePhysics(args, *getPhysics(), true);
	guideRailLeft->addMeshCollider(args, false, rp3d::Transform(), "guideRailLeft.obj");

	Object* guideRailRight = new Object();
	getSceneRoot()->addChild(guideRailRight);
	guideRailRight->InitializePhysics(args, *getPhysics(), true);
	guideRailRight->addMeshCollider(args, false, rp3d::Transform(), "guideRailRight.obj");

	Object* topWall = new Object();
	getSceneRoot()->addChild(topWall);
	topWall->InitializePhysics(args, *getPhysics(), true);
	topWall->addMeshCollider(args, false, rp3d::Transform(), "topWall.obj");

	Object* topWallReinforcement_1 = new Object();
	getSceneRoot()->addChild(topWallReinforcement_1);
	topWallReinforcement_1->setPosition(glm::vec3(0, 1, 18.1));
	topWallReinforcement_1->InitializePhysics(args, *getPhysics(), true);
	topWallReinforcement_1->addBoxCollider(glm::vec3(1, 1, 1));

	Object* topWallReinforcement_2 = new Object();
	getSceneRoot()->addChild(topWallReinforcement_1);
	topWallReinforcement_2->setPosition(glm::vec3(1.6303, 1, 17.961));
	topWallReinforcement_2->rotate(glm::radians(7.91), glm::vec3(0, 1, 0));
	topWallReinforcement_2->InitializePhysics(args, *getPhysics(), true);
	topWallReinforcement_2->addBoxCollider(glm::vec3(1, 1, 1));

	Object* sCurveLeft = new Object();
	getSceneRoot()->addChild(sCurveLeft);
	sCurveLeft->InitializePhysics(args, *getPhysics(), true);
	sCurveLeft->addMeshCollider(args, false, rp3d::Transform(), "sCurveLeft.obj");

	Object* sCurveLeftCylinderTop = new Object();
	getSceneRoot()->addChild(sCurveLeftCylinderTop);
	sCurveLeftCylinderTop->setPosition(glm::vec3(8.5, 1, 1.9741));
	sCurveLeftCylinderTop->InitializePhysics(args, *getPhysics(), true);
	sCurveLeftCylinderTop->addCapsuleCollider(1, 2);

	Object* sCurveLeftCube = new Object();
	getSceneRoot()->addChild(sCurveLeftCube);
	sCurveLeftCube->setPosition(glm::vec3(9.0105, 1, 2.7422));
	sCurveLeftCube->rotate(glm::radians(33.0), glm::vec3(0, 1, 0));
	sCurveLeftCube->InitializePhysics(args, *getPhysics(), true);
	sCurveLeftCube->addBoxCollider(glm::vec3(1, 1, 1));

	Object* sCurveRight = new Object();
	getSceneRoot()->addChild(sCurveRight);
	sCurveRight->InitializePhysics(args, *getPhysics(), true);
	sCurveRight->addMeshCollider(args, false, rp3d::Transform(), "sCurveRight.obj");

	Object* sCurveRightCylinderTop = new Object();
	getSceneRoot()->addChild(sCurveRightCylinderTop);
	sCurveRightCylinderTop->setPosition(glm::vec3(-6.5, 1, 1.9741));
	sCurveRightCylinderTop->InitializePhysics(args, *getPhysics(), true);
	sCurveRightCylinderTop->addCapsuleCollider(1, 2);

	Object* sCurveLRightCube = new Object();
	getSceneRoot()->addChild(sCurveLRightCube);
	sCurveLRightCube->setPosition(glm::vec3(-7.0105, 1, 2.7422));
	sCurveLRightCube->rotate(glm::radians(-33.0), glm::vec3(0, 1, 0));
	sCurveLRightCube->InitializePhysics(args, *getPhysics(), true);
	sCurveLRightCube->addBoxCollider(glm::vec3(.5, 1, 1));

	Object* guardLeft = new Object();
	getSceneRoot()->addChild(guardLeft);
	guardLeft->InitializePhysics(args, *getPhysics(), true);
	guardLeft->addMeshCollider(args, false, rp3d::Transform(), "guardLeft.obj");

	Object* guardRight = new Object();
	getSceneRoot()->addChild(guardRight);
	guardRight->InitializePhysics(args, *getPhysics(), true);
	guardRight->addMeshCollider(args, false, rp3d::Transform(), "guardRight.obj");

	Object* guardBumperLeft = new Object();
	getSceneRoot()->addChild(guardBumperLeft);
	guardBumperLeft->InitializePhysics(args, *getPhysics(), true);
	guardBumperLeft->addMeshCollider(args, false, rp3d::Transform(), "guardBumperLeft.obj");
	getPhysics()->addContactCallback(guardBumperLeft, std::bind(bounceBallWithPoints, std::placeholders::_1, bumperLight));

	Object* guardBumperRight = new Object();
	getSceneRoot()->addChild(guardBumperRight);
	guardBumperRight->InitializePhysics(args, *getPhysics(), true);
	guardBumperRight->addMeshCollider(args, false, rp3d::Transform(), "guardBumperRight.obj");
	getPhysics()->addContactCallback(guardBumperRight, std::bind(bounceBallWithPoints, std::placeholders::_1, bumperLight));

	Object* topArc = new Object();
	getSceneRoot()->addChild(topArc);
	topArc->InitializePhysics(args, *getPhysics(), true);
	topArc->addMeshCollider(args, false, rp3d::Transform(), "topArc.obj");

	Object* sideCurves = new Object();
	getSceneRoot()->addChild(sideCurves);
	sideCurves->InitializePhysics(args, *getPhysics(), true);
	sideCurves->addMeshCollider(args, false, rp3d::Transform(), "sideCurves.obj");

	Object* entryGuide = new Object();
	getSceneRoot()->addChild(entryGuide);
	entryGuide->InitializePhysics(args, *getPhysics(), true);
	entryGuide->addMeshCollider(args, false, rp3d::Transform(), "entryGuide.obj");

	// Object* rightPaddleCollider = new Object();
	// rightPaddle->addChild(rightPaddleCollider);
	// a->InitializeGraphics(args, "paddlecollider.obj");
	// rightPaddleCollider->addMeshCollider(false);

	// Register ourself as a listener for keyboard events
    keyboardEvent += [this](const SDL_KeyboardEvent& event){
        this->KeyboardCallback(event);
    };

	// Welcome message
	std::cout << std::endl << std::string(60, '-') << std::endl << "You have 3 balls... get as high of a score as you can! Good luck!" << std::endl;

	return ret;
}

void Application::Update(float dt){

    // Get what keys are pressed
	const Uint8* keyState = SDL_GetKeyboardState(NULL);

	// Only read game input if playing
	if (gameState == GameState::Playing) {
		// If A or Left is pressed flip the left paddle
		leftPaddleMoving = keyState[SDL_SCANCODE_A] || keyState[SDL_SCANCODE_LEFT];
		// If D or Right is pressed flip the right paddle
		rightPaddleMoving = keyState[SDL_SCANCODE_D] || keyState[SDL_SCANCODE_RIGHT];

		// If S or down is pressed pull the plunger back
		if(keyState[SDL_SCANCODE_S] || keyState[SDL_SCANCODE_DOWN])
			ballLaunchPower += powerIncreaseSpeed * dt;
		else
			ballLaunchPower -= 17000 * dt;
	}

	// Update paddle angle if necessary
	if(leftPaddleMoving)
		leftPaddleAngle += paddleSpeed * dt;
	else
		leftPaddleAngle -= paddleSpeed * dt;
	if(rightPaddleMoving)
		rightPaddleAngle -= paddleSpeed * dt;
	else
		rightPaddleAngle += paddleSpeed * dt;

	// Clamp the paddle angles
	// If clamping, paddles are not moving
	if(leftPaddleAngle > 250) {
		leftPaddleAngle = 250;
		leftPaddleMoving = false;
	}
	else if(leftPaddleAngle < 160) {
		leftPaddleAngle = 160;
		leftPaddleMoving = false;
	}
	if(rightPaddleAngle < -70) {
		rightPaddleAngle = -70;
		rightPaddleMoving = false;
	}
	if(rightPaddleAngle > 20) {
		rightPaddleAngle = 20;
		rightPaddleMoving = false;
	}

	// Apply the paddle angles to the physics simulation
	rp3d::Transform t = leftPaddle->getPhysicsTransform();
	t.setOrientation(rp3d::Quaternion::fromEulerAngles(0, glm::radians(leftPaddleAngle), 0));
	leftPaddle->setPhysicsTransform(t);
	t = rightPaddle->getPhysicsTransform();
	t.setOrientation(rp3d::Quaternion::fromEulerAngles(0, glm::radians(rightPaddleAngle), 0));
	rightPaddle->setPhysicsTransform(t);

	// Clamp the ball launch power
	if(ballLaunchPower > 2000) ballLaunchPower = 2000;
	if(ballLaunchPower < 0) ballLaunchPower = 0;

	// Set the plunger position
	t = plunger->getPhysicsTransform();
	t.setPosition(rp3d::Vector3(-8.75,0.75,-14.638 - ballLaunchPower/1400.0));
	plunger->setPhysicsTransform(t);

	// Cap the balls velocity at 40
	rp3d::Vector3 velocity = ball->getRigidBody().getLinearVelocity();
	if(velocity.lengthSquare() > 30 * 30){
		velocity.normalize();
		ball->getRigidBody().setLinearVelocity(velocity * 30);
	}
}

void Application::Reset() {
	// Reset the application to starting point
	cout << "Resetting Game" << endl;
	ballsRemaining = 3;
	score = 0;
	resetBall();
	gameState = GameState::Playing;
}

void Application::KeyboardCallback(const SDL_KeyboardEvent& event){
    // Force
	rp3d::Vector3 force(0.0,0.0,0.0);

    // Space toggles between fragment and vertex shader
	if(event.keysym.sym == SDLK_SPACE && event.type == SDL_KEYUP)
		getGraphics()->useFragShader = !getGraphics()->useFragShader;
	else if((event.keysym.sym == SDLK_DOWN || event.keysym.sym == SDLK_s) && event.type == SDL_KEYUP && !hasLaunched){ // TODO: should only happen before launched
		ball->getRigidBody().applyForceAtLocalPosition(rp3d::Vector3(0, 0, 1) * ballLaunchPower, rp3d::Vector3(0, 0, 0));
		hasLaunched = true;
	} else if(event.keysym.sym == SDLK_r && event.type == SDL_KEYUP)
		resetBall();
}

void Application::resetBall() {
	std::cout << ballsRemaining << " balls left with score: " << score << std::endl;

	// Teleport the ball back to the plunger
	rp3d::Transform t = ball->getPhysicsTransform();
	t.setPosition(rp3d::Vector3(-8.75, 0.75, -12.5));
	ball->setPhysicsTransform(t);
	ball->getRigidBody().setLinearVelocity(rp3d::Vector3(0, 0, 0)); // Remove the ball's velocity when we teleport it
	ball->getRigidBody().setAngularVelocity(rp3d::Vector3(0, 0, 0));
}

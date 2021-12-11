#include "application.h"
#include "window.h"
#include "graphics.h"
#include "sound.h"
#include "physics.h"
#include "camera.h"
#include "threadTimer.h"
#include "shader.h"
#include "npc.h"
#include "chunk.h"

bool Application::initialize(const Arguments& args) {
	bool ret = Engine::initialize(args);

	this->args = args;

	ufo = std::make_shared<Object>();
	getSceneRoot()->addChild(ufo);
	ufo->setPosition({8, 0, 8});
	ufo->initializeGraphics(args, "ufo.obj", "texturemap.png");
	Engine::getGraphics()->getCamera()->setFocus(ufo);
	ufo->initializePhysics(args, Engine::getPhysics(), CollisionGroups::UFO, /*mass*/ 100);
	ufo->createMeshCollider(args, Engine::getPhysics(), CONVEX_MESH, "ufo.obj");
	ufo->makeDynamic();
	ufo->getRigidBody().setGravity({0, 0, 0}); // Disable gravity on the UFO

	ufoLight = std::make_shared<SpotLight>();
	ufo->addChild(ufoLight);
	ufoLight->setAmbient({.6, .6, 1, 1});
	ufoLight->setDiffuse({.7, .7, 1, 1});
	ufoLight->setSpecular({.8, .8, 1, 1});

	auto ambient = std::make_shared<AmbientLight>();
	getSceneRoot()->addChild(ambient);
	ambient->setAmbient({.5, .5, .5, 1});

	auto light = std::make_shared<DirectionalLight>();
	getSceneRoot()->addChild(light);
	light->setDirection({-.5, -1, -.3});
	light->setDiffuse({.8, .8, .8, 1});

	world.initialize();

	// Create the NPCs
	int numCows = 10;
	for (int i = 0; i < numCows; i++) {
		createNPC("cow");
	}

	// Hookup the input events
	Engine::keyboardEvent += [&](auto event) { keyboard(event);};
	Engine::mouseButtonEvent += [&](auto event) { mouseButton(event); };
	Engine::mouseMotionEvent += [&](auto event) { mouseMotion(event); };
	Engine::mouseWheelEvent += [&](auto event) { mouseWheel(event); };

	return ret;
}

void Application::createNPC(std::string type) {
	std::shared_ptr<NPC> npc = std::make_shared<NPC>();
	std::string modelFile = "";
	if (type == "cow") {
		modelFile = "cow.obj";
	}
	getSceneRoot()->addChild(npc);
	npc->initializeGraphics(args, modelFile, "texturemap.png");

	float range = 50;
	float x = ufo->getPosition().x + (rand() % (int) range) -(range/2.0f);
	float z = ufo->getPosition().z + (rand() % (int) range) -(range/2.0f);
	float y = -50;
	if (!isnan(world.getWorldHeight(glm::ivec3(x, 0, z))))
		y = world.getWorldHeight(glm::ivec3(x, 0, z));

	npc->setPosition(glm::vec3(x, y, z));
	// Create Physics
	npc->initializePhysics(args, Engine::getPhysics(), CollisionGroups::Cow, /*mass*/ 100);
	npc->createMeshCollider(args, Engine::getPhysics(), CONVEX_MESH, "cube.obj");
	npc->makeDynamic();
	npcs.push_back(npc);
}

void Application::controlUFO(float dt) {
	// Capture input
	const Uint8* keystate = SDL_GetKeyboardState(NULL);
	inputDirection = glm::vec3();
	abducting = false;
	if (keystate[SDL_SCANCODE_W] || keystate[SDL_SCANCODE_UP])
		inputDirection += glm::vec3(1,0,0);
	if (keystate[SDL_SCANCODE_S] || keystate[SDL_SCANCODE_DOWN])
		inputDirection += glm::vec3(-1,0,0);
	if (keystate[SDL_SCANCODE_A] || keystate[SDL_SCANCODE_LEFT])
		inputDirection += glm::vec3(0,-1,0);
	if (keystate[SDL_SCANCODE_D] || keystate[SDL_SCANCODE_RIGHT])
		inputDirection += glm::vec3(0,1,0);
	if (keystate[SDL_SCANCODE_Q] || keystate[SDL_SCANCODE_PAGEUP])
		inputDirection += glm::vec3(0,0,1);
	if (keystate[SDL_SCANCODE_E] || keystate[SDL_SCANCODE_PAGEDOWN])
		inputDirection += glm::vec3(0,0,-1);
	if (keystate[SDL_SCANCODE_SPACE])
		abducting = true;

	// UFO Control
	float speed = 10;
	glm::vec3 camDirection = Engine::getGraphics()->getCamera()->getLookDirection();
	glm::vec3 planeDirection = glm::normalize(glm::vec3(camDirection.x, 0, camDirection.z));
	glm::vec3 force = speed * inputDirection;

	desiredVelocity = 
		planeDirection * force.x + // forward movement
		glm::cross(planeDirection,glm::vec3(0,1,0)) * force.y + // up movement
		glm::vec3(0,1,0) * force.z; // side movement
    glm::vec3 diff = desiredVelocity - velocity;
    velocity += diff * accelerationRate * dt;

	ufo->setLinearVelocity(velocity);
	
	// If the UFO is outside a 4 unit extension of the chunk...
	glm::vec2 pointA = world.getPlayerChunkCoordinates() * 16 - glm::ivec2(CHUNK_WIDTH / 4); // TODO: if we do chunk scale this will need to be updated
	glm::vec2 pointB = pointA + glm::vec2(CHUNK_WIDTH / 4) + glm::vec2(CHUNK_WIDTH);
	if(glm::vec3 position = ufo->getPosition(); !(position.x > pointA.x && position.z > pointA.y && position.x < pointB.x && position.z < pointB.y) ){
		// Step the world in whichever direction the ufo moved
		if(position.x > pointB.x)
			world.stepPlayerPosX();
		else if(position.x < pointA.x)
			world.stepPlayerNegX();
		else if(position.z > pointB.y)
			world.stepPlayerPosZ();
		else if(position.z < pointA.y)
			world.stepPlayerNegZ();
	}

    // Tilt the ufo relative to velocity
	glm::vec3 noYVel = glm::vec3(velocity.x, 0, velocity.z);
	ufo->setRotation(glm::quat(glm::vec3(0,1,0), glm::normalize(glm::vec3(0,speed,0) + noYVel)), true);

	// Update the ufo light direction
	ufoLight->setDirection(ufo->down());
}

void Application::repositionNPCs(float dt) {
	float proximity = 50;
	//float angle = (float) (rand() % 360);
	//float probOfRepos = 0.2;
	float innerRadiusPercentage = .75;
	glm::vec3 direction = glm::normalize(velocity);

	// Check a NPC position in relationship to the moving UFO
	if (glm::distance(npcs[npci]->getPosition(), glm::vec3(ufo->getPosition().x, npcs[npci]->getPosition().y, ufo->getPosition().z)) > proximity) {
		// Get the angle relative to UFO movement and within some random range
		int angleTolerance = 90;
		float angle = std::atan2(direction.x, direction.z) + glm::radians((float) (rand() % angleTolerance) - (angleTolerance / 2));
		//std::cout << glm::degrees(angle) << " " << direction.x << " " << direction.z << std::endl;
		glm::vec3 newPos = ufo->getPosition() + (glm::vec3(glm::sin(angle), 0, glm::cos(angle)) * proximity * innerRadiusPercentage);
		float y = world.getWorldHeight((glm::ivec3) newPos + glm::ivec3(0,10,0));
		if (!isnan(y)) {
			newPos.y = y;
			//std::cout << glm::distance(newPos, glm::vec3(ufo->getPosition().x, npcs[npci]->getPosition().y, ufo->getPosition().z)) << std::endl;
			npcs[npci]->setPosition(newPos);
			npcs[npci]->setLinearVelocity(glm::vec3());
			npcs[npci]->clearTargets();
		}
	}
	// Increment a once per frame index for npcs
	npci ++;
	if (npci >= npcs.size()) {
		npci = 0;
	}
}

void Application::update(float dt) {
	// Update the physics world
	world.update(dt);

	// Move the UFO with input
	controlUFO(dt);

	// Only modify npcs when UFO is moving
	float realSpeed = glm::length(velocity);
	if (realSpeed > 0.2f) {
		// Move NPC's to new locations
		repositionNPCs(dt);
	}

	// Attempt to abduct something
	float abductionDistance = 20;
	if (abducting) {
		// Find a new abduction target
		ufoLight->setCutoffAngle(75);
		if (abductionTarget == nullptr) {
			std::shared_ptr<NPC> closest = nullptr;
			float bestDistance = 999999;
			for (std::shared_ptr<NPC> npc : npcs) {
				float distance = glm::distance(npc->getPosition(), ufo->getPosition());
				if (distance < abductionDistance && distance < bestDistance) {
					bestDistance = distance;
					closest = npc;
				}
			}
			abductionTarget = closest;
			if (abductionTarget != nullptr) {
				// if new target found

				abductionTarget->getRigidBody().setGravity({0, 0, 0});
				abductionTarget->setMovementState(false);
				abductionTarget->setAngularVelocity(glm::vec3((rand() % 2) - 1, (rand() % 2) - 1, (rand() % 2) - 1));
			}
		} else {
			if (glm::distance(abductionTarget->getPosition(), ufo->getPosition()) < abductionDistance) {
				glm::vec3 direction = glm::normalize(ufo->getPosition() - abductionTarget->getPosition());
				abductionTarget->setLinearVelocity(direction * 100.0f * dt);
			} else {
				// We moved away and need to drop the target
				abductionTarget->getRigidBody().setGravity({0, -9.81, 0});
				abductionTarget->setMovementState(true);
				abductionTarget = nullptr;
			}
		}
	} else {
		// Release target
		if (abductionTarget != nullptr) {
			abductionTarget->getRigidBody().setGravity({0, -9.81, 0});
			abductionTarget->setMovementState(true);
			abductionTarget = nullptr;
		}
		ufoLight->setCutoffAngle(0);
	}
}

void Application::render(Shader* boundShader){
	// Set the player's position
	glUniform3fv(boundShader->getUniformLocation("playerPosition"), 1, glm::value_ptr(ufo->getPosition()));
	// Set the radius of the world
	glUniform1f(boundShader->getUniformLocation("worldRadius"), (WORLD_RADIUS - 1) * 16);

	world.render(boundShader);
}

void Application::drawGUI(){

}

void Application::keyboard(const SDL_KeyboardEvent& e) {


}

void Application::mouseButton(const SDL_MouseButtonEvent& e) {

}

void Application::mouseMotion(const SDL_MouseMotionEvent& e) {

}

void Application::mouseWheel(const SDL_MouseWheelEvent& e) {

}

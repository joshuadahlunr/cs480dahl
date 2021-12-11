#include "application.h"
#include "light.h"
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

	auto ambient = std::make_shared<AmbientLight>();
	getSceneRoot()->addChild(ambient);
	ambient->setAmbient({.5, .5, .5, 1});

	auto light = std::make_shared<DirectionalLight>();
	getSceneRoot()->addChild(light);
	light->setDirection({-.5, -1, -.3});
	light->setDiffuse({1, 1, 1, 1});

	world.initialize();

	// Create the NPCs
	int numNPCs = 10;
	for (int i = 0; i < numNPCs; i++) {
		std::shared_ptr<NPC> npc = std::make_shared<NPC>();
		getSceneRoot()->addChild(npc);
		std::string modelFile = "cow.obj";
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

	// Hookup the input events
	Engine::keyboardEvent += [&](auto event) { keyboard(event);};
	Engine::mouseButtonEvent += [&](auto event) { mouseButton(event); };
	Engine::mouseMotionEvent += [&](auto event) { mouseMotion(event); };
	Engine::mouseWheelEvent += [&](auto event) { mouseWheel(event); };

	return ret;
}

void Application::update(float dt) {
	world.update(dt);

	// Capture input
	const Uint8* keystate = SDL_GetKeyboardState(NULL);
	inputDirection = glm::vec3();
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

	// Create NPCs at a radius around the UFO if not enough NPCs in proximity
	float proximity = 50;
	//float angle = (float) (rand() % 360);
	//float probOfRepos = 0.2;
	float innerRadiusPercentage = .75;
	float realSpeed = glm::length(velocity);
	glm::vec3 direction = glm::normalize(velocity);
	
	// Only modify things when UFO is moving
	if (realSpeed > 0.2f) {
		// Check a NPC position in relationship to the moving UFO
		if (glm::distance(npcs[npci]->getPosition(), glm::vec3(ufo->getPosition().x, npcs[npci]->getPosition().y, ufo->getPosition().z)) > proximity) {
			int angleTolerance = 90;
			float angle = std::atan2(direction.x, direction.z) + glm::radians((float) (rand() % angleTolerance) - (angleTolerance / 2));
			//std::cout << glm::degrees(angle) << " " << direction.x << " " << direction.z << std::endl;
			glm::vec3 newPos = ufo->getPosition() + (glm::vec3(glm::sin(angle), 0, glm::cos(angle)) * proximity * innerRadiusPercentage);
			float y = world.getWorldHeight((glm::ivec3) newPos);
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

	//ufo->getPosition() + glm::vec3(glm::sin(glm::radians(angle)) * proximity * innerRadiusPercentage, 0, glm::cos(glm::radians(angle)) * proximity * innerRadiusPercentage)


	
	// Print world height under ufo
	//std::cout << "Height: " << world.getWorldHeight((glm::ivec3) ufo->getPosition()) << std::endl;
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

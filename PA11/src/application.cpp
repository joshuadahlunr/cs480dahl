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

	auto ambient = std::make_shared<AmbientLight>();
	getSceneRoot()->addChild(ambient);
	ambient->setAmbient({.5, .5, .5, 1});

	auto point = std::make_shared<PointLight>();
	getSceneRoot()->addChild(point);
	point->setPosition({0, 15, 0});
	point->setDiffuse({1, 1, 1, 1});

	world.initialize();

	ufo = std::make_shared<Object>();
	getSceneRoot()->addChild(ufo);
	ufo->setPosition({8, 0, 8});
	ufo->initializeGraphics(args, "ufo.obj");
	Engine::getGraphics()->getCamera()->setFocus(ufo);
	ufo->initializePhysics(args, Engine::getPhysics(), CollisionGroups::UFO, /*mass*/ 100);
	ufo->createMeshCollider(args, Engine::getPhysics(), CONVEX_MESH, "ufo.obj");
	ufo->makeDynamic();
	ufo->getRigidBody().setGravity({0, 0, 0}); // Disable gravity on the UFO
	// ufo->getRigidBody().setActivationState(DISABLE_DEACTIVATION);

	std::shared_ptr<NPC> npc = std::make_shared<NPC>();
	getSceneRoot()->addChild(npc);
	npc->setPosition({0, 0, 0});
	npc->initializeGraphics(args, "cube.obj");

	npcs.push_back(npc);

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
	float speed = 20;
	glm::vec3 camDirection = Engine::getGraphics()->getCamera()->getLookDirection();
	glm::vec3 direction = glm::normalize(glm::vec3(camDirection.x, 0, camDirection.z));
	glm::vec3 force = speed * inputDirection;

	desiredVelocity = 
		direction * force.x + // forward movement
		glm::cross(direction,glm::vec3(0,1,0)) * force.y + // up movement
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
	float proximity = 100;
	if (npcs.size() < proximity) {
		std::shared_ptr<NPC> npc = std::make_shared<NPC>();
		getSceneRoot()->addChild(npc);
		// Set position to somewhere in a circle around the UFO
		float angle = (float) (rand() % 360);
		float innerRadiusPercentage = 0.49;
		npc->setPosition(ufo->getPosition() + glm::vec3(glm::sin(glm::radians(angle)) * proximity * innerRadiusPercentage, 0, glm::cos(glm::radians(angle)) * proximity * innerRadiusPercentage));
		npc->initializeGraphics(args, "cube.obj");

		npcs.push_back(npc);
	}

	// Destroy NPCs if they go out of range from UFO
	for (int i = 0; i < npcs.size(); i++) {
		if (glm::distance(npcs[i]->getPosition(), ufo->getPosition()) > proximity) {
			npcs[i].get()->Object::~Object();
			npcs.erase(npcs.begin() + i);
		}
	}
	
	// Print world height under ufo
	std::cout << "Height: " << world.getWorldHeight((glm::ivec3) ufo->getPosition()) << std::endl;
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

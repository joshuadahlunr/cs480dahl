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
	ufo->setPosition({0, 0, 0});
	ufo->initializeGraphics(args, "ufo.obj");
	Engine::getGraphics()->getCamera()->setFocus(ufo);

	npc = std::make_shared<NPC>();
	getSceneRoot()->addChild(npc);
	npc->setPosition({0, 0, 0});
	npc->initializeGraphics(args, "cube.obj");



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

    ufo->setPosition(ufo->getPosition() + (velocity * dt));

	glm::vec2 pointA = world.getPlayerChunk() * 16 - glm::ivec2(CHUNK_X_SIZE / 4); // TODO: if we do chunk scale this will need to be updated
	glm::vec2 pointB = pointA + glm::vec2(CHUNK_X_SIZE / 4) + glm::vec2(CHUNK_X_SIZE);

	// If the UFO is outside a 4 unit extension of the chunk...
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
	ufo->setRotation(glm::quat(glm::vec3(0,1,0), glm::normalize(glm::vec3(0,speed,0) + velocity)), true);
}

void Application::render(Shader* boundShader){
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

#include "application.h"
#include "light.h"
#include "window.h"
#include "graphics.h"
#include "sound.h"
#include "physics.h"
#include "camera.h"
#include "threadTimer.h"
#include "shader.h"

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
	ufo->initializeGraphics(args, "ufo.obj");
	Engine::getGraphics()->getCamera()->setFocus(ufo);

	// Hookup the input events
	Engine::keyboardEvent += [&](auto event) { keyboard(event);};
	Engine::mouseButtonEvent += [&](auto event) { mouseButton(event); };
	Engine::mouseMotionEvent += [&](auto event) { mouseMotion(event); };
	Engine::mouseWheelEvent += [&](auto event) { mouseWheel(event); };

	return ret;
}

float timer = 0;
void Application::update(float dt) {
	world.update(dt);

	timer += dt;
	if(timer > 5){
		timer = 0;
		world.stepPlayerNegZ();
	}

	// UFO Control
	glm::vec3 direction = Engine::getGraphics()->getCamera()->getLookDirection();
	float forwardforce = 5 * inputDirection.x;
	float sideForce = 5 * inputDirection.y;

	desiredVelocity = forwardforce*direction + sideForce*glm::cross(direction,glm::vec3(0,1,0));
    glm::vec3 diff = desiredVelocity - velocity;
    velocity += diff * accelerationRate * dt;

    ufo->setPosition(ufo->getPosition() + (velocity * dt));

    // Tilt if diff in velocity large
    //setRotation(glm::quat(diff));
}

void Application::render(Shader* boundShader){
	world.render(boundShader);
}

void Application::drawGUI(){

}

void Application::keyboard(const SDL_KeyboardEvent& e) {

	inputDirection = glm::vec2();
	if (e.type == SDL_KEYDOWN) {
		if (e.keysym.sym == SDLK_w)
			inputDirection += glm::vec2(1,0);
		if (e.keysym.sym == SDLK_s)
			inputDirection += glm::vec2(-1,0);
		if (e.keysym.sym == SDLK_a)
			inputDirection += glm::vec2(0,-1);
		if (e.keysym.sym == SDLK_d)
			inputDirection += glm::vec2(0,1);
	}
}

void Application::mouseButton(const SDL_MouseButtonEvent& e) {

}

void Application::mouseMotion(const SDL_MouseMotionEvent& e) {

}

void Application::mouseWheel(const SDL_MouseWheelEvent& e) {

}

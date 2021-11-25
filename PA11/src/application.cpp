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
}

void Application::render(Shader* boundShader){
	world.render(boundShader);
}

void Application::drawGUI(){

}

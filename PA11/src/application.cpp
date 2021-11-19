#include "application.h"
#include "light.h"
#include "window.h"
#include "graphics.h"
#include "sound.h"
#include "physics.h"
#include "camera.h"
#include "threadTimer.h"

#include "chunk.h"


bool Application::initialize(const Arguments& args) {
	bool ret = Engine::initialize(args);

	auto ambient = std::make_shared<AmbientLight>();
	getSceneRoot()->addChild(ambient);
	ambient->setAmbient({.5, .5, .5, 1});

	auto point = std::make_shared<PointLight>();
	getSceneRoot()->addChild(point);
	point->setPosition({0, 15, 0});
	point->setDiffuse({1, 1, 1, 1});

	auto chunk = std::make_shared<Chunk>();
	getSceneRoot()->addChild(chunk);
	chunk->generateVoxels(0, 0);
	chunk->rebuildMesh(args);
	chunk->loadTextureFile(args, args.getResourcePath() + "textures/invalid.png");

	return ret;
}

void Application::update(float dt) {

}

void Application::drawGUI(){

}

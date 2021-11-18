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
	ambient->setAmbient({1, 1, 1, 1});

	auto directional = std::make_shared<DirectionalLight>();
	getSceneRoot()->addChild(directional);
	directional->setDirection({0, -3, -1});

	auto chunk = std::make_shared<Chunk>();
	getSceneRoot()->addChild(chunk);
	chunk->generateVoxels(0, 0);
	chunk->rebuildMesh(args);
	chunk->loadTextureFile(args, args.getResourcePath() + "textures/invalid.png");
	chunk->finalizeModel();

	return ret;
}

void Application::update(float dt) {

}

void Application::drawGUI(){

}
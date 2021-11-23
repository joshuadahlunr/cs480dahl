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


	for(int z = -WORLD_RADIUS; z <= WORLD_RADIUS; z++){
		auto chunks = world.generateChunksZ(args, 0, z);
		for(int x = 0; x < chunks.size() - 1; x++){
			auto& chunk = chunks[x];
			chunk->setPosition({16 * (x - WORLD_RADIUS), 0, 16 * z});
			meshingQueue.push(chunk);
		}

		world.AddPosZ(chunks);
	}

	return ret;
}

int x = WORLD_RADIUS + 1, z = WORLD_RADIUS;
float timer = 0;

void Application::update(float dt) {
	world.update(dt);

	timer += dt;
	if(timer > 5){
		timer = 0;

		// TODO: why is it derpy at the start?
		auto chunks = world.generateChunksX(args, x, -WORLD_RADIUS);
		for(int z = 0; z < chunks.size() - 1; z++){
			auto& chunk = chunks[z];
			chunk->setPosition({16 * x, 0, 16 * (z - WORLD_RADIUS)});
			meshingQueue.push(chunk);
		}

		world.AddPosX(chunks);
		x++;
	}

	// If there are chunks which need meshes generated for them... generate a mesh for those chunks
	if(!meshingQueue.empty()){
		auto nextMesh = meshingQueue.front();
		meshingQueue.pop();
		nextMesh->rebuildMesh(args);
		nextMesh->loadTextureFile(args, args.getResourcePath() + "textures/invalid.png");
	}
}

void Application::render(Shader* boundShader){
	world.render(boundShader);
}

void Application::drawGUI(){

}

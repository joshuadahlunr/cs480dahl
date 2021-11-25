#ifndef APPLICATION_H
#define APPLICATION_H

#include "engine.h"
#include "leaderboard.h"
#include "voxel_world.h"

// Class which provides engine related internals
class Application: public Engine {
public:
	Application(std::string name, int width, int height): Engine(name, width, height), world(args) {}
	Application(std::string name): Engine(name), world(args) {}

	bool initialize(const Arguments& args);
	void update(float dt) override;
	void render(Shader* boundShader) override;
	void drawGUI();

protected:
	Arguments args;

	VoxelWorld world;
};

#endif // APPLICATION_H

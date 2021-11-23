#ifndef APPLICATION_H
#define APPLICATION_H

#include "engine.h"
#include "leaderboard.h"
#include "voxel_world.h"

#include <queue>

// Class which provides engine related internals
class Application: public Engine {
public:
	using Engine::Engine;

	bool initialize(const Arguments& args);
	void update(float dt) override;
	void render(Shader* boundShader) override;
	void drawGUI();

protected:
	Arguments args;

	VoxelWorld world;

	std::queue<std::shared_ptr<Chunk>> meshingQueue;
};

#endif // APPLICATION_H

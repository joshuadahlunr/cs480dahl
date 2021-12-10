#ifndef APPLICATION_H
#define APPLICATION_H

#include "engine.h"
#include "leaderboard.h"
#include "voxel_world.h"

#include <vector>

class NPC;

// Class which provides engine related internals
class Application: public Engine {
public:
	Application(std::string name, int width, int height): Engine(name, width, height), world(args) {}
	Application(std::string name): Engine(name), world(args) {}

	bool initialize(const Arguments& args);
	void update(float dt) override;
	void render(Shader* boundShader) override;
	void drawGUI();

	virtual void keyboard(const SDL_KeyboardEvent& e);
	void mouseButton(const SDL_MouseButtonEvent& e);
	void mouseMotion(const SDL_MouseMotionEvent& e);
	void mouseWheel(const SDL_MouseWheelEvent& e);

	Object::ptr ufo;
	std::vector<std::shared_ptr<NPC>> npcs;

	Arguments args;

	VoxelWorld world;
private:
	glm::vec3 inputDirection;
	glm::vec3 desiredVelocity;
	glm::vec3 velocity;
	float accelerationRate = 0.5;
	int npci = 0; // npc index
};

#endif // APPLICATION_H

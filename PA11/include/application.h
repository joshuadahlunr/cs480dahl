#ifndef APPLICATION_H
#define APPLICATION_H

#include "engine.h"
#include "leaderboard.h"
#include "voxel_world.h"
#include "light.h"

#include <vector>

class NPC;

// Class which provides engine related internals
class Application: public Engine {
public:
	Application(std::string name, int width, int height): Engine(name, width, height), world(std::make_shared<VoxelWorld>(args)) {}
	Application(std::string name): Engine(name), world(std::make_shared<VoxelWorld>(args)) {}

	~Application() {leaderboard.save(); };

	bool initialize(const Arguments& args);
	void update(float dt) override;
	void render(Shader* boundShader) override;
	void drawGUI();
	void reset();

	void createNPC(std::string type);
	void controlUFO(float dt);
	void repositionNPC(std::shared_ptr<NPC> npc, bool checkDistance);
	int getScore() {return points; };
	float getTimeRemaining() {return timeRemaining;};

	virtual void keyboard(const SDL_KeyboardEvent& e);
	void mouseButton(const SDL_MouseButtonEvent& e);
	void mouseMotion(const SDL_MouseMotionEvent& e);
	void mouseWheel(const SDL_MouseWheelEvent& e);

	Object::ptr ufo;
	std::vector<std::shared_ptr<NPC>> npcs;

	std::shared_ptr<VoxelWorld> getWorld() const { return world; }

	Arguments args;

	int points = 0;
	bool gameOver = false;
	Leaderboard leaderboard;

private:
	std::shared_ptr<VoxelWorld> world;

	glm::vec3 inputDirection;
	glm::vec3 desiredVelocity;
	glm::vec3 velocity;

	bool abducting = false;
	float timeRemaining = 0;

	float accelerationRate = 0.5;
	int npci = 0; // npc index

	std::shared_ptr<SpotLight> ufoLight;
};

#endif // APPLICATION_H

#ifndef APPLICATION_H
#define APPLICATION_H

#include "engine.h"

// Class which provides engine related internals
class Application: public Engine {
public:
    using Engine::Engine;

	bool Initialize(const Arguments& args);
	void Update(float dt) override;

   
	std::vector<Light*> lights;

private:
	void KeyboardCallback(const SDL_KeyboardEvent& event);
	void resetBall();

	Object *leftPaddle, *rightPaddle;
	Object *ball;
    Object *plunger;
    Object *bottomWallCenter;
	float leftPaddleAngle = 0, rightPaddleAngle = 0;
	float paddleSpeed = 720;

	bool leftPaddleMoving, rightPaddleMoving;

	float ballLaunchPower = 0;
	float powerIncreaseSpeed = 900;
	bool hasLaunched = false;

	// Game logic stuff
	int score = 0;
	int ballsRemaining = 3;
	std::string playerName;

	// Leaderboard
	int* scores; // list of scores
	std::string* players; // list of players 
};

#endif // APPLICATION_H

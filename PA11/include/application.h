#ifndef APPLICATION_H
#define APPLICATION_H

#include "engine.h"
#include "leaderboard.h"

//class Leaderboard;

// Class which provides engine related internals
class Application: public Engine {
public:
	enum GameState : unsigned int {
		Welcome = 0,
		Playing = 1,
		GameOver = 2
	};

	using Engine::Engine;
	~Application();

	bool initialize(const Arguments& args);
	void update(float dt) override;
	void reset();

	int getScore() {return score;}

	Leaderboard* leaderboard;
	std::vector<std::shared_ptr<Light>> lights;
	GameState gameState = GameState::Playing;
	std::shared_ptr<Object> ball;

private:
	void keyboardCallback(const SDL_KeyboardEvent& event);
	void resetBall();

	std::shared_ptr<Object> leftPaddle, rightPaddle;
	std::shared_ptr<Object> plunger;
	std::shared_ptr<Object> bottomWallCenter;
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

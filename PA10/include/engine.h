#ifndef ENGINE_H
#define ENGINE_H

#include <sys/time.h>
#include <assert.h>
#include <SDL2/SDL.h>

#include "arguments.h"
#include "nytl/callback.hpp"

// Forward declarations
class Window;
class Graphics;
class Physics;
class Object;
class Celestial;

// Class which provides engine related internals
class Engine {
public:
	Engine(std::string name, int width, int height);
	Engine(std::string name);
	~Engine();
	bool Initialize(const Arguments& args);
	void Run();

	// Recursively initializes a scene tree from the provided json data
	Celestial* CelestialFromJson(const Arguments& args, json j, uint depth = 0);

	// Time functions
	unsigned int getDT();
	long long GetCurrentTimeMillis();

	// Keyboard and Mouse callbacks
	nytl::Callback<void(const SDL_KeyboardEvent&)> keyboardEvent;
	nytl::Callback<void(const SDL_MouseMotionEvent&)> mouseMotionEvent;
	nytl::Callback<void(const SDL_MouseButtonEvent&)> mouseButtonEvent;
	nytl::Callback<void(const SDL_MouseWheelEvent&)> mouseWheelEvent;

	Window* getWindow() const { return m_window; }
	Graphics* getGraphics() const { return m_graphics; }
	Physics* getPhysics() const { return m_physics; }

private:
	// Window related variables
	Window* m_window;
	std::string m_WINDOW_NAME;
	int m_WINDOW_WIDTH;
	int m_WINDOW_HEIGHT;
	bool m_FULLSCREEN;
	SDL_Event m_event;

	Graphics* m_graphics;
	Physics* m_physics;
	unsigned int m_DT;
	long long m_currentTimeMillis;
	bool m_running;

	Object* sceneRoot;
	Object *leftPaddle, *rightPaddle;
	float leftPaddleAngle = 0, rightPaddleAngle = 0;
	float paddleSpeed = 720;

	// Game logic stuff
	static int score;
	int ballsRemaining = 3;
	std::string playerName;

	// Leaderboard
	int* scores; // list of scores
	std::string* players; // list of players 
	
};

#endif // ENGINE_H

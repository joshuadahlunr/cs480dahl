#ifndef ENGINE_H
#define ENGINE_H

#include <chrono>
#include <sys/time.h>
#include <assert.h>
#include <SDL2/SDL.h>

#include "arguments.h"
#include "nytl/callback.hpp"
#include "circular_buffer.hpp"

// Forward declarations
class Window;
class Graphics;
class Physics;
class Object;
class Celestial;
class Light;
class Sound;
class Shader;

// Class which provides engine related internals
class Engine {
public:
	Engine(std::string name, int width, int height);
	Engine(std::string name);
	~Engine();
	virtual bool initialize(const Arguments& args);
	virtual void run();
	virtual void update(float dt) {}
	virtual void render(Shader* boundShader) {}

	// Time functions
	float getDT();
	float getAverageFPS();

	// Keyboard and Mouse callbacks
	nytl::Callback<void(const SDL_KeyboardEvent&)> keyboardEvent;
	nytl::Callback<void(const SDL_MouseMotionEvent&)> mouseMotionEvent;
	nytl::Callback<void(const SDL_MouseButtonEvent&)> mouseButtonEvent;
	nytl::Callback<void(const SDL_MouseWheelEvent&)> mouseWheelEvent;

	Window* getWindow() const { return window; }
	Graphics* getGraphics() const { return graphics; }
	Physics* getPhysics() const { return physics; }
	Sound* getSound() const { return sound; }
	std::shared_ptr<Object> getSceneRoot() const { return sceneRoot; }

private:
	// Window related variables
	Window* window;
	std::string WINDOW_NAME;
	int WINDOW_WIDTH;
	int WINDOW_HEIGHT;
	bool FULLSCREEN;
	SDL_Event event;

	Graphics* graphics;
	Physics* physics;
	Sound* sound;

	float DT;
	float physicsAccumulator = 0;
	std::chrono::high_resolution_clock::time_point frameStartTime;
	circular_buffer<float, std::array<float, 60>> fpsMeasurements;
	bool running;

	std::shared_ptr<Object> sceneRoot;
};

#endif // ENGINE_H

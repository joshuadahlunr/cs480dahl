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

// Class which provides engine related internals
class Engine {
public:
	Engine(std::string name, int width, int height);
	Engine(std::string name);
	~Engine();
	virtual bool Initialize(const Arguments& args);
	virtual void Run();
	virtual void Update(float dt) {}

	// Time functions
	float getDT();
	float getAverageFPS();

	// Keyboard and Mouse callbacks
	nytl::Callback<void(const SDL_KeyboardEvent&)> keyboardEvent;
	nytl::Callback<void(const SDL_MouseMotionEvent&)> mouseMotionEvent;
	nytl::Callback<void(const SDL_MouseButtonEvent&)> mouseButtonEvent;
	nytl::Callback<void(const SDL_MouseWheelEvent&)> mouseWheelEvent;

	Window* getWindow() const { return m_window; }
	Graphics* getGraphics() const { return m_graphics; }
	Physics* getPhysics() const { return m_physics; }
	Sound* getSound() const { return m_sound; }
	Object* getSceneRoot() const { return sceneRoot; }

private:
	// Window related variables
	Window* m_window;
	std::string m_WINDOW_NAME;
	int m_WINDOW_WIDTH;
	int m_WINDOW_HEIGHT;
	bool m_FULLSCREEN;
	SDL_Event m_event;
	circular_buffer<float> fpsMeasurements{60};

	Graphics* m_graphics;
	Physics* m_physics;
	Sound* m_sound;
	float m_DT;
	float physicsAccumulator = 0;
	std::chrono::high_resolution_clock::time_point frameStartTime;
	bool m_running;

	Object* sceneRoot;
};

#endif // ENGINE_H

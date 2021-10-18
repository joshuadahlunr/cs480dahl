#ifndef ENGINE_H
#define ENGINE_H

#include <sys/time.h>
#include <assert.h>

#include "window.h"
#include "graphics.h"
#include "arguments.h"
#include "nytl/callback.hpp"

// Class which provides engine related internals
class Engine {
public:
	Engine(string name, int width, int height);
	Engine(string name);
	~Engine();
	bool Initialize(const Arguments& args);
	void Run();

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

private:
	// Window related variables
	Window *m_window;
	string m_WINDOW_NAME;
	int m_WINDOW_WIDTH;
	int m_WINDOW_HEIGHT;
	bool m_FULLSCREEN;
	SDL_Event m_event;

	Graphics *m_graphics;
	unsigned int m_DT;
	long long m_currentTimeMillis;
	bool m_running;
};

#endif // ENGINE_H

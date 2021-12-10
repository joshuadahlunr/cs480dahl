#ifndef WINDOW_H
#define WINDOW_H

#include <SDL2/SDL.h>
#include <string>

#include <glm/glm.hpp>


// Uncomment to enable VSYNC
// #define USE_VSYNC

// Class holding the window and OpenGL Context
class Window {
public:
	Window();
	~Window();
	bool initialize(const std::string &name, int* width, int* height);
	void swap();

	SDL_Window* getWindow() const { return gWindow; }
	SDL_GLContext getContext() const { return gContext; }
	glm::ivec2 getDimensions() const { return dimensions; }

private:
	SDL_Window* gWindow;
	SDL_GLContext gContext;

	glm::ivec2 dimensions;
};

#endif /* WINDOW_H */

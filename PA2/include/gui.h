#ifndef GUI_H
#define GUI_H

#include <iostream>
#include <string>

#include "graphics_headers.h"
#include "imgui.h"

class Window;
typedef union SDL_Event SDL_Event;

class GUI {
public:
	struct ShouldProcessEvents {
		 bool keyboard, mouse;
	};

public:
	GUI();
	~GUI();
	bool Initialize(Window* window, const char* glsl_version = "#version 330");
	ShouldProcessEvents ProcessEvent(SDL_Event& event);
	void Update(unsigned int dt);
	void Render();

private:
	ImGuiIO& io;
};

#endif /* GUI_H */

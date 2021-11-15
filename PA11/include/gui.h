#ifndef GUI_H
#define GUI_H

#include <iostream>
#include <string>

#include "graphics_headers.h"
#include "imgui.h"

// Forward declarations
class Application;
class Engine;
class Graphics;
typedef union SDL_Event SDL_Event;

// Provides GUI for the program
class GUI {
public:
	// Struct definging if the GUI is actively handling keyboard or mouse events which should then be ignored by the rest of the program
	struct ShouldProcessEvents {
		 bool keyboard, mouse;
	};

public:
	GUI();
	~GUI();
	bool Initialize(Engine* engine, const char* glsl_version = "#version 330");
	void Update(float dt);
	void Render();

	// bool setupFramebuffer();

	// Function which processes mouse and keyboard events, returning if they should continue to be propigated
	ShouldProcessEvents ProcessEvent(SDL_Event& event);
private:
	ImGuiIO& io;

	Application* app;
	Graphics* graphics;

	GLuint framebuffer = 0;
	GLuint renderedTexture = 0;
};

#endif /* GUI_H */

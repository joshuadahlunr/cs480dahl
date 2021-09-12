#include "gui.h"

#include <SDL2/SDL.h>
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include "engine.h"
#include "planet.h"
#include "moon.h"

// Initialize the ImGUI IO instance
GUI::GUI() : io( []() -> ImGuiIO& {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	return ImGui::GetIO();
}() ){ }

GUI::~GUI(){
	ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

bool GUI::Initialize(Engine* engine, const char* glsl_version/* = "#version 330"*/){
	// Setup Dear ImGui style
    ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
    if( !ImGui_ImplSDL2_InitForOpenGL(engine->getWindow()->getWindow(), engine->getWindow()->getContext()) )
		return false;
    if( !ImGui_ImplOpenGL3_Init(glsl_version) )
		return false;

	// Save the pointer to the graphics
	this->graphics = engine->getGraphics();

	return true;
}

GUI::ShouldProcessEvents GUI::ProcessEvent(SDL_Event& event){
	ImGui_ImplSDL2_ProcessEvent(&event);

	return { !io.WantCaptureKeyboard, !io.WantCaptureMouse };
}

void GUI::Render(){
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();


	// UI Generation
	if (ImGui::BeginMainMenuBar()) {
    	if (ImGui::BeginMenu("Cube")) {
			if (ImGui::MenuItem((graphics->getSelected<Planet>()->orbitIsPaused() ? "Resume Orbit" : "Pause Orbit"), "O"))
				graphics->getSelected<Planet>()->toggleOrbit();
			if (ImGui::MenuItem((graphics->getSelected<Planet>()->rotationIsPaused() ? "Resume Rotation" : "Pause Rotation"), "R"))
				graphics->getSelected<Planet>()->toggleRotation();

			ImGui::Separator();

			if (ImGui::MenuItem("Reverse Orbit", "L"))
				graphics->getSelected<Planet>()->reverseOrbit();
			if (ImGui::MenuItem("Reverse Rotation", "F"))
				graphics->getSelected<Planet>()->reverseRotation();
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Moon")) {
			Moon* moon = reinterpret_cast<Moon*>( graphics->getSelected<Planet>()->getChildren()[0] );

			// Moon scale
			float scale = moon->getScale();
			if(ImGui::SliderFloat("Scale", &scale, 0.1f, 5.0f))
				moon->setScale(scale);

			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}


	// Render the UI results
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

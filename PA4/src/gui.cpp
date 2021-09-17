#include "gui.h"

#include <SDL2/SDL.h>
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include "engine.h"
#include "showcase.h"

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

	// bool open = true;
	// ImGui::ShowDemoWindow(&open);

	// UI Generation
	if (ImGui::BeginMainMenuBar()) {
    	if (ImGui::BeginMenu("Showcase")) {
			// Get Reference to the showcase
			Showcase* showcase = graphics->getSelected<Showcase>();

			// Pause options
			if (ImGui::MenuItem((showcase->orbitIsPaused() ? "Resume Orbit" : "Pause Orbit"), "O"))
				showcase->toggleOrbit();
			if (ImGui::MenuItem((showcase->rotationIsPaused() ? "Resume Rotation" : "Pause Rotation"), "R"))
				showcase->toggleRotation();

			ImGui::Separator();

			// Reverse options
			if (ImGui::MenuItem("Reverse Orbit", "L"))
				showcase->reverseOrbit();
			if (ImGui::MenuItem("Reverse Rotation", "F"))
				showcase->reverseRotation();

			ImGui::Separator();

			// Speed options
			float speed = (showcase->orbitIsPaused() ? 0 : showcase->getOrbitSpeed());
			if(ImGui::SliderFloat((showcase->orbitIsReversed() ? "Orbit Reversed Speed" : "Orbit Speed"), &speed, 0.1f, 5.0f))
				if(!showcase->orbitIsPaused())
					showcase->setOrbitSpeed(speed);
			speed = (showcase->rotationIsPaused() ? 0 : showcase->getRotationSpeed());
			if(ImGui::SliderFloat((showcase->rotationIsReversed() ? "Rotation Reversed Speed" : "Rotation Speed"), &speed, 0.1f, 5.0f))
				if(!showcase->rotationIsPaused())
					showcase->setRotationSpeed(speed);

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}


	// Render the UI results
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

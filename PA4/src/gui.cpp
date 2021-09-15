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

	// bool open = true;
	// ImGui::ShowDemoWindow(&open);

	// UI Generation
	if (ImGui::BeginMainMenuBar()) {
    	if (ImGui::BeginMenu("Planet")) {
			// Get Reference to the planet
			Planet* planet = graphics->getSelected<Planet>();

			// Pause options
			if (ImGui::MenuItem((planet->orbitIsPaused() ? "Resume Orbit" : "Pause Orbit"), "O"))
				planet->toggleOrbit();
			if (ImGui::MenuItem((planet->rotationIsPaused() ? "Resume Rotation" : "Pause Rotation"), "R"))
				planet->toggleRotation();

			ImGui::Separator();

			// Reverse options
			if (ImGui::MenuItem("Reverse Orbit", "L"))
				planet->reverseOrbit();
			if (ImGui::MenuItem("Reverse Rotation", "F"))
				planet->reverseRotation();

			ImGui::Separator();

			// Speed options
			float speed = (planet->orbitIsPaused() ? 0 : planet->getOrbitSpeed());
			if(ImGui::SliderFloat((planet->orbitIsReversed() ? "Orbit Reversed Speed" : "Orbit Speed"), &speed, 0.1f, 5.0f))
				if(!planet->orbitIsPaused())
					planet->setOrbitSpeed(speed);
			speed = (planet->rotationIsPaused() ? 0 : planet->getRotationSpeed());
			if(ImGui::SliderFloat((planet->rotationIsReversed() ? "Rotation Reversed Speed" : "Rotation Speed"), &speed, 0.1f, 5.0f))
				if(!planet->rotationIsPaused())
					planet->setRotationSpeed(speed);

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

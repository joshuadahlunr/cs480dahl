#include "gui.h"

#include <SDL2/SDL.h>
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include "window.h"

// Initalize the ImGUI IO instance
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

bool GUI::Initialize(Window* window, const char* glsl_version/* = "#version 330"*/){
	// Setup Dear ImGui style
    ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
    if( !ImGui_ImplSDL2_InitForOpenGL(window->getWindow(), window->getContext()) )
		return false;
    if( !ImGui_ImplOpenGL3_Init(glsl_version) )
		return false;

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
	bool trash;
	ImGui::ShowDemoWindow(&trash);


	// Render the UI results
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

#include "gui.h"

#include <SDL2/SDL.h>
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include "engine.h"
#include "window.h"
#include "celestial.h"

// Provide a backing for the globalTimeScale global variable
float globalTimeScale = 1;
// Provide a backing for the globalShouldScale global variable;
bool globalShouldScale = true;

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
	// Pass events along to ImGUI
	ImGui_ImplSDL2_ProcessEvent(&event);

	// Return if ImGUI is hogging our events
	return { !io.WantCaptureKeyboard, !io.WantCaptureMouse };
}

void TextCenter(std::string text) {
    float font_size = ImGui::GetFontSize() * text.size() / 2;
    ImGui::SameLine(
        ImGui::GetWindowSize().x / 2 -
        font_size + (font_size / 2)
    );

    ImGui::Text(text.c_str());
	ImGui::NewLine();
}

void GUI::Render(){
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();

	// UI Generation
	if (ImGui::BeginMainMenuBar()) {
		if(ImGui::BeginMenu("Simulation Controls")) {
			float speed = globalTimeScale;
			if(ImGui::SliderFloat("Simulation Time Scale", &speed, 0.001, 10))
				globalTimeScale = speed;

			if (ImGui::MenuItem((globalShouldScale ? "Use Actual Data" : "Use Scaled Data")))
				globalShouldScale = !globalShouldScale;

			ImGui::EndMenu();
		}

		if(ImGui::BeginMenu("Help")) {
			ImGui::NewLine();
			ImGui::NewLine();
			TextCenter("Solar System");
			ImGui::NewLine();
			TextCenter("A 2021 CS 480/680 Group Project");
			TextCenter("- Joshua Dahl");
			TextCenter("- Bryan Dedeurwaerder");
			TextCenter("- Jonathan Peters");
			ImGui::NewLine();
			TextCenter("University of Nevada, Reno");
			ImGui::NewLine();



			ImGui::TextColored(ImVec4(.9,.9,1,1), "Key Bindings");
			if (ImGui::BeginTable("", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
				ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::TableHeader("Device");
					ImGui::TableSetColumnIndex(1);
					ImGui::TableHeader("Key");
					ImGui::TableSetColumnIndex(2);
					ImGui::TableHeader("Action");

				ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Mouse");
					ImGui::TableSetColumnIndex(1);
					ImGui::Text("Right Button");
					ImGui::TableSetColumnIndex(2);
					ImGui::Text("Move Camera");

				ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text(" ");
					ImGui::TableSetColumnIndex(1);
					ImGui::Text("Scroll Wheel");
					ImGui::TableSetColumnIndex(2);
					ImGui::Text("Zoom");

				ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("KeyBoard");
					ImGui::TableSetColumnIndex(1);
					ImGui::Text("Tab");
					ImGui::TableSetColumnIndex(2);
					ImGui::Text("Change Focus Celestial");

				ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text(" ");
					ImGui::TableSetColumnIndex(1);
					ImGui::Text("Left Shift");
					ImGui::TableSetColumnIndex(2);
					ImGui::Text("Slow Zoom Speed");

				ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text(" ");
					ImGui::TableSetColumnIndex(1);
					ImGui::Text("Escape");
					ImGui::TableSetColumnIndex(2);
					ImGui::Text("Quit");

				ImGui::EndTable();
			}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}


	// Render the UI results
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

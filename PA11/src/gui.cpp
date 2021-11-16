#include "gui.h"

#include <SDL2/SDL.h>
#include <iomanip>
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include "application.h"
#include "window.h"
#include "graphics.h"

// initialize the ImGUI IO instance
GUI::GUI() : io( []() -> ImGuiIO& {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	return ImGui::GetIO();
}() ) { }

GUI::~GUI() {
	ImGui_ImplOpenGL3_Shutdown();
	 ImGui_ImplSDL2_Shutdown();
	 ImGui::DestroyContext();
}

bool GUI::initialize(Engine* engine, const char* glsl_version/* = "#version 330"*/) {
	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/renderer backends
	if( !ImGui_ImplSDL2_InitForOpenGL(engine->getWindow()->getWindow(), engine->getWindow()->getContext()) )
		return false;
	if( !ImGui_ImplOpenGL3_Init(glsl_version) )
		return false;

	// Save the pointer to the application
	app = reinterpret_cast<Application*>(engine);
	// Save the pointer to the graphics
	graphics = engine->getGraphics();

	//return setupFramebuffer();
	return true;
}

GUI::ShouldProcessEvents GUI::processEvent(SDL_Event& event) {
	// Pass events along to ImGUI
	ImGui_ImplSDL2_ProcessEvent(&event);

	// Return if ImGUI is hogging our events
	return { !io.WantCaptureKeyboard, !io.WantCaptureMouse };
}

void TextCenter(std::string text) {
	float font_size = ImGui::GetFontSize() * text.size() / 2;
	ImGui::SameLine(ImGui::GetWindowSize().x / 2 - font_size + (font_size / 2));

	ImGui::Text(text.c_str());
	ImGui::NewLine();
}

void GUI::render() {
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();

	// TODO: Update
	// UI Generation
	if (ImGui::BeginMainMenuBar()) {

		// Leaderboard
		if(ImGui::BeginMenu("Leaderboard")) {

			ImGui::TextColored(ImVec4(.9,.9,1,1), "Leaderboard");
			if (ImGui::BeginTable("", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
				ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::TableHeader("Player");
					ImGui::TableSetColumnIndex(1);
					ImGui::TableHeader("Score");
				for (std::pair<std::string, float> stat: app->leaderboard->leaderstats) {
					ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::Text(stat.first.c_str());
						ImGui::TableSetColumnIndex(1);
						ImGui::Text(std::to_string(stat.second).c_str());
				}

				ImGui::EndTable();
			}
			ImGui::EndMenu();
		}

		// Game over window
		if(app->gameState == Application::GameState::GameOver) {
			ImGui::Begin("Round Ended");
			TextCenter("Game Over!");

			TextCenter("Your Score: " + std::to_string(app->getScore()));

			ImGui::Text("Enter your name to save your score");
 			static char name[128] = "";
			ImGui::Text("Name: "); ImGui::SameLine(); ImGui::InputText("", name, IM_ARRAYSIZE(name));
			if (ImGui::Button("Confirm")) {
				app->leaderboard->updateScore(name, app->getScore());
				app->reset();
			}
			ImGui::End();
		}

		// Light Controls
		if(ImGui::BeginMenu("Lighting Controls")) {
			ImGui::Text("ADS Lighting Settings");

			Light::ptr ambient = app->lights[0];
			Light::ptr ballSpotlight = app->lights[2];

			ImGui::ColorEdit3("Ambient Color", glm::value_ptr(ambient->lightAmbient));

			glm::vec3 specular(app->lights[1]->lightSpecular);
			glm::vec3 diffuse(app->lights[1]->lightDiffuse);

			ImGui::ColorEdit3("Specular Color", glm::value_ptr(specular));
			ImGui::ColorEdit3("Diffuse Color", glm::value_ptr(diffuse));

			for(Light::ptr light : app->lights) {
				light->lightSpecular = glm::vec4(specular, 1.0);
				light->lightDiffuse = glm::vec4(diffuse, 1.0);
			}

			float lightCutoffAngle = glm::degrees(glm::acos(ballSpotlight->lightCutoffAngleCosine));
			ImGui::SliderFloat("BallSpotlight Cutoff Angle", &lightCutoffAngle, 0.0f, 180.0f);
			ballSpotlight->lightCutoffAngleCosine = glm::cos(glm::radians(lightCutoffAngle));
			ImGui::SliderFloat("BallSpotlight Intensity", &ballSpotlight->lightIntensity, 0.0f, 5.0f);

			ImGui::EndMenu();
		}

		// render help menu
		if(ImGui::BeginMenu("Help")) {
			ImGui::NewLine();
			ImGui::NewLine();
			TextCenter("Pin Ball Game");
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
					ImGui::Text("");
					ImGui::TableSetColumnIndex(1);
					ImGui::Text("A/Left Arrow");
					ImGui::TableSetColumnIndex(2);
					ImGui::Text("Flip left paddle.");

				ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("");
					ImGui::TableSetColumnIndex(1);
					ImGui::Text("D/Right Arrow");
					ImGui::TableSetColumnIndex(2);
					ImGui::Text("Flip right paddle.");

				ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("");
					ImGui::TableSetColumnIndex(1);
					ImGui::Text("S/Down Arrow");
					ImGui::TableSetColumnIndex(2);
					ImGui::Text("Charge plunger.");

				ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("");
					ImGui::TableSetColumnIndex(1);
					ImGui::Text("R");
					ImGui::TableSetColumnIndex(2);
					ImGui::Text("Reset the ball, lose a life.");

				ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text(" ");
					ImGui::TableSetColumnIndex(1);
					ImGui::Text("Space");
					ImGui::TableSetColumnIndex(2);
					ImGui::Text("Toggles between per-vertex and per-fragment lighting (on release)");

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
					ImGui::Text("Tab");
					ImGui::TableSetColumnIndex(2);
					ImGui::Text("Focus on Ball");

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

		std::stringstream fps;
		fps << "FPS: " << std::setprecision(4) << app->getAverageFPS();
		// Right justify the fps text
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::CalcTextSize(fps.str().c_str()).x - 10);
		ImGui::Text(fps.str().c_str());


		ImGui::EndMainMenuBar();
	}


	// Render the UI results
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

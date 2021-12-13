#include "gui.h"

#include <SDL2/SDL.h>
#include <iomanip>
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include "application.h"
#include "window.h"
#include "graphics.h"

#include <sstream>

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
				for (std::pair<std::string, float> stat: app->leaderboard.leaderstats) {
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


		// Render help menu
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

			TextCenter("The objective of the game is to collect\n as many cows as you can within the time limit.\n Collecting aliens reduces your score while collecting cows increases your score.");
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
					ImGui::Text("UFO tilt left");

				ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("");
					ImGui::TableSetColumnIndex(1);
					ImGui::Text("D/Right Arrow");
					ImGui::TableSetColumnIndex(2);
					ImGui::Text("UFO tilt right");

				ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("");
					ImGui::TableSetColumnIndex(1);
					ImGui::Text("S/Down Arrow");
					ImGui::TableSetColumnIndex(2);
					ImGui::Text("UFO tilt back");

				ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("");
					ImGui::TableSetColumnIndex(1);
					ImGui::Text("S/Down Arrow");
					ImGui::TableSetColumnIndex(2);
					ImGui::Text("UFO tilt forward");

				// ImGui::TableNextRow();
				// 	ImGui::TableSetColumnIndex(0);
				// 	ImGui::Text("");
				// 	ImGui::TableSetColumnIndex(1);
				// 	ImGui::Text("R");
				// 	ImGui::TableSetColumnIndex(2);
				// 	ImGui::Text("Reset the ball, lose a life.");

				ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text(" ");
					ImGui::TableSetColumnIndex(1);
					ImGui::Text("Space");
					ImGui::TableSetColumnIndex(2);
					ImGui::Text("Abducting");

				// ImGui::TableNextRow();
				// 	ImGui::TableSetColumnIndex(0);
				// 	ImGui::Text(" ");
				// 	ImGui::TableSetColumnIndex(1);
				// 	ImGui::Text("Left Shift");
				// 	ImGui::TableSetColumnIndex(2);
				// 	ImGui::Text("Slow Zoom Speed");

				// ImGui::TableNextRow();
				// 	ImGui::TableSetColumnIndex(0);
				// 	ImGui::Text(" ");
				// 	ImGui::TableSetColumnIndex(1);
				// 	ImGui::Text("Tab");
				// 	ImGui::TableSetColumnIndex(2);
				// 	ImGui::Text("Focus on Ball");

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

		ImGui::Begin("Your Score");
		//TextCenter("Current");
		std::stringstream stream;
		stream << std::fixed << std::setprecision(2) << app->getTimeRemaining();

		TextCenter("Time Remaining: " + stream.str() + "s");
		TextCenter("Points: " + std::to_string(app->getScore()));
		ImGui::End();

		// Game over window
		if(app->gameOver) {
			ImGui::Begin("Round Ended");
			TextCenter("Game Over!");

			TextCenter("Your Score: " + std::to_string(app->getScore()));

            ImGui::Text("Enter your name to save your score");
 			static char name[128] = "";
			ImGui::Text("Name: "); ImGui::SameLine(); ImGui::InputText("", name, IM_ARRAYSIZE(name));
			if (ImGui::Button("Confirm"))
			{
				app->leaderboard.updateScore(name, app->getScore());
				app->reset();
			}
            ImGui::End();
		}

		app->drawGUI();

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

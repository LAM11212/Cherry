#pragma once
#ifndef WORLD_EDITOR_H
#define WORLD_EDITOR_H
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include "WorldObject.h"
#include <vector>
#include <string>
#include <nlohmann/json.hpp>

class WorldEditor {
public:
	GLFWwindow* window;

	WorldEditor(GLFWwindow* window) {
		this->window = window;
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		ImGui::StyleColorsDark();

		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 330");
	}

	void BeginFrame() {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void DrawUI() {
		ImGui::Begin("World Editor");
		ImGui::Text("Edit your world here!");

		float scale = 1.0f;
		ImGui::SliderFloat("Scale", &scale, 0.0f, 100.0f);

		float xPos;
		ImGui::SliderFloat("X", &xPos, -1000.0f, 1000.0f);

		float yPos;
		ImGui::SliderFloat("Y", &yPos, -1000.0f, 1000.0f);

		float zPos;
		ImGui::SliderFloat("Z", &zPos, -1000.0f, 1000.0f);
		ImGui::End();
	}

};




#endif
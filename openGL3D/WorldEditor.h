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

std::vector<WorldObject> worldObjects;

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
		ImGui::End();
		//for(int i = 0; i < worldObjects.size(); i++)
	}

};




#endif
#pragma once
#ifndef WORLD_EDITOR_H
#define WORLD_EDITOR_H
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include "WorldObject.h"
#include "WorldFactory.h"
#include <vector>
#include <string>
#include <nlohmann/json.hpp>

class WorldFactory;

class WorldEditor {
public:
	GLFWwindow* window;

	std::vector<WorldObject>* EditableWorldObjects = nullptr;
	WorldObject* selectedObject = nullptr;

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

	void BindWorld(WorldFactory& factory) {
		EditableWorldObjects = &factory.getObjects();
	}

	void DrawUI() {
		ImGui::Begin("World Editor");
		
		if (!EditableWorldObjects) {
			ImGui::Text("No world bound.");
			ImGui::End();
			return;
		}

		for (int i = 0; i < EditableWorldObjects->size(); i++) {
			WorldObject& obj = (*EditableWorldObjects)[i];
			const bool selected = (selectedObject == &obj);

			std::string label = obj.type + "##" + std::to_string(i);

			if (ImGui::Selectable(label.c_str(), selected)) {
				selectedObject = &obj;
			}
		}

		ImGui::End();
	}

};




#endif
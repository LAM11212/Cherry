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
	bool isSelected = false;
	int cubeCount = 0;
	int wallCount = 0;
	int floorCount = 0;

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
			std::string label = obj.name + "##" + std::to_string(i);

			if (ImGui::Selectable(label.c_str(), selected)) {
				selectedObject = &obj;
			}
		}

		ImGui::End();
	}

	void Inspector() {
		ImGui::Begin("Inspector");
		static bool snap = false;
		ImGui::Checkbox("Snap to Grid", &snap);

		if (selectedObject) {
			ImGui::Text("Selected Object");
			ImGui::Separator();

			ImGui::Text("Type: %s", selectedObject->type.c_str());
			ImGui::Spacing();

			if (ImGui::DragFloat3("Position", &selectedObject->transform.position.x, 0.1f)) {
				if (snap) {
					selectedObject->transform.position = glm::round(selectedObject->transform.position);
				}
				else {
					selectedObject->transform.position = selectedObject->transform.position;
				}
			}

			ImGui::DragFloat3("Rotation", &selectedObject->transform.rotation.x, 1.0f);
			ImGui::DragFloat3("Scale", &selectedObject->transform.scale.x, 0.1f, 0.01f, 100.0f);

			static char buffer[256];
			strncpy_s(buffer, sizeof(buffer), selectedObject->name.c_str(), _TRUNCATE);
			buffer[sizeof(buffer) - 1] = '\0';

			if (ImGui::InputText("Name", buffer, sizeof(buffer))) {
				selectedObject->name = std::string(buffer);
			}


		}   else {
			ImGui::TextDisabled("No object selected");
		}

		ImGui::End();
	}

	// the menu that will be used to spawn new objects into the engine.
	void ObjectMenu() {
		ImGui::Begin("ObjectMenu");
		if(ImGui::Selectable("cube", isSelected)) {
			isSelected = !isSelected;
			WorldFactory::createObject("cube");
		}
		ImGui::End();
	}

};
#endif
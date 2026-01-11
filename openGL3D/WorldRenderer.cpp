#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include "WorldObject.h"

class WorldRenderer {
public:
	static void RenderObject(const WorldObject& obj, GLuint cubeVAO, Camera& camera, glm::mat4 projection, glm::mat4 view);
};
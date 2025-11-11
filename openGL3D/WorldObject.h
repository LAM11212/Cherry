#pragma once
#include <glm/glm.hpp>
#include "shaders/Shader.h"
#include "MaterialType.h"

struct WorldObject {
	glm::vec3 position;
	glm::vec3 scale;
	MaterialType material;
	GLuint textureID = 0;

	void render(GLuint cubeVAO,
		Shader& shader,
		bool lightCasters,
		bool flashLight);
};
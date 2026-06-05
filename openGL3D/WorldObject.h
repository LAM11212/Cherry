#pragma once

#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "MaterialType.h"
#include "Transform.h"

// Forward declarations
class Shader;
class Camera;

//void SetBasicShaderUniforms(Shader& shader, bool flashLight, const Camera& camera);

struct WorldObject {
    std::string name;
    std::string type;          
    Transform transform;

    MaterialType material = MaterialType::Colored;
    unsigned int textureID = 0;

    void Render(
        GLuint cubeVAO,
        const glm::mat4& projection,
        const glm::mat4& view,
        Camera& camera
    );

};

void InitWorldShaders();
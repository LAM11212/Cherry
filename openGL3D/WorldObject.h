#pragma once

#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>

// Forward declarations
class Shader;
class Camera;

enum class MaterialType {
    Colored,
    Textured,
    LightEmissive
};

struct Transform {
    glm::vec3 position{ 0.0f, 0.0f, 0.0f };
    glm::vec3 rotation{ 0.0f, 0.0f, 0.0f }; // degrees
    glm::vec3 scale{ 1.0f, 1.0f, 1.0f };

    glm::mat4 getMatrix() const;
};

struct WorldObject {
    std::string type;          // prefab type (e.g. "crate", "wall")
    Transform transform;

    MaterialType material = MaterialType::Colored;
    unsigned int textureID = 0;

    void Render(
        GLuint cubeVAO,
        bool flashLight,
        const glm::mat4& projection,
        const glm::mat4& view,
        const Camera& camera
    );

};

void InitWorldShaders();
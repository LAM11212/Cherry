#include "WorldObject.h"
#include "shaders/shader.h"
#include "camera/camera.h"
#include <glm/gtc/matrix_transform.hpp>

static Shader* lightCasterShader = nullptr;
static Shader* colorShader = nullptr;
static Shader* universalShader = nullptr;

void setBasicShaderUniforms(Shader& shader, bool flashLight, Camera& camera);
void InitWorldShaders();

Shader* renderShaders(const WorldObject& obj, Shader& shader, bool lightCasters, bool flashLight, Camera& camera)
{

    Shader* activeShader = nullptr;
    // Shader declarations

    switch (obj.material)
    {
    case MaterialType::Colored: 
        activeShader = universalShader;
        setBasicShaderUniforms(*activeShader, flashLight, camera);
        break;
    case MaterialType::Textured: 
		activeShader = universalShader;
        setBasicShaderUniforms(*activeShader, flashLight, camera);
        break;
    case MaterialType::LightEmissive: 
        //this case is reserved for objects that will cast lighting over other objects
        activeShader = lightCasterShader;
        lightCasterShader->use();
        lightCasterShader->setInt("material.diffuse", 0);
        lightCasterShader->setInt("material.specular", 1);

        lightCasterShader->setVec3("light.position", obj.position);
        lightCasterShader->setVec3("viewPos", glm::vec3(0.0f, 0.0f, 3.0f));

        lightCasterShader->setVec3("light.ambient", glm::vec3(0.2f));
        lightCasterShader->setVec3("light.diffuse", glm::vec3(0.5f));
        lightCasterShader->setVec3("light.specular", glm::vec3(1.0f));
        lightCasterShader->setFloat("light.constant", 1.0f);
        lightCasterShader->setFloat("light.linear", 0.09f);
        lightCasterShader->setFloat("light.quadratic", 0.032f);

        lightCasterShader->setFloat("material.shininess", 32.0f);
        break;
    default: 
        activeShader = colorShader;
        colorShader->setVec3("objectColor", glm::vec3(1.0f));
        colorShader->setVec3("lightColor", glm::vec3(1.0f));
        break;
    }

    if (activeShader == nullptr) {
        std::cerr << "shader is nullptr" << std::endl;
    } 

    return activeShader;
}

void WorldObject::render(GLuint cubeVAO, Shader& shader, bool lightCasters, bool flashLight, glm::mat4 projection, glm::mat4 view, Camera& camera) 
{
	Shader* activeShader = renderShaders(*this, shader, lightCasters, flashLight, camera);
    if (!activeShader) {
        std::cerr << "active shader is null" << std::endl;
        return;
    }

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, position);
	model = glm::scale(model, scale);
	activeShader->use();
	activeShader->setMat4("model", model);
    activeShader->setMat4("projection", projection);
	activeShader->setMat4("view", view);

	if (material == MaterialType::Textured && textureID != 0)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);
        activeShader->setInt("material.diffuse", 0);
	}

	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
}

void setBasicShaderUniforms(Shader& shader, bool flashLight, Camera& camera)
{
    shader.use();
    shader.setVec3("light.position", camera.Position); // camera position
    shader.setVec3("light.direction", camera.Front); // camera front
    shader.setFloat("light.cutOff", glm::cos(glm::radians(12.5f)));
    shader.setFloat("light.outerCutOff", glm::cos(glm::radians(17.5f)));
    shader.setVec3("viewPos", glm::vec3(0.0f, 0.0f, 3.0f)); // camera position

    shader.setVec3("light.ambient", glm::vec3(0.1f));
    shader.setVec3("light.diffuse", glm::vec3(0.8f));
    shader.setVec3("light.specular", glm::vec3(1.0f));
    shader.setFloat("light.constant", 1.0f);
    shader.setFloat("light.linear", 0.09f);
    shader.setFloat("light.quadratic", 0.032f);

    if(flashLight)
        shader.setInt("light.type", 1); // flashlight
    else
		shader.setInt("light.type", 0);
    shader.setFloat("material.shininess", 32.0f);
}

void InitWorldShaders()
{
    if (lightCasterShader || colorShader || universalShader)
        return; // already initialized

    // Make sure OpenGL context exists here
    lightCasterShader = new Shader("shaders/light_casters.vs", "shaders/light_casters.fs");
    colorShader = new Shader("shaders/colors.vs", "shaders/colors.fs");
    universalShader = new Shader("shaders/universal_lit.vs", "shaders/universal_lit.fs");

    std::cout << "World shaders initialized!" << std::endl;
}
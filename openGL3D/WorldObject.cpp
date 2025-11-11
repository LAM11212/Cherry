#include "WorldObject.h"
#include "shaders/shader.h"
#include <glm/gtc/matrix_transform.hpp>


Shader* renderShaders(const WorldObject& obj, Shader& shader, bool lightCasters, bool flashLight)
{
    // Step 1: Decide which shader to use.
// For now, we’ll just use the same one (baseShader), but you could extend this later.
    Shader* activeShader = &shader;

    // Step 2: Activate the shader.
    activeShader->use();

    //im pretty sure we dont need this?
    /*if (flashLight)
    {
        setBasicShaderUniforms(*activeShader, flashLight);
    }*/

    switch (obj.material)
    {
    case MaterialType::Colored:
		setBasicShaderUniforms(*activeShader, flashLight);
        break;

    case MaterialType::Textured:
        setBasicShaderUniforms(*activeShader, flashLight);
        break;
    case MaterialType::LightEmissive:
            //this case is reserved for objects that will cast lighting over other objects
            Shader shader("shaders/light_casters.vs", "shaders/light_casters.fs");
            activeShader = &shader;
            shader.use();
            shader.setInt("material.diffuse", 0);
            shader.setInt("material.specular", 1);

            shader.setVec3("light.position", obj.position);
			shader.setVec3("viewPos", glm::vec3(0.0f, 0.0f, 3.0f));

			shader.setVec3("light.ambient", glm::vec3(0.2f));
			shader.setVec3("light.diffuse", glm::vec3(0.5f));
			shader.setVec3("light.specular", glm::vec3(1.0f));
            shader.setFloat("light.constant", 1.0f);
			shader.setFloat("light.linear", 0.09f);
			shader.setFloat("light.quadratic", 0.032f);

            shader.setFloat("material.shininess", 32.0f);
    default:
        activeShader->setVec3("objectColor", glm::vec3(1.0f));
        break;
    }

    // Step 5: Return the shader pointer.
    return activeShader;
}

void WorldObject::render(GLuint cubeVAO, Shader& shader, bool lightCasters, bool flashLight) 
{
	Shader* activeShader = renderShaders(*this, shader, lightCasters, flashLight);
	if (!activeShader) return;

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, position);
	model = glm::scale(model, scale);
	activeShader->use();
	activeShader->setMat4("model", model);

	if (material == MaterialType::Textured && textureID != 0)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);
	}

	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
}

void setBasicShaderUniforms(Shader& shader, bool flashLight)
{
	Shader* activeShader = &shader;

    activeShader->setVec3("light.position", glm::vec3(0.0f, 0.0f, 3.0f)); // camera position
    activeShader->setVec3("light.direction", glm::vec3(0.0f, 0.0f, -1.0f)); // camera front
    activeShader->setFloat("light.cutOff", glm::cos(glm::radians(12.5f)));
    activeShader->setVec3("viewPos", glm::vec3(0.0f, 0.0f, 3.0f)); // camera position

    activeShader->setVec3("light.ambient", glm::vec3(0.1f));
    activeShader->setVec3("light.diffuse", glm::vec3(0.8f));
    activeShader->setVec3("light.specular", glm::vec3(1.0f));
    activeShader->setFloat("light.constant", 1.0f);
    activeShader->setFloat("light.linear", 0.09f);
    activeShader->setFloat("light.quadratic", 0.032f);

    if(flashLight)
        activeShader->setInt("light.type", 1); // flashlight
    else
		activeShader->setInt("light.type", 0);
    activeShader->setFloat("material.shininess", 32.0f);
}
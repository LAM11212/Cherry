#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "WorldObject.h"
#include "shaders/shader.h"
#include "camera/camera.h"
#include "WorldShaders.h"

static Shader* lightCasterShader = nullptr;
static Shader* colorShader = nullptr;
static Shader* universalShader = nullptr;
static Shader* worldLightingShader = nullptr;


//void SetBasicShaderUniforms(Shader& shader, const Camera& camera);
void InitWorldShaders();
void SetWorldLightUniforms(Shader& shader, const Camera& camera);

/*This method returns the appropriate shader for a given WorldObject based on its material type.*/
static Shader* GetShaderForObject(const WorldObject& obj, Camera& camera) 
{
	Shader* shader = nullptr;

	switch (obj.material)
	{
	case MaterialType::Colored:
		shader = colorShader;
		shader->use();

		shader->setVec3("objectColor", glm::vec3(1.0f));
		shader->setVec3("lightColor", worldLight.diffuse);

		SetWorldLightUniforms(*shader, camera);
		break;
	case MaterialType::Textured:
		shader = universalShader;
		shader->use();
		shader->setVec3("light.direction", worldLight.direction);
		shader->setVec3("viewPos", camera.Position);
		shader->setVec3("light.ambient", worldLight.ambient);
		shader->setVec3("light.diffuse", worldLight.diffuse);
		shader->setVec3("light.specular", worldLight.specular);
		shader->setFloat("material.shininess", 32.0f);
		break;
	default:
		shader = colorShader;
		shader->use();

		shader->setVec3("objectColor", glm::vec3(1.0f));
		shader->setVec3("lightColor", worldLight.diffuse);

		SetWorldLightUniforms(*shader, camera);
		break;
	}

	return shader;
}

/*This method renders the WorldObject using the appropriate shader and transformation matrices.*/
void WorldObject::Render(GLuint cubeVAO, const glm::mat4& projection, const glm::mat4& view, Camera& camera)
{
	Shader* shader = GetShaderForObject(*this, camera);
	if (!shader) 
	{
		std::cerr << "WorldObject::Render:FAILED:: SHADER IS NULL\n";
		return;
	}

	shader->use();

	glm::mat4 model = transform.getMatrix();
	shader->setMat4("model", model);
	shader->setMat4("view", view);
	shader->setMat4("projection", projection);

	//std::cout << "model matrix:\n" << transform.position.x << ", " << transform.position.y << ", " << transform.position.z << "\n";

	if (this->material == MaterialType::Textured && this->textureID != 0)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);
		shader->setInt("material.diffuse", 0);
	}

	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
}

void SetWorldLightUniforms(Shader& shader, const Camera& camera) 
{
	shader.use();

	shader.setVec3("viewPos", camera.Position);

	shader.setVec3("light.direction", worldLight.direction);

	shader.setVec3("light.ambient", worldLight.ambient);
	shader.setVec3("light.diffuse", worldLight.diffuse);
	shader.setVec3("light.specular", worldLight.specular);
}

void InitWorldShaders()
{
	if (lightCasterShader || colorShader || universalShader)
		return;

	lightCasterShader = new Shader("shaders/light_casters.vs", "shaders/light_casters.fs");

	colorShader = new Shader("shaders/colors.vs", "shaders/colors.fs");

	universalShader = new Shader("shaders/basic_lighting.vs", "shaders/basic_lighting.fs");


	std::cout << "World shaders initialized\n";
}
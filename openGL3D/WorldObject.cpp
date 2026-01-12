#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "WorldObject.h"
#include "shaders/shader.h"
#include "camera/camera.h"

static Shader* lightCasterShader = nullptr;
static Shader* colorShader = nullptr;
static Shader* universalShader = nullptr;

void SetBasicShaderUniforms(Shader& shader, bool flashLight, const Camera& camera);
void InitWorldShaders();

static Shader* GetShaderForObject(const WorldObject& obj, bool flashLight, Camera& camera) 
{
	Shader* shader = nullptr;

	switch (obj.material)
	{
	case MaterialType::Colored:
	case MaterialType::Textured:
		shader = universalShader;
		SetBasicShaderUniforms(*shader, flashLight, camera);
		break;

	case MaterialType::LightEmissive:
		shader = lightCasterShader;
		shader->use();

		shader->setVec3("light.position", obj.transform.position);
		shader->setVec3("viewPos", camera.Position);

		shader->setVec3("light.ambient", glm::vec3(0.2f));
		shader->setVec3("light.diffuse", glm::vec3(0.5f));
		shader->setVec3("light.specular", glm::vec3(1.0f));

		shader->setFloat("light.constant", 1.0f);
		shader->setFloat("light.linear", 0.09f);
		shader->setFloat("light.quadratic", 0.032f);

		shader->setFloat("material.shininess", 32.0f);
		break;

	default:
		shader = colorShader;
		shader->use();
		shader->setVec3("objectColor", glm::vec3(1.0f));
		shader->setVec3("lightColor", glm::vec3(1.0f));
		break;
	}

	return shader;
}

void WorldObject::Render(GLuint cubeVAO, bool flashLight, const glm::mat4& projection, const glm::mat4& view, Camera& camera)
{
	Shader* shader = GetShaderForObject(*this, flashLight, camera);
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

	if (this->material == MaterialType::Textured && this->textureID != 0)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);
		shader->setInt("material.diffuse", 0);
	}

	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
}

void SetBasicShaderUniforms(Shader& shader, bool flashLight, const Camera& camera) 
{
	shader.use();

	shader.setVec3("viewPos", camera.Position);

	shader.setVec3("light.position", camera.Position);
	shader.setVec3("light.direction", camera.Front);

	shader.setFloat("light.cutOff", glm::cos(glm::radians(12.5f)));
	shader.setFloat("light.outerCutOff", glm::cos(glm::radians(17.5f)));

	shader.setVec3("light.ambient", glm::vec3(0.5f));
	shader.setVec3("light.diffuse", glm::vec3(0.8f));
	shader.setVec3("light.specular", glm::vec3(1.0f));

	shader.setFloat("light.constant", 1.0f);
	shader.setFloat("light.linear", 0.09f);
	shader.setFloat("light.quadratic", 0.032f);

	shader.setInt("light.type", flashLight ? 1 : 0);
	shader.setFloat("material.shininess", 32.0f);
}

void InitWorldShaders()
{
	if (lightCasterShader || colorShader || universalShader)
		return;

	lightCasterShader = new Shader("shaders/light_casters.vs", "shaders/light_casters.fs");

	colorShader = new Shader("shaders/colors.vs", "shaders/colors.fs");

	universalShader = new Shader("shaders/universal_lit.vs", "shaders/universal_lit.fs");

	std::cout << "World shaders initialized\n";
}
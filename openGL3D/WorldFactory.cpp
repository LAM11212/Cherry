#include "worldFactory.h"
#include "stb_image.h"
#include "MaterialType.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

WorldObject WorldFactory::fromPrefab(const std::string& type, const glm::vec3& position)
{
	WorldObject obj{};
	obj.type = type;

	if (type == "floor") 
	{
		obj.transform.scale = { 20.0f, 0.2f, 20.0f };
		obj.material = MaterialType::Textured;
		obj.textureID = loadTexture("textures/stoneTexture.jpg");
	}
	else if (type == "crate")
	{
		obj.transform.scale = { 1.0f, 1.0f, 1.0f };
		obj.material = MaterialType::Textured;
		obj.textureID = loadTexture("textures/container2.png");
	}
	else if (type == "wall")
	{
		obj.transform.scale = { 0.2f, 8.0f, 10.0f };
		obj.material = MaterialType::Textured;
		obj.textureID = loadTexture("textures/backroomswall.jpeg");
	}
	else if (type == "light_emitter")
	{
		obj.transform.scale = { 0.5f, 0.5f, 0.5f };
		obj.material = MaterialType::LightEmissive;
	}
	else {
		std::cerr << "Unknown prefab type: " << type << std::endl;
	}

	return obj;
}

std::vector<WorldObject> WorldFactory::loadWorld(const std::string& path) {
	std::vector<WorldObject> objects;
	std::ifstream file(path);

	if (!file.is_open()) {
		std::cerr << "Failed to open world file: " << path << "\n";
		return objects;
	}

	json data;
	file >> data;

	for (auto& objData : data["objects"]) {
		std::string type = objData["type"];
		std::cout << "object type: " << type << std::endl;
		glm::vec3 pos = glm::vec3(
			objData["position"][0],
			objData["position"][1],
			objData["position"][2]
		);
		objects.push_back(fromPrefab(type, pos));
	}

	if (objects.empty()) {
		std::cerr << "No objects found in the json file: " << path << std::endl;
	}

	return objects;
}

unsigned int WorldFactory::loadTexture(char const* path) {
	unsigned int textureID;
	glGenTextures(1, &textureID);

	stbi_set_flip_vertically_on_load(true);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data) {
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		std::cout << "Loading texture: " << path << std::endl;
		std::cout << "Width: " << width << " Height: " << height << " Components: " << nrComponents << std::endl;
		stbi_image_free(data);
	}
	else {
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}
	return textureID;
}

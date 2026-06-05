#include "worldFactory.h"
#include "stb_image.h"
#include "MaterialType.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
std::vector<WorldObject> objects;

/*this method is responsible for creating a worldObject based on certain types, it also loads textures based on if the material requires that or not.*/
WorldObject WorldFactory::fromPrefab(const std::string& type, const glm::vec3& position)
{
	WorldObject obj{};
	obj.type = type;

	obj.transform.position = position;

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
	else if (type == "cube")
	{
		obj.transform.scale = { 1.0f, 1.0f, 1.0f };
		obj.material = MaterialType::Colored;
	}
	else {
		std::cerr << "Unknown prefab type: " << type << std::endl;
	}

	return obj;
}

/*This method takes the world.json file and then parses it to create WorldObjects, it then those values are retrieved in openGL3D.cpp by doing
  std::vector<WorldObject>& worldObjects = worldFactory.getObjects(); */
void WorldFactory::loadWorld(const std::string& path) {
	objects.clear();
	std::ifstream file(path);

	if (!file.is_open()) {
		std::cerr << "Failed to open world file: " << path << "\n";
		return;
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

		WorldObject obj = fromPrefab(type, pos);
		obj.transform.position = pos;
		objects.push_back(obj);
	}

	if (objects.empty()) {
		std::cerr << "No objects found in the json file: " << path << std::endl;
	}
	file.close();
}

/*This method loads a texture from a file and returns the OpenGL texture ID.*/
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

std::vector<WorldObject>& WorldFactory::getObjects() {
	return objects;
}

void WorldFactory::clearObjects() {
	objects.clear();
}

/*This method creates a new object of the specified type and adds it to the world. It also updates the world.json file accordingly.*/
void WorldFactory::createObject(const std::string type) {
	if (type == "cube") {
		glm::vec3 pos = { 0.0f, 0.0f, 0.0f };
		WorldObject obj = fromPrefab(type, pos);
		obj.transform.position = pos;
		obj.material = MaterialType::Colored;
		objects.push_back(obj);

		std::ifstream in("world.json");
		json worldData;
		in >> worldData;
		in.close();

		worldData["objects"].push_back({
			{"type", type},
			{"position", {pos.x, pos.y, pos.z}}
			});

		std::ofstream out("world.json");
		out << std::setw(4) << worldData;
	}
}

void WorldFactory::SaveChanges(const std::string& path) {
	json worldData;
	for (const WorldObject& obj : objects) {
		worldData["objects"].push_back({
			{"type", obj.type},
			{"position", {obj.transform.position.x, obj.transform.position.y, obj.transform.position.z}}
			}); 
	}

	std::ofstream o(path);

	if (o) {
		o << std::setw(4) << worldData << std::endl;
	}
}
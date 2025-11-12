#include "worldFactory.h"
#include "stb_image.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

WorldObject WorldFactory::fromPrefab(const std::string& type, const glm::vec3& position)
{
	WorldObject obj{};
	obj.position = position;

	if (type == "floor") 
	{
		obj.scale = { 20.0f, 0.2f, 20.0f };
		obj.material = MaterialType::Textured;
	}
	else if (type == "crate")
	{
		obj.scale = { 1.0f, 1.0f, 1.0f };
		obj.material = MaterialType::Textured;
	}
	else if (type == "wall")
	{
		obj.scale = { 0.2f, 8.0f, 10.0f };
		obj.material = MaterialType::Textured;
	}
	else if (type == "light_emitter")
	{
		obj.scale = { 0.5f, 0.5f, 0.5f };
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
		std::cout << "position: " << pos.x << ", " << pos.y << ", " << pos.z << std::endl;
		objects.push_back(fromPrefab(type, pos));
	}

	if (objects.empty()) {
		std::cerr << "No objects found in the json file: " << path << std::endl;
	}

	return objects;
}
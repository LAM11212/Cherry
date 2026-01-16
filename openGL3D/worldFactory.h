#pragma once
#include "Worldobject.h"
#include "MaterialType.h"
#include <glm/glm.hpp>
#include <vector>
#include <string>

class WorldFactory {
public:
	static WorldObject fromPrefab(const std::string& type, const glm::vec3& position);
	static std::vector<WorldObject> loadWorld(const std::string& path);
	static unsigned int loadTexture(char const* path);
	std::vector<WorldObject>& getObjects();
	void clearObjects();
};
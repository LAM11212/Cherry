#pragma once
#include "Worldobject.h"
#include "MaterialType.h"
#include <glm/glm.hpp>
#include <vector>
#include <string>

class WorldFactory {
public:
	static WorldObject fromPrefab(const std::string& type, const glm::vec3& position);
	void loadWorld(const std::string& path);
	static unsigned int loadTexture(char const* path);
	std::vector<WorldObject>& getObjects();
	void clearObjects();
	static void createObject(const std::string type);
	void SaveChanges(const std::string& path);
private:
	int nextObjectID = 1;
};
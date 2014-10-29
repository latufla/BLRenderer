#pragma once
#include <glm.hpp>
#include "Texture2d.h"

class Material3d {
public:
	Material3d() = default;
	Material3d(Texture2d&&, glm::vec4&&, glm::vec4&&, glm::vec4&&, glm::vec4&&, float, float, bool);

	~Material3d() = default;

	Texture2d& getTexture(){ return texture; }

private:
	Texture2d texture;
	
	glm::vec4 emission;
	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;

	float shininess;
	float indexOfRefraction;
	
	bool doubleSided;
};


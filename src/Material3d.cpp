#include "stdafx.h"
#include "Material3d.h"

using glm::vec4;

Material3d::Material3d(Texture2d&& texture, 
	vec4&& emission, 
	vec4&& ambient,
	vec4&& diffuse,
	vec4&& specular,
	float shininess, 
	float indexOfRefraction, 
	bool doubleSided) {

	this->texture = std::move(texture);
	this->emission = std::move(emission);
	this->ambient = std::move(ambient);
	this->diffuse = std::move(diffuse);
	this->specular = std::move(specular);
	this->shininess = shininess;
	this->indexOfRefraction = indexOfRefraction;
	this->doubleSided = doubleSided;
}

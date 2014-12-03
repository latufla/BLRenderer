#include "stdafx.h"
#include "Material3d.h"

using glm::vec4;

namespace br {
	Material3d::Material3d(const Texture2d& texture, 
		const vec4& emission,
		const vec4& ambient,
		const vec4& diffuse,
		const vec4& specular,
		float shininess, 
		float indexOfRefraction, 
		bool doubleSided) {
	
		this->texture = texture;
		this->emission = emission;
		this->ambient = ambient;
		this->diffuse = diffuse;
		this->specular = specular;
		this->shininess = shininess;
		this->indexOfRefraction = indexOfRefraction;
		this->doubleSided = doubleSided;
	}
}

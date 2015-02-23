#include "../utils/SharedHeaders.h"
#include "Material3d.h"

using glm::vec4;

namespace br {
	Material3d::Material3d(Texture2d const& texture, 
		vec4 const& emission,
		vec4 const& ambient,
		vec4 const& diffuse,
		vec4 const& specular,
		float shininess, 
		float indexOfRefraction, 
		bool doubleSided) 
		: texture(texture), 
		emission(emission),
		ambient(ambient),
		diffuse(diffuse),
		specular(specular),
		shininess(shininess),
		indexOfRefraction(indexOfRefraction),
		doubleSided(doubleSided){
	}

	Material3d::~Material3d() {

	}

	Texture2d& Material3d::getTexture() {
		return texture;
	}

	glm::vec4 const& Material3d::getEmission() const {
		return emission;
	}

	glm::vec4 const& Material3d::getAmbient() const {
		return ambient;
	}

	glm::vec4 const& Material3d::getDiffuse() const {
		return diffuse;
	}

	glm::vec4 const& Material3d::getSpecular() const {
		return specular;
	}

	float Material3d::getShininess() const {
		return shininess;
	}

	float Material3d::getIndexOfRefraction() const {
		return indexOfRefraction;
	}

	bool Material3d::getDoubleSided() const {
		return doubleSided;
	}

}

#include "SharedHeaders.h"
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
}

#pragma once
#include <glm.hpp>
#include "Texture2d.h"

namespace br {
	class Material3d {
	public:
		Material3d() = default;
		Material3d(const Texture2d&, const glm::vec4&, const glm::vec4&, const glm::vec4&, const glm::vec4&, float, float, bool);
	
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
}


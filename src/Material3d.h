#pragma once
#include <glm.hpp>
#include "Texture2d.h"

namespace br {
	class Material3d {
	public:
		Material3d() = delete;
		Material3d(const Texture2d& texture,
			const glm::vec4& emission,
			const glm::vec4& ambient,
			const glm::vec4& diffuse,
			const glm::vec4& specular,
			float shininess,
			float indexOfRefraction,
			bool doubleSided);

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


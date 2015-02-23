#pragma once
#include <glm.hpp>
#include "Texture2d.h"

namespace br {
	class Material3d {
	public:
		Material3d() = delete;
		Material3d(Texture2d const& texture,
			glm::vec4 const& emission,
			glm::vec4 const& ambient,
			glm::vec4 const& diffuse,
			glm::vec4 const& specular,
			float shininess,
			float indexOfRefraction,
			bool doubleSided);

		virtual ~Material3d();
	
		Texture2d& getTexture();

		glm::vec4 const& getEmission() const;
		glm::vec4 const& getAmbient() const;
		glm::vec4 const& getDiffuse() const;
		glm::vec4 const& getSpecular() const;

		float getShininess() const;
		float getIndexOfRefraction() const;

		bool getDoubleSided() const;
	
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


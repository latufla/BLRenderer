#pragma once
#include <vector>
#include <glm.hpp>
#include "../Texture2d.h"
#include <assimp/Importer.hpp>
#include <array>

namespace br {
	class Util
	{
	public:
		Util() = delete;
		~Util() = delete;
	
		static Texture2d loadTexture(std::string path);
	
		static glm::mat4 Util::assimpToGlm(const aiMatrix4x4&);
		static aiMatrix4x4 Util::glmToAssimp(const glm::mat4&);
	
		static glm::mat4 Util::assimpMat3ToGlm(const aiMatrix3x3&);
		static aiMatrix3x3 Util::glmToAssimpMat3(const glm::mat4&);
	
		static glm::vec3 Util::assimpToGlm(const aiVector3D&);
		static glm::vec4 Util::assimpToGlm(const aiColor4D&);
		
		static std::string Util::toString(const glm::mat4&);
		static std::string Util::toString(const glm::vec3&);
	
		static glm::vec3 interpolate(const glm::vec3& from, const glm::vec3& to, float alpha);
		static glm::mat4 interpolate(const glm::mat4& from, const glm::mat4& to, float alpha);
	
	private:
		static std::vector<uint8_t> loadFile(const std::string&);
		static int decodePNG(std::vector<unsigned char>&, unsigned long&, unsigned long&, const unsigned char*, size_t, bool = true);
	
	};
}


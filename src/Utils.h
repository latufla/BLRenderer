#pragma once
#include <vector>
#include <glm.hpp>
#include "Texture2d.h"
#include <assimp/Importer.hpp>
#include <array>

class Utils
{
public:
	Utils() = delete;
	~Utils() = delete;

	static std::string defaultVertexShader;
	static std::string defaultFragmentShader;

	static bool loadTexture(std::string, Texture2d&);

	static glm::mat4 Utils::assimpToGlm(const aiMatrix4x4&);
	static aiMatrix4x4 Utils::glmToAssimp(const glm::mat4&);

	static glm::mat4 Utils::assimpMat3ToGlm(const aiMatrix3x3&);
	static aiMatrix3x3 Utils::glmToAssimpMat3(const glm::mat4& m);

	static glm::vec3 Utils::assimpToGlm(const aiVector3D&);
	static glm::vec4 Utils::assimpToGlm(const aiColor4D&);
	
	static std::array<float, 16> Utils::toArray(const glm::mat4&);

	static std::string Utils::toString(const glm::mat4&);
	static std::string Utils::toString(const glm::vec3&);

	static glm::vec3 interpolate(const glm::vec3&, const glm::vec3&, float);
	static glm::mat4 interpolate(const glm::mat4&, const glm::mat4&, float);

private:
	static std::vector<uint8_t> loadFile(const std::string&);
	static int decodePNG(std::vector<unsigned char>&, unsigned long&, unsigned long&, const unsigned char*, size_t, bool = true);

};


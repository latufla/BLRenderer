#pragma once
#include <iostream>
#include <vector>
#include <fstream>
#include <glm.hpp>
#include "Material3d.h"
#include "tree\Node.h"
#include <assimp/Importer.hpp>
#include <array>

class Utils
{
public:
	Utils() = delete;
	~Utils() = delete;

	static void print(glm::mat4);
	static void print(glm::vec4);
	static int decodePNG(std::vector<unsigned char>& out_image, unsigned long& image_width, unsigned long& image_height, const unsigned char* in_png, size_t in_size, bool convert_to_rgba32 = true);
	static void loadFile(std::vector<unsigned char>& buffer, const std::string& filename);
	static bool loadTexture(std::string, Material3d&);

	static void testNodeTree();

	static glm::mat4 Utils::assimpToGlmMatrix(const aiMatrix4x4&);
	static glm::mat4 Utils::assimp3x3ToGlmMatrix(const aiMatrix3x3&);
	static aiMatrix4x4 Utils::glmToAssimpMatrix(const glm::mat4&);
	static aiMatrix3x3 Utils::glmToAssimpMatrix3x3(const glm::mat4& m);

	static glm::vec3 Utils::assimpToGlmVector3d(const aiVector3D&);

	static std::array<float, 16> Utils::glmMatrixToArray(const glm::mat4&);

	static std::string Utils::glmToString(const glm::mat4&);
	static std::string Utils::glmToString(const glm::vec3&);
	static glm::vec3 interpolate(const glm::vec3& start, const glm::vec3& end, float alpha);
	static glm::mat4 interpolateQ(const glm::mat4& start, const glm::mat4& end, float alpha);
};


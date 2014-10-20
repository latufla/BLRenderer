#pragma once
#include <iostream>
#include <vector>
#include <fstream>
#include <glm.hpp>
#include "Material3d.h"
#include "tree\Node.h"
#include <assimp/Importer.hpp>

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

	static glm::mat4 Utils::assimpToGlmMatrix(const aiMatrix4x4& m);
};


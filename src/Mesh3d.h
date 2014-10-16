#pragma once
#include <vector>

class Mesh3d {
public:
	Mesh3d();
	Mesh3d(std::string, const std::vector<float>&, const std::vector<unsigned short>&, uint32_t);

	~Mesh3d();

	std::string getName() const { return name; }

	std::vector<float>& getVertices() { return vertices; }
	std::vector<unsigned short>& getIndices() { return indices; }

	uint32_t getTextureId() const { return textureId; }

	operator std::string() const;

private:
	std::string name;	
	
	std::vector<float> vertices;
	std::vector<unsigned short> indices;

	uint32_t textureId;
};


#pragma once
#include <vector>

struct Vertex3d {
	float x;
	float y;
	float z;

	float s;
	float t;

	uint16_t boneIds[4];
	float weigths[4];
};

class Mesh3d {
public:
	Mesh3d();
	Mesh3d(std::string, const std::vector<Vertex3d>&, const std::vector<uint16_t>&, uint32_t);

	~Mesh3d();

	std::string getName() const { return name; }

	std::vector<Vertex3d>& getVertices() { return vertices; }
	std::vector<uint16_t>& getIndices() { return indices; }

	uint32_t getTextureId() const { return textureId; }

	operator std::string() const;

	static const uint8_t VERTEX3D_POSITION = 3;
	static const uint8_t VERTEX3D_TEXTURE = 2;
	static const uint8_t VERTEX3D_BONEIDS = 4;
	static const uint8_t VERTEX3D_WEIGHTS = 4;
	static const uint8_t VERTEX3D_STRIDE = VERTEX3D_POSITION * sizeof(float) 
		+ VERTEX3D_TEXTURE * sizeof(float) 
		+ VERTEX3D_BONEIDS * sizeof(uint16_t) 
		+ VERTEX3D_WEIGHTS * sizeof(float);
private:
	std::string name;	
	
	std::vector<Vertex3d> vertices;
	std::vector<uint16_t> indices;

	uint32_t textureId;
};


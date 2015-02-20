#pragma once
#include <vector>
#include <unordered_map>
#include <glm.hpp>


namespace br {
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
		Mesh3d() = delete;
		Mesh3d(std::string name, const std::vector<Vertex3d>& vertices, const std::vector<uint16_t>& indices, uint32_t materialId);
	
		~Mesh3d() = default;
	
		std::string getName() const { return name; }
	
		std::vector<Vertex3d>& getVertices() { return vertices; }
		std::vector<uint16_t>& getIndices() { return indices; }
	
		uint32_t getMaterialId() const { return materialId; }
	
		operator std::string() const;
		
		void setVertexBoneInfo(uint32_t vertexId, uint32_t boneId, float weight);
		void setBoneOffset(uint32_t boneId, glm::mat4 val);
	
		std::unordered_map<uint32_t, glm::mat4>& getBoneIdToOffset() { return boneIdToOffset; }
	
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
	
		uint32_t materialId;
	
		std::unordered_map<uint32_t, glm::mat4> boneIdToOffset;
	};
}

#pragma once
#include <vector>
#include <unordered_map>
#include <map>
#include <glm.hpp>


namespace br {
	struct Vertex3d {
		float x;
		float y;
		float z;

		float s;
		float t;

		std::map<uint32_t, float> boneToWeight;
	};
	
	class Mesh3d {
	public:
		Mesh3d() = delete;
		Mesh3d(std::string name, const std::vector<Vertex3d>& vertices, const std::vector<uint16_t>& indices, uint32_t materialId);
	
		virtual ~Mesh3d();
	
		std::string getName() const;
	
		std::vector<Vertex3d>& getVertices();
		std::vector<uint16_t>& getIndices();
	
		uint32_t getMaterialId() const;
	
		void setBoneWeight(uint32_t vertexId, uint32_t boneId, float weight);
		void setBoneOffset(uint32_t boneId, glm::mat4 offset);
	
		std::unordered_map<uint32_t, glm::mat4>& getBoneIdToOffset();
	
		void buildRawVertices();
		std::vector<float>& getRawVertices();
		
		static uint8_t GetRawVertexPosition() { return 3; }
		static uint8_t GetRawVertexPositionSize() { return sizeof(float); }

		static uint8_t GetRawVertexTexture() { return 2; }
		static uint8_t GetRawVertexTextureSize() { return sizeof(float); }

		static uint8_t GetRawVertexBoneIds() { return 4; }
		static uint8_t GetRawVertexBoneIdsSize() { return sizeof(float); }

		static uint8_t GetRawVertexWeights() { return GetRawVertexBoneIds(); }
		static uint8_t GetRawVertexWeightsSize() { return sizeof(float); }

		static uint8_t GetRawVertexStride();

	private:
		std::string name;	
		
		std::vector<Vertex3d> vertices;
		std::vector<uint16_t> indices;
	
		uint32_t materialId;
	
		std::unordered_map<uint32_t, glm::mat4> boneIdToOffset;
	
		std::vector<float> rawVertices;
	};
}


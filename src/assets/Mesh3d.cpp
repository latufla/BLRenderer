#include "../utils/SharedHeaders.h"
#include "Mesh3d.h"

using std::vector;
using std::unordered_map;
using std::string;
using std::to_string;

namespace br {
	
	Mesh3d::Mesh3d(string name, const vector<Vertex3d>& vertices, const vector<uint16_t>& indices, uint32_t materialId) 
		: name(name), vertices(vertices), indices(indices), materialId(materialId){
	}

	Mesh3d::~Mesh3d() {

	}


	string Mesh3d::getName() const {
		return name;
	}

	vector<Vertex3d>& Mesh3d::getVertices() {
		return vertices;
	}

	vector<uint16_t>& Mesh3d::getIndices() {
		return indices;
	}

	uint32_t Mesh3d::getMaterialId() const {
		return materialId;
	}

	unordered_map<uint32_t, glm::mat4>& Mesh3d::getBoneIdToOffset() {
		return boneIdToOffset;
	}


	void Mesh3d::setBoneWeight(uint32_t vertexId, uint32_t boneId, float weight) {
		Vertex3d& v = vertices.at(vertexId);
		v.boneToWeight.emplace(boneId, weight);
	}
	
	void Mesh3d::setBoneOffset(uint32_t boneId, glm::mat4 val) {
		boneIdToOffset.emplace(boneId, val);
	}


	vector<float>& Mesh3d::getRawVertices() {
		return rawVertices;
	}

	uint8_t Mesh3d::GetRawVertexStride() {
		return (GetRawVertexPosition() +
			GetRawVertexTexture() +
			GetRawVertexBoneIds() +
			GetRawVertexWeights()) * sizeof(float);
	}

	void Mesh3d::buildRawVertices() {
		rawVertices.clear();
		for(auto& s : vertices) {
			rawVertices.push_back(s.x);
			rawVertices.push_back(s.y);
			rawVertices.push_back(s.z);
			rawVertices.push_back(s.s);
			rawVertices.push_back(s.t);

			uint32_t maxB = s.boneToWeight.size();
		
			auto it = s.boneToWeight.cbegin();
			uint32_t n = GetRawVertexBoneIds();
			for(uint32_t i = 0; i < n; ++i) {
				float id = 0.0f;
				if(it != s.boneToWeight.cend()) {
					id = (float)it->first;
					it++;
				}
				rawVertices.push_back(id);
			}
			
			it = s.boneToWeight.cbegin();
			n = GetRawVertexWeights();
			for(uint32_t i = 0; i < n; ++i) {
				float weight = 0.0f;
				if(it != s.boneToWeight.cend()) {
					weight = it->second;
					it++;
				}
				rawVertices.push_back(weight);
			}
		}
	}
}



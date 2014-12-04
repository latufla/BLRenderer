#include "stdafx.h"
#include "Mesh3d.h"

using std::vector;
using std::string;
using std::to_string;

namespace br {
	
	Mesh3d::Mesh3d(string name, const vector<Vertex3d>& vertices, const vector<uint16_t>& indices, uint32_t materialId) 
		: name(name), vertices(vertices), indices(indices), materialId(materialId){
	}
	
	Mesh3d::operator string() const {
		string res = "";
		res += "{name: " + name + "\nvertices: {\n";
	// 	for (auto& i : vertices) {
	// 		res += to_string(i);
	// 		res += " ";
	// 	}
	// 	res += " }\n";
	// 	res += "indices: {\n";
	// 	for (auto& j : indices) {
	// 		res += to_string(j);
	// 		res += " ";
	// 	}
	// 	res += " }}";
		return res;
	}
	
	void Mesh3d::setVertexBoneInfo(uint32_t vertexId, uint32_t boneId, float weight) {
		Vertex3d& v = vertices.at(vertexId);
		for (uint32_t i = 0; i < 4; ++i) {
			if (v.weigths[i] == 0.0) { // TODO: wonder can we compare doubles
				v.boneIds[i] = boneId;
				v.weigths[i] = weight;
				break;
			}
		}
	}
	
	void Mesh3d::setBoneOffset(uint32_t boneId, glm::mat4 val) {
		boneIdToOffset.emplace(boneId, val);
	}
}



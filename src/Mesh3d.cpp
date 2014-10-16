#include "stdafx.h"
#include "Mesh3d.h"

using std::vector;
using std::string;
using std::to_string;

Mesh3d::Mesh3d() {

}

Mesh3d::Mesh3d(string name, const vector<float>& vertices, const vector<unsigned short>& indices, uint32_t textureId) {
	this->name = name;

	this->vertices = vertices;
	this->indices = indices;

	this->textureId = textureId;
}

Mesh3d::~Mesh3d() {
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


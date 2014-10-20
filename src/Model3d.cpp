#include "stdafx.h"
#include "Model3d.h"
#include "Utils.h"

using std::vector;
using std::string;
using std::exception;
using std::shared_ptr;

Model3d::Model3d() {
}

Model3d::Model3d(string name, const vector<Mesh3d>& meshes, const vector<string>& materials, shared_ptr<Node> boneTree, shared_ptr<Animation3d> animation) {
	this->name = name;
	this->meshes = meshes;
	this->boneTree = boneTree;
	this->animation = animation;

	for (auto& i : materials) {
		Material3d m;
		if (i != "" && !Utils::loadTexture(i, m))
			throw exception("Model3d: can`t load texture");

		this->materials.push_back(m);
	}
}


Model3d::~Model3d() {
}


Model3d::operator string() const {
	string res = "";
	res += "{ name : " + name + "\nmeshes: {\n";
	for (auto& i : meshes) {
		res += i;
		res += "\n";
	}
	res += "}";

	res += "\nmaterials: {\n";
	for (auto& j : materials) {
		res += j.getName();
		res += "\n";
	}
	res += "}}";
	return res;
}

string Model3d::getUniqueMeshName(const Mesh3d& mesh) {
	return name + mesh.getName();
}

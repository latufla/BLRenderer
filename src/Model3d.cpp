#include "stdafx.h"
#include "Model3d.h"
#include "Utils.h"

using std::vector;
using std::string;
using std::exception;
using std::shared_ptr;

Model3d::Model3d(string name, const vector<Mesh3d>& meshes, const vector<string>& materials, const TNode<BoneNodeData>& boneTree, const Animation3d& defaultAnimation) {
	this->name = name;
	this->meshes = meshes;
	this->boneTree = boneTree;
	nameToAnimation[defaultAnimation.getName()] = defaultAnimation;

	for (auto& i : materials) {
		Material3d m;
		if (i != "" && !Utils::loadTexture(i, m))
			throw exception("Model3d: can`t load texture"); // TODO: fix this dirt

		this->materials.push_back(m);
	}
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

Animation3d& Model3d::getAnimation(std::string name) {
	return nameToAnimation.at(name);
}

bool Model3d::addAnimation(Animation3d& anim) {
	nameToAnimation[anim.getName()] = anim;
	return true;
}

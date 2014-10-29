#include "stdafx.h"
#include "Model3d.h"
#include "Utils.h"

using std::vector;
using std::string;
using std::exception;

Model3d::Model3d(string name, const vector<Mesh3d>& meshes, const vector<Material3d>& materials, const TNode<BoneNodeData>& boneTree, const Animation3d& defaultAnimation) {
	this->name = name;
	this->meshes = meshes;
	this->materials = materials;
	this->boneTree = boneTree;
	nameToAnimation.emplace(defaultAnimation.getName(), defaultAnimation);
}

Model3d::operator string() const {
	string res = "";
	res += "{ name : " + name + "\nmeshes: {\n";
	for (auto& i : meshes) {
		res += i;
		res += "\n";
	}
	res += "}";

	res += "\ntextures: {\n";
	for (auto& j : materials) {
		//res += j.getTexture().getName();
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
	nameToAnimation.emplace(anim.getName(), anim);
	return true;
}

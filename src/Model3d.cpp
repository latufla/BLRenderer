#include "SharedHeaders.h"
#include "Model3d.h"
#include "Utils.h"

using std::vector;
using std::string;
using std::exception;

namespace br {
	Model3d::Model3d(string path, 
		const vector<Mesh3d>& meshes, 
		const vector<Material3d>& materials,
		const BNode<BoneNodeData>& boneTree, 
		const Animation3d& defaultAnimation)
		: path(path),
		meshes(meshes),
		materials(materials),
		boneTree(boneTree) {
		nameToAnimation.emplace(defaultAnimation.getName(), defaultAnimation);
	}
	
	Model3d::operator string() const {
		string res = "";
		res += "{ path : " + path + "\nmeshes: {\n";
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
		return path + mesh.getName();
	}
	
	Animation3d& Model3d::getAnimationBy(std::string name) {
		return nameToAnimation.at(name);
	}
	
	void Model3d::addAnimation(Animation3d& anim) {
		nameToAnimation.emplace(anim.getName(), anim);
	}
}

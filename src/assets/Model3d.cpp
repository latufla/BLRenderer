#include "../utils/SharedHeaders.h"
#include "Model3d.h"
#include "../utils/Util.h"
#include "../exceptions/Exception.h"

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

		initHitMesh(this->meshes);
		nameToAnimation.emplace(defaultAnimation.getName(), defaultAnimation);
	}
	
	Material3d& Model3d::getMaterialBy(Mesh3d& mesh) {
		auto id = mesh.getMaterialId();
		try {
			return materials.at(id);
		} catch(std::out_of_range&){
			throw LogicException(EXCEPTION_INFO, "mesh " + mesh.getName() + " has no material");
		}
	}

	Texture2d& Model3d::getTextureBy(Mesh3d& mesh) {
		return getMaterialBy(mesh).getTexture();
	}

	string Model3d::getUniqueMeshName(const Mesh3d& mesh) {
		return path + mesh.getName();
	}
	
	Animation3d& Model3d::getAnimationBy(std::string name) {
		try {
			return nameToAnimation.at(name);
		} catch(std::out_of_range&) {
			throw AssetException(EXCEPTION_INFO, name, "no such animation");
		}
	}
	
	void Model3d::addAnimation(Animation3d& anim) {
		string name = anim.getName();
		auto res = nameToAnimation.emplace(name, anim);
		if(!res.second)
			throw AssetException(EXCEPTION_INFO, name, "has same animation");
	}

	void Model3d::initHitMesh(std::vector<Mesh3d>& meshes) {
		auto it = find_if(begin(meshes), end(meshes), [](Mesh3d& mesh) {
			return mesh.getName() == "HitMesh";
		});

		if(it != end(meshes)) {
			hitMesh = std::make_shared<Mesh3d>(*it);
			meshes.erase(it);
		}
	}
}

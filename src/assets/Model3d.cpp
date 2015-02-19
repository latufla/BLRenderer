#include "../utils/SharedHeaders.h"
#include "Model3d.h"
#include "../utils/Util.h"
#include "../exceptions/Exception.h"

using std::vector;
using std::string;
using std::exception;
using std::out_of_range;
using std::shared_ptr;

namespace br {
	Model3d::Model3d(string path, 
		vector<Mesh3d> const& meshes, 
		vector<Material3d> const& materials,
		BNode<BoneNodeData> const& boneTree,
		Animation3d const& defaultAnimation)
		: path(path),
		meshes(meshes),
		materials(materials),
		boneTree(boneTree) {

		initHitMesh(this->meshes);
		nameToAnimation.emplace(defaultAnimation.getName(), defaultAnimation);
	}

	string Model3d::getPath() const {
		return path;
	}

	vector<Mesh3d>& Model3d::getMeshes() {
		return meshes;
	}

	shared_ptr<Mesh3d> Model3d::getHitMesh() {
		return hitMesh;
	}

	vector<Material3d>& Model3d::getMaterials() {
		return materials;
	}

	BNode<BoneNodeData>& Model3d::getBoneTree() {
		return boneTree;
	}

	Material3d& Model3d::getMaterialBy(Mesh3d const& mesh) {
		auto id = mesh.getMaterialId();
		try {
			return materials.at(id);
		} catch(out_of_range&){
			throw LogicException(EXCEPTION_INFO, "mesh " + mesh.getName() + " has no material");
		}
	}

	Texture2d& Model3d::getTextureBy(Mesh3d const& mesh) {
		return getMaterialBy(mesh).getTexture();
	}

	string Model3d::getUniqueMeshName(Mesh3d const& mesh) {
		return path + mesh.getName();
	}
	
	Animation3d& Model3d::getAnimationBy(std::string name) {
		try {
			return nameToAnimation.at(name);
		} catch(out_of_range&) {
			throw AssetException(EXCEPTION_INFO, name, "no such animation");
		}
	}
	
	void Model3d::addAnimation(Animation3d const& anim) {
		string name = anim.getName();
		auto res = nameToAnimation.emplace(name, anim);
		if(!res.second)
			throw AssetException(EXCEPTION_INFO, name, "has same animation");
	}


	void Model3d::initHitMesh(vector<Mesh3d>& meshes) {
		auto it = find_if(begin(meshes), end(meshes), [](Mesh3d& mesh) {
			return mesh.getName() == "HitMesh";
		});

		if(it != end(meshes)) {
			hitMesh = std::make_shared<Mesh3d>(*it);
			meshes.erase(it);
		}
	}

}

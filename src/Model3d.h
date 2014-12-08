#pragma once
#include <vector>
#include "Mesh3d.h"
#include "bone\BNode.h"
#include "Animation3d.h"
#include "bone\BoneNodeData.h"
#include <unordered_map>
#include "Material3d.h"

namespace br {
	class Model3d {
	public:
		Model3d(std::string path,
			const std::vector<Mesh3d>& meshes,
			const std::vector<Material3d>& materials, 
			const BNode<BoneNodeData>& boneTree,
			const Animation3d& defaultAnimation);

		std::string getPath() const { return path; }
		std::vector<Mesh3d>& getMeshes() { return meshes; }
		std::vector<Material3d>& getMaterials() { return materials; }
	
		Animation3d& getAnimationBy(std::string name = Animation3d::DEFAULT_ANIMATION_NAME);
		void addAnimation(Animation3d&);
	
		BNode<BoneNodeData>& getBoneTree(){ return boneTree; }
	
		operator std::string() const;
	
		std::string getUniqueMeshName(const Mesh3d&);
	
		glm::mat4& getGlobalInverseTransform() { return globalInverseTransform; }
		void setGlobalInverseTransform(glm::mat4& val) { globalInverseTransform = val; }
	
	private:
		std::string path;
		
		std::vector<Mesh3d> meshes;
		std::vector<Material3d> materials;
	
		BNode<BoneNodeData> boneTree;
	
		std::unordered_map<std::string, Animation3d> nameToAnimation;
	
		glm::mat4 globalInverseTransform;
	};
}


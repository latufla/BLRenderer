#pragma once
#include <vector>
#include <memory>
#include "Mesh3d.h"
#include "Material3d.h"
#include "tree\TNode.h"
#include "Animation3d.h"
#include "tree\BoneNodeData.h"
#include <unordered_map>

class Model3d {
public:
	Model3d(std::string, std::vector<Mesh3d>&&, std::vector<std::string>&&, TNode<BoneNodeData>&&, Animation3d&&);

	std::string getName() const { return name; }
	std::vector<Mesh3d>& getMeshes() { return meshes; }
	std::vector<Material3d>& getMaterials() { return materials; }

	Animation3d& getAnimation(std::string = Animation3d::DEFAULT_ANIMATION_NAME);
	bool addAnimation(Animation3d&&);

	TNode<BoneNodeData>& getBoneTree(){ return boneTree; }

	operator std::string() const;

	std::string getUniqueMeshName(const Mesh3d&);

	glm::mat4& getGlobalInverseTransform() { return globalInverseTransform; }
	void setGlobalInverseTransform(glm::mat4& val) { globalInverseTransform = val; }

private:
	std::string name;
	
	std::vector<Mesh3d> meshes;
	std::vector<Material3d> materials;

	TNode<BoneNodeData> boneTree;

	std::unordered_map<std::string, Animation3d> nameToAnimation;

	glm::mat4 globalInverseTransform;
};


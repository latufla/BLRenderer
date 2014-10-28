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
	// TODO: move maybe, now it copies inside
	Model3d(std::string, const std::vector<Mesh3d>&, const std::vector<std::string>&, const TNode<BoneNodeData>&, const Animation3d&);

	std::string getName() const { return name; }
	std::vector<Mesh3d>& getMeshes() { return meshes; }
	std::vector<Material3d>& getMaterials() { return materials; }

	Animation3d& getAnimation(std::string = Animation3d::DEFAULT_ANIMATION_NAME);

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


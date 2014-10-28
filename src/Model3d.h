#pragma once
#include <vector>
#include <memory>
#include "Mesh3d.h"
#include "Material3d.h"
#include "tree\TNode.h"
#include "Animation3d.h"
#include "tree\BoneNodeData.h"

class Model3d {
public:
	Model3d(std::string, const std::vector<Mesh3d>&, const std::vector<std::string>&, TNode<BoneNodeData>&, std::shared_ptr<Animation3d>);

	std::string getName() const { return name; }
	std::vector<Mesh3d>& getMeshes() { return meshes; }
	std::vector<Material3d>& getMaterials() { return materials; }

	std::shared_ptr<Animation3d> getAnimation() const { return animation; }

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
	std::shared_ptr<Animation3d> animation;

	glm::mat4 globalInverseTransform;
};


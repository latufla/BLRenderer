#pragma once
#include <map>
#include <memory>
#include <iostream>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Model3d.h"
#include "tree/BoneNodeData.h"

class Model3dLoader {
public:
	Model3dLoader();
	~Model3dLoader();

	bool load(std::string, std::string);

	std::shared_ptr<Model3d> getModel3d(std::string);
	
private:
	std::map<std::string, std::shared_ptr<Model3d>> models;

	void forEachNode(aiNode*, void(*)(aiNode*, int), int = 0);
	void forEachNode(const aiScene*, aiNode*, void(*)(const aiScene*, aiNode*, std::vector<Mesh3d>&), std::vector<Mesh3d>&);
	
	TNode<BoneNodeData> collectBones(const aiScene*, std::string = "Armature");
	TNode<BoneNodeData> parseBones(const aiNode*);

	void collectBoneWeightsAndOffsets(const aiScene*, TNode<BoneNodeData>&, std::vector<Mesh3d>&);

	// TODO: collect all 
	std::shared_ptr<Animation3d> collectAnimations(const aiScene*, TNode<BoneNodeData>&);
};

void loadMeshes(const aiScene*, aiNode*, std::vector<Mesh3d>&);
void printNode(aiNode*, int = 0);

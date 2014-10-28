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
	Model3dLoader() {};
	~Model3dLoader() {};

	bool loadModel(std::string, std::string);

	std::shared_ptr<Model3d> getModel(std::string);
	
private:
	std::map<std::string, std::shared_ptr<Model3d>> models;
	
	static const uint8_t TRIANGLE_FACE_TYPE;
	std::vector<Mesh3d> collectMeshes(const aiScene*);
	void parseMeshes(const aiNode*, aiMesh**, std::vector<Mesh3d>&);

	std::vector<std::string> collectMaterials(const aiScene*, std::string);

	static const std::string BONES_ROOT_NODE;
	TNode<BoneNodeData> collectBones(const aiScene*, std::string = BONES_ROOT_NODE);
	TNode<BoneNodeData> parseBones(const aiNode*);

	void collectBoneWeightsAndOffsets(const aiScene*, TNode<BoneNodeData>&, std::vector<Mesh3d>&);

	// TODO: collect all 
	std::shared_ptr<Animation3d> collectAnimations(const aiScene*, TNode<BoneNodeData>&);
};
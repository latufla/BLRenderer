#pragma once
#include <map>
#include <iostream>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Model3d.h"
#include "bone/BoneNodeData.h"
#include "Material3d.h"

class Model3dLoader {
public:
	Model3dLoader() {};
	~Model3dLoader() {};

	bool loadModel(std::string, std::string);

	Model3d& getModel(std::string);
	bool attachAnimation(std::string, std::string, std::string);

private:
	std::map<std::string, Model3d> models;

	Assimp::Importer importer;

	static const uint8_t TRIANGLE_FACE_TYPE;
	std::vector<Mesh3d> collectMeshes(const aiScene*);
	void parseMeshes(const aiNode*, aiMesh**, std::vector<Mesh3d>&);

	std::vector<Material3d> collectMaterials(const aiScene*, std::string);

	static const std::string BONES_ROOT_NODE;
	BNode<BoneNodeData> collectBones(const aiScene*, std::string = BONES_ROOT_NODE);
	BNode<BoneNodeData> parseBones(const aiNode*);

	void collectBoneWeightsAndOffsets(const aiScene*, BNode<BoneNodeData>&, std::vector<Mesh3d>&);

	Animation3d collectAnimation(const aiScene*, BNode<BoneNodeData>&, std::string name);
};
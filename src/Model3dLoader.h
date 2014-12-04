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

namespace br {
	class Model3dLoader {
	public:
		Model3dLoader() {};
		~Model3dLoader() = default;
	
		bool loadModel(std::string directory, std::string name);
	
		Model3d& getModel(std::string name);
		bool attachAnimation(std::string modelName, std::string animPath, std::string animName);
	
	private:
		std::map<std::string, Model3d> pathToModel;
	
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
}
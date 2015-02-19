#pragma once
#include <map>
#include <iostream>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Model3d.h"
#include "utils/bones/BoneNodeData.h"
#include "Material3d.h"
#include "processors/text/FontLoader.h"

namespace br {
	class AssetLoader {
	public:
		AssetLoader() {};
		~AssetLoader() = default;
	
		void loadModel(std::string pathAsKey, std::string textureDirectory);

		Model3d& getModelBy(std::string path);
		void attachAnimation(std::string toModel, std::string byNameAsKey, std::string withPath);

		void loadTexture(std::string pathAsKey);
		Texture2d& getTextureBy(std::string path);

		void loadFont(std::string path, std::string name, uint8_t size);
		Font getFontBy(std::string name, uint8_t size);

	private:
		std::map<std::string, Model3d> pathToModel;
		std::map<std::string, Texture2d> pathToTexture;

		FontLoader fontLoader;

		Assimp::Importer importer;
		
		static const uint8_t TRIANGLE_FACE_TYPE;
		std::vector<Mesh3d> collectMeshes(const aiScene*);
		void parseMeshes(const aiNode*, aiMesh**, std::vector<Mesh3d>&);
	
		std::vector<Material3d> collectMaterials(const aiScene*, std::string);
	
		static const std::string BONES_ROOT_NODE;
		BNode<BoneNodeData> collectBones(const aiNode*, std::string);
		BNode<BoneNodeData> parseBones(const aiNode*);
	
		void collectBoneWeightsAndOffsets(const aiScene*, BNode<BoneNodeData>&, std::vector<Mesh3d>&);
	
		Animation3d collectAnimation(const aiScene*, BNode<BoneNodeData>&, std::string, std::string);
	};
}
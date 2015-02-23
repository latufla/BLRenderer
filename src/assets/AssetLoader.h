#pragma once
#include <unordered_map>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "../utils/bones/BoneNodeData.h"
#include "../processors/text/FontLoader.h"
#include "interfaces/IAssetLoader.h"

namespace br {
	class AssetLoader : public IAssetLoader {
	public:
		AssetLoader() = default;
		virtual ~AssetLoader();
	
		virtual void loadModel(std::string pathAsKey, std::string textureDirectory) override;
		virtual std::shared_ptr<IModel3d> getModelBy(std::string pathAsKey) override;

		virtual void loadAnimation(std::string toModel, std::string byNameAsKey, std::string withPath) override;

		virtual void loadTexture(std::string pathAsKey) override;
		virtual Texture2d& getTextureBy(std::string pathAsKey) override;

		virtual void loadFont(std::string path, std::string name, uint8_t size) override;
		virtual Font& getFontBy(std::string name, uint8_t size) override;

	private:
		std::unordered_map<std::string, std::shared_ptr<IModel3d>> pathToModel;
		std::unordered_map<std::string, Texture2d> pathToTexture;

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
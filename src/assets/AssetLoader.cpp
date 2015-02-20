#include "../utils/SharedHeaders.h"
#include "AssetLoader.h"
#include "../utils/bones/BoneNodeData.h"
#include "../utils/Util.h"
#include <unordered_map>
#include "../exceptions/Exception.h"

using std::string;
using std::to_string;
using std::map;
using std::vector;
using std::unordered_map;

namespace br {
	const uint8_t AssetLoader::TRIANGLE_FACE_TYPE = 3;
	const std::string AssetLoader::BONES_ROOT_NODE = "Armature";
	
	void AssetLoader::loadModel(string pathAsKey, string textureDirectory) {
		if(pathToModel.find(pathAsKey) != pathToModel.cend())
			throw AssetException(EXCEPTION_INFO, pathAsKey, "has same model");
		
		
		const aiScene* modelAi = importer.ReadFile(pathAsKey, aiProcess_FlipUVs);
		if (!modelAi)
			throw AssetException(EXCEPTION_INFO, pathAsKey,"invalid collada model");
		
		
		vector<Mesh3d> meshes = collectMeshes(modelAi);
		if (meshes.empty())
			throw AssetException(EXCEPTION_INFO, pathAsKey, "no meshes");

		
		vector<Material3d> materials = collectMaterials(modelAi, textureDirectory);
		if(materials.empty())
			throw AssetException(EXCEPTION_INFO, pathAsKey, "no materials");
	
		
		aiNode* root = modelAi->mRootNode;
		const aiNode* bonesRoot = root->FindNode(BONES_ROOT_NODE.c_str());
		BNode<BoneNodeData> bones = collectBones(bonesRoot, pathAsKey);
		collectBoneWeightsAndOffsets(modelAi, bones, meshes);
	
		
		Animation3d defaultAnimation = collectAnimation(modelAi, bones, Animation3d::DEFAULT_ANIMATION_NAME, pathAsKey);
		
		aiNode* rootAi = modelAi->mRootNode;
		auto glTrans = Util::assimpToGlm(rootAi->mTransformation);
		
		Model3d model{pathAsKey, meshes, materials, bones, defaultAnimation};
		model.setGlobalInverseTransform(glTrans);

		pathToModel.emplace(pathAsKey, model);
	}
	
	void AssetLoader::attachAnimation(string toModel, string byNameAsKey, string withPath) {
		const aiScene* animationAi = importer.ReadFile(withPath, aiProcess_Triangulate | aiProcess_FlipUVs);
		if (!animationAi)
			throw AssetException(EXCEPTION_INFO, withPath, "invalid collada model");

		Model3d& model = getModelBy(toModel);
		Animation3d animation = collectAnimation(animationAi, model.getBoneTree(), byNameAsKey, toModel);
		model.addAnimation(animation);
	}
	
	Model3d& AssetLoader::getModelBy(string path) {
		try {
			return pathToModel.at(path);
		} catch (std::out_of_range&){
			throw AssetException(EXCEPTION_INFO, path, "can`t get model by path");
		}
	}
	
	
	vector<Mesh3d> AssetLoader::collectMeshes(const aiScene* modelAi) {
		vector<Mesh3d> outMeshes;
		parseMeshes(modelAi->mRootNode, modelAi->mMeshes, outMeshes);
		return outMeshes;
	}
	
	void AssetLoader::parseMeshes(const aiNode* rNodeAi, aiMesh** meshesAi, std::vector<Mesh3d>& outMeshes) {
		vector<Vertex3d> vertices;
		vector<uint16_t> indices;
		
		aiVector3D defaultTexCoords{0.0f, 0.0f, 0.0f};
		unsigned int* meshAiIds = rNodeAi->mMeshes;
		uint32_t nMeshesAi = rNodeAi->mNumMeshes;
		for (uint32_t i = 0; i < nMeshesAi; i++) {
			unsigned int meshAiId = meshAiIds[i];
			aiMesh* meshAi = meshesAi[meshAiId];
			
			aiVector3D* verticesAi = meshAi->mVertices;
			aiVector3D* textureCoordsAi = meshAi->mTextureCoords[0];
			uint32_t nVerticesAi = meshAi->mNumVertices;
			for (uint32_t j = 0; j < nVerticesAi; j++) {
				aiVector3D& v = verticesAi[j];
				aiVector3D& t = textureCoordsAi ? textureCoordsAi[j] : defaultTexCoords;
				Vertex3d vertex{
					v.x, v.y, v.z,
					t.x, t.y,
					{0, 0, 0, 0},
					{0.0, 0.0, 0.0, 0.0}
				};
				vertices.push_back(vertex);
			}
	
			uint32_t nFacesAi = meshAi->mNumFaces;
			for (uint32_t j = 0; j < nFacesAi; j++) {
				aiFace& faceAi = meshAi->mFaces[j];
				if (faceAi.mNumIndices != TRIANGLE_FACE_TYPE)
					continue;
	
				unsigned int* indicesAi = faceAi.mIndices;
				uint32_t nIndicesAi = faceAi.mNumIndices;
				for (uint32_t k = 0; k < nIndicesAi; k++) {
					uint32_t vertexId = indicesAi[k];
					indices.push_back(vertexId);
				}
			}
	
			Mesh3d myMesh(meshAi->mName.C_Str(), vertices, indices, meshAi->mMaterialIndex);
			outMeshes.push_back(myMesh);
		}
	
		uint32_t nNodesAi = rNodeAi->mNumChildren;
		for (uint32_t i = 0; i < nNodesAi; ++i) {
			parseMeshes(rNodeAi->mChildren[i], meshesAi, outMeshes);
		}
	}
	
	std::vector<Material3d> AssetLoader::collectMaterials(const aiScene* modelAi, std::string dir) {
		uint32_t nMaterialsAi = modelAi->mNumMaterials;
		vector<Material3d> materials;
		for (uint32_t i = 0; i < nMaterialsAi; i++) {
			aiMaterial* materialAi = modelAi->mMaterials[i];
	
			aiString textureAi;
			materialAi->GetTexture(aiTextureType_DIFFUSE, 0, &textureAi);
			Texture2d texture = Util::loadTexture(dir + textureAi.C_Str());

			aiColor4D emissionAi;
			aiGetMaterialColor(materialAi, AI_MATKEY_COLOR_EMISSIVE, &emissionAi);
	
			aiColor4D ambientAi;
			aiGetMaterialColor(materialAi, AI_MATKEY_COLOR_AMBIENT, &ambientAi);
	
			aiColor4D diffuseAi;
			aiGetMaterialColor(materialAi, AI_MATKEY_COLOR_DIFFUSE, &diffuseAi);
	
			aiColor4D specularAi;
			aiGetMaterialColor(materialAi, AI_MATKEY_COLOR_SPECULAR, &specularAi);
	
			float shininess;
			uint32_t max = 1;
			aiGetMaterialFloatArray(materialAi, AI_MATKEY_SHININESS, &shininess, &max);
	
			float indexOfRefraction;
			max = 1;
			aiGetMaterialFloatArray(materialAi, AI_MATKEY_REFRACTI, &indexOfRefraction, &max);
	
			int32_t twoSidedAi;
			max = 1;
			aiGetMaterialIntegerArray(materialAi, AI_MATKEY_TWOSIDED, &twoSidedAi, &max);
			bool twoSided = (twoSidedAi != 0);

			Material3d mat{
				texture,
				Util::assimpToGlm(emissionAi),
				Util::assimpToGlm(ambientAi),
				Util::assimpToGlm(diffuseAi),
				Util::assimpToGlm(specularAi),
				shininess, indexOfRefraction, twoSided
			};
			materials.push_back(mat);
		}
		return materials;
	}
	
	
	BNode<BoneNodeData> AssetLoader::collectBones(const aiNode* bonesRoot, string modelPath) {
		if(!bonesRoot || !bonesRoot->mNumChildren)
			throw AssetException(EXCEPTION_INFO, modelPath, "no bones");

		aiNode* rootBone = bonesRoot->mChildren[0];	
		BNode<BoneNodeData> boneTree = parseBones(rootBone);
		
		BNode<BoneNodeData>::ArrangeIds(boneTree);
		
		return boneTree;
	}
	
	BNode<BoneNodeData> AssetLoader::parseBones(const aiNode* node) {
		glm::mat4 transform = Util::assimpToGlm(node->mTransformation);
		BNode<BoneNodeData> bones{ 0, node->mName.C_Str(), BoneNodeData(transform) };
		
		uint32_t nNodes = node->mNumChildren;
		if (nNodes == 0)
			return bones;
	
		for (uint32_t i = 0; i < nNodes; ++i) {
			bones.addChild(parseBones(node->mChildren[i]));
		}
		return bones;
	}
	
	Animation3d AssetLoader::collectAnimation(const aiScene* scene, BNode<BoneNodeData>& allBones, string animName, string modelPath) {
		uint32_t nAnims = scene->mNumAnimations;
		if(!nAnims)
			throw AssetException(EXCEPTION_INFO, modelPath, "no animation " + animName);

		aiAnimation* anim = scene->mAnimations[0]; // one scene - one animation
		uint32_t nChannels = anim->mNumChannels;
		if (!nChannels)
			throw AssetException(EXCEPTION_INFO, modelPath, "empty animation " + animName);

		unordered_map<uint32_t, Animation3d::BoneAnimation> idToBoneAnimation;
		for (uint32_t i = 0; i < nChannels; ++i) {
			aiNodeAnim* animNode = anim->mChannels[i];
			string nName = animNode->mNodeName.C_Str();
			
			BNode<BoneNodeData>* myBone = BNode<BoneNodeData>::FindNode(allBones, nName);
			if (!myBone)
				continue;
	
			vector<Animation3d::Vec3Key> positions;
			uint32_t nPositions = animNode->mNumPositionKeys;
			for (uint32_t j = 0; j < nPositions; ++j) {
				aiVectorKey& posKey = animNode->mPositionKeys[j];
				Animation3d::Vec3Key myPosKey{ posKey.mTime, Util::assimpToGlm(posKey.mValue) };
				positions.push_back(myPosKey);			
			}
	
			vector<Animation3d::Mat4Key> rotations;
			uint32_t nRotations = animNode->mNumRotationKeys;
			for (uint32_t j = 0; j < nRotations; ++j) {
				aiQuatKey& rotKey = animNode->mRotationKeys[j];
				aiMatrix4x4 rotMtx(rotKey.mValue.GetMatrix());
				Animation3d::Mat4Key myRotKey{ rotKey.mTime, Util::assimpToGlm(rotMtx) };
				rotations.push_back(myRotKey);
			}
	
			vector<Animation3d::Vec3Key> scalings;
			uint32_t nScalings = animNode->mNumScalingKeys;
			for (uint32_t j = 0; j < nScalings; ++j) {
				aiVectorKey& scaleKey = animNode->mScalingKeys[j];
				Animation3d::Vec3Key myScaleKey{ scaleKey.mTime, Util::assimpToGlm(scaleKey.mValue) };
				scalings.push_back(myScaleKey);
			}
	
			uint32_t boneId = myBone->getId();
			Animation3d::BoneAnimation myBoneAnimation{ boneId, myBone->getName(), positions, rotations, scalings };
			idToBoneAnimation.emplace(boneId, myBoneAnimation);
		}
	
		return{ animName, anim->mDuration, anim->mTicksPerSecond, idToBoneAnimation };
	}
	
	void AssetLoader::collectBoneWeightsAndOffsets(const aiScene* scene, BNode<BoneNodeData>& boneTree, vector<Mesh3d>& meshes) {
		std::map<string, aiMesh*> nameToMeshAi;
		uint32_t nMeshAi = scene->mNumMeshes;
		for (uint32_t i = 0; i < nMeshAi; ++i) {
			aiMesh* meshAi = scene->mMeshes[i];
			nameToMeshAi[meshAi->mName.C_Str()] = meshAi;
		}
	
		uint32_t nMesh = meshes.size();
		for (uint32_t i = 0; i < nMesh; ++i) {
			Mesh3d& myMesh = meshes[i];
			aiMesh* meshAi = nameToMeshAi[myMesh.getName()];
	
			vector<Vertex3d>& myVertices = myMesh.getVertices();
			uint32_t nBonesAi = meshAi->mNumBones;
			for (uint32_t j = 0; j < nBonesAi; ++j) {
				aiBone* boneAi = meshAi->mBones[j];
				BNode<BoneNodeData>* myBone = BNode<BoneNodeData>::FindNode(boneTree, boneAi->mName.C_Str());
				uint32_t myBoneId = myBone->getId();
				myMesh.setBoneOffset(myBoneId, Util::assimpToGlm(boneAi->mOffsetMatrix));
				
				uint32_t nNumWeightsAi = boneAi->mNumWeights;
				for (uint32_t k = 0; k < nNumWeightsAi; ++k) {
					aiVertexWeight& weightAi = boneAi->mWeights[k];
					myMesh.setVertexBoneInfo(weightAi.mVertexId, myBoneId, weightAi.mWeight);
				}
			}
		}
	}

	void AssetLoader::loadTexture(string pathAsKey) {
		if(pathToTexture.find(pathAsKey) != pathToTexture.cend())
			throw AssetException(EXCEPTION_INFO, pathAsKey, "has same texture");

		Texture2d texture = Util::loadTexture(pathAsKey);
		pathToTexture.emplace(pathAsKey, texture);
	}

	Texture2d& AssetLoader::getTextureBy(string path) {
		try {
			return pathToTexture.at(path);
		} catch(std::out_of_range&) {
			throw AssetException(EXCEPTION_INFO, path, "can`t get texture");
		}
	}

	void AssetLoader::loadFont(string path, string name, uint8_t size) {
		fontLoader.loadFont(path, name, size);
	}

	Font AssetLoader::getFontBy(string name, uint8_t size) {
		return fontLoader.getFontBy(name, size);
	}

}
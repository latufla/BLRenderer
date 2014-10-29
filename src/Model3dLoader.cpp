#include "stdafx.h"
#include "Model3dLoader.h"
#include "tree\BoneNodeData.h"
#include "Utils.h"

using std::string;
using std::to_string;
using std::map;
using std::vector;

using std::shared_ptr;
using std::make_shared;

const uint8_t Model3dLoader::TRIANGLE_FACE_TYPE = 3;
const std::string Model3dLoader::BONES_ROOT_NODE = "Armature";


bool Model3dLoader::loadModel(string dir, string name) {
	string path = dir + name;
	
	const aiScene* modelAi = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
	if (!modelAi)
		throw std::exception("Model3dLoader::loadModel invalid collada model");
		
	vector<Mesh3d> meshes = collectMeshes(modelAi);
	if (meshes.empty())
		throw std::exception("Model3dLoader::loadModel no meshes");

	vector<Material3d> materials = collectMaterials(modelAi, dir);
	if (materials.empty())
		throw std::exception("Model3dLoader::loadModel no materials");

	TNode<BoneNodeData> bones = collectBones(modelAi);
	collectBoneWeightsAndOffsets(modelAi, bones, meshes);

	Animation3d defaultAnimation = collectAnimation(modelAi, bones, Animation3d::DEFAULT_ANIMATION_NAME);
	
	aiNode* rootAi = modelAi->mRootNode;
	auto glTrans = Utils::assimpToGlmMatrix(rootAi->mTransformation);
	
	Model3d model{ path, meshes, materials, bones, defaultAnimation };
	model.setGlobalInverseTransform(glTrans);
	models.emplace(path, model);
	
	return true;
}

bool Model3dLoader::attachAnimation(string modelName, string animPath, string animName) {
	const aiScene* animationAi = importer.ReadFile(animPath, aiProcess_Triangulate | aiProcess_FlipUVs);
	if (!animationAi)
		throw std::exception("Model3dLoader::attachAnimation invalid collada model");

	Model3d& model = getModel(modelName);
	Animation3d defaultAnimation = collectAnimation(animationAi, model.getBoneTree(), animName);
	model.addAnimation(defaultAnimation);

	return true;
}

Model3d& Model3dLoader::getModel(string name) {	
	return models.at(name);
}


vector<Mesh3d> Model3dLoader::collectMeshes(const aiScene* modelAi) {
	vector<Mesh3d> outMeshes;
	parseMeshes(modelAi->mRootNode, modelAi->mMeshes, outMeshes);
	return outMeshes;
}

void Model3dLoader::parseMeshes(const aiNode* rNodeAi, aiMesh** meshesAi, std::vector<Mesh3d>& outMeshes) {
	vector<Vertex3d> vertices;
	vector<uint16_t> indices;
	
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
			aiVector3D& t = textureCoordsAi[j];
			vertices.push_back({
				v.x, v.y, v.z,
				t.x, t.y,
				{ 0, 0, 0, 0 },
				{ 0.0, 0.0, 0.0, 0.0 }
			});
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

std::vector<Material3d> Model3dLoader::collectMaterials(const aiScene* modelAi, std::string dir) {
	uint32_t nMaterialsAi = modelAi->mNumMaterials;
	vector<Material3d> materials;
	for (uint32_t i = 0; i < nMaterialsAi; i++) {
		aiMaterial* materialAi = modelAi->mMaterials[i];

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
		bool twoSided = twoSidedAi & 1;

		aiString textureAi;
		materialAi->GetTexture(aiTextureType_DIFFUSE, 0, &textureAi);
		Texture2d texture;
		if (!Utils::loadTexture(dir + textureAi.C_Str(), texture))
			throw std::exception("Model3dLoader::collectMaterials can`t load texture");

		auto converter = Utils::assimpToGlmVector4d;		
		Material3d mat{
			texture,
			converter(emissionAi),
			converter(ambientAi),
			converter(diffuseAi),
			converter(specularAi),
			shininess, indexOfRefraction, twoSided
		};
		materials.push_back(mat);
	}
	return materials;
}


TNode<BoneNodeData> Model3dLoader::collectBones(const aiScene* scene, string bonesRoot) {
	aiNode* root = scene->mRootNode;
	aiNode* armature = root->FindNode(bonesRoot.c_str());
	if (!armature || !armature->mNumChildren)
		throw std::exception("Model3dLoader::collectBones: no bones");

	aiNode* rootBone = armature->mChildren[0];	
	TNode<BoneNodeData> boneTree = parseBones(rootBone);

	uint32_t firstId = 0; 
	TNode<BoneNodeData>::ArrangeIds(boneTree, firstId);
	
	return boneTree;
}

TNode<BoneNodeData> Model3dLoader::parseBones(const aiNode* node) {
	glm::mat4 transform = Utils::assimpToGlmMatrix(node->mTransformation);
	TNode<BoneNodeData> bones{ 0, node->mName.C_Str(), BoneNodeData(transform) };
	
	uint32_t nNodes = node->mNumChildren;
	if (nNodes == 0)
		return bones;

	for (uint32_t i = 0; i < nNodes; ++i) {
		bones.addChild(parseBones(node->mChildren[i]));
	}
	return bones;
}

Animation3d Model3dLoader::collectAnimation(const aiScene* scene, TNode<BoneNodeData>& allBones, string name) {
	uint32_t nAnims = scene->mNumAnimations;
	if (!nAnims)
		throw std::exception("Model3dLoader::collectAnimation: no animation");

	aiAnimation* anim = scene->mAnimations[0];

	uint32_t nChannels = anim->mNumChannels;
	if (!nChannels)
		throw std::exception("Model3dLoader::collectAnimation: empty animation");

	// TODO: drop not bones, wonder am i right
	vector<BoneAnimation> boneAnimations;
	for (uint32_t i = 0; i < nChannels; ++i) {
		aiNodeAnim* animNode = anim->mChannels[i];
		string nName = animNode->mNodeName.C_Str();
		
		bool found = false;
		TNode<BoneNodeData>* myBone = TNode<BoneNodeData>::FindNode(allBones, nName, found);
		if (!myBone)
			continue;

		vector<Vec3Key> positions;
		uint32_t nPositions = animNode->mNumPositionKeys;
		for (uint32_t j = 0; j < nPositions; ++j) {
			aiVectorKey& posKey = animNode->mPositionKeys[j];
			Vec3Key myPosKey{ posKey.mTime, Utils::assimpToGlmVector3d(posKey.mValue) };
			positions.push_back(myPosKey);			
		}

		vector<Mat4Key> rotations;
		uint32_t nRotations = animNode->mNumRotationKeys;
		for (uint32_t j = 0; j < nRotations; ++j) {
			aiQuatKey& rotKey = animNode->mRotationKeys[j];
			aiMatrix4x4 rotMtx(rotKey.mValue.GetMatrix());
			Mat4Key myRotKey{ rotKey.mTime, Utils::assimpToGlmMatrix(rotMtx) };
			rotations.push_back(myRotKey);
		}

		vector<Vec3Key> scalings;
		uint32_t nScalings = animNode->mNumScalingKeys;
		for (uint32_t j = 0; j < nScalings; ++j) {
			aiVectorKey& scaleKey = animNode->mScalingKeys[j];
			Vec3Key myScaleKey{ scaleKey.mTime, Utils::assimpToGlmVector3d(scaleKey.mValue) };
			scalings.push_back(myScaleKey);
		}
		BoneAnimation myBoneAnimation{ myBone->getId(), myBone->getName(), positions, rotations, scalings };
		boneAnimations.push_back(myBoneAnimation);
	}

	return{ name, anim->mDuration, anim->mTicksPerSecond, boneAnimations };
}

void Model3dLoader::collectBoneWeightsAndOffsets(const aiScene* scene, TNode<BoneNodeData>& boneTree, vector<Mesh3d>& meshes) {
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
			bool found = false;
			TNode<BoneNodeData>* myBone = TNode<BoneNodeData>::FindNode(boneTree, boneAi->mName.C_Str(), found);
			uint32_t myBoneId = myBone->getId();
			myMesh.setBoneOffset(myBoneId, Utils::assimpToGlmMatrix(boneAi->mOffsetMatrix));
			
			uint32_t nNumWeightsAi = boneAi->mNumWeights;
			for (uint32_t k = 0; k < nNumWeightsAi; ++k) {
				aiVertexWeight& weightAi = boneAi->mWeights[k];
				myMesh.setVertexBoneInfo(weightAi.mVertexId, myBoneId, weightAi.mWeight);
			}
		}
	}
}











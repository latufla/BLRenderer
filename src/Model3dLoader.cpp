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
using std::move;


const uint8_t Model3dLoader::TRIANGLE_FACE_TYPE = 3;
const std::string Model3dLoader::BONES_ROOT_NODE = "Armature";


bool Model3dLoader::loadModel(string dir, string name) {
	string path = dir + name;
	
	Assimp::Importer importer;
	const aiScene* modelAi = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
	if (!modelAi)
		throw std::exception("Model3dLoader::loadModel invalid collada model");
		
	vector<Mesh3d> meshes = collectMeshes(modelAi);
	if (meshes.empty())
		throw std::exception("Model3dLoader::loadModel no meshes");

	vector<string> textures = collectMaterials(modelAi, dir);
	if (textures.empty())
		throw std::exception("Model3dLoader::loadModel no textures");
	
	TNode<BoneNodeData> bones = collectBones(modelAi);
	collectBoneWeightsAndOffsets(modelAi, bones, meshes);

	shared_ptr<Animation3d> animation = collectAnimations(modelAi, bones);
	
	aiNode* rootAi = modelAi->mRootNode;
	auto glTrans = Utils::assimpToGlmMatrix(rootAi->mTransformation);
	
	Model3d model{ path, meshes, textures, bones, animation };
	model.setGlobalInverseTransform(glTrans);
	models.emplace(path, move(model));
	
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


std::vector<string> Model3dLoader::collectMaterials(const aiScene* modelAi, string dir) {
	uint32_t nMaterialsAi = modelAi->mNumMaterials;
	vector<string> materials;
	for (uint32_t i = 0; i < nMaterialsAi; i++) {
		aiString textureAi;
		modelAi->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, 0, &textureAi);
		materials.push_back(dir + textureAi.C_Str());
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

shared_ptr<Animation3d> Model3dLoader::collectAnimations(const aiScene* scene, TNode<BoneNodeData>& allBones) {
	uint32_t nAnims = scene->mNumAnimations;
	if (!nAnims)
		return nullptr;

	aiAnimation* anim = scene->mAnimations[0];

	uint32_t nChannels = anim->mNumChannels;
	if (!nChannels)
		return nullptr;
		
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
			positions.push_back(move(myPosKey));			
		}

		vector<Mat4Key> rotations;
		uint32_t nRotations = animNode->mNumRotationKeys;
		for (uint32_t j = 0; j < nRotations; ++j) {
			aiQuatKey& rotKey = animNode->mRotationKeys[j];
			aiMatrix4x4 rotMtx(rotKey.mValue.GetMatrix());
			Mat4Key myRotKey{ rotKey.mTime, Utils::assimpToGlmMatrix(rotMtx) };
			rotations.push_back(move(myRotKey));
		}

		vector<Vec3Key> scalings;
		uint32_t nScalings = animNode->mNumScalingKeys;
		for (uint32_t j = 0; j < nScalings; ++j) {
			aiVectorKey& scaleKey = animNode->mScalingKeys[j];
			Vec3Key myScaleKey{ scaleKey.mTime, Utils::assimpToGlmVector3d(scaleKey.mValue) };
			scalings.push_back(move(myScaleKey));
		}
		BoneAnimation myBoneAnimation{ myBone->getId(), myBone->getName(), move(positions), move(rotations), move(scalings) };
		boneAnimations.push_back(move(myBoneAnimation));
	}

	string aName = "default";//anim->mName.C_Str();
	double duration = anim->mDuration;
	double ticksPerSecond = anim->mTicksPerSecond;
	shared_ptr<Animation3d> myAnimation = make_shared<Animation3d>(aName, duration, ticksPerSecond, boneAnimations);
	return myAnimation;
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











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

using std::cout;
using std::endl;

Model3dLoader::Model3dLoader() {
}


Model3dLoader::~Model3dLoader() {
}

bool Model3dLoader::load(string dir, string name) {
	string path = dir + name + ".dae"; // TODO: hardcoded dae
	
	Assimp::Importer importer;
	const aiScene* model3D = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
	if (!model3D)
		return false;
		
	vector<Mesh3d> myMeshes;
	aiNode* root = model3D->mRootNode;
	forEachNode(model3D, root, loadMeshes, myMeshes);
	forEachNode(root, printNode);

	TNode<BoneNodeData> boneTree = collectBones(model3D);
	collectBoneWeightsAndOffsets(model3D, boneTree, myMeshes);

	shared_ptr<Animation3d> animation = collectAnimations(model3D, boneTree);
//	cout << static_cast<string>(*(animation.get()));

	uint32_t nAllTextures = model3D->mNumMaterials;
	vector<string> myTextures;
	for (uint32_t i = 0; i < nAllTextures; i++) {
		aiString texPath;
		model3D->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, 0, &texPath);
		myTextures.push_back(dir + texPath.C_Str());
	}

	shared_ptr<Model3d> myModel = std::make_shared<Model3d>(path, myMeshes, myTextures, boneTree, animation);
	myModel->setGlobalInverseTransform(Utils::assimpToGlmMatrix(root->mTransformation));
	models[path] = myModel;

	return true;
}

void loadMeshes(const aiScene* model, aiNode* node, std::vector<Mesh3d>& outMeshes) {
	aiMesh** meshes = model->mMeshes;
	uint32_t nMeshes = node->mNumMeshes;
	if (nMeshes == 0)
		return;

	vector<Vertex3d> myVertices;
	vector<unsigned short> myIndices;
	unsigned int* meshIds = node->mMeshes;
	for (uint32_t i = 0; i < nMeshes; i++) {
		unsigned int meshId = meshIds[i];
		aiMesh* mesh = meshes[meshId];
		uint32_t nFaces = mesh->mNumFaces;
		aiVector3D* vertices = mesh->mVertices;
		aiVector3D* texCoords = mesh->mTextureCoords[0];
		uint32_t nVertices = mesh->mNumVertices;
		for (uint32_t j = 0; j < nVertices; j++) {
			aiVector3D& v = vertices[j];
			aiVector3D& t = texCoords[j];
			myVertices.push_back({ 
				v.x, v.y, v.z, 
				t.x, t.y,
				{ 0, 0, 0, 0 },
				{0.0, 0.0, 0.0, 0.0}
			});
		}

		for (uint32_t j = 0; j < nFaces; j++) {
			aiFace& face = mesh->mFaces[j];
			if (face.mNumIndices != 3)
				continue;

			unsigned int* indices = face.mIndices;
			uint32_t nIndices = face.mNumIndices;
			for (uint32_t k = 0; k < nIndices; k++) {
				uint32_t vertexId = indices[k];
				myIndices.push_back(vertexId);
			}
		}

		Mesh3d myMesh(mesh->mName.C_Str(), myVertices, myIndices, mesh->mMaterialIndex);
		outMeshes.push_back(myMesh);
	}
}

shared_ptr<Model3d> Model3dLoader::getModel3d(string name) {	
	return models[name];
}


TNode<BoneNodeData> Model3dLoader::collectBones(const aiScene* scene, string bonesRoot) {
	aiNode* root = scene->mRootNode;
	aiNode* armature = root->FindNode(bonesRoot.c_str());
	if (!armature || !armature->mNumChildren)
		return TNode<BoneNodeData>();

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


void Model3dLoader::forEachNode(aiNode* node, void(*eacher)(aiNode*, int), int level) {
	uint32_t nNodes = node->mNumChildren;

	eacher(node, level + 1);
	cout << endl;

	if (nNodes == 0)
		return;

	for (uint32_t i = 0; i < nNodes; ++i) {
		forEachNode(node->mChildren[i], eacher, level + 1);
	}
}

void Model3dLoader::forEachNode(const aiScene* scene, aiNode* node, void(*eacher)(const aiScene*, aiNode*, std::vector<Mesh3d>&), std::vector<Mesh3d>& outMeshes) {
	uint32_t nNodes = node->mNumChildren;

	eacher(scene, node, outMeshes);

	if (nNodes == 0)
		return;

	for (uint32_t i = 0; i < nNodes; ++i) {
		forEachNode(scene, node->mChildren[i], eacher, outMeshes);
	}
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
	int i = 0;
}

void printNode(aiNode* node, int level) {
	for (int i = 0; i < level; ++i) {
		cout << " ";
	}
	cout << "name:" << node->mName.C_Str();
	cout << " nChildren: " << node->mNumChildren;
}









#include "stdafx.h"
#include "Model3dLoader.h"

using std::string;
using std::to_string;
using std::map;
using std::vector;

using std::shared_ptr;

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
	
	uint32_t nAllTextures = model3D->mNumMaterials;
	vector<string> myTextures;
	for (int32_t i = 0; i < nAllTextures; i++) {
		aiString texPath;
		model3D->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, 0, &texPath);
		myTextures.push_back(dir + texPath.C_Str());
	}

	//loadAnimations(model3D);

	shared_ptr<Model3d> myModel = std::make_shared<Model3d>(path, myMeshes, myTextures);
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
	for (int j = 0; j < nMeshes; j++) {
		unsigned int meshId = meshIds[j];
		aiMesh* mesh = meshes[meshId];
		uint32_t nFaces = mesh->mNumFaces;
		aiVector3D* vertices = mesh->mVertices;
		aiVector3D* texCoords = mesh->mTextureCoords[0];
		uint32_t nVertices = mesh->mNumVertices;
		for (int l = 0; l < nVertices; l++) {
			aiVector3D& v = vertices[l];
			aiVector3D& t = texCoords[l];
			myVertices.push_back({ v.x, v.y, v.z, t.x, t.y });
		}

		for (int k = 0; k < nFaces; k++) {
			aiFace& face = mesh->mFaces[k];
			if (face.mNumIndices != 3)
				continue;

			unsigned int* indices = face.mIndices;
			uint32_t nIndices = face.mNumIndices;
			for (int s = 0; s < nIndices; s++) {
				uint32_t vertexId = indices[s];
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

void Model3dLoader::loadAnimations(const aiScene* model) {
	aiNode* root = model->mRootNode;
	uint32_t nNodes = root->mNumChildren;
	for (uint32_t i = 0; i < nNodes; ++i) {
		aiNode* node = root->mChildren[i];
		cout << "nodes: ";
		cout << endl << "name: " << node->mName.C_Str() << endl;

		uint32_t nMeshes = node->mNumMeshes;
		if (nMeshes == 0)
			continue;

		cout << "meshes: " << endl;
		for (uint32_t j = 0; j < nMeshes; ++j) {
			uint32_t meshId = node->mMeshes[j];
			aiMesh* mesh = model->mMeshes[meshId];
			cout << mesh->mName.C_Str() << endl;
			
			uint32_t nBones = mesh->mNumBones;
			if (nBones == 0)
				continue;

			cout << "bones: " << endl;
			for (uint32_t k = 0; k < nBones; ++k) {
				aiBone* bone = mesh->mBones[k];
				cout << bone->mName.C_Str() << endl;
				
// 				uint32_t nWeights = bone->mNumWeights;
// 				if (nWeights == 0)
// 					continue;
// 
// 				cout << "weights: " << endl;
// 				for (uint32_t s = 0; s < nWeights; ++s) {
// 					aiVertexWeight& vWeight = bone->mWeights[s];
// 					cout << vWeight.mVertexId << " -> " << vWeight.mWeight << endl;
// 				}
			}
		}


	}

	cout << endl;

	uint32_t nAnimations = model->mNumAnimations;
	if (nAnimations == 0)
		return;
	
	cout << "animations: " << endl;
	for (uint32_t q = 0; q < nAnimations; ++q) {
		aiAnimation* animation = model->mAnimations[q];
		cout << "name:" << animation->mName.C_Str() << endl;
		cout << "duration:" << animation->mDuration << endl;
		cout << "tps:" << animation->mTicksPerSecond << endl; // WAT
		uint32_t nBones = animation->mNumChannels;
		if (nBones == 0)
			continue;

		cout << "bones: " << endl;
		for (uint32_t w = 0; w < nBones; ++w) {
			aiNodeAnim* bone = animation->mChannels[w];
			cout << bone->mNodeName.C_Str() << " key frames: "
				<< bone->mNumPositionKeys << " "
				<< bone->mNumRotationKeys << " "
				<< bone->mNumScalingKeys << endl;
		}
	}

	cout << endl;
}

void Model3dLoader::forEachNode(aiNode* node, void(*eacher)(aiNode*, int), int level) {
	uint32_t nNodes = node->mNumChildren;

	eacher(node, level + 1);
	cout << endl;

	if (nNodes == 0)
		return;

	for (int i = 0; i < nNodes; ++i) {
		forEachNode(node->mChildren[i], eacher, level + 1);
	}
}

void Model3dLoader::forEachNode(const aiScene* scene, aiNode* node, void(*eacher)(const aiScene*, aiNode*, std::vector<Mesh3d>&), std::vector<Mesh3d>& outMeshes) {
	uint32_t nNodes = node->mNumChildren;

	eacher(scene, node, outMeshes);

	if (nNodes == 0)
		return;

	for (int i = 0; i < nNodes; ++i) {
		forEachNode(scene, node->mChildren[i], eacher, outMeshes);
	}
}

void printNode(aiNode* node, int level) {
	for (int i = 0; i < level; ++i) {
		cout << " ";
	}
	cout << "name:" << node->mName.C_Str();
	cout << " nChildren: " << node->mNumChildren;
}









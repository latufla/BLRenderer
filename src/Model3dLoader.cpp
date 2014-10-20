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

	shared_ptr<Node> boneTree = collectBones(model3D);
//	Node::forEachNode(boneTree, [](Node::NodePtr node, std::uint32_t level){
//		string spacing(level, ' ');
//		cout << spacing << static_cast<string>(*(node.get())) << endl;
//	});
	
	uint32_t nAllTextures = model3D->mNumMaterials;
	vector<string> myTextures;
	for (int32_t i = 0; i < nAllTextures; i++) {
		aiString texPath;
		model3D->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, 0, &texPath);
		myTextures.push_back(dir + texPath.C_Str());
	}

	//loadAnimations(model3D);

	shared_ptr<Model3d> myModel = std::make_shared<Model3d>(path, myMeshes, myTextures, boneTree, nullptr);
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


Node::NodePtr Model3dLoader::collectBones(const aiScene* scene, string bonesRoot) {
	aiNode* root = scene->mRootNode;
	aiNode* armature = root->FindNode(bonesRoot.c_str());
	if (!armature || !armature->mNumChildren)
		return nullptr;

	aiNode* rootBone = armature->mChildren[0];	
	Node::NodePtr boneTree = parseBones(rootBone);

	uint32_t firstId = 0; 
	Node::arrangeIds(boneTree, firstId);
	
	return boneTree;
}

Node::NodePtr Model3dLoader::parseBones(const aiNode* node) {
	glm::mat4 transform = Utils::assimpToGlmMatrix(node->mTransformation);
	Node::NodePtr bones = Node::createNode(0, node->mName.C_Str(), make_shared<BoneNodeData>(transform));
	
	uint32_t nNodes = node->mNumChildren;
	if (nNodes == 0)
		return bones;

	for (int i = 0; i < nNodes; ++i) {
		bones->addChild(parseBones(node->mChildren[i]));
	}
	return bones;
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


// shared_ptr<Node> Model3dLoader::forEachNode(const aiNode* node, void(*eacher)(const aiNode*, shared_ptr<Node>, int), int level) {
// 	uint32_t nNodes = node->mNumChildren;
// 
// 	eacher(node, level + 1);
// 	cout << endl;
// 
// 	if (nNodes == 0)
// 		return;
// 
// 	for (int i = 0; i < nNodes; ++i) {
// 		forEachNode(node->mChildren[i], eacher, level + 1);
// 	}
// 
// }

void printNode(aiNode* node, int level) {
	for (int i = 0; i < level; ++i) {
		cout << " ";
	}
	cout << "name:" << node->mName.C_Str();
	cout << " nChildren: " << node->mNumChildren;
}









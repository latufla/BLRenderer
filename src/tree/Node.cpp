#include "stdafx.h"
#include "Node.h"
#include "BoneNodeData.h"

using std::string;
using std::shared_ptr;
using std::to_string;
using std::make_shared;

Node::Node() {
}

Node::Node(uint32_t id, string name, shared_ptr<NodeData> data) {
	this->id = id;
	this->name = name;
	this->data = data;
}

Node::~Node() {
}

bool Node::addChild(NodePtr node) {
	children.push_back(node);
	return true;
}

Node::operator string() const {
	string res = "{Node id: " + to_string(id) + " name: " + name + " data: " + static_cast<string>(*data.get()) + "}";
	return res;
}


void Node::forEachNode(NodePtr node, void(*eacher)(NodePtr, uint32_t), uint32_t level) {
	uint32_t nextLevel = level + 1;
	eacher(node, nextLevel);

	ChildrenList& chList = node->children;
	if (!chList.size())
		return;
	
	for (auto& i : chList) {
		forEachNode(i, eacher, nextLevel);
	}
}

void Node::arrangeIds(NodePtr node, uint32_t& idHelper) {
	node->setId(idHelper);
	
	uint32_t nNodes = node->children.size();
	if (nNodes == 0)
		return;

	for (uint32_t i = 0; i < nNodes; ++i) {
		arrangeIds(node->children[i], ++idHelper);
	}
}


Node::NodePtr Node::createNode(uint32_t id, string name, shared_ptr<NodeData> data) {
	return make_shared<Node>(id, name, data);
}

Node::NodePtr Node::findNode(NodePtr inTree, string withName, bool& found) {
	if (found)
		return nullptr;

	if (inTree->getName() == withName) {
		found = true;
		return inTree;
	}
	
	uint32_t nNodes = inTree->children.size();
	if (nNodes == 0)
		return nullptr;

	for (uint32_t i = 0; i < nNodes; ++i) {
		NodePtr res = findNode(inTree->children[i], withName, found);
		if (res)
			return res;
	}
	return nullptr;
}

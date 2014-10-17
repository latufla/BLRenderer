#pragma once
#include <vector>
#include <memory>
#include "NodeData.h"

// TODO: this code is VERYx5 slow
class Node : public std::enable_shared_from_this<Node>{
public:
	Node();
	Node(uint32_t, std::string, std::shared_ptr<NodeData>);

	~Node();

	typedef std::shared_ptr<Node> NodePtr;
	typedef std::vector<NodePtr> ChildrenList;

	typedef std::shared_ptr<NodeData> NodeDataPtr;
	
	bool addChild(NodePtr);
	
	uint32_t getId() const { return id; }
	void setId(uint32_t val) { id = val; }

	ChildrenList& getChildren() { return children; }
	void setChildren(const ChildrenList& val) { children = val; }

	operator std::string() const;

	static NodePtr createNode(uint32_t, std::string, NodeDataPtr);
	
	static void forEachNode(NodePtr, void(*)(NodePtr, uint32_t), uint32_t = 0);
	static void arrangeIds(NodePtr, uint32_t&);

private:
	uint32_t id;
	std::string name;
	NodeDataPtr data;

	ChildrenList children;
};


#pragma once
#include <string>
#include <vector>

// runtime tree
template <class T>
class TNode {
public:
	TNode() = default;
	TNode(uint32_t, std::string, T&&);

	~TNode() = default;

	bool addChild(uint32_t, std::string, T&&);
	bool addChild(TNode<T>&&);

	uint32_t getId() const { return id; }
	void setId(uint32_t val) { id = val; }

	std::string getName() const { return name; }
	T& getData() { return data; }
	
	typedef std::vector<TNode<T>> ChildrenList;
	ChildrenList& getChildren() { return children; }
	void setChildren(ChildrenList val) { children = val; }

	operator std::string() const;

	static void ForEachNode(TNode<T>&, void(*)(TNode<T>&, uint32_t), uint32_t = 0);
	static void ForEachNode(TNode<T>&, void*, void*, void(*)(TNode<T>&, void*, void*, uint32_t), uint32_t = 0);
	
	static void ArrangeIds(TNode<T>&, uint32_t&);

	static TNode<T>* FindNode(TNode<T>&, std::string, bool&);

private:
	uint32_t id;
	std::string name;
	T data;
	
	ChildrenList children;
};

template <class T>
TNode<T>::TNode(uint32_t id, std::string name, T&& data) {
	this->id = id;
	this->name = name;
	this->data = data;
}

template <class T>
bool TNode<T>::addChild(uint32_t id, std::string name, T&& data) {
	children.push_back({id, name, (T&&)data});
	return true;
}

template <class T>
bool TNode<T>::addChild(TNode<T>&& child) {
	children.push_back((TNode<T>&&)child);
	return true;
}


template <class T>
TNode<T>::operator std::string() const {
	return "{TNode id: " + std::to_string(id) + " name: " + name + " data: " + std::to_string(data) + "}";
}

template <class T>
void TNode<T>::ForEachNode(TNode<T>& node, void(*eacher)(TNode<T>& node, uint32_t level), uint32_t level) {
	uint32_t nextLevel = level + 1;
	eacher(node, nextLevel);

	auto& children = node.getChildren();
	if (!children.size())
		return;

	for (auto& i : children) {
		ForEachNode(i, eacher, nextLevel);
	}
}

template <class T>
void TNode<T>::ForEachNode(TNode<T>& node, void* inData, void* outData, void(*eacher)(TNode<T>&, void*, void*, uint32_t), uint32_t level) {
	eacher(node, inData, outData, level);

	auto& children = node.getChildren();
	if (!children.size())
		return;

	for (auto& i : children) {
		ForEachNode(i, inData, outData, eacher, level + 1);
	}
}

template <class T>
void TNode<T>::ArrangeIds(TNode<T>& node, uint32_t& idHelper) {
	node.setId(idHelper);

	auto& children = node.getChildren();
	if (!children.size())
		return;

	for (auto& i : children) {
		ArrangeIds(i, ++idHelper);
	}
}

template <class T>
TNode<T>* TNode<T>::FindNode(TNode<T>& node, std::string withName, bool& found) {
	if (found)
		return nullptr;

	if (node.getName() == withName) {
		found = true;
		return &node;
	}
	
	auto& children = node.getChildren();
	if (!children.size())
		return nullptr;

	for (auto& i : children) {
		auto res = FindNode(i, withName, found);
		if (res)
			return res;
	}

	return nullptr;
}

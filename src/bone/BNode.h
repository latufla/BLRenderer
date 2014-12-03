#pragma once
#include <string>
#include <vector>

namespace br {
	template <class B>
	class BNode {
	public:
		BNode() = default;
		BNode(uint32_t, std::string, B&);
	
		~BNode() = default;
	
		bool addChild(uint32_t, std::string, B&);
		bool addChild(BNode<B>&);
	
		uint32_t getId() const { return id; }
		void setId(uint32_t val) { id = val; }
	
		std::string getName() const { return name; }
		B& getData() { return data; }
		
		typedef std::vector<BNode<B>> ChildrenList;
		ChildrenList& getChildren() { return children; }
		void setChildren(ChildrenList val) { children = val; }
	
		operator std::string() const;
	
		static void ForEachNode(BNode<B>&, void(*)(BNode<B>&, uint32_t), uint32_t = 0);
		static void ForEachNode(BNode<B>&, void*, void*, void(*)(BNode<B>&, void*, void*, uint32_t), uint32_t = 0);
		
		static void ArrangeIds(BNode<B>&);
	
		static BNode<B>* BNode<B>::FindNode(BNode<B>&, std::string);
	
	private:
		uint32_t id;
		std::string name;
		B data;
		
		ChildrenList children;
	
		static void ArrangeIds(BNode<B>&, uint32_t&);
		static BNode<B>* FindNode(BNode<B>&, std::string, bool&);
	};
	
	template <class B>
	BNode<B>::BNode(uint32_t id, std::string name, B& data) {
		this->id = id;
		this->name = name;
		this->data = data;
	}
	
	template <class B>
	bool BNode<B>::addChild(uint32_t id, std::string name, B& data) {
		children.push_back({id, name, data});
		return true;
	}
	
	template <class B>
	bool BNode<B>::addChild(BNode<B>& child) {
		children.push_back(child);
		return true;
	}
	
	
	template <class B>
	BNode<B>::operator std::string() const {
		return "{TNode id: " + std::to_string(id) + " name: " + name + " data: " + std::to_string(data) + "}";
	}
	
	template <class B>
	void BNode<B>::ForEachNode(BNode<B>& node, void(*eacher)(BNode<B>& node, uint32_t level), uint32_t level) {
		eacher(node, level);
	
		auto& children = node.getChildren();
		if (!children.size())
			return;
	
		for (auto& i : children) {
			ForEachNode(i, eacher, level + 1);
		}
	}
	
	template <class B>
	void BNode<B>::ForEachNode(BNode<B>& node, void* inData, void* outData, void(*eacher)(BNode<B>&, void*, void*, uint32_t), uint32_t level) {
		eacher(node, inData, outData, level);
	
		auto& children = node.getChildren();
		if (!children.size())
			return;
	
		for (auto& i : children) {
			ForEachNode(i, inData, outData, eacher, level + 1);
		}
	}
	
	template <class B>
	void BNode<B>::ArrangeIds(BNode<B>& node) {
		uint32_t firstId = 0;
		ArrangeIds(node, firstId);
	}
	
	template <class B>
	BNode<B>* BNode<B>::FindNode(BNode<B>& node, std::string withName) {
		bool found = false;
		return BNode<B>::FindNode(node, withName, found);
	}
	
	
	template <class B>
	void BNode<B>::ArrangeIds(BNode<B>& node, uint32_t& idHelper) {
		node.setId(idHelper);
	
		auto& children = node.getChildren();
		if (!children.size())
			return;
	
		for (auto& i : children) {
			ArrangeIds(i, ++idHelper);
		}
	}
	
	template <class B>
	BNode<B>* BNode<B>::FindNode(BNode<B>& node, std::string withName, bool& found) {
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
}

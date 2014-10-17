#pragma once

class NodeData {
public:
	NodeData();
	~NodeData();
	
	virtual operator std::string() const;
};


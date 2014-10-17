#pragma once
#include "NodeData.h"

class BoneNodeData : public NodeData{
public:
	BoneNodeData();
	~BoneNodeData();

	operator std::string() const override;
};


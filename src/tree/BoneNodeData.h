#pragma once
#include "NodeData.h"
#include <glm.hpp>

class BoneNodeData : public NodeData{
public:
	BoneNodeData();
	BoneNodeData(const glm::mat4&);
	~BoneNodeData();

	operator std::string() const override;

private:
	glm::mat4 transform;
};


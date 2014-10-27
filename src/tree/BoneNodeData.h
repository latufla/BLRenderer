#pragma once
#include <glm.hpp>

class BoneNodeData{
public:
	BoneNodeData();
	BoneNodeData(const glm::mat4&);
	~BoneNodeData();

	operator std::string() const;

	glm::mat4& getTransform() { return transform; }

private:
	glm::mat4 transform;
};


#pragma once
#include <glm.hpp>

class BoneNodeData{
public:
	BoneNodeData() = default;
	BoneNodeData(const glm::mat4&);
	
	BoneNodeData(BoneNodeData&& that) {
		this->transform = std::move(that.transform);
	};

	BoneNodeData& operator=(BoneNodeData&& that) {
		this->transform = std::move(that.transform);
		return *this;
	}

	~BoneNodeData() = default;
	
	operator std::string() const;

	glm::mat4& getTransform() { return transform; }

private:
	glm::mat4 transform;
};


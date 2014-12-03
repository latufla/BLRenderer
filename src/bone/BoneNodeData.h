#pragma once
#include <glm.hpp>

namespace br {
	class BoneNodeData{
	public:
		BoneNodeData() = default;
		BoneNodeData(const glm::mat4&);
	
		~BoneNodeData() = default;
		
		operator std::string() const;
	
		glm::mat4& getTransform() { return transform; }
	
	private:
		glm::mat4 transform;
	};
}


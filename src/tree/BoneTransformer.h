#pragma once
#include <glm.hpp>
#include <memory>
#include <unordered_map>
#include "..\Animation3d.h"
#include "TNode.h"
#include "BoneNodeData.h"
#include "..\View.h"
#include <vector>

class BoneTransformer {
public:
	BoneTransformer();
	~BoneTransformer();

	struct BoneData {
		glm::mat4 offset;
		glm::mat4 finalTransform;
	};

	typedef std::unordered_map<uint32_t, BoneData> BonesDataMap;
	void transform(TNode<BoneNodeData>&, View&, const glm::mat4&, std::shared_ptr<Animation3d>, glm::mat4, BonesDataMap&);

private:
	glm::vec3 calcTimeInterpolation(uint32_t time, std::vector<Vec3Key> vecs);
	glm::mat4 calcTimeInterpolation(uint32_t time, std::vector<Mat4Key> mats);
};


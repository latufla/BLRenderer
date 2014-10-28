#include "stdafx.h"
#include "BoneTransformer.h"
#include "..\Utils.h"
#include <gtc\matrix_transform.hpp>

using std::vector;


void BoneTransformer::transform(View& object, Model3d& model, BonesDataMap& outBonesData) {
	auto& boneTree = model.getBoneTree();
	auto& anim = model.getAnimation(object.getAnimationName());
	uint32_t animTime = object.getAnimationTime();
	auto& gTrans = model.getGlobalInverseTransform();
	glm::mat4 pTrans;
	doTransform(boneTree, anim, animTime, gTrans, pTrans, outBonesData);
}

void BoneTransformer::doTransform(TNode<BoneNodeData>& boneTree, Animation3d& animation, uint32_t animationTime, const glm::mat4& globalInverseTransform, glm::mat4 parentTransform, BonesDataMap& outBonesData) {
	uint32_t boneId = boneTree.getId();
	BoneNodeData& bNData = boneTree.getData();
	glm::mat4 nodeTransform = bNData.getTransform();

	BoneAnimation* bAnim = animation.getBoneAnimation(boneId);
	if (bAnim) {
		glm::vec3 scalingV = calcTimeInterpolation(animationTime, bAnim->scalings);
		glm::mat4 scalingM;
		scalingM = glm::scale(scalingM, scalingV);

		glm::mat4 rotationM = calcTimeInterpolation(animationTime, bAnim->rotations);

		glm::vec3 translationV = calcTimeInterpolation(animationTime, bAnim->positions);
		glm::mat4 translationM;
		translationM = glm::translate(translationM, translationV);

		nodeTransform = translationM * rotationM * scalingM;
	}

	glm::mat4 globalTransform = parentTransform * nodeTransform;

	if (outBonesData.find(boneId) != end(outBonesData)) {
		BoneData& outBData = outBonesData[boneId];
		outBData.finalTransform = globalInverseTransform * globalTransform * outBData.offset;
	}

	TNode<BoneNodeData>::ChildrenList& children = boneTree.getChildren();
	uint32_t nChildren = children.size();
	for (uint32_t i = 0; i < nChildren; ++i) {
		doTransform(children[i], animation, animationTime, globalInverseTransform, globalTransform, outBonesData);
	}
}

glm::vec3 BoneTransformer::calcTimeInterpolation(uint32_t time, std::vector<Vec3Key> vecs) {
	uint32_t n = vecs.size() - 1;
	uint32_t frame1Id = n;
	uint32_t frame2Id = n;
	for (uint32_t i = 0; i < n; ++i) {
		if (time < vecs[i + 1].time * 1000) {
			frame1Id = i;
			frame2Id = i + 1;
			break;
		}
	}

	auto& frame1 = vecs[frame1Id];
	auto& frame2 = vecs[frame2Id];

	double delta = (frame2.time - frame1.time) * 1000;
	double factor = (time - (frame1.time * 1000)) / delta;
	if (factor > 1)
		factor = 1;

	return Utils::interpolate(frame1.value, frame2.value, (float)factor);
}

glm::mat4 BoneTransformer::calcTimeInterpolation(uint32_t time, std::vector<Mat4Key> mats) {
	uint32_t n = mats.size() - 1;
	uint32_t frame1Id = n;
	uint32_t frame2Id = n;
	for (uint32_t i = 0; i < n; ++i) {
		if (time < mats[i + 1].time * 1000) {
			frame1Id = i;
			frame2Id = i + 1;
			break;
		}
	}

	auto& frame1 = mats[frame1Id];
	auto& frame2 = mats[frame2Id];

	double delta = (frame2.time - frame1.time) * 1000;
	double factor = (time - (frame1.time * 1000)) / delta;
	if (factor > 1)
		factor = 1;

	return Utils::interpolateQ(frame1.value, frame2.value, (float)factor);
}

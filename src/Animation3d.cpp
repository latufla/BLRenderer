#include "stdafx.h"
#include "Animation3d.h"
#include "Utils.h"

using std::string;
using std::to_string; 

using std::vector;


Animation3d::Animation3d() {
}

Animation3d::Animation3d(string name, double duration, double ticksPerSecond, vector<BoneAnimation>& boneAnimations) {
	this->name = name;
	this->duration = duration;
	this->ticksPerSecond = ticksPerSecond;
	this->boneAnimations = boneAnimations;
}


Animation3d::~Animation3d() {
}

Animation3d::operator string() const {
	string res = "{Animation3d name: " + name + " duration: " + to_string(duration) + " ticksPerSecond: " + to_string(ticksPerSecond);
	res += " boneAnimations: {";
	for (auto& i : boneAnimations) {
		res += "\n{BoneAnimation boneId: " + to_string(i.boneId) + " boneName:" + i.boneName;
		res += "\n positions (" + to_string(i.positions.size()) + ")";

#ifdef FULL_REPORT
		res += ": { \n";
		for (auto& j : i.positions) {
			res += "{Vec3Key time: " + to_string(j.time) + " value: " + Utils::glmToString(j.value) + "}\n";
		}
#endif

		res += " rotations (" + to_string(i.rotations.size()) + ")";

#ifdef FULL_REPORT
		res += ": { \n";
		for (auto& j : i.rotations) {
			res += "{Mat4Key time: " + to_string(j.time) + " value: \n" + Utils::glmToString(j.value) + "}\n";
		}
#endif 
		
		res += " scalings (" + to_string(i.scalings.size()) + ")";

#ifdef FULL_REPORT
		res += ": { \n";
		for (auto& j : i.scalings) {
			res += "{Vec3Key time: " + to_string(j.time) + " value: " + Utils::glmToString(j.value) + "}\n";
		}

#endif

		res += "}\n";
	}

	res += "}}";
	return res;
}

BoneAnimation* Animation3d::getBoneAnimation(uint32_t id) {
	for (auto& i : boneAnimations) {
		if (i.boneId == id)
			return &i;
	}
	return nullptr;
}

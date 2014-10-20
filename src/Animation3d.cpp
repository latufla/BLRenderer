#include "stdafx.h"
#include "Animation3d.h"

using std::string;
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

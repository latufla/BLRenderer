#include "stdafx.h"
#include "BoneNodeData.h"

#define FULL_REPORT

using std::string;
using std::to_string;

BoneNodeData::BoneNodeData() {
}

BoneNodeData::BoneNodeData(const glm::mat4& transform) {
	this->transform = transform;
}

BoneNodeData::~BoneNodeData() {
}

BoneNodeData::operator string() const {
	string res = "{BoneNodeData";
	
#ifdef FULL_REPORT
		res += " transform:\n";
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				res += to_string(transform[j][i]) + " ";
				res += " ";
			}
			res += "\n";
		}
#endif

	res += "}";
	return res;
}

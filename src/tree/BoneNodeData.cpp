#include "stdafx.h"
#include "BoneNodeData.h"

using std::string;
using std::to_string;

BoneNodeData::BoneNodeData() {
}


BoneNodeData::~BoneNodeData() {
}

BoneNodeData::operator string() const {
	string res = "{BoneNodeData}";
	return res;
}

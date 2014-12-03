#include "stdafx.h"
#include "BoneNodeData.h"

using std::string;
using std::to_string;
using glm::mat4;

namespace br {
	BoneNodeData::BoneNodeData(const mat4& transform) {
		this->transform = transform;
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
}

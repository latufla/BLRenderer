#pragma once
#include <glm.hpp>
#include <vector>
#include <unordered_map>
#include "BNode.h"
#include "BoneNodeData.h"
#include "..\..\View.h"
#include "..\..\Model3d.h"

namespace br {
	class BoneTransformer {
	public:
		BoneTransformer() = default;
		~BoneTransformer() = default;
		
		struct BoneData {
			glm::mat4 offset;
			glm::mat4 finalTransform;
		};
	
		typedef std::unordered_map<uint32_t, BoneData> BonesDataMap;
		void transform(View& object, Model3d& model, BonesDataMap& outBonesData);
	
	private:
		// only to hide useless params
		void doTransform(BNode<BoneNodeData>&, Animation3d&, long long, const glm::mat4&, glm::mat4, BonesDataMap&);
		
		template <class B, class R>
		B calcTimeInterpolation(long long time, std::vector<R> vecs);
	};
	
	template <class B, class R>
	B BoneTransformer::calcTimeInterpolation(long long time, std::vector<R> v) {
		uint32_t n = v.size() - 1;
		uint32_t frame1Id = n;
		uint32_t frame2Id = n;
		for (uint32_t i = 0; i < n; ++i) {
			if (time < v[i + 1].time * 1000) {
				frame1Id = i;
				frame2Id = i + 1;
				break;
			}
		}
	
		auto& frame1 = v[frame1Id];
		auto& frame2 = v[frame2Id];
	
		double delta = (frame2.time - frame1.time) * 1000;
		double factor = (time - (frame1.time * 1000)) / delta;
		if (factor > 1)
			factor = 1;
	
		return Util::interpolate(frame1.value, frame2.value, (float)factor);
	}
}

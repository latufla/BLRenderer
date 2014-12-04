#pragma once
#include <vector>
#include <glm.hpp>
#include <unordered_map>

namespace br {
	class Animation3d {
	public:
		struct Vec3Key {
			double time;
			glm::vec3 value;
		};
	
		struct Mat4Key {
			double time;
			glm::mat4 value;
		};
	
		struct BoneAnimation {
			// from our bone tree
			uint32_t boneId; // id in tree
			std::string boneName; // natural born name
	
			std::vector<Vec3Key> positions;
			std::vector<Mat4Key> rotations;
			std::vector<Vec3Key> scalings;
		};
	
		Animation3d() = delete;
		Animation3d(std::string name, double duration, double ticksPerSecond, std::unordered_map<uint32_t, BoneAnimation>& idToBoneAnimation);
	
		~Animation3d() = default;
	
		static const std::string DEFAULT_ANIMATION_NAME;
	
		operator std::string() const;
	
		BoneAnimation* getBoneAnimation(uint32_t boneId);
	
		std::string getName() const { return name; }
		double getDuration() const { return duration; }
	
	private:
		std::string name;
		double duration;
		double ticksPerSecond;
	
		std::unordered_map<uint32_t, BoneAnimation> idToBoneAnimation;
	};
}


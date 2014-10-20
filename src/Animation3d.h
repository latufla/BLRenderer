#pragma once
#include <vector>
#include <map>
#include <glm.hpp>

struct BoneAnimation {
	// from our bone tree
	uint32_t boneId; // id in tree
	std::string boneName; // natural born name

	std::map<double, glm::vec3> timeToPosition;
	std::map<double, glm::mat4> timeToRotation;
	std::map<double, glm::vec3> timeToScale;
};

class Animation3d {
public:
	Animation3d(); // = delete;
	Animation3d(std::string, double, double, std::vector<BoneAnimation>&);

	~Animation3d();

private:
	std::string name;

	double duration;
	double ticksPerSecond;

	std::vector<BoneAnimation> boneAnimations;
};


#pragma once
#include <vector>
#include <glm.hpp>

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

class Animation3d {
public:
	Animation3d(); // = delete;
	Animation3d(std::string, double, double, std::vector<BoneAnimation>&);

	~Animation3d();

	operator std::string() const;

	BoneAnimation* getBoneAnimation(uint32_t); // TODO: fix this dirt

	double getDuration() const { return duration; }

private:
	std::string name;

	double duration;
	double ticksPerSecond;

	std::vector<BoneAnimation> boneAnimations;
};


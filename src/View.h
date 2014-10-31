#pragma once
#include <glm.hpp>

class View {
public:
	View(uint32_t, std::string, std::string);

	uint32_t getId() const { return id; }
	std::string getName() const { return name; }
	std::string getPath() const { return path; }

	glm::mat4& getTransform() { return transform; }
	void setTransform(const glm::mat4& val) { transform = val; }

	void setAnimation(std::string, uint32_t, bool = false);

	std::string getAnimationName() const { return animation.name;  }
	uint32_t getAnimationTime() const { return animation.timeMSec; }

	bool doAnimationStep(uint32_t);

private:
	uint32_t id;
	std::string name;
	std::string path;
	
	glm::mat4 transform;

	struct AnimationData {
		std::string name;
		bool loop;
		uint32_t durationMSec = 0;
		uint32_t timeMSec = 0;
	} animation;
};


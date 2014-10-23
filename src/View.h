#pragma once
#include "Model3dInfo.h"
#include <glm.hpp>

class View {
public:
	View(); // = delete;
	View(uint32_t, std::string, std::string);
	
	~View();

	uint32_t getId() const { return id; }
	std::string getName() const { return name; }
	std::string getPath() const { return path; }

	glm::mat4& getTransform() { return transform; }
	void setTransform(const glm::mat4& val) { transform = val; }

	void setAnimation(std::string, uint32_t, bool = false);

	uint32_t getAnimationTime() const { return animationTime; }

	bool doAnimationStep(uint32_t);

private:
	uint32_t id;
	std::string name;
	std::string path;
	
	glm::mat4 transform;

	std::string animation;
	bool loopAnimation = false;
	uint32_t animationDuration = 0;
	uint32_t animationTime = 0;
};


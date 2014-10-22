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

	std::string getAnimation() const { return animation; }
	void setAnimation(std::string val) { animation = val; }

private:
	uint32_t id;
	std::string name;
	std::string path;
	
	glm::mat4 transform;

	std::string animation;
};


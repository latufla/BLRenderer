#pragma once
#include <vector>
#include <memory>
#include <gtc\matrix_transform.hpp>
#include "ObjectInfo.h"

class ObjectBase
{
public:
	ObjectBase();
	ObjectBase(std::string);
	~ObjectBase(){};

	void translate(float, float, float);
	void scale(float, float, float);
	void rotateY(float);

	std::string getInfo() const { return info; }
	glm::mat4& getOrientation() { return orientation; }

private:
	std::string info;
	glm::mat4 orientation;
};


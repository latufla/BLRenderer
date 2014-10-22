#pragma once
#include <vector>
#include <memory>
#include <gtc\matrix_transform.hpp>

class ObjectBase
{
public:
	ObjectBase();
	ObjectBase(uint32_t, std::string);
	~ObjectBase(){};

	void translate(float, float, float);
	void scale(float, float, float);
	void rotateY(float);

	uint32_t getId() const { return id; }
	
	std::string getInfo() const { return info; }
	glm::mat4& getOrientation() { return orientation; }

private:
	uint32_t id;
	
	std::string info;
	glm::mat4 orientation;
};


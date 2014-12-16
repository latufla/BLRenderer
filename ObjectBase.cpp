#include "src/utils/SharedHeaders.h"
#include "ObjectBase.h"

using std::string;
using glm::vec3;

ObjectBase::ObjectBase(uint32_t id, string info) {
	this->id = id;
	this->info = info;
}

void ObjectBase::translate(float x, float y, float z)
{
	orientation = glm::translate(orientation, vec3{x, y, z});
}

void ObjectBase::scale(float x, float y, float z)
{
	orientation = glm::scale(orientation, vec3{ x, y, z });
}

void ObjectBase::rotateY(float angle)
{
	orientation = glm::rotate(orientation, angle, vec3{ 0, 1, 0 });
}

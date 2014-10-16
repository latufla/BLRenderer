#include "stdafx.h"
#include "ObjectBase.h"
#include "Utils.h"

using namespace std;
using namespace glm;

ObjectBase::ObjectBase(string info) {
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

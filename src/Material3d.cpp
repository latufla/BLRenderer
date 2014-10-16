#include "stdafx.h"
#include "../stdafx.h"
#include "Material3d.h"

using std::string;
using std::vector;

Material3d::Material3d() {
}

Material3d::Material3d(string name, vector<unsigned char>& data, int16_t width, int16_t height) {
	this->name = name;
	
	this->data = data;
	this->width = width;
	this->height = height;
}


Material3d::~Material3d() {
}

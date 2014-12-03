#include "stdafx.h"
#include "Texture2d.h"

using std::string;
using std::vector;

namespace br {
	Texture2d::Texture2d(string name, vector<uint8_t>& data, int16_t width, int16_t height) {
		this->name = name;
		this->data = data;
		this->width = width;
		this->height = height;
	}
}

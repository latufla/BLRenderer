#include "Texture2d.h"

using std::string;
using std::vector;

namespace br {
	Texture2d::Texture2d(string path, vector<uint8_t>& data, uint32_t width, uint32_t height)
		: path(path), data(data), width(width), height(height) {
	}
}

#pragma once
#include "../utils/SharedHeaders.h"
#include <vector>

namespace br {
	class Texture2d {
	public:
		Texture2d() = delete;
		Texture2d(std::string path, std::vector<uint8_t>& data, uint32_t width, uint32_t height);
		
		~Texture2d() = default;
	
		std::string getPath() const { return path; }
		
		std::vector<uint8_t>& getData() { return data; }
		
		uint32_t getWidth() const { return width; }
		uint32_t getHeight() const { return height; }
		
	private:
		std::string path;
	
		std::vector<uint8_t> data;
		uint32_t width;
		uint32_t height;
	};
	
}

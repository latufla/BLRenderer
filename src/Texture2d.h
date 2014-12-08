#pragma once
#include <vector>

namespace br {
	class Texture2d {
	public:
		Texture2d() = delete;
		Texture2d(std::string path, std::vector<uint8_t>& data, int16_t width, int16_t height);
		
		~Texture2d() = default;
	
		std::string getPath() const { return path; }
		
		std::vector<uint8_t>& getData() { return data; }
		
		int16_t getWidth() const { return width; }
		int16_t getHeight() const { return height; }
		
	private:
		std::string path;
	
		std::vector<uint8_t> data;
		int16_t width;
		int16_t height;
	};
	
}

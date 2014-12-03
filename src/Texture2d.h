#pragma once
#include <vector>

namespace br {
	class Texture2d {
	public:
		Texture2d() = default;
		Texture2d(std::string, std::vector<uint8_t>&, int16_t, int16_t);
		
		~Texture2d() = default;
	
		std::string getName() const { return name; }
		void setName(std::string val) { name = val; }
	
		std::vector<uint8_t>& getData() { return data; }
		
		int16_t getWidth() const { return width; }
		void setWidth(int16_t val) { width = val; }
		
		int16_t getHeight() const { return height; }
		void setHeight(int16_t val) { height = val; }
	
	private:
		std::string name;
	
		std::vector<uint8_t> data;
		int16_t width;
		int16_t height;
	};
	
}

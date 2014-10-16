#pragma once
#include <vector>

// just texture now
class Material3d {
public:
	Material3d();
	Material3d(std::string, std::vector<unsigned char>&, int16_t width, int16_t height);
	
	~Material3d();

	std::string getName() const { return name; }
	void setName(std::string val) { name = val; }

	std::vector<unsigned char>& getData() { return data; }
	void setData(std::vector<unsigned char>& val) { data = val; }
	
	int16_t getWidth() const { return width; }
	void setWidth(int16_t val) { width = val; }
	
	int16_t getHeight() const { return height; }
	void setHeight(int16_t val) { height = val; }

private:
	std::string name;

	std::vector<unsigned char> data;
	int16_t width;
	int16_t height;
};


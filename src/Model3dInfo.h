#pragma once
#include <string>

class Model3dInfo {
public:
	Model3dInfo();
	Model3dInfo(std::string);
	
	~Model3dInfo();

	std::string getName() const { return name; }
	
	std::string getModelPath() const {
		return getModelDir() + getName() + ".dae";
	}

	std::string getModelDir()const {
		return "models/" + getName() + "/";
	}

private:
	std::string name;
};


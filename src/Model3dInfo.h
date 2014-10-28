#pragma once
#include <string>

class Model3dInfo {
public:
	Model3dInfo();
	Model3dInfo(std::string);
	
	~Model3dInfo();

	std::string getName() const { return name; }
	
	std::string getModelName() const { return name + ".dae"; }

	std::string getModelDir()const {
		return "models/" + getName() + "/";
	}
	std::string getModelPath() const {
		return getModelDir() + getModelName();
	}

private:
	std::string name;
};


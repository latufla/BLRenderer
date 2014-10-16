#pragma once
#include <string>

class ObjectInfo {
public:
	ObjectInfo();
	ObjectInfo(std::string);
	
	~ObjectInfo();

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


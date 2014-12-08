#pragma once
#include <string>
#include <unordered_map>

class Model3dInfo {
public:
	Model3dInfo() = delete;
	Model3dInfo(std::string name, std::unordered_map<std::string, std::string> nameToAnimation);
	
	~Model3dInfo() = default;

	std::string getName() const { return name; }
	
	std::string getModelName() const { return name + ".dae"; }

	std::string getModelDirectory()const {
		return "models/" + getName() + "/";
	}

	std::string getModelPath() const {
		return getModelDirectory() + getModelName();
	}

	std::unordered_map<std::string, std::string>& getNameToAnimation() { return nameToAnimation; }

private:
	std::string name;
	std::unordered_map<std::string, std::string> nameToAnimation;
};


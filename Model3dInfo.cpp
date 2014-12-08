#include "src/SharedHeaders.h"
#include "Model3dInfo.h"

using std::string;
using std::unordered_map;

Model3dInfo::Model3dInfo(string name, unordered_map<string, string>) 
	: name(name), nameToAnimation(nameToAnimation) {
}

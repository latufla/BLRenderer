#include "../utils/SharedHeaders.h"
#include "Font.h"

using std::string;
using std::to_string;
using std::unordered_map;

namespace br {
	Font::Font(string name, uint8_t size, Texture2d& atlas, unordered_map < char, Character >& codeToCharacter) 
		: name(name), size(size), atlas(atlas), codeToCharacter(codeToCharacter) {
	}

	string Font::getUniqueName() {
		return getUniqueName(name, size);
	}

	std::string Font::getUniqueName(std::string name, uint8_t size) {
		return name + " " + to_string(size);
	}

	br::Character Font::getCharacterBy(char code) {
		return codeToCharacter.at(code);
	}


}

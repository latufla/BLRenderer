#pragma once

#include <unordered_map>
#include "../Texture2d.h"

namespace br {
	struct Character {
		// till next character
		float pixToNextCharX;
		float pixToNextCharY;

		// size
		float left;
		float top;
		float width;
		float height;

		// location in font texture atlas
		float texOffsetX;
		
		struct RGBA {uint8_t r, g, b, a;};
		std::vector<RGBA> texture;
	};

	class Font {
	public:
		Font() = delete;
		Font(std::string name, uint8_t size, Texture2d& atlas, std::unordered_map < char, Character >& codeToCharacter);

		~Font() = default;

		std::string getName() const { return name; }
		uint8_t getSize() const { return size; }
		Texture2d& getAtlas() { return atlas; }

		Character getCharacterBy(char code);

		std::string getUniqueName();
		static std::string getUniqueName(std::string name, uint8_t size);
	private:
		std::string name;
		uint8_t size;

		Texture2d atlas;
		std::unordered_map < char, Character > codeToCharacter;
	};
}

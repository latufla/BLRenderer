#pragma once
#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <unordered_map>
#include "../../assets/Texture2d.h"
#include "Font.h"

namespace br {
	class FontLoader {
	public:
		FontLoader();
		~FontLoader() = default;

		void loadFont(std::string path, std::string name, uint8_t size = 20);
		Font& getFontBy(std::string name, uint8_t size);

	private:
		FT_Library library;
		std::unordered_map < std::string, Font > nameToFont;
	};
}

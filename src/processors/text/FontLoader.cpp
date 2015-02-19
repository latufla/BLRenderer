#include "FontLoader.h"
#include "../../exceptions/Exception.h"
#include <algorithm>

using std::string;
using std::vector;
using std::to_string;
using std::max;

namespace br {
	FontLoader::FontLoader() {
		auto error = FT_Init_FreeType(&library);
		if(error)
			throw AssetException(EXCEPTION_INFO, "", "can`t init free type library");
	}

	void FontLoader::loadFont(string path, string name, uint8_t size) {
		string nameAsKey = Font::getUniqueName(name, size);
		if(nameToFont.find(nameAsKey) != nameToFont.cend())
			throw AssetException(EXCEPTION_INFO, path, "has same font");

		FT_Face fontFace;
		auto error = FT_New_Face(library, path.c_str(), 0, &fontFace);
		if(error)
			throw AssetException(EXCEPTION_INFO, path, "can`t load font");

		FT_Set_Pixel_Sizes(fontFace, 0, size);

		// make RGBA atlas with right size
		std::unordered_map<char, Character> codeToCharacter;
		uint32_t atlasW = 0, atlasH = 0;
		for(int i = 32; i < 128; i++) {
			error = FT_Load_Char(fontFace, i, FT_LOAD_RENDER);
			if(error)
				throw AssetException(EXCEPTION_INFO, path, "can`t load character" + to_string((char)(i)));
			
			auto g = fontFace->glyph;
			Character ch{
				((float)g->advance.x) / 64,
				((float)g->advance.y) / 64,
				(float)g->bitmap_left,
				(float)g->bitmap_top,
				(float)g->bitmap.width,
				(float)g->bitmap.rows,
				0,
				{}
			};

			// fill character texture
			auto& gTex = g->bitmap.buffer;
			uint32_t n = (uint32_t)(ch.width * ch.height);
			for(uint32_t j = 0; j < n; ++j) {
				ch.texture.push_back({255, 255, 255, gTex[j]});
			}

			codeToCharacter.emplace(i, ch);

			atlasW += g->bitmap.width;
			atlasH = max(atlasH, (uint32_t)g->bitmap.rows);
		}
		
		// fill Font with character textures
		vector<Character::RGBA> atlasData(atlasW * atlasH, {0, 0, 0, 0});
		float nextCharacterX = 0;
		for(auto& i : codeToCharacter){
			Character& ch = i.second;
			
			// TODO: bad approach, really messy
			ch.texOffsetX = nextCharacterX / atlasW;
			//----

			// register, but not draw, ex - '32' space
			if(!(ch.width || ch.height))
				continue;
			
			// fill atlas with character
			for(uint32_t k = 0; k < ch.height; ++k) {
				for(uint32_t j = 0; j < ch.width; ++j) {
					uint32_t charInAtlas = k * atlasW + j + (uint32_t)nextCharacterX;
					uint32_t charInData = k * (uint32_t)ch.width + j;
					atlasData[charInAtlas] = ch.texture[charInData];
				}
			}
			nextCharacterX += ch.width;
		}

		vector<uint8_t> data;
		for(auto& i : atlasData) {
			data.push_back(i.r);
			data.push_back(i.g);
			data.push_back(i.b);
			data.push_back(i.a);
		}

		Texture2d atlas{nameAsKey, data, atlasW, atlasH};
		Font font{name, size, atlas, codeToCharacter};
		nameToFont.emplace(nameAsKey, font);
	}

	Font& FontLoader::getFontBy(string name, uint8_t size) {
		std::string nameAsKey = Font::getUniqueName(name, size);
		try {
			return nameToFont.at(nameAsKey);
		} catch(std::out_of_range&) {
			throw AssetException(EXCEPTION_INFO, nameAsKey, "can`t get font");
		}
	}
}


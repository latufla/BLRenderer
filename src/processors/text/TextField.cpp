#include "TextField.h"
#include "../../utils/SharedHeaders.h"

using std::string;
using std::to_string;
using std::array;

namespace br {
	TextField::TextField(Font& font, string text, array<float, 4> color, glm::vec2& position, glm::vec2& scaleFactor)
		: fontName(font.getName()),
		fontSize(font.getSize()),
		text(text),
		color(color),
		position(position) {

		float nextX = 0, nextY = 0;
		float sx = scaleFactor.x, sy = scaleFactor.y;
		Texture2d& atlas = font.getAtlas();
		uint32_t atlasW = atlas.getWidth(), atlasH = atlas.getHeight();
		for(auto& i : text) {
			auto ch = font.getCharacterBy(i);

			uint32_t sz = vertices.size();
			indices.push_back(sz + 0);
			indices.push_back(sz + 1);
			indices.push_back(sz + 2);
			indices.push_back(sz + 3);
			indices.push_back(sz + 0);
			indices.push_back(sz + 2);


			float x = nextX + ch.left * sx;
			float y = nextY + ch.top * sy;
			nextX += ch.pixToNextCharX * sx;
			nextY += ch.pixToNextCharY * sy;

			float w = ch.width * sx;
			float h = ch.height * sy;

			float nextCharTexX = ch.texOffsetX + ch.width / atlasW;
			float nextCharTexY = 0.0f + ch.height / atlasH;

			vertices.push_back({
				x + w, y - h, 0,
				nextCharTexX, nextCharTexY,
				{0, 0, 0, 0},
				{0, 0, 0, 0}
			});
			vertices.push_back({
				x + w, y, 0,
				nextCharTexX, 0,
				{0, 0, 0, 0},
				{0, 0, 0, 0}
			});
			vertices.push_back({
				x, y, 0,
				ch.texOffsetX, 0,
				{0, 0, 0, 0},
				{0, 0, 0, 0}
			});
			vertices.push_back({
				x, y - h, 0,
				ch.texOffsetX, nextCharTexY,
				{0, 0, 0, 0},
				{0, 0, 0, 0}
			});
		}

	}

	string TextField::getUniqueName() {
		return fontName + to_string(fontSize) + text;
	}
}

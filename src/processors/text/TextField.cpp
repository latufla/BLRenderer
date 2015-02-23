#include "TextField.h"
#include "../../utils/SharedHeaders.h"

using std::string;
using std::to_string;
using std::array;

namespace br {
	TextField::TextField(Font& font, string text, const glm::vec4& color, const glm::vec2& position, const glm::vec2& scaleFactor)
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
				nextCharTexX, nextCharTexY
			});
			vertices.push_back({
				x + w, y, 0,
				nextCharTexX, 0
			});
			vertices.push_back({
				x, y, 0,
				ch.texOffsetX, 0
			});
			vertices.push_back({
				x, y - h, 0,
				ch.texOffsetX, nextCharTexY
			});
		}

		for(auto& i : vertices) {
			rawVertices.push_back(i.x);
			rawVertices.push_back(i.y);
			rawVertices.push_back(i.z);
			rawVertices.push_back(i.s);
			rawVertices.push_back(i.t);

			rawVertices.push_back(0.0f);
			rawVertices.push_back(0.0f);
			rawVertices.push_back(0.0f);
			rawVertices.push_back(0.0f);
		
			rawVertices.push_back(0.0f);
			rawVertices.push_back(0.0f);
			rawVertices.push_back(0.0f);
			rawVertices.push_back(0.0f);
		}
	}

	string TextField::getUniqueName() {
		return fontName + to_string(fontSize) + text;
	}
}

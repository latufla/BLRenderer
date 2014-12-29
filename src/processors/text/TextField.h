#pragma once
#include "FontLoader.h"
#include <vector>
#include <array>
#include "../../Mesh3d.h"

namespace br {
	class TextField {
	public:
		TextField() = delete;
		TextField(Font& font, std::string text, std::array<float, 4> color, glm::vec2& position, glm::vec2& scaleFactor);
		
		~TextField() = default;
	
		std::string getFontName() const { return fontName; }
		uint8_t getFontSize() const { return fontSize; }

		std::vector<Vertex3d> getVertices() const { return vertices; }
		std::vector<uint16_t> getIndices() const { return indices; }

		std::array<float, 4>& getColor() { return color; }

		std::string getText() const { return text; }
		glm::vec2& getPosition() { return position; }
		void setPosition(glm::vec2& val) { position = val; }

		std::string getUniqueName();
	private:
		std::string text;
		glm::vec2 position;

		std::string fontName;
		uint8_t fontSize;
		std::array<float, 4> color;
	
		std::vector<Vertex3d> vertices;
		std::vector<uint16_t> indices;
	};
}

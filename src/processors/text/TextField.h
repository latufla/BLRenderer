#pragma once
#include "FontLoader.h"
#include <vector>
#include <array>
#include "../../assets/Mesh3d.h"
#include <memory>

namespace br {
	class TextField {
	public:
		TextField() = delete;
		TextField(Font& font, std::string text, const glm::vec4& color, const glm::vec2& position, const glm::vec2& scaleFactor);
		
		~TextField() = default;
	
		std::string getFontName() const { return fontName; }
		uint8_t getFontSize() const { return fontSize; }

		glm::vec4& getColor() { return color; }

		std::string getText() const { return text; }
		glm::vec2& getPosition() { return position; }
		void setPosition(const glm::vec2& val) { position = val; }

		std::shared_ptr<Mesh3d> getMesh() { return mesh; }

		std::string getUniqueName();
	private:
		std::string text;
		glm::vec2 position;

		std::string fontName;
		uint8_t fontSize;
		glm::vec4 color;
	
		std::shared_ptr<Mesh3d> mesh;
	};
}

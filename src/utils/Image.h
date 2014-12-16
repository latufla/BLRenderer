#pragma once
#include <array>
#include "../Mesh3d.h"
#include "../Texture2d.h"

namespace br {
	class Image {
	public:
		Image() = delete;
		Image(std::string path, glm::vec2& position, float w, float h);
		
		~Image() = default;
	
		std::array<Vertex3d, 4>& getVertices() { return vertices; }
		std::array<uint16_t, 6> getIndices() const { return indices; }
		std::string getPath() { return path; }
		glm::vec2& getPosition() { return position; }

	private:
		std::string path;
		glm::vec2 position;

		std::array<Vertex3d, 4> vertices;
		std::array<uint16_t, 6> indices;
	};
	
}

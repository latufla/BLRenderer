#pragma once
#include <array>
#include "../../assets/Mesh3d.h"
#include "../../assets/Texture2d.h"
#include <memory>

namespace br {
	class Image {
	public:
		Image() = delete;
		Image(Texture2d& texture, const glm::vec2& position, float sx, float sy);
		
		~Image() = default;
	
		std::string getPath() { return path; }
		glm::vec2& getPosition() { return position; }

		std::shared_ptr<Mesh3d> getMesh() { return mesh; }

	private:
		std::string path;
		glm::vec2 position;

		std::shared_ptr<Mesh3d> mesh;
	};
}

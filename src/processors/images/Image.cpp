#include "Image.h"

namespace br{	
	Image::Image(Texture2d& texture, const glm::vec2& position, float sx, float sy) 
		: path(texture.getPath()), position(position){

		float w = texture.getWidth() * sx;
		float h = texture.getHeight() * sy;

		std::vector<Vertex3d> vertices;
		vertices.push_back({
			0, 0, 0,
			0.0f, 1.0f
		});
		vertices.push_back({
			0, h, 0,
			0.0f, 0.0f
		});
		vertices.push_back({
			w, h, 0,
			1.0f, 0.0f
		});
		vertices.push_back({
			w, 0, 0,
			1.0f, 1.0f
		});
		std::vector<uint16_t> indices{0, 1, 2, 3, 0, 2};
		mesh = std::make_shared<Mesh3d>(texture.getPath(), vertices, indices, 0);
		mesh->buildRawVertices();
	}
}
#include "Image.h"

namespace br{	
	Image::Image(Texture2d& texture, glm::vec2& position, float sx, float sy) 
		: path(texture.getPath()), position(position){

		float w = texture.getWidth() * sx;
		float h = texture.getHeight() * sy;

		vertices[0] = {
			0, 0, 0,
			0.0f, 1.0f,
			{0, 0, 0, 0},
			{0, 0, 0, 0}
		};
		vertices[1] = {
			0, h, 0,
			0.0f, 0.0f,
			{0, 0, 0, 0},
			{0, 0, 0, 0}
		};
		vertices[2] = {
			w, h, 0,
			1.0f, 0.0f,
			{0, 0, 0, 0},
			{0, 0, 0, 0}
		};
		vertices[3] = {
			w, 0, 0,
			1.0f, 1.0f,
			{0, 0, 0, 0},
			{0, 0, 0, 0}
		};

		indices[0] = 0;
		indices[1] = 1;
		indices[2] = 2;
		indices[3] = 3;
		indices[4] = 0;
		indices[5] = 2;
	}
}
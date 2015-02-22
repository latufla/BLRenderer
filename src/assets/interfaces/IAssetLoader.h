#pragma once
#include "../../utils/SharedHeaders.h"
#include "../Model3d.h"
#include "../../processors/text/Font.h"

namespace br {
	class IAssetLoader {
	public:
		virtual void loadModel(std::string pathAsKey, std::string textureDirectory) = 0;
		virtual Model3d& getModelBy(std::string path) = 0;

		virtual void loadAnimation(std::string toModel, std::string byNameAsKey, std::string withPath) = 0;

		virtual void loadTexture(std::string pathAsKey) = 0;
		virtual Texture2d& getTextureBy(std::string path) = 0;

		virtual void loadFont(std::string path, std::string name, uint8_t size) = 0;
		virtual Font& getFontBy(std::string name, uint8_t size) = 0;
	};
}
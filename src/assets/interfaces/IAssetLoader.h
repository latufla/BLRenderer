#pragma once
#include "../../utils/SharedHeaders.h"
#include "../../processors/text/Font.h"
#include "IModel3d.h"
#include "IProgram3d.h"

namespace br {
	class IAssetLoader {
	public:
		virtual void loadModel(std::string pathAsKey, std::string textureDirectory) = 0;
		virtual std::shared_ptr<IModel3d> getModelBy(std::string path) = 0;

		virtual void loadAnimation(std::string toModel, std::string byNameAsKey, std::string withPath) = 0;

		virtual void loadTexture(std::string pathAsKey) = 0;
		virtual Texture2d& getTextureBy(std::string path) = 0;

		virtual void loadProgram(std::string name, std::string vShaderPath, std::string fShaderPath) = 0;
		virtual std::shared_ptr<IProgram3d>& getProgramBy(std::string name) = 0;

		virtual void loadFont(std::string path, std::string name, uint8_t size) = 0;
		virtual Font& getFontBy(std::string name, uint8_t size) = 0;
	};
}
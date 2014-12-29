#pragma once
#include "..\ProcessorBase.h"
#include "Image.h"

namespace br{
	class ImageRenderProcessor : public ProcessorBase {
	public:
		ImageRenderProcessor(std::shared_ptr<AssetLoader>loader, std::pair<std::string, std::string> shaders);
		~ImageRenderProcessor();
	
		void addImage(uint32_t id, std::string path, std::pair<float, float> position);
		void removeImage(uint32_t id);
	
	protected:
		void doStep(StepData& stepData) override;
		ProgramContext fillProgramContext(uint32_t pObject) override;

		std::unordered_map<uint32_t, Image> idToImage;

		void loadImageToGpu(Image&);
		void deleteImageFromGpu(std::string pathAsKey);
		bool hasImageWithTexture(std::string pathAsKey);
	};
}


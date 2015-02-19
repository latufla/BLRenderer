#pragma once
#include "..\ProcessorBase.h"
#include "Image.h"

namespace br{
	class ImageRenderProcessor : public ProcessorBase {
	public:
		ImageRenderProcessor(std::shared_ptr<IAssetLoader>loader);
		~ImageRenderProcessor();
	
		void addImage(uint32_t id, std::string path,  const glm::vec2& position);
		void removeImage(uint32_t id);
	
	protected:
		void doStep(const StepData& stepData) override;

		std::unordered_map<uint32_t, Image> idToImage;

		void loadImageToGpu(Image&);
		void deleteImageFromGpu(std::string pathAsKey);
		bool hasImageWithTexture(std::string pathAsKey);
	};
}


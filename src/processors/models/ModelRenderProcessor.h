#pragma once
#include <array>
#include "../ProcessorBase.h"
#include "../../View.h"
#include "../../utils/bones/BoneTransformer.h"

namespace br{
	class ModelRenderProcessor : public ProcessorBase{
	public:
		ModelRenderProcessor(std::shared_ptr<AssetLoader>loader, std::pair<std::string, std::string> shaiders);
		~ModelRenderProcessor();
	
		void addObject(uint32_t id, std::string modelPath);
		void removeObject(uint32_t id);

		void playAnimation(uint32_t objId, std::string animName = Animation3d::DEFAULT_ANIMATION_NAME, bool loop = true);
		void stopAnimation(uint32_t objId, std::string animName = Animation3d::DEFAULT_ANIMATION_NAME);
		void transformObject(uint32_t objId, const glm::mat4& transform);

	private:
		void doStep(const StepData& stepData) override;

		std::unordered_map<uint32_t, View> idToObject;
	
		void loadModelToGpu(std::string);
		void deleteModelFromGpu(std::string);
		bool hasObjectWithModel(std::string);
	
		BoneTransformer boneTransformer;
		BoneTransformer::BonesDataMap prepareAnimationStep(View&, Mesh3d&);
	};
	
}

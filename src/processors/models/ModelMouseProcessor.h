#pragma once
#include "..\ProcessorBase.h"
#include "..\..\bones\BoneTransformer.h"

namespace br {
	class ModelMouseProcessor : public ProcessorBase{
	public:
		ModelMouseProcessor(std::shared_ptr<AssetLoader>loader, std::pair<std::string, std::string> shaders);
		~ModelMouseProcessor();
	
		int32_t getMouseOver() const { return mouseOver; }
	protected:
		void start(std::weak_ptr<IGraphicsConnector> graphics) override;
		void stop() override;

		void doStep(const StepData& stepData) override;

		BoneTransformer boneTransformer;
		int32_t mouseOver = -1;

		struct Triangle {
			glm::vec3 a;
			glm::vec3 b;
			glm::vec3 c;
		};
		static const float MAX_DISTANCE;
		float calcDistance(glm::vec3& rFrom, glm::vec3& rTo, Triangle&);
	};
}


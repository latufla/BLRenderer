#pragma once
#include <memory>
#include "../assets/interfaces/IAssetLoader.h"
#include "../graphics/interfaces/IGraphicsConnector.h"
#include "../assets/interfaces/IProgram3d.h"
#include "../graphics/interfaces/IProgramContext.h"

namespace br {
	class ProcessorBase {
	public:
		ProcessorBase(std::shared_ptr<IAssetLoader> loader);
		virtual ~ProcessorBase();

		void addProcessor(std::shared_ptr<ProcessorBase>);
		void removeProcessor(std::shared_ptr<ProcessorBase>);

	protected:
		std::shared_ptr<IAssetLoader> loader;

		std::vector<std::shared_ptr<ProcessorBase>> processors;

		std::weak_ptr<IGraphicsConnector> graphics;
		bool enabled = false;
		virtual void start(std::weak_ptr<IGraphicsConnector> graphics);
		virtual void stop();

		struct StepData {
			long long stepMSec;
			glm::mat4 perspectiveView;
			glm::mat4 ortho;
			void* extraData;
		};
		void tryDoStep(StepData& stepData);
		virtual void doStep(const StepData& stepData);
		virtual bool canDoStep();


		std::unordered_map<std::string, IGraphicsConnector::GpuBufferData> meshToBuffer;
		void loadGeometryToGpu(std::string key, std::vector<float>& vertices, std::vector<uint16_t>& indices);
		void deleteGeometryFromGpu(std::string key);

		std::unordered_map<std::string, uint32_t> textureToId;
		
		void loadTextureToGpu(Texture2d& texture);
		void deleteTextureFromGpu(std::string pathAsKey);
		bool hasTextureInGpu(std::string pathAsKey);

		std::unordered_map<std::string, std::shared_ptr<IProgramContext>> nameToProgramContext;
		void loadProgramToGpu(std::string key, std::weak_ptr<IProgram3d> program);
		void deleteProgramFromGpu(std::string key);
		bool hasProgramInGpu(std::string key);

		friend class Renderer;
	};
}


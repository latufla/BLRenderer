#pragma once
#include <memory>
#include "../AssetLoader.h"
#include "../WindowVendor.h"

namespace br {
	class ProcessorBase {
	public:
		ProcessorBase(std::shared_ptr<AssetLoader> loader, std::pair<std::string, std::string> shaders);
		virtual ~ProcessorBase();

	protected:
		std::shared_ptr<AssetLoader> loader;
		std::pair<std::string, std::string> shaders;

		std::weak_ptr<WindowVendor> window;
		bool enabled = false;
		void start(std::weak_ptr<WindowVendor> window);
		void stop();

		struct StepData {
			long long stepMSec;
			glm::mat4 perspectiveView;
			glm::mat4 ortho;
		};
		void tryDoStep(StepData& stepData);
		virtual void doStep(StepData& stepData) = 0;
		virtual bool canDoStep();


		struct ProgramContext {
			int32_t id = -1;

			int32_t position = -1;
			int32_t texPosition = -1;

			int32_t bones = -1;
			int32_t boneIds = -1;
			int32_t weights = -1;

			int32_t sampler = -1;

			int32_t mvpMatrix = -1;
			int32_t color = -1;
		} program;

		ProgramContext createProgram(std::pair<std::string, std::string> shaders);
		virtual ProgramContext fillProgramContext(uint32_t);
		uint32_t createShader(uint32_t type, const char* source);
		
	
		struct GpuBufferData {
			uint32_t vBuffer;
			uint32_t iBuffer;
			uint32_t iBufferLenght;

			uint32_t texture;
		};

		std::unordered_map<std::string, GpuBufferData> meshToBuffer;


		std::unordered_map<std::string, uint32_t> textureToId;
		
		void loadTextureToGpu(Texture2d& texture);
		void deleteTextureFromGpu(std::string pathAsKey);
		bool hasTextureInGpu(std::string pathAsKey);

		friend class Renderer;
	};
}


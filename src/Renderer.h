#pragma once
#include <vector>
#include <memory>
#include "AssetLoader.h"
#include "utils\GraphicsConnector.h"

namespace br {	
	class ProcessorBase;
	class Renderer
	{
	public:
		Renderer() = delete;
		~Renderer() = default;

		Renderer(std::shared_ptr<AssetLoader> loader,
			uint32_t wndX,
			uint32_t wndY, 
			uint32_t wndW, 
			uint32_t wndH);
			
		bool doStep(long long stepMSec);
		
		void setCamera(float x, float y, float z);

		void addProcessor(std::shared_ptr<ProcessorBase> val);
		void removeProcessor(std::shared_ptr<ProcessorBase> val);

	private:
		std::shared_ptr<AssetLoader> loader;
	
		std::shared_ptr<GraphicsConnector> gConnector;

		struct Camera {
			float x;
			float y;
			float z;
		} camera;
	
		std::vector<std::shared_ptr<ProcessorBase>> processors;
	};
}

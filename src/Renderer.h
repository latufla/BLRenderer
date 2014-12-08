#pragma once
#include <vector>
#include <memory>
#include <unordered_map>
#include <glm.hpp>

#include "WindowVendor.h"
#include "Utils.h"

#include "Model3dLoader.h"
#include "View.h"
#include <array>
#include "bones\BNode.h"
#include "bones\BoneTransformer.h"

namespace br {	
	class Renderer
	{
	public:
		Renderer() = delete;

		Renderer(std::shared_ptr<Model3dLoader> loader,
			uint32_t wndX,
			uint32_t wndY, 
			uint32_t wndW, 
			uint32_t wndH);

		~Renderer();
	
		void addObject(uint32_t id, std::string modelPath);
		void removeObject(uint32_t id);
	
		bool doStep(uint32_t stepMSec);
		
		bool playAnimation(uint32_t objId, std::string animName = Animation3d::DEFAULT_ANIMATION_NAME);
		bool transform(uint32_t objId, const std::array<float, 16> tForm);
	
		void setCamera(float x, float y, float z);
	
	private:
		std::shared_ptr<WindowVendor> window;

		std::shared_ptr<Model3dLoader> loader;
		
		struct Camera {
			float x;
			float y;
			float z;
		} camera;
	
		std::unordered_map<uint32_t, View> idToObject;
		
		// ogl
		struct GpuBufferData {
			uint32_t vBuffer;
			uint32_t iBuffer;
			uint32_t iBufferLenght;
	
			uint32_t texture;
		};
		std::unordered_map<std::string, GpuBufferData> meshToBuffer;
	
		bool loadModelToGpu(std::string);
		bool deleteModelFromGpu(std::string);
	
		bool hasObjectWithModel(std::string);
		
		struct EglContext {
			void* display;
			void* surface;
			void* context;
		} eglContext;
		
		int32_t defaultProgram;
		
		void initEgl();
		void initShaders(std::string, std::string);
	
		uint32_t createShader(uint32_t, const char*);
		uint32_t loadTextureToGpu(std::vector<uint8_t>&, int16_t, int16_t);
		// ---
		
		// animation
		BoneTransformer boneTransformer;
		BoneTransformer::BonesDataMap prepareAnimationStep(View&, Mesh3d&, uint32_t);
		//
	};
}

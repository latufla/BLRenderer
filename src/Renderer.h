#pragma once
#include <vector>
#include <memory>
#include <unordered_map>
#include <glm.hpp>

#include "WindowVendor.h"
#include "utils/Util.h"

#include "AssetLoader.h"
#include "View.h"
#include <array>
#include "bones\BNode.h"
#include "bones\BoneTransformer.h"
#include "utils\Image.h"
#include "text\TextField.h"
#include <utility>
#include "utils\Shaders.h"

namespace br {	
	class Renderer
	{
	public:
		Renderer() = delete;

		Renderer(std::shared_ptr<AssetLoader> loader,
			uint32_t wndX,
			uint32_t wndY, 
			uint32_t wndW, 
			uint32_t wndH);

		~Renderer();
	
		void addObject(uint32_t id, std::string modelPath);
		void removeObject(uint32_t id);
	
		bool doStep(uint32_t stepMSec);
		
		void playAnimation(uint32_t objId, std::string animName = Animation3d::DEFAULT_ANIMATION_NAME, bool loop = true);
		void transformObject(uint32_t objId, const std::array<float, 16> tForm);
	
		void setCamera(float x, float y, float z);

		void addTextField(uint32_t id, std::string text, std::string font, uint8_t fontSize, std::array<float, 4> color, std::pair<float, float> position);
		void removeTextField(uint32_t id);
		void translateTextField(uint32_t id, std::pair<float, float> position);

		void addImage(uint32_t id, std::string path, std::pair<float, float> position);
		void removeImage(uint32_t id);

	private:
		std::shared_ptr<WindowVendor> window;

		std::shared_ptr<AssetLoader> loader;

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
	
		void loadModelToGpu(std::string);
		void deleteModelFromGpu(std::string);
	
		bool hasObjectWithModel(std::string);
		
		struct EglContext {
			void* display;
			void* surface;
			void* context;
		} eglContext;
		
		struct ProgramContext {
			uint32_t id;
			
			uint32_t position;
			uint32_t texPosition;
			
			uint32_t bones;
			uint32_t boneIds;
			uint32_t weights;

			uint32_t sampler;
		
			uint32_t mvpMatrix;
			uint32_t color;
		} modelProgram, imageProgram, textProgram;

		void initEgl();

		Shaders shaders;
		void initShaders();
		ProgramContext createProgram(std::string, std::string);
	
		uint32_t createShader(uint32_t, const char*);
		uint32_t loadTextureToGpu(Texture2d&);
		void deleteTextureFromGpu(Texture2d&);
		
		// ui
		void drawUI();

		std::unordered_map<uint32_t, TextField> idToTextField;
		std::unordered_map<std::string, GpuBufferData> fontToBuffer;

		void loadFontToGpu(Font&);
		void deleteFontFromGpu(Font&);

		void loadTextFieldToGpu(TextField&);
		bool hasTextFieldWithFont(Font&);

		void drawTextFields(glm::mat4&);

		std::unordered_map<uint32_t, Image> idToImage;
		std::unordered_map<std::string, GpuBufferData> textureToBuffer;
		
		void loadImageToGpu(Image&);
		bool hasImageWithTexture(std::string path);

		void drawImages(glm::mat4&);
		// ---
		
		// animation
		BoneTransformer boneTransformer;
		BoneTransformer::BonesDataMap prepareAnimationStep(View&, Mesh3d&, uint32_t);
		//	
	};
}

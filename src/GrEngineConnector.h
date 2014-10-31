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
#include "bone\BNode.h"
#include "bone\BoneTransformer.h"

class GrEngineConnector
{
public:
	~GrEngineConnector();
	
	static GrEngineConnector& getInstance(){
		static GrEngineConnector instance;
		return instance;
	}

	int32_t init(uint32_t, uint32_t, uint32_t, uint32_t);
	
	bool loadModel(std::string, std::string);
	bool attachAnimation(std::string, std::string, std::string);

	bool addObject(uint32_t, std::string);
	bool removeObject(uint32_t);

	bool doStep(uint32_t);
	
	bool playAnimation(uint32_t, std::string = "default");
	bool transform(uint32_t, const std::array<float, 16>);

	void setCamera(float, float, float);

private:
	GrEngineConnector(){};
	GrEngineConnector(GrEngineConnector &){};
	GrEngineConnector operator=(GrEngineConnector&){};

	std::shared_ptr<WindowVendor> window;
	uint32_t timeMSec = 0;

	Model3dLoader loader;
	
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
	
	int32_t initEgl();
	int32_t initShaders(std::string, std::string);

	uint32_t createShader(uint32_t, const char*);
	uint32_t loadTextureToGpu(std::vector<unsigned char>&, int16_t, int16_t);
	// ---
	
	// animation
	BoneTransformer boneTransformer;
	BoneTransformer::BonesDataMap prepareAnimationStep(View&, Mesh3d&, uint32_t);
	//
};


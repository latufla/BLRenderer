#pragma once
#include <vector>
#include <memory>

#include <string>
#include "WindowVendor.h"
#include "Utils.h"

#include <array>
#include "Model3dLoader.h"
#include <unordered_map>
#include <glm.hpp>
#include "View.h"

class GrEngineConnector
{
public:
	~GrEngineConnector();
	
	static GrEngineConnector& getInstance(){
		static GrEngineConnector instance;
		return instance;
	}

	int32_t init();
	
	bool loadModel(std::string, std::string);
	
	bool addObject(uint32_t, std::string);
	bool removeObject(uint32_t);

	bool doStep(uint32_t);

	bool transform(uint32_t, const glm::mat4&);

	struct Camera {
		float x;
		float y;
		float z;
	};
	void setCamera(float, float, float);

private:
	GrEngineConnector(){};
	GrEngineConnector(GrEngineConnector &){};
	GrEngineConnector operator=(GrEngineConnector&){};

	std::shared_ptr<WindowVendor> window;
	
	Model3dLoader loader;
	Camera camera;

	std::unordered_map<uint32_t, View> idToObject;
	
	// ogl
	struct GpuBufferData {
		uint32_t vBuffer;
		uint32_t iBuffer;
		uint32_t iBufferLenght;

		uint32_t texture;
	};
	std::unordered_map<std::string, GpuBufferData> meshToBuffer;

	bool loadToGpu(std::string);
	bool deleteFromGpu(std::string);

	bool hasObjectWith(std::string);
	
	void* display;
	void* surface;
	void* context;
	int32_t defaultProgram;
	
	int32_t initEgl();
	int32_t initShaders(std::string, std::string);

	uint32_t loadShader(uint32_t, const char*);
	uint32_t loadTexture(std::vector<unsigned char>&, int16_t, int16_t);
	// ---


	// animation
	struct BoneData {
		glm::mat4 offset;
		glm::mat4 finalTransform;
	};
	typedef std::unordered_map<uint32_t, BoneData> BonesDataMap;
	BonesDataMap createBonesData(std::shared_ptr<Model3d>, std::shared_ptr<Animation3d>, Mesh3d&);
	void transformBonesData(const glm::mat4&, Node::NodePtr, std::shared_ptr<Animation3d>, glm::mat4, BonesDataMap&);
	//
};


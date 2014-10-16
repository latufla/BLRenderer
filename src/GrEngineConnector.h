#pragma once
#include <vector>
#include <memory>

#include <string>
#include "WindowVendor.h"
#include "ObjectBase.h"
#include "Utils.h"

#include <map>
#include <utility>
#include "Model3dLoader.h"

struct BufferInfo {
	uint32_t id;
	uint32_t length; // not size in bytes
};

struct Camera {
	float x;
	float y;
	float z;
};

class GrEngineConnector
{
public:
	~GrEngineConnector();
	
	static GrEngineConnector& getInstance(){
		static GrEngineConnector instance;
		return instance;
	}

	int init();
	bool add(ObjectBase*);
	bool remove(ObjectBase*);
	bool draw();

	void setCamera(float, float, float);
	
	std::shared_ptr<WindowVendor> getWindow(){ return window; }

private:
	GrEngineConnector(){};
	GrEngineConnector(GrEngineConnector &){};
	GrEngineConnector operator=(GrEngineConnector&){};

	std::shared_ptr<WindowVendor> window;

	std::vector<ObjectBase*> objects;
	std::map<ObjectBase*, unsigned int> textures;
	
	std::map<std::string, unsigned int> meshToMaterial;

	typedef std::pair<BufferInfo, BufferInfo> buffer_pair;
	std::map<std::string, buffer_pair> meshToBuffer;

	Model3dLoader loader;

	Camera camera;
	
	// TODO: p implement maybe
	void* display;
	void* surface;
	void* context;
	int defaultProgram;
	//

	int initEgl();
	int initShaders(std::string, std::string);

	unsigned int loadShader(unsigned int, const char*);
	unsigned int loadTexture(std::vector<unsigned char>&, int16_t, int16_t);
};


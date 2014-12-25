// BLRenderer.cpp : Defines the entry point for the console application.
//

#include "src/utils/SharedHeaders.h"
#include <iostream>

#include "ObjectBase.h"
#include "src/Renderer.h"
#include "Model3dInfo.h"
#include <memory>
#include "src/exceptions/Exception.h"
#include <chrono>

const std::string SLIME_WARRIOR = "SlimeRed";
const std::string SLIME_HEALER = "SlimeBlue";
const std::string CURE_FX = "CureFx";
const std::string GAME_OBJECT = SLIME_WARRIOR;

#define FRONT_VIEW 0.0f,1.0f,8.34f
#define RIGHT_VIEW 7.48f,1.0f,0.0f
#define CUSTOM_VIEW 7.48f,2.0f,8.34f
#define DEFAULT_VIEW 7.48f,6.5f,8.34f

std::vector<ObjectBase> objects;

void run();
long long getElapsedTimeMSec();
void handleExceptions();

void runModels(std::shared_ptr<br::AssetLoader>, br::Renderer&);
void runImages(std::shared_ptr<br::AssetLoader>, br::Renderer&);
void runTextFields(std::shared_ptr<br::AssetLoader>, br::Renderer&);

auto runTarget = runModels;

int _tmain(int argc, _TCHAR* argv[]) {
	try {
		run();
 	} catch(...) {
 		handleExceptions();
 	}
	return 0;
}

void run() {
	std::shared_ptr<br::AssetLoader> loader = std::make_shared<br::AssetLoader>();
	br::Renderer renderer{loader, 0, 0, 1024, 768};
	
	runTarget(loader, renderer);
	
	const uint32_t step = 1000 / 60;
	long long begin = getElapsedTimeMSec();
	bool running = true;
	while(running) {
		long long elapsedTime = getElapsedTimeMSec() - begin;		
		if(elapsedTime >= step) {
			running = renderer.doStep(step);
			begin = getElapsedTimeMSec() - (elapsedTime - step);
		}
	}
}

long long getElapsedTimeMSec() {
	using namespace std::chrono;
	auto now = high_resolution_clock::now();
	auto mSec = duration_cast<milliseconds>(now.time_since_epoch());
	return mSec.count();
}

void handleExceptions() {
	std::string error = "";
	try {
		throw;
	} catch(br::Exception& e) {
		error = e.msg();
	} catch(std::exception& e) {
		error = e.what();
	} catch(...) {
		error = "unknown exception";
	}

	if(error != "") {
		std::cout << error;
		std::exit(1);
	}
}

void runModels(std::shared_ptr<br::AssetLoader> loader, br::Renderer& renderer) {
	objects.push_back({42, GAME_OBJECT});

	std::unordered_map<std::string, std::string> nameToAnimation{
	//			{"walk", "Walk.dae"}
	};

	const Model3dInfo info(GAME_OBJECT, nameToAnimation);

	std::string pathAsKey = info.getModelPath();
	std::string modelDirectory = info.getModelDirectory();

	loader->loadModel(pathAsKey, modelDirectory);
	for(auto& i : nameToAnimation) {
		loader->attachAnimation(pathAsKey, i.first, modelDirectory + i.second);
	}

	renderer.setCamera(CUSTOM_VIEW);

	for(auto& s : objects) {
		uint32_t id = s.getId();
		renderer.addObject(id, pathAsKey); // one model whatever
		renderer.transformObject(id, br::Util::toArray(s.getOrientation()));
	}

	renderer.playAnimation(42, "default");
}

void runImages(std::shared_ptr<br::AssetLoader> loader, br::Renderer& renderer) {
	loader->loadTexture("models/cat.png");
	renderer.addImage(0, "models/cat.png", {0.5f, 0.0f});
	renderer.addImage(1, "models/cat.png", {-0.2f, 0.0f});

	// 	renderer.removeImage(0);
	// 	renderer.removeImage(1);
}

void runTextFields(std::shared_ptr<br::AssetLoader> loader, br::Renderer& renderer) {
	loader->loadFont("fonts/arial.ttf", "arial", 20);

	std::array<float, 4> color = {0.0f, 0.0f, 1.0f, 1.0f};
	std::pair<float, float> pos = {0.5f, 0.0f};
	renderer.addTextField(0, "+20 -150 Decrease Def", "arial", 20, color, pos);

	std::array<float, 4> color2 = {1.0f, 0.0f, 1.0f, 1.0f};
	std::pair<float, float> pos2 = {0.5f, 0.5f};
	renderer.addTextField(1, "+20 atk", "arial", 20, color2, pos2);

	std::pair<float, float> pos3 = {-0.5f, 0.0f};
	renderer.translateTextField(1, pos3);

//  	renderer.removeTextField(0);
//  	renderer.removeTextField(1);
}



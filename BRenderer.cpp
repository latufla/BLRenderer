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
#include "src/processors/images/ImageRenderProcessor.h"
#include "src/utils/Shaders.h"
#include "src/processors/text/TextRenderProcessor.h"
#include "src/processors/models/ModelRenderProcessor.h"
#include "src/utils/Util.h"
#include "src/processors/models/ModelMouseProcessor.h"

#include "src/graphics/GlConnector.h"

const std::string SLIME_WARRIOR = "SlimeRed";
// TODO: not ready
// const std::string SLIME_HEALER = "SlimeBlue";
// const std::string CURE_FX = "CureFx";
const std::string GAME_OBJECT = SLIME_WARRIOR;

#define FRONT_VIEW 0.0f,-8.0f,0.0f
#define RIGHT_VIEW 8.0f,0.0f,0.0f
#define CUSTOM_VIEW 8.0f,-8.0f,2.0f

std::vector<ObjectBase> objects;

void run();
long long getElapsedTimeMSec();
void handleExceptions();

void runModels(std::shared_ptr<br::AssetLoader>, br::Renderer&);
void runImages(std::shared_ptr<br::AssetLoader>, br::Renderer&);
void runTextFields(std::shared_ptr<br::AssetLoader>, br::Renderer&);

auto runTarget = runImages;

int _tmain(int argc, _TCHAR* argv[]) {
	try {
		run();
 	} catch(...) {
 		handleExceptions();
 	}

	return 0;
}

void run() {
	auto loader = std::make_shared<br::AssetLoader>();
	const br::IWindowVendor::Rect size{0.0f, 0.0f, 1024.0f, 768.0f};
	auto graphcics = std::make_shared<br::GlConnector>(size);
	br::Renderer renderer(loader, graphcics);
	
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
	ObjectBase obj{42, GAME_OBJECT};
	obj.translate(0.0f, 0.0f, 0.0f);
	objects.push_back(obj);

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

	renderer.setCamera(FRONT_VIEW);

	br::Shaders shaders;
	auto program = shaders.getProgram(br::Shaders::MODEL_PROGRAM);
	auto modelRenderer = std::make_shared<br::ModelRenderProcessor>(loader, program);
// 	auto mouseProcessor = std::make_shared<br::ModelMouseProcessor>(loader, program);
// 	modelRenderer->addProcessor(mouseProcessor);
	renderer.addProcessor(modelRenderer);

	for(auto& s : objects) {
		uint32_t id = s.getId();
		modelRenderer->addObject(id, pathAsKey); // one model whatever
		modelRenderer->transformObject(id, s.getOrientation());
	}

	modelRenderer->playAnimation(42, "default");
}

void runImages(std::shared_ptr<br::AssetLoader> loader, br::Renderer& renderer) {
	loader->loadTexture("models/cat.png");

	br::Shaders shaders;
	auto program = shaders.getProgram(br::Shaders::IMAGE_PROGRAM);
	auto imageRenderer = std::make_shared<br::ImageRenderProcessor>(loader, program);
	renderer.addProcessor(imageRenderer);

	imageRenderer->addImage(0, "models/cat.png", {-1.0f, 0.0f});
	imageRenderer->addImage(1, "models/cat.png", {0.0f, 0.0f});

// 	imageRenderer->removeImage(0);
// 	imageRenderer->removeImage(1);
}

void runTextFields(std::shared_ptr<br::AssetLoader> loader, br::Renderer& renderer) {
	loader->loadFont("fonts/arial.ttf", "arial", 20);

	const glm::vec4 color{0.0f, 0.0f, 1.0f, 1.0f};
	const glm::vec2 pos{0.5f, 0.0f};

	br::Shaders shaders;
	auto program = shaders.getProgram(br::Shaders::TEXT_PROGRAM);
	auto textRenderer = std::make_shared<br::TextRenderProcessor>(loader, program);
	renderer.addProcessor(textRenderer);

	textRenderer->addTextField(0, "+20 -150 Decrease Def", "arial", 20, color, pos);

	const glm::vec4 color2 = {1.0f, 0.0f, 1.0f, 1.0f};
	const glm::vec2 pos2 = {0.5f, 0.5f};
	textRenderer->addTextField(1, "+20 atk", "arial", 20, color2, pos2);

	const glm::vec2 pos3 = {-0.5f, 0.0f};
	textRenderer->translateTextField(1, pos3);

// 	textRenderer->removeTextField(0);
//  textRenderer->removeTextField(1);
}



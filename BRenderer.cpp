// BLRenderer.cpp : Defines the entry point for the console application.
//

#include "src/SharedHeaders.h"
#include <iostream>

#include "ObjectBase.h"
#include "src/Renderer.h"
#include "Model3dInfo.h"
#include <memory>
#include "src/exceptions/Exception.h"

const std::string CUBE = "Cube";
const std::string GUN = "Gun";
const std::string POKEMON_TRAINER = "PokemonTrainer";
const std::string SPIDERMAN_SYMBIOTE = "SpidermanSymbiote";
const std::string STAN_LEE = "StanLee";

std::vector<ObjectBase> objects;


void run();
void handleExceptions();

int _tmain(int argc, _TCHAR* argv[]) {
	try {
		run();
 	} catch(...) {
 		handleExceptions();
 	}

	return 0;
}

void run() {
	objects.push_back({42, CUBE});

	std::unordered_map<std::string, std::string> nameToAnimation{
			{"idle", "CubeIdle.dae"}};

	const Model3dInfo info(CUBE, nameToAnimation);
	
	std::string pathAsKey = info.getModelPath();
	std::string modelDirectory = info.getModelDirectory();
	
	std::shared_ptr<br::Model3dLoader> loader = std::make_shared<br::Model3dLoader>();
	loader->loadModel(pathAsKey, modelDirectory);
	for(auto& i : nameToAnimation) {
		loader->attachAnimation(pathAsKey, i.first, modelDirectory + i.second);
	}
	
	br::Renderer renderer{loader, 0, 0, 1024, 768};

 	renderer.setCamera(7.48f, 6.5f, 5.34f);

	for (auto& s : objects) {
		uint32_t id = s.getId();
		renderer.addObject(id, pathAsKey); // one model whatever
		renderer.transform(id, br::Utils::toArray(s.getOrientation()));
	}

	renderer.playAnimation(42, "idle");

	const float fps = 1.0f / 60;
	const uint32_t step = (uint32_t)(fps * 1000);
	bool done = false;
	while (!done) {
		Sleep(step); // TODO: shirt WND 
		done = !renderer.doStep(step);
	}
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
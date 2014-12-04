// BLRenderer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>

#include "ObjectBase.h"
#include "src\Renderer.h"
#include "Model3dInfo.h"
#include <memory>

using std::vector;

const std::string CUBE = "Cube";
const std::string GUN = "Gun";
const std::string POKEMON_TRAINER = "PokemonTrainer";
const std::string SPIDERMAN_SYMBIOTE = "SpidermanSymbiote";
const std::string STAN_LEE = "StanLee";

vector<ObjectBase> objects;

int _tmain(int argc, _TCHAR* argv[]) {

 	objects.push_back({ 42, CUBE });	

	const Model3dInfo info(CUBE);
	
	std::shared_ptr<br::Model3dLoader> loader = std::make_shared<br::Model3dLoader>();
 	loader->loadModel(info.getModelDir(), info.getModelName());
	loader->attachAnimation(info.getModelPath(), "models/Cube/CubeIdle.dae", "idle");
	
	br::Renderer renderer{loader, 0, 0, 1024, 768};

 	renderer.setCamera(7.48f, 6.5f, 5.34f);

	for (auto& s : objects) {
		uint32_t id = s.getId();
		renderer.addObject(id, info.getModelPath());
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

	return 0;
}

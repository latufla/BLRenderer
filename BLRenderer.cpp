// BLRenderer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <memory>

#include "src\WindowVendor.h"

#include "ObjectBase.h"
#include "src\GrEngineConnector.h"
#include "src\Infos.h"
#include "src\Utils.h"
#include <gtc\type_ptr.hpp>

using namespace std;

vector<shared_ptr<ObjectBase>> objects;

int _tmain(int argc, _TCHAR* argv[]) {
	{
		shared_ptr<ObjectBase> obj1 = make_shared<ObjectBase>(42, CUBE);

		obj1->scale(0.1f, 0.1f, 0.1f);
		obj1->rotateY(90.0f);
		objects.push_back(obj1);

		shared_ptr<ObjectBase> obj2 = make_shared<ObjectBase>(2, CUBE);
		obj2->scale(0.05f, 0.05f, 0.05f);
		//objects.push_back(obj2);
	}

	GrEngineConnector& renderer = GrEngineConnector::getInstance();

	int rendererFail = renderer.init();
	if (rendererFail)
		return rendererFail;

	renderer.setCamera(0.0f, 0.0f, 1.0f);
	

	const Model3dInfo& info = Infos::getInfo(CUBE);
	renderer.loadModel(info.getModelDir(), info.getModelName());

	renderer.attachAnimation(info.getModelPath(), "models/Cube/CubeIdle.dae", "idle");


	for (shared_ptr<ObjectBase> s : objects) {
		uint32_t id = s->getId();
		const Model3dInfo& info = Infos::getInfo(s->getInfo());
		renderer.addObject(id, info.getModelPath());

		renderer.transform(id, Utils::glmMatrixToArray(s->getOrientation()));
	}

	renderer.playAnimation(42, "idle");


// 	renderer.removeObject(2);
// 	renderer.removeObject(1);


	const float fps = 1.0 / 60;
	const uint32_t step = (uint32_t)(fps * 1000);
	bool done = false;
	while (!done) {
		Sleep(step); // TODO: shirt WND 

		shared_ptr<ObjectBase> obj = objects[0];
// 		obj->rotateY(3.0f);
// 		renderer.transform(obj->getId(), Utils::glmMatrixToArray(obj->getOrientation()));

		done = !renderer.doStep(step);
	}

	return 0;
}

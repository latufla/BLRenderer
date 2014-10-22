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

using namespace std;

vector<shared_ptr<ObjectBase>> objects;

int _tmain(int argc, _TCHAR* argv[]) {
	{
		shared_ptr<ObjectBase> obj1 = make_shared<ObjectBase>(1, GUN);

		obj1->scale(0.05f, 0.05f, 0.05f);
		obj1->rotateY(90.0f);
		objects.push_back(obj1);
	}

	GrEngineConnector& renderer = GrEngineConnector::getInstance();

	int rendererFail = renderer.init();
	if (rendererFail)
		return rendererFail;

	renderer.setCamera(0.0f, 0.0f, 1.0f);
	

	const Model3dInfo& info = Infos::getInfo(GUN);
	renderer.loadModel(info.getModelDir(), info.getName());

	for (shared_ptr<ObjectBase> s : objects) {
		uint32_t id = s->getId();
		const Model3dInfo& info = Infos::getInfo(s->getInfo());
		renderer.addObject(id, info.getModelPath());
		renderer.transform(id, s->getOrientation());
	}

	const float fps = 1 / 60;
	const uint32_t step = fps * 1000;
	bool done = false;
	while (!done) {
		Sleep(step); // TODO: shirt WND 

		shared_ptr<ObjectBase> obj = objects[0];
		obj->rotateY(0.05f);
		renderer.transform(obj->getId(), obj->getOrientation());

		done = !renderer.doStep(step);
	}

	return 0;
}

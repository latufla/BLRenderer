// BLRenderer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <memory>

#include "src\WindowVendor.h"

#include "src\ObjectBase.h"
#include "src\GrEngineConnector.h"
#include "src\Infos.h"
#include "src\Utils.h"

using namespace std;

vector<shared_ptr<ObjectBase>> objects;

int _tmain(int argc, _TCHAR* argv[]) {
	{
		shared_ptr<ObjectBase> obj1 = make_shared<ObjectBase>(GUN);

		obj1->scale(0.05f, 0.05f, 0.05f);
		obj1->rotateY(90.0f);
		objects.push_back(obj1);
	}

	GrEngineConnector& renderer = GrEngineConnector::getInstance();

	int rendererFail = renderer.init();
	if (rendererFail)
		return rendererFail;

	renderer.setCamera(0.0f, 0.0f, 1.0f);

	for (shared_ptr<ObjectBase> s : objects) {
		renderer.add(s.get());
	}

	shared_ptr<WindowVendor>mainWindow = renderer.getWindow();

	const float fps = 1 / 60;
	bool done = false;
	while (!done) {
		objects[0].get()->rotateY(0.05f);

		renderer.draw();

		done = !mainWindow->doStep();

		Sleep((unsigned long)(fps * 1000)); // TODO: shirt WND 
	}

	return 0;
}

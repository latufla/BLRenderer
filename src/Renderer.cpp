#include "utils/SharedHeaders.h"
#include "Renderer.h"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include "exceptions/Exception.h"
#include "processors/ProcessorBase.h"

using std::string;

using std::shared_ptr;
using std::make_shared;

using glm::mat4;
using glm::vec3;
using glm::lookAt;
using glm::perspective;
using glm::ortho;


namespace br {
	Renderer::Renderer(shared_ptr<IAssetLoader> loader, shared_ptr<IGraphicsConnector> graphics)
		: loader(loader), graphics(graphics) {
	}

	void Renderer::addProcessor(shared_ptr<ProcessorBase> val) {
		auto it = find(cbegin(processors), cend(processors), val);
		if(it != cend(processors))
			throw LogicException(EXCEPTION_INFO, "has such processor");

		val->start(graphics);
		processors.push_back(val);
	}

	void Renderer::removeProcessor(shared_ptr<ProcessorBase> val) {
		auto it = find(cbegin(processors), cend(processors), val);
		if(it == cend(processors))
			throw LogicException(EXCEPTION_INFO, "no such processor");

		val->stop();
		processors.erase(it);
	}

	void Renderer::setCamera(float x, float y, float z) {
		camera.x = x;
		camera.y = y;
		camera.z = z;
	}

	bool Renderer::doStep(long long stepMSec) {
		auto wSize = graphics->getWindowSize();
		graphics->setViewport(wSize);
		graphics->clear();
	
 		mat4 view = lookAt(vec3{camera.x, camera.y, camera.z}, vec3{0, 0, 0}, vec3{0, 0, 1});
		mat4 projection = perspective(45.0f, wSize.w / wSize.h, 0.1f, 100.0f);
		mat4 projectionView = projection * view;

		float ratio = (float)wSize.w / (float)wSize.h;
		mat4 orthoProjection = ortho(-ratio, ratio, -1.f, 1.f);
		for(auto i : processors) {
			ProcessorBase::StepData stepData {
				stepMSec, 
				projectionView, 
				orthoProjection,
				nullptr
			};
			i->tryDoStep(stepData);
		}
		graphics->swapBuffers();

		return graphics->doStep();
	}
}

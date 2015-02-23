#include "../utils/SharedHeaders.h"
#include "ProcessorBase.h"
#include "../exceptions/Exception.h"

using std::pair;
using std::string;
using std::vector;
using std::shared_ptr;
using std::weak_ptr;

namespace br {
	ProcessorBase::ProcessorBase(shared_ptr<IAssetLoader> loader, pair<string, string> shaders)
		: loader(loader), shaders(shaders) {
	}

	ProcessorBase::~ProcessorBase() {
		stop();

		auto allTextures = textureToId;
		for(auto i : allTextures) {
			deleteTextureFromGpu(i.first);
		}
	}

	void ProcessorBase::addProcessor(shared_ptr<ProcessorBase> val) {
		auto it = find(cbegin(processors), cend(processors), val);
		if(it != cend(processors))
			throw LogicException(EXCEPTION_INFO, "has such processor");

		processors.push_back(val);
	}

	void ProcessorBase::removeProcessor(shared_ptr<ProcessorBase> val) {
		auto it = find(cbegin(processors), cend(processors), val);
		if(it == cend(processors))
			throw LogicException(EXCEPTION_INFO, "no such processor");

		val->stop();
		processors.erase(it);
	}

	void ProcessorBase::loadTextureToGpu(Texture2d& texture) {
		if(hasTextureInGpu(texture.getPath()))
			return;

		auto sGConnector = graphics.lock();
		if(!sGConnector)
			throw WeakPtrException(EXCEPTION_INFO);

		uint32_t textureId = sGConnector->loadTextureToGpu(texture);
		textureToId.emplace(texture.getPath(), textureId);
	}

	void ProcessorBase::deleteTextureFromGpu(string pathAsKey) {
		auto sGConnector = graphics.lock();
		if(!sGConnector)
			throw WeakPtrException(EXCEPTION_INFO);

		uint32_t textureId = textureToId.at(pathAsKey);
		sGConnector->deleteTextureFromGpu(textureId);

		textureToId.erase(pathAsKey);
	}

	bool ProcessorBase::hasTextureInGpu(string pathAsKey) {
		auto& it = find_if(cbegin(textureToId), cend(textureToId), [&pathAsKey](pair<string, uint32_t> i)->bool {
			return i.first == pathAsKey;
		});
		return it != cend(textureToId);
	}

	void ProcessorBase::start(weak_ptr<IGraphicsConnector> graphics) {
		this->graphics = graphics;
		enabled = true;

		auto sGConnector = graphics.lock();
		if(!sGConnector)
			throw WeakPtrException(EXCEPTION_INFO);

		program = sGConnector->createProgram(shaders);

		for(auto s : processors) {
			s->start(graphics);
		}
	}

	void ProcessorBase::stop() {
		enabled = false;

		auto sGConnector = graphics.lock();
		if(!sGConnector)
			throw WeakPtrException(EXCEPTION_INFO);

		sGConnector->deleteProgram(program);

		for(auto s : processors) {
			s->stop();
		}
	}

	void ProcessorBase::tryDoStep(StepData& stepData) {
		if(canDoStep())
			doStep(stepData);
	}

	bool ProcessorBase::canDoStep() {
		return enabled;
	}

	void ProcessorBase::doStep(const StepData& stepData) {
		for(auto& s : processors) {
			s->doStep(stepData);
		}
	}

	void ProcessorBase::loadGeometryToGpu(string key, vector<float>& vertices, vector<uint16_t>& indices) {
		auto sGConnector = graphics.lock();
		if(!sGConnector)
			throw WeakPtrException(EXCEPTION_INFO);

		auto buffer = sGConnector->loadGeometryToGpu(vertices, indices);
		meshToBuffer.emplace(key, buffer);
	}

	void ProcessorBase::deleteGeometryFromGpu(string key) {
		auto sGConnector = graphics.lock();
		if(!sGConnector)
			throw WeakPtrException(EXCEPTION_INFO);

		auto buffer = meshToBuffer.at(key);
		sGConnector->deleteGeometryFromGpu(buffer);

		meshToBuffer.erase(key);
	}

}

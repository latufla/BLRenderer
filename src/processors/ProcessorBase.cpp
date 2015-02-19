#include "../utils/SharedHeaders.h"
#include "ProcessorBase.h"
#include "../exceptions/Exception.h"
#include <algorithm>

using std::pair;
using std::string;
using std::vector;
using std::shared_ptr;
using std::weak_ptr;

namespace br {
	ProcessorBase::ProcessorBase(shared_ptr<IAssetLoader> loader)
		: loader(loader){
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

		
		uint32_t textureId = sGConnector->loadTextureToGpu(texture.getData(), texture.getWidth(), texture.getHeight());
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
		return textureToId.find(pathAsKey) != textureToId.cend();
	}

	void ProcessorBase::start(weak_ptr<IGraphicsConnector> graphics) {
		this->graphics = graphics;
		enabled = true;

		for(auto s : processors) {
			s->start(graphics);
		}
	}

	void ProcessorBase::stop() {
		enabled = false;

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

	void ProcessorBase::loadProgramToGpu(std::string key, std::string vertexShader, std::string fragmentShader) {
		auto sGConnector = graphics.lock();
		if(!sGConnector)
			throw WeakPtrException(EXCEPTION_INFO);

		std::pair <string, string> shaderPair{vertexShader, fragmentShader};
		IGraphicsConnector::ProgramContext context = sGConnector->createProgram(shaderPair);
		nameToProgramContext.emplace(key, context);
	}

	void ProcessorBase::deleteProgramFromGpu(std::string key) {
		auto sGConnector = graphics.lock();
		if(!sGConnector)
			throw WeakPtrException(EXCEPTION_INFO);

		auto context = nameToProgramContext.at(key);
		sGConnector->deleteProgram(context);
		nameToProgramContext.erase(key);
	}

	bool ProcessorBase::hasProgramInGpu(std::string key) {
		return nameToProgramContext.find(key) != nameToProgramContext.cend();
	}
}

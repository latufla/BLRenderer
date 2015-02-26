#include "ProgramContext.h"


namespace br {
	ProgramContext::ProgramContext(int32_t id) 
		: id(id) {
	}

	ProgramContext::~ProgramContext() {
	}


	int32_t ProgramContext::getId() const {
		return id;
	}

	std::string ProgramContext::getPositionBinding() const {
		return "position";
	}

	std::string ProgramContext::getUvBinding() const {
		return "uv";
	}

	std::string ProgramContext::getSamplerBinding() const {
		return "sampler";
	}

	std::string ProgramContext::getMvpBinding() const {
		return "mvp";
	}

	std::string ProgramContext::getBonesBinding() const {
		return "bones";
	}

	std::string ProgramContext::getBoneIdsBinding() const {
		return "boneIds";
	}

	std::string ProgramContext::getBoneWeightsBinding() const {
		return "boneWeights";
	}

	int32_t ProgramContext::getLoc(std::string name) {
		try {
			return nameToLoc.at(name);
		} catch(std::out_of_range&) {
			return -1;
		}
	}

	void ProgramContext::setLoc(std::string name, int32_t val) {
		nameToLoc.emplace(name, val);
	}

}

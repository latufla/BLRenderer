#pragma once
#include "SharedHeaders.h"
#include <unordered_map>
#include <utility>

// default hard coded shaders source
namespace br {
	class Shaders {
	public:
		Shaders();
		~Shaders() = default;

		static const std::string MODEL_PROGRAM;
		static const std::string MODEL_DEBUG_PROGRAM;
		static const std::string TEXT_PROGRAM;
		static const std::string IMAGE_PROGRAM;
		std::pair<std::string, std::string> getProgram(const std::string name);
	private:
		std::unordered_map<std::string, std::pair<std::string, std::string>> nameToShaderPair;
	};
}

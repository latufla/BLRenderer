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

		static const std::string MODEL_SHADER;
		static const std::string TEXT_SHADER;
		static const std::string IMAGE_SHADER;
		std::pair<std::string, std::string> getShaderSrcBy(const std::string name);
	private:
		std::unordered_map<std::string, std::pair<std::string, std::string>> nameToShaderSrc;
	};
}

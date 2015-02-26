#pragma once
#include "../../utils/SharedHeaders.h"
#include <unordered_map>

namespace br {
	class IProgram3d {
	public:
		virtual std::string getName() const = 0;

		virtual std::string getVertexShader() const = 0;
		virtual std::string getFragmentShader() const = 0;

		virtual void bindAttribute(std::string name, std::string bindTo) = 0;
		virtual void bindUniform(std::string name, std::string bindTo) = 0;

		virtual std::unordered_map<std::string, std::string>& getAttributes() = 0;
		virtual std::unordered_map<std::string, std::string>& getUniforms() = 0;
	};
}
#pragma once
#include "../../utils/SharedHeaders.h"
#include <vector>

namespace br {
	class IProgram3d {
	public:
		virtual std::string getName() const = 0;

		virtual std::string getVertexShader() const = 0;
		virtual std::string getFragmentShader() const = 0;

		virtual void setAttribute(std::string name) = 0;
		virtual void setUniform(std::string name) = 0;

		virtual std::vector<std::string>& getAttributes() = 0;
		virtual std::vector<std::string>& getUniforms() = 0;
	};
}
#include "Program3d.h"

using std::string;

namespace br {
	Program3d::Program3d(string name, string vertexShader, string fragmentShader) 
		: name(name), vertexShader(vertexShader), fragmentShader(fragmentShader) {

	}

	Program3d::~Program3d() {
	}

	std::string Program3d::getName() const {
		return name;
	}

	string Program3d::getVertexShader() const {
		return vertexShader;
	}

	string Program3d::getFragmentShader() const {
		return fragmentShader;
	}

	std::vector<std::string>& Program3d::getAttributes() {
		return attributes;
	}

	std::vector<std::string>& Program3d::getUniforms() {
		return uniforms;
	}

	void Program3d::setAttribute(std::string name) {
		attributes.push_back(name);
	}

	void Program3d::setUniform(std::string name) {
		uniforms.push_back(name);
	}

}

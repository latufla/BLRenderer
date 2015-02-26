#include "Program3d.h"
#include "..\graphics\interfaces\IGraphicsConnector.h"

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

	void Program3d::bindAttribute(std::string name, std::string bindTo) {
		attributes.emplace(name, bindTo);
	}

	void Program3d::bindUniform(std::string name, std::string bindTo) {
		uniforms.emplace(name, bindTo);
	}

	std::unordered_map<std::string, std::string>& Program3d::getAttributes() {
		return attributes;
	}

	std::unordered_map<std::string, std::string>& Program3d::getUniforms() {
		return uniforms;
	}

}

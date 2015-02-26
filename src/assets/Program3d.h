#pragma once
#include "interfaces\IProgram3d.h"

namespace br {
	class Program3d : public IProgram3d{
	public:
		Program3d() = delete;
		Program3d(std::string name, std::string vertexShader, std::string fragmentShader);
		
		virtual ~Program3d();

		virtual std::string getName() const override;

		virtual std::string getVertexShader() const override;
		virtual std::string getFragmentShader() const override;

		virtual void bindAttribute(std::string name, std::string bindTo) override;
		virtual void bindUniform(std::string name, std::string bindTo) override;

		virtual std::unordered_map<std::string, std::string>& getAttributes() override;
		virtual std::unordered_map<std::string, std::string>& getUniforms() override;

	private:
		std::string name;

		std::string vertexShader;
		std::string fragmentShader;
	
		std::unordered_map<std::string, std::string> attributes;
		std::unordered_map<std::string, std::string> uniforms;
	};
}


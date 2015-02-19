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

		virtual void setAttribute(std::string name) override;
		virtual void setUniform(std::string name) override;

		virtual std::vector<std::string>& getAttributes() override;
		virtual std::vector<std::string>& getUniforms() override;

	private:
		std::string name;

		std::string vertexShader;
		std::string fragmentShader;
	
		std::vector<std::string> attributes;
		std::vector<std::string> uniforms;
	};
}


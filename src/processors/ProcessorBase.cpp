#include "../utils/SharedHeaders.h"
#include "ProcessorBase.h"

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "../exceptions/Exception.h"

using std::pair;
using std::string;

namespace br {
	ProcessorBase::ProcessorBase(std::shared_ptr<AssetLoader> loader, std::pair<std::string, std::string> shaders) 
		: loader(loader), shaders(shaders){
	}
	
	ProcessorBase::~ProcessorBase() {
		stop();

		auto allTextures = textureToId;
		for(auto i : allTextures) {
			deleteTextureFromGpu(i.first);
		}
	}

	ProcessorBase::ProgramContext ProcessorBase::createProgram(std::pair<std::string, std::string> shaiders) {
		GLuint vShader = createShader(GL_VERTEX_SHADER, shaiders.first.c_str());
		GLuint fShader = createShader(GL_FRAGMENT_SHADER, shaiders.second.c_str());

		GLuint pObject = glCreateProgram();
		if(!pObject)
			throw ShaderException(EXCEPTION_INFO, "can`t create program");

		glAttachShader(pObject, vShader);
		glAttachShader(pObject, fShader);

		glLinkProgram(pObject);

		GLint linked;
		glGetProgramiv(pObject, GL_LINK_STATUS, &linked);
		if(!linked) {
			glDeleteProgram(pObject);
			throw ShaderException(EXCEPTION_INFO, "can`t link program");
		}

		glDetachShader(pObject, vShader);
		glDeleteShader(vShader);
		glDetachShader(pObject, fShader);
		glDeleteShader(fShader);

		return fillProgramContext(pObject);
	}

	ProcessorBase::ProgramContext ProcessorBase::fillProgramContext(uint32_t pObject) {
		ProgramContext program;
		program.id = pObject;
		return program;
	}

	uint32_t ProcessorBase::createShader(uint32_t type, const char* source) {
		GLboolean hasCompiler;
		glGetBooleanv(GL_SHADER_COMPILER, &hasCompiler);
		if(hasCompiler == GL_FALSE)
			throw br::ShaderException(EXCEPTION_INFO, "no compiler");

		GLuint shader = glCreateShader(type);
		if(!shader)
			throw br::ShaderException(EXCEPTION_INFO, "can`t create shader");

		glShaderSource(shader, 1, &source, NULL);
		glCompileShader(shader);

		GLint compiled;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
		if(!compiled) {
			glDeleteShader(shader);
			throw br::ShaderException(EXCEPTION_INFO, "can`t compile shader");
		}

		return shader;
	}

	void ProcessorBase::loadTextureToGpu(Texture2d& texture) {
		if(hasTextureInGpu(texture.getPath()))
			return;

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		GLuint textureId;
		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.getWidth(), texture.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, &texture.getData()[0]);

		if(glGetError() != GL_NO_ERROR)
			throw GpuException(EXCEPTION_INFO, texture.getPath() + " can`t load texture");

		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindTexture(GL_TEXTURE_2D, 0);

		textureToId.emplace(texture.getPath(), textureId);
	}

	void ProcessorBase::deleteTextureFromGpu(std::string pathAsKey) {
		uint32_t textureId = textureToId.at(pathAsKey);
		glDeleteTextures(1, &textureId);

		textureToId.erase(pathAsKey);
	}

	bool ProcessorBase::hasTextureInGpu(std::string pathAsKey) {
		auto& it = find_if(cbegin(textureToId), cend(textureToId), [&pathAsKey](pair<string, uint32_t> i)->bool {
			return i.first == pathAsKey;
		});
		return it != cend(textureToId);
	}

	void ProcessorBase::start(std::weak_ptr<WindowVendor> window) {
		this->window = window;
		
		enabled = true;
		program = createProgram(shaders);
	}

	void ProcessorBase::stop() {
		enabled = false;
		glDeleteProgram(program.id);
	}

	void ProcessorBase::tryDoStep(StepData& stepData) {
		if(canDoStep())
			doStep(stepData);
	}

	bool ProcessorBase::canDoStep() {
		return enabled;
	}
}

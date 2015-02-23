#include "../utils/SharedHeaders.h"
#include "GlesConnector.h"

#include <EGL/egl.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "../exceptions/Exception.h"
#include "WindowVendorWin.h"

using std::weak_ptr;
using std::make_shared;
using glm::mat4;

namespace br {
	GlesConnector::GlesConnector(const IWindowVendor::Rect& size){
		window = make_shared<WindowVendorWin>(size);

		initEgl();

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glDepthMask(true);

		// TODO: turn off when FXs only
		// 		glEnable(GL_CULL_FACE);
		// 		glCullFace(GL_BACK);
		// 		glFrontFace(GL_CCW);
	}

	GlesConnector::~GlesConnector() {

	}


	void GlesConnector::setViewport(const IWindowVendor::Rect& size) {
		glViewport((GLint)size.x, (GLint)size.y, (GLint)size.w, (GLint)size.h);
	}

	void GlesConnector::clear() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void GlesConnector::swapBuffers() {
		eglSwapBuffers(eglContext.display, eglContext.surface);
	}


	void GlesConnector::initEgl() {
		EGLint minorVersion;
		EGLint majorVersion;

		auto display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
		if(display == EGL_NO_DISPLAY)
			throw EglException(EXCEPTION_INFO, "can`t get display");

		if(!eglInitialize(display, &majorVersion, &minorVersion))
			throw EglException(EXCEPTION_INFO, "cant init display");

		const EGLint cfgAttribs[] = {
			EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
			EGL_RED_SIZE, 5,
			EGL_GREEN_SIZE, 6,
			EGL_BLUE_SIZE, 5,
			EGL_ALPHA_SIZE, 8,
			EGL_DEPTH_SIZE, 8,
			EGL_STENCIL_SIZE, 8,
			// EGL_SAMPLE_BUFFERS, 1,
			EGL_NONE
		};


		const EGLint maxConfigs = 2;
		EGLConfig configs[2];
		EGLint numConfigs;
		if(!eglChooseConfig(display, cfgAttribs, configs, maxConfigs, &numConfigs))
			throw EglException(EXCEPTION_INFO, "can`t choose config");

		const EGLint srfAttribs[] = {
			EGL_RENDER_BUFFER, EGL_BACK_BUFFER,
			EGL_NONE
		};

		auto surface = eglCreateWindowSurface(display, configs[0], (EGLNativeWindowType)window->getNativeWindow(), srfAttribs);
		if(surface == EGL_NO_SURFACE)
			throw EglException(EXCEPTION_INFO, "can`t create window surface");


		const EGLint ctxAttribs[] = {
			EGL_CONTEXT_CLIENT_VERSION, 2,
			EGL_NONE
		};

		auto context = eglCreateContext(display, configs[0], EGL_NO_CONTEXT, ctxAttribs);
		if(context == EGL_NO_CONTEXT)
			throw EglException(EXCEPTION_INFO, "can`t create context");


		if(!eglMakeCurrent(display, surface, surface, context))
			throw EglException(EXCEPTION_INFO, "can`t make context current");

		eglContext.display = display;
		eglContext.surface = surface;
		eglContext.context = context;
	}

	WindowVendorWin::Rect GlesConnector::getWindowSize() const {
		return window->getSize();
	}

	float GlesConnector::getAspectRatio() const {
		return window->getAspectRatio();
	}

	bool GlesConnector::doStep() {
		return window->doStep();
	}

	glm::vec2 GlesConnector::getMousePosition() const {
		return window->getMousePosition();
	}

	uint32_t GlesConnector::createShader(uint32_t type, const char* source) {
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

	IGraphicsConnector::ProgramContext GlesConnector::createProgram(std::pair<std::string, std::string> shaders) {
		GLuint vShader = createShader(GL_VERTEX_SHADER, shaders.first.c_str());
		GLuint fShader = createShader(GL_FRAGMENT_SHADER, shaders.second.c_str());

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

		ProgramContext program;
		program.id = pObject;

		program.position = glGetAttribLocation(pObject, "aPosition");
		program.uv = glGetAttribLocation(pObject, "aTexCoord");

		program.bones = glGetUniformLocation(pObject, "bones");
		program.boneIds = glGetAttribLocation(pObject, "boneIds");
		program.weights = glGetAttribLocation(pObject, "weights");

		program.sampler = glGetUniformLocation(pObject, "sTexture");
		program.mvp = glGetUniformLocation(pObject, "mvpMatrix");

		program.color = glGetUniformLocation(pObject, "color");

		return program;
	}

	uint32_t GlesConnector::loadTextureToGpu(Texture2d& texture) {
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		GLuint textureId;
		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.getWidth(), texture.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, &texture.getData()[0]);

		auto error = glGetError();
		if(error != GL_NO_ERROR)
			throw GpuException(EXCEPTION_INFO, texture.getPath() + " can`t load texture");

		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindTexture(GL_TEXTURE_2D, 0);
		
		return textureId;
	}

	void GlesConnector::deleteTextureFromGpu(uint32_t texture) {
		glDeleteTextures(1, &texture);
	}

	void GlesConnector::deleteProgram(ProgramContext& program) {
		glDeleteProgram(program.id);
	}

	IGraphicsConnector::GpuBufferData GlesConnector::loadGeometryToGpu(std::vector<float>& vertices, std::vector<uint16_t>& indices) {
		uint32_t vBuffer;
		glGenBuffers(1, &vBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vBuffer);

		GLint szInBytes = sizeof(float) * vertices.size();
		glBufferData(GL_ARRAY_BUFFER, szInBytes, &vertices[0], GL_STATIC_DRAW);

		GLint loadedBytes = 0;
		glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &loadedBytes);
		if(szInBytes != loadedBytes) {
			glDeleteBuffers(1, &vBuffer);
			throw GpuException(EXCEPTION_INFO, "can`t load vertices");
		}


		uint32_t iBuffer;
		glGenBuffers(1, &iBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iBuffer);

		uint32_t iBufferLength = indices.size();
		szInBytes = sizeof(uint16_t) * iBufferLength;
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, szInBytes, &indices[0], GL_STATIC_DRAW);

		glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &loadedBytes);
		if(szInBytes != loadedBytes) {
			glDeleteBuffers(1, &iBuffer);
			throw GpuException(EXCEPTION_INFO, "can`t load indices");
		}

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		
		GpuBufferData buffer;
		buffer.vBuffer = vBuffer;
		buffer.iBuffer = iBuffer;
		buffer.iBufferLenght = iBufferLength;		
		return buffer;
	}

	void GlesConnector::deleteGeometryFromGpu(GpuBufferData& buffer) {
		glDeleteBuffers(1, &buffer.vBuffer);
		glDeleteBuffers(1, &buffer.iBuffer);
	}

	void GlesConnector::setBlending(bool val) {
		if(val) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		} else {
			glDisable(GL_BLEND);
		}
	}

	void GlesConnector::draw(TextField& object, GpuBufferData& buffer, ProgramContext program, mat4 mvp) {
		glUseProgram(program.id);
		glUniform4fv(program.color, 1, &object.getColor()[0]);
		draw(buffer, program, mvp);
	}
	
	void GlesConnector::draw(GpuBufferData& buffer, ProgramContext& program, mat4& mvp) {
		BoneTransformer::BonesDataMap bonesData;
		draw(buffer, program, mvp, bonesData);
	}

	void GlesConnector::draw(GpuBufferData& buffer, ProgramContext& program, glm::mat4& mvp, BoneTransformer::BonesDataMap& bonesData) {
		glUseProgram(program.id);

		glUniformMatrix4fv(program.mvp, 1, GL_FALSE, &mvp[0][0]);

		if(program.bones != -1) {
			for(auto& i : bonesData) {
				glUniformMatrix4fv(program.bones + i.first, 1, GL_FALSE, &(i.second.finalTransform[0][0]));
			}
		}

		glBindBuffer(GL_ARRAY_BUFFER, buffer.vBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.iBuffer);

		uint8_t offset = 0;
		if(program.position != -1) {
			glEnableVertexAttribArray(program.position);
			glVertexAttribPointer(program.position,
				Mesh3d::GetRawVertexPosition(),
				GL_FLOAT,
				GL_FALSE,
				Mesh3d::GetRawVertexStride(),
				(void*)offset);
		}

		offset += Mesh3d::GetRawVertexPosition() * sizeof(float);

		if(program.uv != -1) {
			glEnableVertexAttribArray(program.uv);
			glVertexAttribPointer(program.uv,
				Mesh3d::GetRawVertexTexture(),
				GL_FLOAT,
				GL_FALSE,
				Mesh3d::GetRawVertexStride(), (void*)offset);
		}

		offset += Mesh3d::GetRawVertexTexture() * sizeof(float);
		if(program.boneIds != -1) {
			glEnableVertexAttribArray(program.boneIds);
			glVertexAttribPointer(program.boneIds,
				Mesh3d::GetRawVertexBoneIds(),
				GL_FLOAT,
				GL_FALSE,
				Mesh3d::GetRawVertexStride(),
				(void*)offset);
		}

		offset += Mesh3d::GetRawVertexBoneIds() * sizeof(float);
		if(program.weights != -1) {
			glEnableVertexAttribArray(program.weights);
			glVertexAttribPointer(program.weights,
				Mesh3d::GetRawVertexWeights(),
				GL_FLOAT,
				GL_FALSE,
				Mesh3d::GetRawVertexStride(),
				(void*)offset);
		}

		if(program.sampler != -1) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, buffer.texture);

			glUniform1i(program.sampler, 0);
		}

		glDrawElements(GL_TRIANGLES, buffer.iBufferLenght, GL_UNSIGNED_SHORT, (void*)0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glDisableVertexAttribArray(program.position);
		glDisableVertexAttribArray(program.uv);
		glDisableVertexAttribArray(program.boneIds);
		glDisableVertexAttribArray(program.weights);

		glUseProgram(0);
	}
}

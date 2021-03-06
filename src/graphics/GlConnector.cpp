#include "../utils/SharedHeaders.h"
#include "GlConnector.h"

#include <GL/glew.h>
#include <windows.h>

#include "../exceptions/Exception.h"
#include "WindowVendorWin.h"
#include "ProgramContext.h"

using std::weak_ptr;
using std::vector;
using std::make_shared;
using glm::mat4;

namespace br {
	GlConnector::GlConnector(const IWindowVendor::Rect& size) {
		window = make_shared<WindowVendorWin>(size);

		initWgl();

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glDepthMask(true);

		// TODO: turn off when FXs only
		// 		glEnable(GL_CULL_FACE);
		// 		glCullFace(GL_BACK);
		// 		glFrontFace(GL_CCW);
	}

	GlConnector::~GlConnector() {

	}


	void GlConnector::initWgl() {
		PIXELFORMATDESCRIPTOR pfd = {
			sizeof(PIXELFORMATDESCRIPTOR),    // size of this pfd 
			1,                                // version number 
			PFD_DRAW_TO_WINDOW |              // support window 
			PFD_SUPPORT_OPENGL |              // support OpenGL 
			PFD_DOUBLEBUFFER,                 // double buffered 
			PFD_TYPE_RGBA,                    // RGBA type 
			24,                               // 24-bit color depth 
			0, 0, 0, 0, 0, 0,                 // color bits ignored 
			0,                                // no alpha buffer 
			0,                                // shift bit ignored 
			0,                                // no accumulation buffer 
			0, 0, 0, 0,                       // accum bits ignored 
			32,                               // 32-bit z-buffer     
			0,                                // no stencil buffer 
			0,                                // no auxiliary buffer 
			PFD_MAIN_PLANE,                   // main layer 
			0,                                // reserved 
			0, 0, 0                           // layer masks ignored 
		};

		int32_t  iPixelFormat;
		HDC deviceContext = GetDC((HWND)window->getNativeWindow());
		if((iPixelFormat = ChoosePixelFormat(deviceContext, &pfd)) == 0)
			throw Exception(EXCEPTION_INFO, "can`t choose pixel format");


		if(SetPixelFormat(deviceContext, iPixelFormat, &pfd) == FALSE)
			throw Exception(EXCEPTION_INFO, "can`t set pixel format");

		HGLRC renderContext;
		if((renderContext = wglCreateContext(deviceContext)) == NULL)
			throw Exception(EXCEPTION_INFO, "can`t create context");

		if((wglMakeCurrent(deviceContext, renderContext)) == NULL)
			throw Exception(EXCEPTION_INFO, "can`t make current context");

		GLenum glew_status = glewInit();
		if(glew_status != GLEW_OK)
			throw Exception(EXCEPTION_INFO, "can`t init glew");
	}


	void GlConnector::setViewport(const IWindowVendor::Rect& size) {
		glViewport((GLint)size.x, (GLint)size.y, (GLint)size.w, (GLint)size.h);
	}

	void GlConnector::clear() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void GlConnector::swapBuffers() {
		window->swapBuffers();
	}

	WindowVendorWin::Rect GlConnector::getWindowSize() const {
		return window->getSize();
	}

	float GlConnector::getAspectRatio() const {
		return window->getAspectRatio();
	}

	bool GlConnector::doStep() {
		return window->doStep();
	}

	glm::vec2 GlConnector::getMousePosition() const {
		return window->getMousePosition();
	}

	uint32_t GlConnector::createShader(uint32_t type, const char* source) {
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

	std::shared_ptr<IProgramContext> GlConnector::createProgram(std::pair<std::string, std::string> shaders,
		std::unordered_map<std::string, std::string> attributeBindings,
		std::unordered_map<std::string, std::string> uniformBindings) {
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

		auto program = std::make_shared<ProgramContext>(pObject);
		for(auto& i : attributeBindings) {
			program->setLoc(i.first, glGetAttribLocation(pObject, i.second.c_str()));
		}
		for(auto& i : uniformBindings) {
			program->setLoc(i.first, glGetUniformLocation(pObject, i.second.c_str()));
		}
		return program;
	}

	uint32_t GlConnector::loadTextureToGpu(vector<uint8_t> const& texture, uint32_t width, uint32_t height) {
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		GLuint textureId;
		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &texture[0]);

		if(glGetError() != GL_NO_ERROR)
			throw GpuException(EXCEPTION_INFO, "can`t load texture"); // what texture?

		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindTexture(GL_TEXTURE_2D, 0);

		return textureId;
	}

	void GlConnector::deleteTextureFromGpu(uint32_t texture) {
		glDeleteTextures(1, &texture);
	}

	void GlConnector::deleteProgram(std::shared_ptr<IProgramContext> program) {
		glDeleteProgram(program->getId());
	}

	IGraphicsConnector::GpuBufferData GlConnector::loadGeometryToGpu(std::vector<float>& vertices, std::vector<uint16_t>& indices) {
		uint32_t vBuffer;
		glGenBuffers(1, &vBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vBuffer);

		GLint szInBytes = sizeof(float) * vertices.size();
		glBufferData(GL_ARRAY_BUFFER, szInBytes, &vertices[0], GL_STATIC_DRAW);

		GLint loadedBytes = 0;
		glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &loadedBytes);
		if(szInBytes != loadedBytes) {
			glDeleteBuffers(1, &vBuffer);
			throw GpuException(EXCEPTION_INFO, "can`t load vertices"); // what model?
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
			throw GpuException(EXCEPTION_INFO, "can`t load indices"); // what model?
		}

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		GpuBufferData buffer;
		buffer.vBuffer = vBuffer;
		buffer.iBuffer = iBuffer;
		buffer.iBufferLenght = iBufferLength;
		return buffer;
	}

	void GlConnector::deleteGeometryFromGpu(GpuBufferData& buffer) {
		glDeleteBuffers(1, &buffer.vBuffer);
		glDeleteBuffers(1, &buffer.iBuffer);
	}

	void GlConnector::setBlending(bool val) {
		if(val) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		} else {
			glDisable(GL_BLEND);
		}
	}

	void GlConnector::draw(GpuBufferData& buffer, std::shared_ptr<IProgramContext> programContext, vector<ProgramParam> params) {
		BoneTransformer::BonesDataMap bonesData;
		draw(buffer, programContext, params, bonesData);
	}

	void GlConnector::draw(GpuBufferData& buffer, std::shared_ptr<IProgramContext> programContext, std::vector<ProgramParam> params, BoneTransformer::BonesDataMap& bonesData) {
		glUseProgram(programContext->getId());

		loadProgramParams(params);
		
		int32_t bonesLoc = programContext->getLoc(programContext->getBonesBinding());
		if(bonesLoc != -1) {
			for(auto& i : bonesData) {
				glUniformMatrix4fv(bonesLoc + i.first, 1, GL_FALSE, &(i.second.finalTransform[0][0]));
			}
		}
		
		glBindBuffer(GL_ARRAY_BUFFER, buffer.vBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.iBuffer);

		uint8_t offset = 0;
		int32_t positionLoc = programContext->getLoc(programContext->getPositionBinding());
		if(positionLoc != -1) {
			glEnableVertexAttribArray(positionLoc);
			glVertexAttribPointer(positionLoc,
				Mesh3d::GetRawVertexPosition(), 
				GL_FLOAT, 
				GL_FALSE, 
				Mesh3d::GetRawVertexStride(), 
				(void*)offset);
		}

		offset += Mesh3d::GetRawVertexPosition() * Mesh3d::GetRawVertexPositionSize();

		int32_t uvLoc = programContext->getLoc(programContext->getUvBinding());
		if(uvLoc != -1) {
			glEnableVertexAttribArray(uvLoc);
			glVertexAttribPointer(uvLoc,
				Mesh3d::GetRawVertexTexture(),
				GL_FLOAT, 
				GL_FALSE, 
				Mesh3d::GetRawVertexStride(), (void*)offset);
		}

		offset += Mesh3d::GetRawVertexTexture() * Mesh3d::GetRawVertexTextureSize();

		int32_t boneIdsLoc = programContext->getLoc(programContext->getBoneIdsBinding());
		if(boneIdsLoc != -1) {
			glEnableVertexAttribArray(boneIdsLoc);
			glVertexAttribPointer(boneIdsLoc,
				Mesh3d::GetRawVertexBoneIds(), 
				GL_FLOAT, 
				GL_FALSE, 
				Mesh3d::GetRawVertexStride(),
				(void*)offset);
		}

		offset += Mesh3d::GetRawVertexBoneIds() * Mesh3d::GetRawVertexBoneIdsSize();
		
		int32_t boneWeightsLoc = programContext->getLoc(programContext->getBoneWeightsBinding());
		if(boneWeightsLoc != -1) {
			glEnableVertexAttribArray(boneWeightsLoc);
			glVertexAttribPointer(boneWeightsLoc,
				Mesh3d::GetRawVertexWeights(),
				GL_FLOAT, 
				GL_FALSE, 
				Mesh3d::GetRawVertexStride(),
				(void*)offset);
		}

		int32_t samplerLoc = programContext->getLoc(programContext->getSamplerBinding());
		if(samplerLoc != -1) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, buffer.texture);

			glUniform1i(samplerLoc, 0);
		}

		glDrawElements(GL_TRIANGLES, buffer.iBufferLenght, GL_UNSIGNED_SHORT, (void*)0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glDisableVertexAttribArray(positionLoc);
		glDisableVertexAttribArray(uvLoc);
		glDisableVertexAttribArray(boneIdsLoc);
		glDisableVertexAttribArray(boneWeightsLoc);

		glUseProgram(0);
	}

	void GlConnector::loadProgramParams(std::vector<ProgramParam> params) {
		for(auto& i : params) {
			if(i.vec4) {
				glm::vec4& v = *(i.vec4.get());
				glUniform4fv(i.id, 1, &v[0]);
			}
			if(i.mat4) {
				glm::mat4& m = *(i.mat4.get());
				glUniformMatrix4fv(i.id, 1, GL_FALSE, &m[0][0]);
			}
		}
	}
}

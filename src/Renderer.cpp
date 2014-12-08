#include "SharedHeaders.h"
#include "Renderer.h"

#include "Utils.h"

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "Model3d.h"
#include "Model3dLoader.h"
#include "Texture2d.h"
#include "bone/BoneNodeData.h"

#include <gtc\matrix_transform.hpp>
#include <utility>
#include <gtc\type_ptr.hpp>

using std::string;
using std::vector;
using std::pair;
using std::array;

using std::shared_ptr;
using std::make_shared;


namespace br {
	// TODO: throw exceptions
	Renderer::Renderer(std::shared_ptr<Model3dLoader> loader,
		uint32_t wndX,
		uint32_t wndY,
		uint32_t wndW, 
		uint32_t wndH)
		: loader(loader) {
		window = make_shared <WindowVendor>(wndX, wndY, wndW, wndH);
		if(!window->nativeWindow)
			return; // EglError::NATIVE_WINDOW_FAIL;
	
		int32_t eglFail = initEgl();
		if(eglFail)
			return; // eglFail;
	
		int32_t shadersFail = initShaders(Utils::defaultVertexShader, Utils::defaultFragmentShader);
		if(shadersFail)
			return; // shadersFail;
	
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glDepthMask(true);
	
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);
	
		return; // 0;
	}
	
	Renderer::~Renderer()
	{
		glDeleteProgram(defaultProgram);
	
		auto allObjects = idToObject;
		for (auto& i : allObjects) {
			removeObject(i.first);
		}
	}
	
	bool Renderer::addObject(uint32_t id, std::string modelPath){
		if (!hasObjectWithModel(modelPath)) // first in
			loadModelToGpu(modelPath);
		
		View object{ id, modelPath };
		idToObject.emplace(id, object);
		return true;
	}
	
	bool Renderer::removeObject(uint32_t id){
		View& view = idToObject.at(id);
		string modelPath = view.getPath();
	
		auto& it = idToObject.find(id);
		if (it != end(idToObject))
			idToObject.erase(it);
	
		if (!hasObjectWithModel(modelPath)) // last out
			deleteModelFromGpu(modelPath);
		
		return true;
	}
	
	bool Renderer::playAnimation(uint32_t objId, std::string animName) {
		auto& it = idToObject.find(objId);
		if (it == end(idToObject))
			return false;
		
		View& object = it->second;
		Model3d& model = loader->getModelBy(object.getPath());
		Animation3d& animation = model.getAnimationBy(animName);
		object.setAnimation(animName, (uint32_t)(animation.getDuration() * 1000), true);
		return true;
	}
	
	void Renderer::setCamera(float x, float y, float z) {
		camera.x = x;
		camera.y = y;
		camera.z = z;
	}
	
	bool Renderer::doStep(uint32_t stepMSec)
	{
		vector<float> rect = window->getRect();
		float wWidth = rect[2];
		float wHeight = rect[3];
		glViewport(0, 0, (uint32_t)wWidth, (uint32_t)wHeight);
	
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
		glUseProgram(defaultProgram);
	
		glm::mat4 view = glm::lookAt(glm::vec3{camera.x, camera.y, camera.z},
			glm::vec3{0, 0, 0},
			glm::vec3{0, 1, 0});
		glm::mat4 projection = glm::perspective(45.0f, wWidth / wHeight, 0.1f, 100.0f);
		glm::mat4 pvMatrix = projection * view;
	
		GLuint posLoc = glGetAttribLocation(defaultProgram, "aPosition");
		GLuint texPosLoc = glGetAttribLocation(defaultProgram, "aTexCoord");
		
		GLuint bonesLoc = glGetUniformLocation(defaultProgram, "bones");
		GLuint boneIdLoc = glGetAttribLocation(defaultProgram, "boneIds");
		GLuint weightsLoc = glGetAttribLocation(defaultProgram, "weights");
		
		GLuint samplerLoc = glGetUniformLocation(defaultProgram, "sTexture");
		GLuint mvpMatrixLoc = glGetUniformLocation(defaultProgram, "mvpMatrix");
		
		for (auto& i : idToObject){
			View& view = i.second;
			const glm::mat4& modelMtx = view.getTransform();
			glm::mat4 mvpMtx = pvMatrix * modelMtx;
			glUniformMatrix4fv(mvpMatrixLoc, 1, GL_FALSE, &mvpMtx[0][0]);
	
			Model3d& model = loader->getModelBy(view.getPath());
			vector<Mesh3d>& meshes = model.getMeshes();
			for (auto& s : meshes) {
				auto bonesData = prepareAnimationStep(view, s, stepMSec);
				for (auto& i : bonesData) {
					glUniformMatrix4fv(bonesLoc + i.first, 1, GL_FALSE, &(i.second.finalTransform[0][0]));
				}
	
				string meshName = model.getUniqueMeshName(s);
				GpuBufferData& buffers = meshToBuffer[meshName];
				glBindBuffer(GL_ARRAY_BUFFER, buffers.vBuffer);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers.iBuffer);
	
				uint8_t offset = 0;
				glEnableVertexAttribArray(posLoc);
				glVertexAttribPointer(posLoc, Mesh3d::VERTEX3D_POSITION, GL_FLOAT, GL_FALSE, Mesh3d::VERTEX3D_STRIDE, (void*)offset);
	
				offset += Mesh3d::VERTEX3D_POSITION * sizeof(float);
				glEnableVertexAttribArray(texPosLoc);
				glVertexAttribPointer(texPosLoc, Mesh3d::VERTEX3D_TEXTURE, GL_FLOAT, GL_FALSE, Mesh3d::VERTEX3D_STRIDE, (void*)offset);
	
				offset += Mesh3d::VERTEX3D_TEXTURE * sizeof(float);
				glEnableVertexAttribArray(boneIdLoc);
				glVertexAttribPointer(boneIdLoc, Mesh3d::VERTEX3D_BONEIDS, GL_UNSIGNED_SHORT, GL_FALSE, Mesh3d::VERTEX3D_STRIDE, (void*)offset);
	
				offset += Mesh3d::VERTEX3D_BONEIDS * sizeof(uint16_t);
				glEnableVertexAttribArray(weightsLoc);
				glVertexAttribPointer(weightsLoc, Mesh3d::VERTEX3D_WEIGHTS, GL_FLOAT, GL_FALSE, Mesh3d::VERTEX3D_STRIDE, (void*)offset);
				
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, buffers.texture);
				glUniform1i(samplerLoc, 0);
	
				glDrawElements(GL_TRIANGLES, buffers.iBufferLenght, GL_UNSIGNED_SHORT, (void*)0);
			}
		}
	
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		
		glDisableVertexAttribArray(posLoc);
		glDisableVertexAttribArray(texPosLoc);
		glDisableVertexAttribArray(boneIdLoc);
		glDisableVertexAttribArray(samplerLoc);
	
		glUseProgram(0);
	
		eglSwapBuffers(eglContext.display, eglContext.surface);
	
		return window->doStep();
	}
	
	// private
	
	int32_t Renderer::initEgl(){
		EGLint minorVersion;
		EGLint majorVersion;
	
		auto display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
		if (display == EGL_NO_DISPLAY)
			return EglError::GET_DISPLAY_FAIL;
	
	
		if (!eglInitialize(display, &majorVersion, &minorVersion))
			return EglError::INIT_DISPLAY_FAIL;
	
	
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
		if (!eglChooseConfig(display, cfgAttribs, configs, maxConfigs, &numConfigs))
			return EglError::CHOSE_CONFIG_FAIL;
	
	
		const EGLint srfAttribs[] = {
			EGL_RENDER_BUFFER, EGL_BACK_BUFFER,
			EGL_NONE
		};
	
		auto surface = eglCreateWindowSurface(display, configs[0], window->nativeWindow, srfAttribs);
		if (surface == EGL_NO_SURFACE)
			return EglError::CREATE_SURFACE_FAIL;
	
	
		const EGLint ctxAttribs[] = {
			EGL_CONTEXT_CLIENT_VERSION, 2,
			EGL_NONE
		};
	
		auto context = eglCreateContext(display, configs[0], EGL_NO_CONTEXT, ctxAttribs);
		if (context == EGL_NO_CONTEXT)
			return EglError::CREATE_CONTEXT_FAIL;
	
	
		if (!eglMakeCurrent(display, surface, surface, context))
			return EglError::MAKE_CONTEXT_CURRENT_FAIL;
	
		eglContext.display = display;
		eglContext.surface = surface;
		eglContext.context = context;
		return 0;
	}
	
	
	int32_t Renderer::initShaders(string vShaderSrc, string fShaderSrc)
	{
		GLuint vShader = createShader(GL_VERTEX_SHADER, vShaderSrc.c_str());
		GLuint fShader = createShader(GL_FRAGMENT_SHADER, fShaderSrc.c_str());
		if (!vShader || !fShader)
			return GlesError::SHADER_LOAD_FAIL;
	
		GLuint pObject = glCreateProgram();
		if (!pObject)
			return GlesError::PROGRAM_CREATE_FAIL;
	
		glAttachShader(pObject, vShader);
		glAttachShader(pObject, fShader);
	
		glLinkProgram(pObject);
	
		GLint linked;
		glGetProgramiv(pObject, GL_LINK_STATUS, &linked);
		if (!linked){
			glDeleteProgram(pObject);
			return GlesError::PROGRAM_LINK_FAIL;
		}
	
		glDetachShader(pObject, vShader);
		glDeleteShader(vShader);
		glDetachShader(pObject, fShader);
		glDeleteShader(fShader);
	
		defaultProgram = pObject;
		return 0;
	}
	
	GLuint Renderer::createShader(GLenum shType, const char* shSource){
		GLboolean hasCompiler;
		glGetBooleanv(GL_SHADER_COMPILER, &hasCompiler);
		if (hasCompiler == GL_FALSE)
			return 0;
	
		GLuint shader = glCreateShader(shType);
		if (!shader)
			return 0;
	
		glShaderSource(shader, 1, &shSource, NULL);
		glCompileShader(shader);
	
		GLint compiled;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
		if (!compiled){
			glDeleteShader(shader);
			return 0;
		}
	
		return shader;
	}
	
	BoneTransformer::BonesDataMap Renderer::prepareAnimationStep(View& object, Mesh3d& m, uint32_t stepMSec) {
		BoneTransformer::BonesDataMap res;
		auto& boneIdToOffset = m.getBoneIdToOffset();
		for (auto& i : boneIdToOffset) {
			BoneTransformer::BoneData bData{ i.second };
			res.emplace(i.first, bData);
		}
		
		object.doAnimationStep(stepMSec);
	
		Model3d& model = loader->getModelBy(object.getPath());
		boneTransformer.transform(object, model, res);
		return res;
	}
	
	bool Renderer::transform(uint32_t objId, const array<float, 16> tForm) {
		auto& it = idToObject.find(objId);
		if (it == end(idToObject))
			return false;
	
		auto t = glm::make_mat4(tForm.data());
		it->second.setTransform(t);
		return true;
	}
	
	
	bool Renderer::hasObjectWithModel(string path) {
		auto& it = find_if(cbegin(idToObject), cend(idToObject), [&path](pair<uint32_t, View> i)->bool{
			return i.second.getPath() == path;
		});
		return it != cend(idToObject);
	}
	
	bool Renderer::loadModelToGpu(string modelPath) {
		Model3d& model = loader->getModelBy(modelPath);
		vector<Mesh3d>& meshes = model.getMeshes();
		for (auto& s : meshes) {
			uint32_t vBuffer;
			glGenBuffers(1, &vBuffer);
	
			auto vertices = s.getVertices();
			glBindBuffer(GL_ARRAY_BUFFER, vBuffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex3d) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
	
			uint32_t iBuffer;
			glGenBuffers(1, &iBuffer);
	
			auto indices = s.getIndices();
			uint32_t iBufferLength = indices.size();
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iBuffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * iBufferLength, &indices[0], GL_STATIC_DRAW);
	
			auto& materials = model.getMaterials();
			Material3d& m = materials.at(s.getMaterialId());
			Texture2d& t = m.getTexture();
			uint32_t texture = loadTextureToGpu(t.getData(), t.getWidth(), t.getHeight());
	
			string meshName = model.getUniqueMeshName(s);
			GpuBufferData buffer{ vBuffer, iBuffer, iBufferLength, texture };
			meshToBuffer.emplace(meshName, buffer);
		}
	
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
		std::cout << static_cast<string>(model);
		return true;
	}
	
	bool Renderer::deleteModelFromGpu(std::string modelPath) {
		Model3d& model = loader->getModelBy(modelPath);
		vector<Mesh3d>& meshes = model.getMeshes();
		for (auto& s : meshes) {
			string mName = model.getUniqueMeshName(s);
			GpuBufferData& buffers = meshToBuffer.at(mName);
	
			glDeleteBuffers(1, &buffers.vBuffer);
			glDeleteBuffers(1, &buffers.iBuffer);
	
			glDeleteTextures(1, &buffers.texture);
	
			auto& bIt = meshToBuffer.find(mName);
			if (bIt != end(meshToBuffer))
				meshToBuffer.erase(bIt);
		}
		return true;
	}
	
	GLuint Renderer::loadTextureToGpu(vector<uint8_t>& texture, int16_t widht, int16_t height) {
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
		GLuint textureId;
		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widht, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &texture[0]);
	
		glGenerateMipmap(GL_TEXTURE_2D);
	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
	
		return textureId;
	}
}
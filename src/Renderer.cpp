#include "SharedHeaders.h"
#include "Renderer.h"

#include "Utils.h"

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "Model3d.h"
#include "Model3dLoader.h"
#include "Texture2d.h"
#include "bones/BoneNodeData.h"

#include <gtc/matrix_transform.hpp>
#include <utility>
#include <gtc/type_ptr.hpp>
#include "exceptions/Exception.h"

using std::string;
using std::vector;
using std::pair;
using std::array;

using std::shared_ptr;
using std::make_shared;

using glm::mat4;
using glm::vec3;
using glm::make_mat4;
using glm::lookAt;
using glm::perspective;


namespace br {
	Renderer::Renderer(std::shared_ptr<Model3dLoader> loader,
		uint32_t wndX,
		uint32_t wndY,
		uint32_t wndW, 
		uint32_t wndH)
		: loader(loader) {
		
		window = make_shared <WindowVendor>(wndX, wndY, wndW, wndH);
		initEgl();
		initShaders(Utils::defaultVertexShader, Utils::defaultFragmentShader);
		
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glDepthMask(true);
	
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);
	}
	
	Renderer::~Renderer()
	{
		glDeleteProgram(defaultProgram.id);
	
		auto allObjects = idToObject;
		for (auto& i : allObjects) {
			removeObject(i.first);
		}
	}
	
	void Renderer::addObject(uint32_t id, std::string modelPath){
		if (!hasObjectWithModel(modelPath)) // first in
			loadModelToGpu(modelPath);
		
		View object{ id, modelPath };
		auto res = idToObject.emplace(id, object);
		if(!res.second)
			throw InvalidObjectIdException(EXCEPTION_INFO, id);

		playAnimation(id, Animation3d::DEFAULT_ANIMATION_NAME); // TODO: find better decision
	}
	
	void Renderer::removeObject(uint32_t id){
		string modelPath = "";
		try {
			View& object = idToObject.at(id);
			modelPath = object.getPath();
		} catch(std::out_of_range&) {
			throw InvalidObjectIdException(EXCEPTION_INFO, id);
		}

		idToObject.erase(id);
	
		if (!hasObjectWithModel(modelPath)) // last out
			deleteModelFromGpu(modelPath);
	}
	
	void Renderer::playAnimation(uint32_t objId, std::string animName) {
		View* object;
		try {
			object = &idToObject.at(objId);
		} catch(std::out_of_range&) {
			throw InvalidObjectIdException(EXCEPTION_INFO, objId);
		}
		
		Model3d& model = loader->getModelBy(object->getPath());
		Animation3d& animation = model.getAnimationBy(animName);
		object->setAnimation(animName, (uint32_t)(animation.getDuration() * 1000), true);
	}
	
	void Renderer::setCamera(float x, float y, float z) {
		camera.x = x;
		camera.y = y;
		camera.z = z;
	}
	
	bool Renderer::doStep(uint32_t stepMSec)
	{
		auto winSize = window->getRect();
		glViewport(0, 0, (uint32_t)winSize.w, (uint32_t)winSize.h);
	
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
		glUseProgram(defaultProgram.id);
	
		mat4 view = lookAt(vec3{camera.x, camera.y, camera.z}, vec3{0, 0, 0}, vec3{0, 1, 0});
		mat4 projection = perspective(45.0f, winSize.w / winSize.h, 0.1f, 100.0f);
		mat4 pvMatrix = projection * view;
	
		for (auto& i : idToObject){
			View& object = i.second;
			mat4 mvpMatrix = pvMatrix * object.getTransform();
			glUniformMatrix4fv(defaultProgram.mvpMatrix, 1, GL_FALSE, &mvpMatrix[0][0]);
	
			Model3d& model = loader->getModelBy(object.getPath());
			vector<Mesh3d>& meshes = model.getMeshes();
			for (auto& s : meshes) {
				auto bonesData = prepareAnimationStep(object, s, stepMSec);
				for (auto& i : bonesData) {
					glUniformMatrix4fv(defaultProgram.bones + i.first, 1, GL_FALSE, &(i.second.finalTransform[0][0]));
				}
	
				string meshName = model.getUniqueMeshName(s);
				GpuBufferData& buffers = meshToBuffer[meshName];
				glBindBuffer(GL_ARRAY_BUFFER, buffers.vBuffer);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers.iBuffer);
	
				uint8_t offset = 0;
				glEnableVertexAttribArray(defaultProgram.position);
				glVertexAttribPointer(defaultProgram.position, Mesh3d::VERTEX3D_POSITION, GL_FLOAT, GL_FALSE, Mesh3d::VERTEX3D_STRIDE, (void*)offset);
	
				offset += Mesh3d::VERTEX3D_POSITION * sizeof(float);
				glEnableVertexAttribArray(defaultProgram.texPosition);
				glVertexAttribPointer(defaultProgram.texPosition, Mesh3d::VERTEX3D_TEXTURE, GL_FLOAT, GL_FALSE, Mesh3d::VERTEX3D_STRIDE, (void*)offset);
	
				offset += Mesh3d::VERTEX3D_TEXTURE * sizeof(float);
				glEnableVertexAttribArray(defaultProgram.boneIds);
				glVertexAttribPointer(defaultProgram.boneIds, Mesh3d::VERTEX3D_BONEIDS, GL_UNSIGNED_SHORT, GL_FALSE, Mesh3d::VERTEX3D_STRIDE, (void*)offset);
	
				offset += Mesh3d::VERTEX3D_BONEIDS * sizeof(uint16_t);
				glEnableVertexAttribArray(defaultProgram.weights);
				glVertexAttribPointer(defaultProgram.weights, Mesh3d::VERTEX3D_WEIGHTS, GL_FLOAT, GL_FALSE, Mesh3d::VERTEX3D_STRIDE, (void*)offset);
				
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, buffers.texture);

				glUniform1i(defaultProgram.sampler, 0);
	
				glDrawElements(GL_TRIANGLES, buffers.iBufferLenght, GL_UNSIGNED_SHORT, (void*)0);
			}
		}
	
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		
		glDisableVertexAttribArray(defaultProgram.position);
		glDisableVertexAttribArray(defaultProgram.texPosition);
		glDisableVertexAttribArray(defaultProgram.boneIds);
		glDisableVertexAttribArray(defaultProgram.weights);
		
		glUseProgram(0);
	
		eglSwapBuffers(eglContext.display, eglContext.surface);
	
		return window->doStep();
	}
	
	// private
	
	void Renderer::initEgl(){
		EGLint minorVersion;
		EGLint majorVersion;
	
		auto display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
		if(display == EGL_NO_DISPLAY)
			throw br::EglException(EXCEPTION_INFO, "can`t get display");
	
		if(!eglInitialize(display, &majorVersion, &minorVersion))
			throw br::EglException(EXCEPTION_INFO, "cant init display");	
	
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
			throw br::EglException(EXCEPTION_INFO, "can`t choose config");
	
		const EGLint srfAttribs[] = {
			EGL_RENDER_BUFFER, EGL_BACK_BUFFER,
			EGL_NONE
		};
	
		auto surface = eglCreateWindowSurface(display, configs[0], window->nativeWindow, srfAttribs);
		if (surface == EGL_NO_SURFACE)
			throw br::EglException(EXCEPTION_INFO, "can`t create window surface");

	
		const EGLint ctxAttribs[] = {
			EGL_CONTEXT_CLIENT_VERSION, 2,
			EGL_NONE
		};
	
		auto context = eglCreateContext(display, configs[0], EGL_NO_CONTEXT, ctxAttribs);
		if (context == EGL_NO_CONTEXT)
			throw br::EglException(EXCEPTION_INFO, "can`t create context");

	
		if(!eglMakeCurrent(display, surface, surface, context))
			throw br::EglException(EXCEPTION_INFO, "can`t make context current");
	
		eglContext.display = display;
		eglContext.surface = surface;
		eglContext.context = context;
	}
	
	
	void Renderer::initShaders(string vShaderSrc, string fShaderSrc)
	{
		GLuint vShader = createShader(GL_VERTEX_SHADER, vShaderSrc.c_str());
		GLuint fShader = createShader(GL_FRAGMENT_SHADER, fShaderSrc.c_str());
	
		GLuint pObject = glCreateProgram();
		if(!pObject)
			throw br::ShaderException(EXCEPTION_INFO, "can`t create program");
	
		glAttachShader(pObject, vShader);
		glAttachShader(pObject, fShader);
	
		glLinkProgram(pObject);
	
		GLint linked;
		glGetProgramiv(pObject, GL_LINK_STATUS, &linked);
		if (!linked){
			glDeleteProgram(pObject);
			throw br::ShaderException(EXCEPTION_INFO, "can`t link program");
		}
	
		glDetachShader(pObject, vShader);
		glDeleteShader(vShader);
		glDetachShader(pObject, fShader);
		glDeleteShader(fShader);
	
		defaultProgram.id = pObject;

		defaultProgram.position = glGetAttribLocation(defaultProgram.id, "aPosition");
		defaultProgram.texPosition = glGetAttribLocation(defaultProgram.id, "aTexCoord");

		defaultProgram.bones = glGetUniformLocation(defaultProgram.id, "bones");
		defaultProgram.boneIds = glGetAttribLocation(defaultProgram.id, "boneIds");
		defaultProgram.weights = glGetAttribLocation(defaultProgram.id, "weights");

		defaultProgram.sampler = glGetUniformLocation(defaultProgram.id, "sTexture");
		defaultProgram.mvpMatrix = glGetUniformLocation(defaultProgram.id, "mvpMatrix");
	}
	
	GLuint Renderer::createShader(GLenum shType, const char* shSource){
		GLboolean hasCompiler;
		glGetBooleanv(GL_SHADER_COMPILER, &hasCompiler);
		if(hasCompiler == GL_FALSE)
			throw br::ShaderException(EXCEPTION_INFO, "no compiler");

		GLuint shader = glCreateShader(shType);
		if (!shader)
			throw br::ShaderException(EXCEPTION_INFO, "can`t create shader");

		glShaderSource(shader, 1, &shSource, NULL);
		glCompileShader(shader);
	
		GLint compiled;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
		if (!compiled){
			glDeleteShader(shader);
			throw br::ShaderException(EXCEPTION_INFO, "can`t compile shader");
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
	
	void Renderer::transform(uint32_t objId, const array<float, 16> tForm) {
		View* object;
		try {
			object = &idToObject.at(objId);
		} catch(std::out_of_range&) {
			throw InvalidObjectIdException(EXCEPTION_INFO, objId);
		}
		
		auto t = glm::make_mat4(tForm.data());
		object->setTransform(t);
	}
	
	
	bool Renderer::hasObjectWithModel(string path) {
		auto& it = find_if(cbegin(idToObject), cend(idToObject), [&path](pair<uint32_t, View> i)->bool{
			return i.second.getPath() == path;
		});
		return it != cend(idToObject);
	}
	
	void Renderer::loadModelToGpu(string modelPath) {
		Model3d& model = loader->getModelBy(modelPath);
		vector<Mesh3d>& meshes = model.getMeshes();
		for (auto& s : meshes) {
			uint32_t vBuffer;
			glGenBuffers(1, &vBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, vBuffer);
			
			auto vertices = s.getVertices();
			GLint szInBytes = sizeof(Vertex3d) * vertices.size();
			glBufferData(GL_ARRAY_BUFFER, szInBytes, &vertices[0], GL_STATIC_DRAW);
			
			GLint loadedBytes = 0;
			glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &loadedBytes);
			if(szInBytes != loadedBytes) {
				glDeleteBuffers(1, &vBuffer);
				throw GpuException(EXCEPTION_INFO, modelPath + " can`t load vertices");
			}
	

			uint32_t iBuffer;
			glGenBuffers(1, &iBuffer);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iBuffer);
	
			auto indices = s.getIndices();
			uint32_t iBufferLength = indices.size();
			szInBytes = sizeof(uint16_t) * iBufferLength;
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, szInBytes, &indices[0], GL_STATIC_DRAW);
			
			glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &loadedBytes);
			if(szInBytes != loadedBytes) {
				glDeleteBuffers(1, &iBuffer);
				throw GpuException(EXCEPTION_INFO, modelPath + " can`t load indices");
			}


			auto& materials = model.getMaterials();
			Material3d& m = materials.at(s.getMaterialId());
			uint32_t texture = loadTextureToGpu(m.getTexture());

	
			string meshName = model.getUniqueMeshName(s);
			GpuBufferData buffer{ vBuffer, iBuffer, iBufferLength, texture };
			meshToBuffer.emplace(meshName, buffer);
		}
	
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	
	void Renderer::deleteModelFromGpu(std::string modelPath) {
		Model3d& model = loader->getModelBy(modelPath);
		vector<Mesh3d>& meshes = model.getMeshes();
		for (auto& s : meshes) {
			string mName = model.getUniqueMeshName(s);
			GpuBufferData& buffers = meshToBuffer.at(mName);
	
			glDeleteBuffers(1, &buffers.vBuffer);
			glDeleteBuffers(1, &buffers.iBuffer);
	
			glDeleteTextures(1, &buffers.texture);
	
			meshToBuffer.erase(mName);
		}
	}
	
	GLuint Renderer::loadTextureToGpu(Texture2d& texture) {
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
	
		return textureId;
	}
}

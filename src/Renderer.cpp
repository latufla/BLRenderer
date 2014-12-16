#include "utils/SharedHeaders.h"
#include "Renderer.h"

#include "utils/Util.h"

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "Model3d.h"
#include "AssetLoader.h"
#include "Texture2d.h"
#include "bones/BoneNodeData.h"

#include <gtc/matrix_transform.hpp>
#include <utility>
#include <gtc/type_ptr.hpp>
#include "exceptions/Exception.h"
#include "text/FontLoader.h"

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
	Renderer::Renderer(std::shared_ptr<AssetLoader> loader,
		uint32_t wndX,
		uint32_t wndY,
		uint32_t wndW, 
		uint32_t wndH)
		: loader(loader) {
		
		window = make_shared <WindowVendor>(wndX, wndY, wndW, wndH);
		initEgl();
		initShaders();

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glDepthMask(true);

		// TODO: turn off when FXs only
// 		glEnable(GL_CULL_FACE);
// 		glCullFace(GL_BACK);
// 		glFrontFace(GL_CCW);
	}
	
	Renderer::~Renderer() {
		glDeleteProgram(modelProgram.id);
		glDeleteProgram(imageProgram.id);
		glDeleteProgram(textProgram.id);
	
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
	
	void Renderer::playAnimation(uint32_t objId, std::string animName, bool loop) {
		View* object;
		try {
			object = &idToObject.at(objId);
		} catch(std::out_of_range&) {
			throw InvalidObjectIdException(EXCEPTION_INFO, objId);
		}
		
		Model3d& model = loader->getModelBy(object->getPath());
		Animation3d& animation = model.getAnimationBy(animName);
		object->setAnimation(animName, (uint32_t)(animation.getDuration() * 1000), loop);
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
	
		glUseProgram(modelProgram.id);
	
		mat4 view = lookAt(vec3{camera.x, camera.y, camera.z}, vec3{0, 0, 0}, vec3{0, 1, 0});
		mat4 projection = perspective(45.0f, winSize.w / winSize.h, 0.1f, 100.0f);
		mat4 pvMatrix = projection * view;
	
		for (auto& i : idToObject){
			View& object = i.second;
			mat4 mvpMatrix = pvMatrix * object.getTransform();
			glUniformMatrix4fv(modelProgram.mvpMatrix, 1, GL_FALSE, &mvpMatrix[0][0]);
	
			Model3d& model = loader->getModelBy(object.getPath());
			vector<Mesh3d>& meshes = model.getMeshes();
			for (auto& s : meshes) {
				auto bonesData = prepareAnimationStep(object, s, stepMSec);
				for (auto& i : bonesData) {
					glUniformMatrix4fv(modelProgram.bones + i.first, 1, GL_FALSE, &(i.second.finalTransform[0][0]));
				}
	
				string meshName = model.getUniqueMeshName(s);
				GpuBufferData& buffers = meshToBuffer.at(meshName);
				glBindBuffer(GL_ARRAY_BUFFER, buffers.vBuffer);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers.iBuffer);
	
				uint8_t offset = 0;
				glEnableVertexAttribArray(modelProgram.position);
				glVertexAttribPointer(modelProgram.position, Mesh3d::VERTEX3D_POSITION, GL_FLOAT, GL_FALSE, Mesh3d::VERTEX3D_STRIDE, (void*)offset);
	
				offset += Mesh3d::VERTEX3D_POSITION * sizeof(float);
				glEnableVertexAttribArray(modelProgram.texPosition);
				glVertexAttribPointer(modelProgram.texPosition, Mesh3d::VERTEX3D_TEXTURE, GL_FLOAT, GL_FALSE, Mesh3d::VERTEX3D_STRIDE, (void*)offset);
	
				offset += Mesh3d::VERTEX3D_TEXTURE * sizeof(float);
				glEnableVertexAttribArray(modelProgram.boneIds);
				glVertexAttribPointer(modelProgram.boneIds, Mesh3d::VERTEX3D_BONEIDS, GL_UNSIGNED_SHORT, GL_FALSE, Mesh3d::VERTEX3D_STRIDE, (void*)offset);
	
				offset += Mesh3d::VERTEX3D_BONEIDS * sizeof(uint16_t);
				glEnableVertexAttribArray(modelProgram.weights);
				glVertexAttribPointer(modelProgram.weights, Mesh3d::VERTEX3D_WEIGHTS, GL_FLOAT, GL_FALSE, Mesh3d::VERTEX3D_STRIDE, (void*)offset);
				
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, buffers.texture);

				glUniform1i(modelProgram.sampler, 0);
	
				glDrawElements(GL_TRIANGLES, buffers.iBufferLenght, GL_UNSIGNED_SHORT, (void*)0);
			}
		}
	
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		
		glDisableVertexAttribArray(modelProgram.position);
		glDisableVertexAttribArray(modelProgram.texPosition);
		glDisableVertexAttribArray(modelProgram.boneIds);
		glDisableVertexAttribArray(modelProgram.weights);
		
		glUseProgram(0);
	
		drawUI();

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

	void Renderer::initShaders() {
		auto modelShaders = shaders.getShaderSrcBy(Shaders::MODEL_SHADER);
		modelProgram = createProgram(modelShaders.first, modelShaders.second);

		auto imageShaders = shaders.getShaderSrcBy(Shaders::IMAGE_SHADER);
		imageProgram = createProgram(imageShaders.first, imageShaders.second);
		
		auto textShaders = shaders.getShaderSrcBy(Shaders::TEXT_SHADER);
		textProgram = createProgram(textShaders.first, textShaders.second);
	}

	Renderer::ProgramContext Renderer::createProgram(string vShaderSrc, string fShaderSrc)
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
	
		ProgramContext program;
		program.id = pObject;

		program.position = glGetAttribLocation(pObject, "aPosition");
		program.texPosition = glGetAttribLocation(pObject, "aTexCoord");

		program.bones = glGetUniformLocation(pObject, "bones");
		program.boneIds = glGetAttribLocation(pObject, "boneIds");
		program.weights = glGetAttribLocation(pObject, "weights");

		program.sampler = glGetUniformLocation(pObject, "sTexture");
		program.mvpMatrix = glGetUniformLocation(pObject, "mvpMatrix");

		program.color = glGetUniformLocation(pObject, "color");
		return program;
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
	
	void Renderer::transformObject(uint32_t objId, const array<float, 16> tForm) {
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

	void Renderer::deleteTextureFromGpu(Texture2d& texture) {
		std::string pathAsKey = texture.getPath();
		GpuBufferData buffer = meshToBuffer.at(pathAsKey);
		glDeleteTextures(1, &buffer.texture);
		textureToBuffer.erase(pathAsKey);
	}


	// image
	void Renderer::addImage(uint32_t id, std::string path, std::pair<float, float> position) {
		Texture2d& texture = loader->getTextureBy(path);
		if(!hasImageWithTexture(path)) {
			uint32_t textureId = loadTextureToGpu(texture);
			GpuBufferData buffer{0, 0, 0, textureId};
			textureToBuffer.emplace(path, buffer);
		}

		auto wndSize = window->getRect();
		float sx = 2.0f / wndSize.w;
		float sy = 2.0f / wndSize.h;
		
		glm::vec2 pos{position.first, position.second};
		Image image{path, 
			pos,
			texture.getWidth() * sx, 
			texture.getHeight() * sy};
		loadImageToGpu(image);
		idToImage.emplace(id, image);
	}

	void Renderer::removeImage(uint32_t id) {
		Image& image = idToImage.at(id);
		std::string pathAsKey = image.getPath();
		Texture2d& texture = loader->getTextureBy(pathAsKey);
		idToImage.erase(id);

		if(!hasImageWithTexture(pathAsKey))
			deleteTextureFromGpu(texture);
	}

	bool Renderer::hasImageWithTexture(std::string path) {
		auto& it = find_if(cbegin(idToImage), cend(idToImage), [&path](pair<uint32_t, Image> i)->bool {
			return i.second.getPath() == path;
		});
		return it != cend(idToImage);
	}


	void Renderer::loadImageToGpu(Image& image) {
		uint32_t vBuffer;
		glGenBuffers(1, &vBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vBuffer);

		auto vertices = image.getVertices();
		GLint szInBytes = sizeof(Vertex3d) * vertices.size();
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

		auto indices = image.getIndices();
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

		GpuBufferData& textureBuffer = textureToBuffer.at(image.getPath());
		GpuBufferData buffer{vBuffer, iBuffer, iBufferLength, textureBuffer.texture};
		meshToBuffer.emplace(image.getPath(), buffer);
	}
	
	// Text
	void Renderer::addTextField(uint32_t id, string text, string fontName, uint8_t fontSize, array<float, 4> color, pair<float, float> position) {
		Font& font = loader->getFontBy(fontName, fontSize);
		if(!hasTextFieldWithFont(font))
			loadFontToGpu(font);

		glm::vec2 pos = {position.first, position.second};

		auto wndSize = window->getRect();
		glm::vec2 scaleFactor = {2.0f / wndSize.w, 2.0f / wndSize.h};
		
		TextField field{font, text, color, pos, scaleFactor};
		loadTextFieldToGpu(field);
		idToTextField.emplace(id, field);
	}

	void Renderer::removeTextField(uint32_t id) {
		TextField& field = idToTextField.at(id);
		Font& font = loader->getFontBy(field.getFontName(), field.getFontSize());
		idToTextField.erase(id);

		if(!hasTextFieldWithFont(font)) // last out
			deleteFontFromGpu(font);
	}

	void Renderer::translateTextField(uint32_t id, std::pair<float, float> position) {
		TextField& field = idToTextField.at(id);
		glm::vec2 pos{position.first, position.second};
		field.setPosition(pos);
	}

	void Renderer::loadFontToGpu(Font& font) {
		int32_t id = loadTextureToGpu(font.getAtlas());
		GpuBufferData data;
		data.texture = id;
		fontToBuffer.emplace(font.getUniqueName(), data);
	}
	
	void Renderer::deleteFontFromGpu(Font& font) {
		string nameAsKey = font.getUniqueName();
		GpuBufferData fontBuffer = fontToBuffer.at(nameAsKey);
		glDeleteTextures(1, &fontBuffer.texture);
		fontToBuffer.erase(nameAsKey);
	}
	
	bool Renderer::hasTextFieldWithFont(Font& font) {
		auto& it = find_if(cbegin(idToTextField), cend(idToTextField), [&font](pair<uint32_t, TextField> i)->bool {
			return i.second.getFontName() == font.getName() 
				&& i.second.getFontSize() == font.getSize();
		});
		return it != cend(idToTextField);
	}

	void Renderer::loadTextFieldToGpu(TextField& field) {
		uint32_t vBuffer;
		glGenBuffers(1, &vBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vBuffer);

		auto vertices = field.getVertices();
		GLint szInBytes = sizeof(Vertex3d) * vertices.size();
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

		auto indices = field.getIndices();
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
		

		Font& font = loader->getFontBy(field.getFontName(), field.getFontSize());
		GpuBufferData& fontBuffer = fontToBuffer.at(font.getUniqueName());

		GpuBufferData buffer{vBuffer, iBuffer, iBufferLength, fontBuffer.texture};
		meshToBuffer.emplace(field.getUniqueName(), buffer);
	}
	//
		
	void Renderer::drawUI() {
		auto winSize = window->getRect();
		float sx = (winSize.w / 1024.0f);
		float sy = (winSize.h / 768.0f);
		mat4 ortho = glm::ortho(-sx, sx, -sy, sy);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		drawImages(ortho);
		drawTextFields(ortho);

		glDisable(GL_BLEND);
	}

	void Renderer::drawImages(mat4& projection) {
		glUseProgram(imageProgram.id);

		for(auto& i : idToImage) {
			Image& object = i.second;

			mat4 translation = glm::translate(mat4(), vec3(object.getPosition(), 0.0f));
			mat4 mvpMatrix = translation * projection;
			glUniformMatrix4fv(textProgram.mvpMatrix, 1, GL_FALSE, &mvpMatrix[0][0]);
	
			GpuBufferData& buffers = meshToBuffer[object.getPath()];
			glBindBuffer(GL_ARRAY_BUFFER, buffers.vBuffer);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers.iBuffer);

			uint8_t offset = 0;
			glEnableVertexAttribArray(imageProgram.position);
			glVertexAttribPointer(imageProgram.position, Mesh3d::VERTEX3D_POSITION, GL_FLOAT, GL_FALSE, Mesh3d::VERTEX3D_STRIDE, (void*)offset);

			offset += Mesh3d::VERTEX3D_POSITION * sizeof(float);
			glEnableVertexAttribArray(imageProgram.texPosition);
			glVertexAttribPointer(imageProgram.texPosition, Mesh3d::VERTEX3D_TEXTURE, GL_FLOAT, GL_FALSE, Mesh3d::VERTEX3D_STRIDE, (void*)offset);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, buffers.texture);

			glUniform1i(imageProgram.sampler, 0);

			glDrawElements(GL_TRIANGLES, buffers.iBufferLenght, GL_UNSIGNED_SHORT, (void*)0);
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glDisableVertexAttribArray(imageProgram.position);
		glDisableVertexAttribArray(imageProgram.texPosition);
		
		glUseProgram(0);
	}

	void Renderer::drawTextFields(mat4& projection) {
		glUseProgram(textProgram.id);

		for(auto& i : idToTextField) {
			TextField& object = i.second;

			mat4 translation = glm::translate(mat4(), vec3(object.getPosition(), 0.0f));
			mat4 mvpMatrix = translation * projection;
			glUniformMatrix4fv(textProgram.mvpMatrix, 1, GL_FALSE, &mvpMatrix[0][0]);
			glUniform4fv(textProgram.color, 1, &object.getColor()[0]);

			GpuBufferData& buffers = meshToBuffer.at(object.getUniqueName());
			glBindBuffer(GL_ARRAY_BUFFER, buffers.vBuffer);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers.iBuffer);

			uint8_t offset = 0;
			glEnableVertexAttribArray(textProgram.position);
			glVertexAttribPointer(textProgram.position, Mesh3d::VERTEX3D_POSITION, GL_FLOAT, GL_FALSE, Mesh3d::VERTEX3D_STRIDE, (void*)offset);

			offset += Mesh3d::VERTEX3D_POSITION * sizeof(float);
			glEnableVertexAttribArray(textProgram.texPosition);
			glVertexAttribPointer(textProgram.texPosition, Mesh3d::VERTEX3D_TEXTURE, GL_FLOAT, GL_FALSE, Mesh3d::VERTEX3D_STRIDE, (void*)offset);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, buffers.texture);

			glUniform1i(textProgram.sampler, 0);

			glDrawElements(GL_TRIANGLES, buffers.iBufferLenght, GL_UNSIGNED_SHORT, (void*)0);
		}

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glDisableVertexAttribArray(textProgram.position);
		glDisableVertexAttribArray(textProgram.texPosition);

		glUseProgram(0);
	}
}

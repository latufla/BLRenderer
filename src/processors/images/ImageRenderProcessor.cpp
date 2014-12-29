#include "../../utils/SharedHeaders.h"
#include "ImageRenderProcessor.h"

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "../../exceptions/Exception.h"

#include <gtc/matrix_transform.hpp>

using std::pair;
using glm::mat4;
using glm::vec3;
using glm::vec2;
using glm::translate;
using std::string;

namespace br {
	ImageRenderProcessor::ImageRenderProcessor(std::shared_ptr<AssetLoader>loader, std::pair<std::string, std::string> shaders)
		: ProcessorBase(loader, shaders) {
	}

	ImageRenderProcessor::~ImageRenderProcessor() {
		auto allImages = idToImage;
		for(auto& i : allImages) {
			removeImage(i.first);
		}
	}

	void ImageRenderProcessor::addImage(uint32_t id, std::string path, std::pair<float, float> position) {
		if(!enabled)
			throw LogicException(EXCEPTION_INFO, "ImageRenderProcessor not added to Renderer");

		auto sWindow = window.lock();
		if(!sWindow)
			throw WeakPtrException(EXCEPTION_INFO);

		auto it = idToImage.find(id);
		if(it != cend(idToImage))
			throw InvalidObjectIdException(EXCEPTION_INFO, id);


		auto wndSize = sWindow->getSize();
		float sx = 2.0f / wndSize.w;
		float sy = 2.0f / wndSize.h;

		vec2 pos{position.first, position.second};
		Texture2d& texture = loader->getTextureBy(path);
		Image image{texture, pos, sx, sy};

		if(!hasImageWithTexture(path))
			loadImageToGpu(image);

		idToImage.emplace(id, image);
	}

	void ImageRenderProcessor::removeImage(uint32_t id) {
		if(!enabled)
			throw LogicException(EXCEPTION_INFO, "ImageRenderProcessor not added to Renderer");
		
		Image* image;
		try {
			image = &idToImage.at(id);
		} catch(std::out_of_range&) {
			throw InvalidObjectIdException(EXCEPTION_INFO, id);
		}


		string pathAsKey = image->getPath();
		idToImage.erase(id);

		if(!hasImageWithTexture(pathAsKey))
			deleteImageFromGpu(pathAsKey);
	}
	

	ProcessorBase::ProgramContext ImageRenderProcessor::fillProgramContext(uint32_t pObject) {
		ProgramContext program = __super::fillProgramContext(pObject);

		program.position = glGetAttribLocation(pObject, "aPosition");
		program.texPosition = glGetAttribLocation(pObject, "aTexCoord");

		program.sampler = glGetUniformLocation(pObject, "sTexture");
		program.mvpMatrix = glGetUniformLocation(pObject, "mvpMatrix");

		return program;
	}

	void ImageRenderProcessor::doStep(StepData& stepData) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		glUseProgram(program.id);

		for(auto& i : idToImage) {
			Image& object = i.second;

			mat4 translation = translate(mat4(), vec3(object.getPosition(), 0.0f));
			mat4 mvpMatrix = translation * stepData.ortho;
			glUniformMatrix4fv(program.mvpMatrix, 1, GL_FALSE, &mvpMatrix[0][0]);

			GpuBufferData& buffers = meshToBuffer[object.getPath()];
			glBindBuffer(GL_ARRAY_BUFFER, buffers.vBuffer);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers.iBuffer);

			uint8_t offset = 0;
			glEnableVertexAttribArray(program.position);
			glVertexAttribPointer(program.position, Mesh3d::VERTEX3D_POSITION, GL_FLOAT, GL_FALSE, Mesh3d::VERTEX3D_STRIDE, (void*)offset);

			offset += Mesh3d::VERTEX3D_POSITION * sizeof(float);
			glEnableVertexAttribArray(program.texPosition);
			glVertexAttribPointer(program.texPosition, Mesh3d::VERTEX3D_TEXTURE, GL_FLOAT, GL_FALSE, Mesh3d::VERTEX3D_STRIDE, (void*)offset);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, buffers.texture);

			glUniform1i(program.sampler, 0);

			glDrawElements(GL_TRIANGLES, buffers.iBufferLenght, GL_UNSIGNED_SHORT, (void*)0);
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glDisableVertexAttribArray(program.position);
		glDisableVertexAttribArray(program.texPosition);

		glUseProgram(0);

		glDisable(GL_BLEND);
	}


	bool ImageRenderProcessor::hasImageWithTexture(string pathAsKey) {
		auto& it = find_if(cbegin(idToImage), cend(idToImage), [&pathAsKey](pair<uint32_t, Image> i)->bool {
			return i.second.getPath() == pathAsKey;
		});
		return it != cend(idToImage);
	}

	void ImageRenderProcessor::loadImageToGpu(Image& image) {
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
		

		string pathAsKey = image.getPath();
		Texture2d& texture = loader->getTextureBy(pathAsKey);
		loadTextureToGpu(texture);
		
		uint32_t textureId = textureToId.at(pathAsKey);
		GpuBufferData buffer{vBuffer, iBuffer, iBufferLength, textureId};
		meshToBuffer.emplace(image.getPath(), buffer);
	}

	void ImageRenderProcessor::deleteImageFromGpu(std::string pathAsKey) {
		GpuBufferData& buffers = meshToBuffer.at(pathAsKey);

		glDeleteBuffers(1, &buffers.vBuffer);
		glDeleteBuffers(1, &buffers.iBuffer);

		meshToBuffer.erase(pathAsKey);
	
		deleteTextureFromGpu(pathAsKey);
	}
}

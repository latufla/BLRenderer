#include "TextRenderProcessor.h"
#include "../../utils/SharedHeaders.h"

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "../../exceptions/Exception.h"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

using std::string;
using std::array;
using std::pair;
using glm::mat4;
using glm::vec3;
using glm::vec2;
using glm::translate;

namespace br {
	TextRenderProcessor::TextRenderProcessor(std::shared_ptr<AssetLoader>loader, std::pair<std::string, std::string> shaders)
		: ProcessorBase(loader, shaders) {
	}

	TextRenderProcessor::~TextRenderProcessor() {
		auto allTextFields = idToTextField;
		for(auto& i : allTextFields) {
			removeTextField(i.first);
		}
	}

	void TextRenderProcessor::addTextField(uint32_t id, string text, string fontName, uint8_t fontSize, array<float, 4> color, pair<float, float> position) {
		auto sWindow = window.lock();
		if(!sWindow)
			throw WeakPtrException(EXCEPTION_INFO);

		auto it = idToTextField.find(id);
		if(it != cend(idToTextField))
			throw InvalidObjectIdException(EXCEPTION_INFO, id);


		Font& font = loader->getFontBy(fontName, fontSize);
		if(!hasTextFieldWithFont(font))
			loadFontToGpu(font);

		auto wndSize = sWindow->getSize();
		vec2 scaleFactor = {2.0f / wndSize.w, 2.0f / wndSize.h};		
		vec2 pos = {position.first, position.second};		

		TextField field{font, text, color, pos, scaleFactor};
		loadTextFieldToGpu(field);
		
		idToTextField.emplace(id, field);
	}

	void TextRenderProcessor::removeTextField(uint32_t id) {
		auto sWindow = window.lock();
		if(!sWindow)
			throw WeakPtrException(EXCEPTION_INFO);

		TextField* field;
		try {
			field = &idToTextField.at(id);
		} catch(std::out_of_range&) {
			throw InvalidObjectIdException(EXCEPTION_INFO, id);
		}
		 

		Font& font = loader->getFontBy(field->getFontName(), field->getFontSize());
		deleteTextFieldFromGpu(*field);
		idToTextField.erase(id);

		if(!hasTextFieldWithFont(font)) // last out
			deleteFontFromGpu(font);
	}

	void TextRenderProcessor::translateTextField(uint32_t id, std::pair<float, float> position) {
		auto sWindow = window.lock();
		if(!sWindow)
			throw WeakPtrException(EXCEPTION_INFO);

		TextField* field;
		try {
			field = &idToTextField.at(id);
		} catch(std::out_of_range&) {
			throw InvalidObjectIdException(EXCEPTION_INFO, id);
		}

		vec2 pos{position.first, position.second};
		field->setPosition(pos);
	}

	ProcessorBase::ProgramContext TextRenderProcessor::fillProgramContext(uint32_t pObject) {
		ProgramContext program = __super::fillProgramContext(pObject);

		program.position = glGetAttribLocation(pObject, "aPosition");
		program.texPosition = glGetAttribLocation(pObject, "aTexCoord");

		program.sampler = glGetUniformLocation(pObject, "sTexture");
		program.mvpMatrix = glGetUniformLocation(pObject, "mvpMatrix");

		program.color = glGetUniformLocation(pObject, "color");
		return program;
	}

	void TextRenderProcessor::doStep(const StepData& stepData) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glUseProgram(program.id);

		for(auto& i : idToTextField) {
			TextField& object = i.second;

			mat4 translation = translate(mat4(), vec3(object.getPosition(), 0.0f));
			mat4 mvpMatrix = translation * stepData.ortho;
			glUniformMatrix4fv(program.mvpMatrix, 1, GL_FALSE, &mvpMatrix[0][0]);
			glUniform4fv(program.color, 1, &object.getColor()[0]);

			GpuBufferData& buffers = meshToBuffer.at(object.getUniqueName());
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


	void TextRenderProcessor::loadTextFieldToGpu(TextField& field) {
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
		Texture2d& atlas = font.getAtlas();
		uint32_t textureId = textureToId.at(atlas.getPath());
		
		GpuBufferData buffer{vBuffer, iBuffer, iBufferLength, textureId};
		meshToBuffer.emplace(field.getUniqueName(), buffer);
	}

	void TextRenderProcessor::deleteTextFieldFromGpu(TextField& field) {
		std::string nameAsKey = field.getUniqueName();
		GpuBufferData& buffers = meshToBuffer.at(nameAsKey);

		glDeleteBuffers(1, &buffers.vBuffer);
		glDeleteBuffers(1, &buffers.iBuffer);

		meshToBuffer.erase(nameAsKey);
	}


	void TextRenderProcessor::loadFontToGpu(Font& font) {
		Texture2d& atlas = font.getAtlas();
		loadTextureToGpu(atlas);
	}

	void TextRenderProcessor::deleteFontFromGpu(Font& font) {
		Texture2d& atlas = font.getAtlas();
		deleteTextureFromGpu(atlas.getPath());
	}

	bool TextRenderProcessor::hasTextFieldWithFont(Font& font) {
		auto& it = find_if(cbegin(idToTextField), cend(idToTextField), [&font](pair<uint32_t, TextField> i)->bool {
			return i.second.getFontName() == font.getName()
				&& i.second.getFontSize() == font.getSize();
		});
		return it != cend(idToTextField);
	}
}
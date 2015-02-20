#include "TextRenderProcessor.h"
#include "../../utils/SharedHeaders.h"

#include "../../exceptions/Exception.h"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

using std::string;
using std::array;
using std::pair;
using glm::mat4;
using glm::vec4;
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

	void TextRenderProcessor::addTextField(uint32_t id, string text, string fontName, uint8_t fontSize, const vec4& color, const vec2& position) {
		auto it = idToTextField.find(id);
		if(it != cend(idToTextField))
			throw InvalidObjectIdException(EXCEPTION_INFO, id);

		auto sGConnector = graphics.lock();
		if(!sGConnector)
			throw WeakPtrException(EXCEPTION_INFO);

		Font& font = loader->getFontBy(fontName, fontSize);
		if(!hasTextFieldWithFont(font))
			loadFontToGpu(font);

		auto wSize = sGConnector->getWindowSize();
		vec2 scaleFactor = {(2.0f * sGConnector->getAspectRatio()) / wSize.w, 2.0f / wSize.h};

		TextField field{font, text, color, position, scaleFactor};
		loadTextFieldToGpu(field);
		
		idToTextField.emplace(id, field);
	}

	void TextRenderProcessor::removeTextField(uint32_t id) {
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

	void TextRenderProcessor::translateTextField(uint32_t id, const glm::vec2& position) {
		TextField* field;
		try {
			field = &idToTextField.at(id);
		} catch(std::out_of_range&) {
			throw InvalidObjectIdException(EXCEPTION_INFO, id);
		}

		field->setPosition(position);
	}

	void TextRenderProcessor::doStep(const StepData& stepData) {
		auto sGConnector = graphics.lock();
		if(!sGConnector)
			throw WeakPtrException(EXCEPTION_INFO);

		sGConnector->setBlending(true);

		for(auto& i : idToTextField) {
			TextField& object = i.second;

			mat4 translation = translate(mat4(), vec3(object.getPosition(), 0.0f));
			mat4 mvp = translation * stepData.ortho;

			auto& buffer = meshToBuffer.at(object.getUniqueName());
			sGConnector->draw(object, buffer, program, mvp);
		}

		sGConnector->setBlending(false);		
	}


	void TextRenderProcessor::loadTextFieldToGpu(TextField& field) {
		loadGeometryToGpu(field.getUniqueName(), field.getVertices(), field.getIndices());

		Font& font = loader->getFontBy(field.getFontName(), field.getFontSize());
		Texture2d& atlas = font.getAtlas();
		uint32_t textureId = textureToId.at(atlas.getPath());
		
		auto& buffer = meshToBuffer.at(field.getUniqueName());
		buffer.texture = textureId;
	}

	void TextRenderProcessor::deleteTextFieldFromGpu(TextField& field) {
		deleteGeometryFromGpu(field.getUniqueName());
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
#pragma once
#include "..\ProcessorBase.h"
#include <array>
#include "TextField.h"

namespace br {
	class TextRenderProcessor : public ProcessorBase {
	public:
		TextRenderProcessor(std::shared_ptr<AssetLoader>loader, std::pair<std::string, std::string> shaiders);
		~TextRenderProcessor();

		void addTextField(uint32_t id,
			std::string text,
			std::string font,
			uint8_t fontSize,
			std::array<float, 4> color,
			std::pair<float, float> position);

		void removeTextField(uint32_t id);
		void translateTextField(uint32_t id, std::pair<float, float> position);

	private:
		void doStep(const StepData& stepData) override;

		std::unordered_map<uint32_t, TextField> idToTextField;

		void loadFontToGpu(Font&);
		void deleteFontFromGpu(Font&);

		void loadTextFieldToGpu(TextField&);
		void deleteTextFieldFromGpu(TextField&);
		bool hasTextFieldWithFont(Font&);
	};
}

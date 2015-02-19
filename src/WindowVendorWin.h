#pragma once

#include "IWindowVendor.h"

namespace br {
	class WindowVendorWin : public IWindowVendor {
	public:
		WindowVendorWin() = delete;
		WindowVendorWin(const IWindowVendor::Rect& size);

		virtual ~WindowVendorWin();

		bool doStep() const override;

		IWindowVendor::Rect getSize() const override;

		glm::vec2 getScaleFactor() const override;

		glm::vec2 getMousePosition() const override;

		bool getMouseDownLeft() const override;
		bool getMouseDownRight() const override;

		void* getNativeWindow() override;

	private:
		void* nativeWindow;
		Rect initialSize;
	};

}


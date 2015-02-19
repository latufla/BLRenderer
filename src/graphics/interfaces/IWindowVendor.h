#pragma once
#include <glm.hpp>

namespace br {
	class IWindowVendor {
	public:
		virtual bool doStep() = 0;
		virtual void* getNativeWindow() = 0;

		virtual void swapBuffers() = 0;

		struct Rect {
			float x;
			float y;
			float w;
			float h;
		};
		virtual Rect getSize() const = 0;

		virtual float getAspectRatio() const = 0;
		
		virtual glm::vec2 getMousePosition() const = 0;
	
		virtual bool getMouseDownLeft() const = 0;
		virtual bool getMouseDownRight() const = 0;
	};
}


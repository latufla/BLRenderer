#pragma once
#include <EGL/egl.h>
#include <vector>
#include <utility>

namespace br {
	class WindowVendor
	{
	public:
		WindowVendor() = delete;
		WindowVendor(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
		
		~WindowVendor() = default;
	
		bool doStep() const;
	
		EGLNativeWindowType nativeWindow;
	
		struct Rect {
			float x;
			float y;
			float w;
			float h;
		};
		Rect getSize();

		std::pair<float, float> getScaleFactor();
	
		std::pair<float, float> getMousePosition();
		bool getMouseDownLeft();
		bool getMouseDownRight();
	private:
		Rect initialSize;
	};
}


#pragma once
#include <EGL/egl.h>
#include <vector>

namespace br {
	class WindowVendor
	{
	public:
		WindowVendor() = delete;
		WindowVendor(uint32_t x = 0, uint32_t y = 0, uint32_t w = 640, uint32_t h = 480);
		
		~WindowVendor() = default;
	
		bool doStep() const;
	
		EGLNativeWindowType nativeWindow;
	
		struct Rect {
			float x;
			float y;
			float w;
			float h;
		};
		Rect getRect();
	};
}


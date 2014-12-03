#pragma once
#include <EGL/egl.h>
#include <vector>

namespace br {
	class WindowVendor
	{
	public:
		WindowVendor(uint32_t = 0, uint32_t = 0, uint32_t = 640, uint32_t = 480);
		~WindowVendor();
	
		bool doStep() const;
	
		EGLNativeWindowType nativeWindow;
	
		std::vector<float> getRect();
	};
}


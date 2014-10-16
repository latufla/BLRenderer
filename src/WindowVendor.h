#pragma once
#include <windows.h>
#include <EGL/egl.h>
#include <vector>

class WindowVendor
{
public:
	WindowVendor(int = 0, int = 0, int = 640, int = 480);
	~WindowVendor();

	bool doStep(int = 0) const;

	EGLNativeWindowType nativeWindow;

	std::vector<float> getRect();
};


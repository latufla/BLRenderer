#include "../utils/SharedHeaders.h"

#include <windows.h>
#include <vector>

#include "WindowVendorWin.h"
#include "../exceptions/Exception.h"

using std::vector;
using std::pair;

namespace br {
	LRESULT CALLBACK processMessages(HWND, UINT, WPARAM, LPARAM);
	
	WindowVendorWin::WindowVendorWin(const IWindowVendor::Rect& size)
		: initialSize(size) {
		HINSTANCE hInstance = GetModuleHandle(nullptr);
	
		WNDCLASSEX wClass;
		wClass.cbSize = sizeof(WNDCLASSEX);
		wClass.style = CS_HREDRAW | CS_VREDRAW;
		wClass.lpfnWndProc = processMessages;
		wClass.cbClsExtra = 0;
		wClass.cbWndExtra = 0;
		wClass.hInstance = hInstance;
		wClass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
		wClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		wClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		wClass.lpszMenuName = NULL;
		wClass.lpszClassName = _T("CustomWindow");
		wClass.hIconSm = LoadIcon(wClass.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
	
		if(!RegisterClassEx(&wClass))
			throw br::NativeWindowException(EXCEPTION_INFO, "can`t register window class");
			
		RECT wRect{(LONG)size.x, (LONG)size.y, (LONG)size.w, (LONG)size.h};
		AdjustWindowRect(&wRect, WS_BORDER | WS_DLGFRAME, false);
			 
		auto hWnd = CreateWindow(
			wClass.lpszClassName,
			_T("Windows"),
			WS_OVERLAPPEDWINDOW,
			(int32_t)size.x, (int32_t)size.y,
			wRect.right - wRect.left, wRect.bottom - wRect.top,
			NULL,
			NULL,
			hInstance,
			NULL
			);
	
		if (!hWnd)
			throw br::NativeWindowException(EXCEPTION_INFO, "can`t create window");

		nativeWindow = hWnd;
	
		ShowWindow(hWnd, true);
		UpdateWindow(hWnd);
	}

	WindowVendorWin::~WindowVendorWin() {
		// destroy window here
	}


	void WindowVendorWin::swapBuffers() {
		HDC device = GetDC((HWND)nativeWindow);
		SwapBuffers(device);
	}

	IWindowVendor::Rect WindowVendorWin::getSize() const
	{
		RECT rect;
		GetClientRect((HWND)nativeWindow, &rect);
		return {
				(float)rect.left,
				(float)rect.top,
				(float)rect.right - (float)rect.left,
				(float)rect.bottom - (float)rect.top
		};
	}
	
	glm::vec2 WindowVendorWin::getMousePosition() const {
		POINT pos;
		GetCursorPos(&pos);
		ScreenToClient((HWND)nativeWindow, &pos);
		glm::vec2 res{(float)pos.x, (float)pos.y};
		return res;
	}

	bool WindowVendorWin::getMouseDownLeft() const {
		return (GetKeyState(VK_LBUTTON) & 0x80) != 0;
	}

	bool WindowVendorWin::getMouseDownRight() const {
		return (GetKeyState(VK_RBUTTON) & 0x80) != 0;
	}

	bool WindowVendorWin::doStep()
	{
		MSG msg;
		if (!PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			return true;
	
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	
		return msg.message != WM_QUIT;
	}

	float WindowVendorWin::getAspectRatio() const {
		auto wSize = getSize();
		return (float)wSize.w / (float)wSize.h;
	}

	void* WindowVendorWin::getNativeWindow() {
		return nativeWindow;
	}


	LRESULT CALLBACK processMessages(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg) {
			case WM_PAINT:
				ValidateRect(hWnd, NULL);
				break;
			case WM_CLOSE:
				DestroyWindow(hWnd);
				UnregisterClass(_T("CustomWindow"), GetModuleHandle(nullptr));
				break;
			case WM_DESTROY:
				PostQuitMessage(0);
				break;
			default:
				return DefWindowProc(hWnd, msg, wParam, lParam);
		}
		return 0;
	}	
}

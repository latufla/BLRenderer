#include "utils/SharedHeaders.h"
#include <windows.h>
#include "WindowVendor.h"
#include "exceptions\Exception.h"

using std::vector;
using std::pair;

namespace br {
	LRESULT CALLBACK processMessages(HWND, UINT, WPARAM, LPARAM);
	
	WindowVendor::WindowVendor(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
		: initialSize({(float)x, (float)y, (float)w, (float)h}) {
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
			
		RECT wRect{ x, y, w, h };
		AdjustWindowRect(&wRect, WS_BORDER | WS_DLGFRAME, false);
			 
		EGLNativeWindowType hWnd = CreateWindow(
			wClass.lpszClassName,
			_T("Windows"),
			WS_OVERLAPPEDWINDOW,
			x, y,
			wRect.right - wRect.left, wRect.bottom - wRect.top,
			NULL,
			NULL,
			hInstance,
			NULL
			);
	
		if (!hWnd)
			throw br::NativeWindowException(EXCEPTION_INFO, "can`t create window");

		nativeWindow = hWnd;
	
		// TODO: maybe not here
		ShowWindow(hWnd, true);
		UpdateWindow(hWnd);
	}
		
	WindowVendor::Rect WindowVendor::getSize()
	{
		RECT rect;
		GetClientRect(nativeWindow, &rect);
		return {
				(float)rect.left,
				(float)rect.top,
				(float)rect.right - (float)rect.left,
				(float)rect.bottom - (float)rect.top
		};
	}
	
	std::pair<float, float> WindowVendor::getMousePosition() {
		POINT pos;
		GetCursorPos(&pos);
		ScreenToClient(nativeWindow, &pos);
		std::pair<float, float> res{(float)pos.x, (float)pos.y};
		return res;
	}

	bool WindowVendor::getMouseDownLeft() {
		return (GetKeyState(VK_LBUTTON) & 0x80) != 0;
	}

	bool WindowVendor::getMouseDownRight() {
		return (GetKeyState(VK_RBUTTON) & 0x80) != 0;
	}

	bool WindowVendor::doStep() const
	{
		MSG msg;
		if (!PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			return true;
	
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	
		return msg.message != WM_QUIT;
	}

	pair<float, float> WindowVendor::getScaleFactor() {
		auto size = getSize();
		return{size.w / initialSize.w, size.h / initialSize.h};
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

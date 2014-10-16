#include "stdafx.h"

#ifdef ANDROID_PLATFORM
#include "WindowVendor.h"

using namespace std;

LRESULT CALLBACK processMessages(HWND, UINT, WPARAM, LPARAM);

WindowVendor::WindowVendor(int x, int y, int w, int h)
{
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

	if (!RegisterClassEx(&wClass))
		return;

	EGLNativeWindowType hWnd = CreateWindow(
		wClass.lpszClassName,
		_T("Android"),
		WS_OVERLAPPEDWINDOW,
		x, y, w, h,
		NULL,
		NULL,
		hInstance,
		NULL
		);

	if (!hWnd)
		return;

	nativeWindow = hWnd;

	// TODO: maybe not here
	ShowWindow(hWnd, true);
	UpdateWindow(hWnd);
}

WindowVendor::~WindowVendor()
{
	// TODO: implement if needed
}

bool WindowVendor::doStep(int stepMSec) const
{
	MSG msg;
	if (!GetMessage(&msg, NULL, 0, 0))
		return false;

	TranslateMessage(&msg);
	DispatchMessage(&msg);

	return msg.message != WM_QUIT;
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
#endif



#include "stdafx.h"

#include "WindowVendor.h"

using std::vector;

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

vector<float> WindowVendor::getRect()
{
	RECT rect;
	GetClientRect(nativeWindow, &rect);
	return vector < float > {
			(float)rect.left,
			(float)rect.top,
			(float)rect.right - (float)rect.left,
			(float)rect.bottom - (float)rect.top
	};
}


bool WindowVendor::doStep(int stepMSec) const
{
	MSG msg;
	if (!PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		return true;

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


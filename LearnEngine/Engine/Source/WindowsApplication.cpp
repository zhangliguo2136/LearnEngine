#include "WindowsApplication.h"
#include "Engine.h"
#include <tchar.h>


void TWindowsApplication::Tick(TEngine* Engine)
{
	Engine->Update(0.f);
}

bool TWindowsApplication::IsQuit()
{
	return bQuit;
}

bool TWindowsApplication::Initialize()
{
	// ≥ı ºªØ¥∞ø⁄
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = mhAppInst;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);

	// ∑¿÷π±≥æ∞÷ÿªÊ
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszClassName = _T("LearnEngine");

	RegisterClassEx(&wc);

	mhMainWnd = CreateWindowEx(0,
		_T("LearnEngine"),
		_T("LearnEngine"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		WindowWidth,
		WindowHeight,
		NULL,
		NULL,
		mhAppInst,
		NULL);

	ShowWindow(mhMainWnd, SW_SHOW);
	UpdateWindow(mhMainWnd);
	return false;
}

bool TWindowsApplication::Finalize()
{
	return false;
}

LRESULT TWindowsApplication::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_PAINT:
	{

	}
	break;
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}
	break;
	case WM_LBUTTONDOWN:
	{

	}
	break;
	case WM_LBUTTONUP:
	{

	}
	break;
	case WM_RBUTTONDOWN:
	{

	}
	break;
	case WM_RBUTTONUP:
	{

	}
	break;
	case WM_KEYDOWN:
	{

	}
	break;
	case WM_KEYUP:
	{
		if (wParam == VK_ESCAPE)
		{
			PostQuitMessage(0);
		}

		return 0;
	}
	break;
	case WM_MOUSEMOVE:
	{

	}
	break;
	case WM_MOUSEWHEEL:
	{
		//printf("%d\n", (int)GET_WM_VSCROLL_POS(wParam, lParam));
	}
	break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

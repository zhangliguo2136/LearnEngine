#pragma once

#include <windows.h>
#include <windowsx.h>
#include <stdint.h>

class TEngine;
class TWindowsApplication
{
public:
	int Run(TEngine* Engine);
	bool Initialize();
	bool Finalize();

public:
	static LRESULT CALLBACK WindowProc(HWND hWnd,
		UINT message,
		WPARAM wParam,
		LPARAM lParam);

public:
	HWND GetWindowHandle() { return mhMainWnd; }

	uint32_t GetWindowWidth() { return WindowWidth; }
	uint32_t GetWindowHeight() { return WindowHeight; }

private:
	HINSTANCE mhAppInst = nullptr; // application instance handle
	HWND      mhMainWnd = nullptr; // main window handle

	static const uint32_t WindowWidth = 960;
	static const uint32_t WindowHeight = 540;
};
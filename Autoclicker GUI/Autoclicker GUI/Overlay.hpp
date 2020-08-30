#pragma once

#include <Windows.h>
#include <windowsx.h>
#include <string>
#include <d2d1_2.h>
#include <dxgi1_3.h>
#include <dcomp.h>
#include <ddraw.h>
#include <d3d9.h>
#include <dwmapi.h>
#include <vector>
#include <chrono>
#include <thread>
#include <wtsapi32.h>
#include <winapifamily.h>
#include <hidusage.h>
#include <shellscalingapi.h>
#pragma comment(lib, "Imm32.lib")

#include "GUI/CoGUI.hpp"
#include "GUI/Draw/Draw.hpp"
#include "GUI/IO/IO.hpp"

typedef HWND(WINAPI* CreateWindowInBandEx)(
	_In_ DWORD dwExStyle,
	_In_opt_ ATOM atom,
	_In_opt_ LPCWSTR lpWindowName,
	_In_ DWORD dwStyle,
	_In_ int X,
	_In_ int Y,
	_In_ int nWidth,
	_In_ int nHeight,
	_In_opt_ HWND hWndParent,
	_In_opt_ HMENU hMenu,
	_In_opt_ HINSTANCE hInstance,
	_In_opt_ LPVOID lpParam,
	_In_ DWORD dwBand,
	_In_ DWORD dwTypeFlags
	);

namespace Overlay
{
	BOOL Init(HINSTANCE hInstance);
	BOOL Work();

	BOOL Toggle();
	void Destroy();
	HWND GetWindow();
	BOOL AlreadyRunning();

	ATOM OverlayRegisterClass(HINSTANCE hInstance);
	BOOL InitInstance(HINSTANCE, ATOM, int);
	LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
	LRESULT CALLBACK LowLevelKeyboardProc(int, WPARAM, LPARAM);
	BOOL COGUIProc(UINT elementID, UINT message, void* pStruct);

	HRESULT Redraw();
}

using namespace std;
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
#include "Def.hpp"

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
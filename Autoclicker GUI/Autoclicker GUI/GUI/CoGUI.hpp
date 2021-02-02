#pragma once

#include <Windows.h>
#include <Windowsx.h>
#include <string>
#include <d3d11.h>
#include <vector>

#include "Elements.hpp"
#include "../../../Log.hpp"
#include "Draw/Draw.hpp"
#include "IO/IO.hpp"
#include "Memory/Memory.hpp"
#include "Def.hpp"

// lParam - pointer to the element
constexpr auto COGUI_CREATE = 1;

// lParam - pointer to the element
constexpr auto COGUI_RENDER = 2;

// lParam - pointer to the element
constexpr auto COGUI_INPUT = 3;

constexpr auto COGUI_DL_PREOPEN = 1;
constexpr auto COGUI_DL_POSTCLOSE = 2;
constexpr auto COGUI_DL_CHOOSE = 3;

namespace COGUI
{
	BOOL       Init(HWND _hWnd, LPVOID lpfnWndProc);

	LRESULT    WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	BOOL       Routine(DWORD dwMessageID, LPARAM lParam);

	D2D1_COLOR_F COGUI_COLOR(FLOAT r, FLOAT g, FLOAT  b, FLOAT a = 1.0f);
	D2D1_COLOR_F COGUI_COLOR(INT r, INT g, INT  b, INT a = 255);

	BOOL       CreateElement(UINT elementID, LPCWSTR lpElementName, FLOAT x, FLOAT y, FLOAT nWidth, FLOAT nHeight, UINT uiParams, INT ID);
	BOOL       Render();
	BOOL       IsVisible(INT elementID, bool* lpBool);
	BOOL       IsActive(INT elementID);
	BOOL       ShowElement(INT elementID, bool bVisible);
	INT        GetID(Element* lpElement);
	INT        GetElementType(Element* lpElement);
	//BOOL       AddTableColumn(INT elementID, LPWCH pwchName);
	BOOL       AddTableString(INT elementID, wstring pwchName);
	BOOL       AddDropListString(INT elementID, wstring pStr);
	INT        GetDropListSelectedLine(INT elementID);
	wstring    GetDropListSelectedLineName(INT elementID);
	HWND       GetWindow();
	D2D1_SIZE_F GetWindowSize();
	INT        GetActiveElementID();
	BOOL       SetActiveElementID(INT eID);
	BOOL       SetElementText(INT elementID, wstring wText);
	INT        GetTableSelectedLine(INT elementID);
	BOOL       DeleteTableLine(INT elementID, INT nLine);
	BOOL       ClearDropList(INT elementID);
}
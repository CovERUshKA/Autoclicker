#pragma once

#include <Windows.h>
#include <Windowsx.h>
#include <string>
#include <d3d11.h>

#include "Elements.hpp"
#include "../../../Log.hpp"
#include "Draw/Draw.hpp"
#include "IO/IO.hpp"
#include "Def.hpp"
#include "Elements/ID.hpp"

constexpr auto COGUI_DL_PREOPEN = 1;
constexpr auto COGUI_DL_POSTCLOSE = 2;
constexpr auto COGUI_DL_CHOOSE = 3;

struct DropListReceive
{
	std::wstring pStr;
};

struct RecordButtonReceive
{
	unsigned char key;

	std::wstring pStr;
};

namespace COGUI
{
	BOOL       Init(HWND _hWnd, LPVOID lpfnWndProc);

	LRESULT    WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	D2D1_COLOR_F COGUI_COLOR(FLOAT r, FLOAT g, FLOAT  b, FLOAT a = 1.0f);
	D2D1_COLOR_F COGUI_COLOR(INT r, INT g, INT  b, INT a = 255);

	BOOL       CreateElement(UINT elementID, LPCWSTR lpElementName, FLOAT x, FLOAT y, FLOAT nWidth, FLOAT nHeight, UINT uiParams, INT ID);
	BOOL       Render();
	BOOL       IsVisible(INT elementID, bool* lpBool);
	BOOL       ShowElement(INT elementID, bool bVisible);
	INT        GetElementID(COGUIHANDLE hElement);
	BOOL       AddTableColumn(INT elementID, LPWCH pwchName, INT strLength);
	BOOL       AddTableString(INT elementID, LPWCH pwchName, INT strLength);
	BOOL       AddDropListString(INT elementID, LPWCH pStr, INT strLength);
	HWND       GetWindow();
	D2D1_SIZE_F GetWindowSize();
	INT        GetActiveElementID();
	BOOL       SetActiveElementID(INT eID);
	BOOL       SetElementText(INT elementID, const wchar_t* cBuf, UINT cbSize);
	INT        GetTableSelectedLine(INT elementID);
	BOOL       DeleteTableLine(INT elementID, INT nLine);
	BOOL       ClearDropList(INT elementID);
}
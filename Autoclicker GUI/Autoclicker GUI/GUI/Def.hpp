#pragma once

#include <Windows.h>

typedef BYTE* COGUIHANDLE;

typedef BOOL(*pfnCOGUIWndProc)
(
	UINT elementID,
	UINT message,
	void* pStruct
	);

struct COGUI_DropListReceive
{
	std::wstring pStr;
};

struct COGUI_ElementCreateStruct
{
	UINT elementID;
	LPCWSTR lpElementName;
	FLOAT x;
	FLOAT y;
	FLOAT nWidth;
	FLOAT nHeight;
	UINT uiParams;
	INT ID;
};

enum COGUI_ElementType
{
	// Header
	COGUI_MinimizeButton = 1,
	COGUI_ScaleButton = 2,
	COGUI_CloseButton = 3,

	// Client area
	COGUI_Button = 4,
	COGUI_TextEdit = 5,
	COGUI_TextLabel = 6,
	COGUI_RecordButton = 7,
	COGUI_Table = 8,
	COGUI_DropList = 9,
};
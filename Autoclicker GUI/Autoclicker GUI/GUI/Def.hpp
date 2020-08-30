#pragma once

#include <Windows.h>

typedef BYTE* COGUIHANDLE;

typedef BOOL(*pfnCOGUIWndProc)
(
	UINT elementID,
	UINT message,
	void* pStruct
	);
#pragma once

#ifndef D3D9_COGUI_ELEMENT_DROPLIST_H
#define D3D9_COGUI_ELEMENT_DROPLIST_H

#include <Windows.h>
#include "../../Draw/Draw.hpp"
#include "../../CoGUI.hpp"
#include "../../IO/IO.hpp"
#include "../../Def.hpp"

class DropList : public Element
{
public:
	DropList();
	TextInformation textInfo;

	bool opened = false;
	DWORD activeLine;
	DWORD selectedLine;

	FLOAT fDropListHeight;
	FLOAT fLineHeight;

	vector<wstring> vecLineNames;

	BOOL Render();
	BOOL ApplyMessage(LPVOID lpCOGUIWndProc);
};

#endif

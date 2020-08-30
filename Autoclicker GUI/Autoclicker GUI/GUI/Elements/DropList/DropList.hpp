#pragma once

#ifndef D3D9_COGUI_ELEMENT_DROPLIST_H
#define D3D9_COGUI_ELEMENT_DROPLIST_H

#include <Windows.h>
#include "../../Draw/Draw.hpp"
#include "../../CoGUI.hpp"
#include "../../IO/IO.hpp"
#include "../../Def.hpp"
#include "../ID.hpp"

class DropList : public Element
{
public:
	DropList();
	TextInformation textInfo;

	bool opened = false;
	INT activeLine = 0;

	FLOAT dListHeight;
	FLOAT lHeight;

	LPWCH* pFileNames;
	LPINT lFileNames;
	UINT cFileNames;

	BOOL Render();
	BOOL ApplyMessage(LPVOID lpCOGUIWndProc);
};

#endif

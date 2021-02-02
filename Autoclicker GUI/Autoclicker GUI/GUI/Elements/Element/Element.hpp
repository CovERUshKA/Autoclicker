#pragma once

#include <Windows.h>
#include <string>

#include "../../Def.hpp"

class Element
{
public:
	// Type of the element( Button, TextEdit and etc. )
	INT type;
	// Visible element or not
	bool visible;
	// ID of the element
	INT ID;

	// Position of the lement
	FLOAT x, y;
	// Size of the element
	FLOAT width, height;

	// Additional parameters for the element
	UINT params = 0;

	// Name of the element
	std::wstring wchElementName;

	Element();

	virtual void Init(COGUI_ElementCreateStruct createStruct);

	virtual BOOL Render() = 0;
	virtual BOOL ApplyMessage(LPVOID lpCOGUIWndProc) = 0;
};
#pragma once

#include <Windows.h>
#include "../../Draw/Draw.hpp"
#include "../../CoGUI.hpp"
#include "../../IO/IO.hpp"
#include "../../Def.hpp"
#include "../ID.hpp"
#include "../../Elements.hpp"

class Button : public Element
{
public:
	Button();
	TextInformation textInfo;

	bool pressed;

	BOOL Render(COGUIHANDLE lpElement);
	BOOL ApplyMessage(LPVOID lpCOGUIWndProc);
};
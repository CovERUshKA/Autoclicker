#pragma once

#include <Windows.h>
#include "../../../Draw/Draw.hpp"
#include "../../../CoGUI.hpp"
#include "../../../IO/IO.hpp"
#include "../../../Def.hpp"
#include "../../ID.hpp"

class MinimizeButton : public Element
{
public:
	MinimizeButton();

	bool pressed;

	BOOL Render();
	BOOL ApplyMessage(LPVOID lpCOGUIWndProc);
};
#pragma once

#include <Windows.h>
#include "../../../Draw/Draw.hpp"
#include "../../../CoGUI.hpp"
#include "../../../IO/IO.hpp"
#include "../../../Def.hpp"
#include "../../ID.hpp"

class ScaleButton : public Element
{
public:
	ScaleButton();

	bool pressed;

	BOOL Render();
	BOOL ApplyMessage(LPVOID lpCOGUIWndProc);
};
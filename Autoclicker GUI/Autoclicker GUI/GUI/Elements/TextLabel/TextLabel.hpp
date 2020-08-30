#pragma once

#include <Windows.h>
#include "../../Draw/Draw.hpp"
#include "../../CoGUI.hpp"
#include "../../IO/IO.hpp"
#include "../../Def.hpp"
#include "../ID.hpp"

class TextLabel : public Element
{
public:
	TextLabel();
	TextInformation textInfo;

	BOOL Render(COGUIHANDLE lpElement);
	BOOL ApplyMessage(LPVOID lpCOGUIWndProc);
};
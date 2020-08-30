#pragma once

#include <Windows.h>
#include "../../Draw/Draw.hpp"
#include "../../CoGUI.hpp"
#include "../../IO/IO.hpp"
#include "../../Def.hpp"
#include "../ID.hpp"

class RecordButton : public Element
{
public:
	RecordButton();
	TextInformation textInfo;

	bool pressed = false;
	bool recording = false;

	BOOL Render(COGUIHANDLE lpElement);
	BOOL ApplyMessage(LPVOID lpCOGUIWndProc);
};
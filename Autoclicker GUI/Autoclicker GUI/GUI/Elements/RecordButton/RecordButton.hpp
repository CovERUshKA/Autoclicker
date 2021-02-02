#pragma once

#include <Windows.h>
#include "../../Draw/Draw.hpp"
#include "../../CoGUI.hpp"
#include "../../IO/IO.hpp"
#include "../../Def.hpp"

struct RecordButtonReceive
{
	CHAR key;

	LPWCH pStr;
	DWORD strLength;
};

class RecordButton : public Element
{
public:
	RecordButton();
	TextInformation textInfo;

	bool pressed = false;
	bool recording = false;

	BOOL Render() override;
	BOOL ApplyMessage(LPVOID lpCOGUIWndProc) override;
};
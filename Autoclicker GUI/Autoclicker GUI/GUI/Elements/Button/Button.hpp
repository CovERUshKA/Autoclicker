#pragma once

#include <Windows.h>
#include "../../Draw/Draw.hpp"
#include "../../CoGUI.hpp"
#include "../../IO/IO.hpp"
#include "../../Def.hpp"
#include "../../Elements.hpp"

class Button : public Element
{
public:
	Button();
	TextInformation textInfo;

	bool pressed;

	BOOL Render() override;
	BOOL ApplyMessage(LPVOID lpCOGUIWndProc) override;
};
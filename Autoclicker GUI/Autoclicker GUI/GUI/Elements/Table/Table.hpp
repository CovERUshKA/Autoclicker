#pragma once

#include <Windows.h>
#include "../../Draw/Draw.hpp"
#include "../../CoGUI.hpp"
#include "../../IO/IO.hpp"
#include "../../Def.hpp"

struct Column
{
	LPWCH wchName;

	LPWCH* pwchLinesName;
};

class Table : public Element
{
public:
	Table();
	TextInformation textInfo;

	vector<wstring> vecLineStrings;

	DWORD activeLine;
	float lHeight;
	INT selectedLine;

	BOOL Render() override;
	BOOL ApplyMessage(LPVOID COGUIWndProc) override;
};
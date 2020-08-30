#pragma once

#include <Windows.h>
#include "../../Draw/Draw.hpp"
#include "../../CoGUI.hpp"
#include "../../IO/IO.hpp"
#include "../../Def.hpp"
#include "../ID.hpp"

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

	LPWCH* pwchLineStrings;
	LPINT strLengths;

	INT activeLine;
	INT cLines;
	float lHeight;
	INT selectedLine;

	BOOL Render();
	BOOL ApplyMessage();
};
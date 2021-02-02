#pragma once

#include <Windows.h>
#include "../../Draw/Draw.hpp"
#include "../../CoGUI.hpp"
#include "../../IO/IO.hpp"
#include "../../Def.hpp"

class TextEdit : public Element
{
public:
	TextEdit();
	TextInformation textInfo;

	struct
	{
		LONGLONG lastTime = 0;
		bool visible = true;
		USHORT millisecondsTime = 1500;
		UINT Pos = 0;
	} cursor;

	struct
	{
		bool selecting = false;
		UINT startSymbolPos = 0;
		UINT endSymbolPos = 0;
	} selection;

	BOOL Render() override;
	BOOL ApplyMessage(LPVOID lpCOGUIWndProc) override;

private:
	DWORD GetNearestSymbol();

	void add_chr(UINT symbol, UINT posNumber, BOOL uppercase);
	void delete_chr(UINT number);
	void delete_selection();
};
#include "TextEdit.hpp"

#define FONT_NAME L"Consolas"
#define FONT_SIZE 15

// uPos - 0 equal to the first place
void TextEdit::add_chr(UINT character, UINT uPos, BOOL uppercase)
{
	if (uPos > wchElementName.length()
		|| wchElementName.length() == NULL)
		return;

	wchElementName.insert(wchElementName.begin() + uPos, character);

	cursor.Pos += 1;

	return;
}

// number - 1 equal to the first character
void TextEdit::delete_chr(UINT number)
{
	if (number == NULL
		|| number > wchElementName.length()
		|| wchElementName.length() == NULL)
		return;

	number -= 1;

	wchElementName.erase(wchElementName.begin() + number);

	cursor.Pos -= 1;

	return;
}

void TextEdit::delete_selection()
{
	if (wchElementName.length() == NULL)
		return;

	UINT start = selection.startSymbolPos > selection.endSymbolPos ? selection.endSymbolPos : selection.startSymbolPos;
	UINT end = selection.startSymbolPos < selection.endSymbolPos ? selection.endSymbolPos : selection.startSymbolPos;

	if (start == end)
		delete_chr(cursor.Pos);
	else
	{
		for (UINT i = start; i < end; i++)
			delete_chr(start + 1);

		cursor.Pos = start;

		selection.selecting = false;
		selection.startSymbolPos = cursor.Pos;
		selection.endSymbolPos = cursor.Pos;
	}

	return;
}

DWORD TextEdit::GetNearestSymbol()
{
	RECT rc = { 0, 0, 0, 0 };
	float fDistance = 0;
	float fNearestDistance = 999;
	UINT pSymbol = 0;
	Vector2D dVectors = { 0, 0 };
	Vector2D textSize = { 0, 0 };

	Vector2D curPos = { io.mousePos.x - x, io.mousePos.y - y };

	if (params == DT_LEFT)
	{
		for (UINT i = 0; i < wchElementName.length() + 1; i++)
		{
			ZeroMemory(&rc, sizeof(RECT));

			textSize = draw.GetTextSize(wchElementName.c_str(), i, FONT_NAME, FONT_SIZE);

			fDistance = curPos.x - textSize.x;
			if (fDistance < 0)
				fDistance = -fDistance;

			if (fNearestDistance > fDistance)
			{
				fNearestDistance = fDistance;
				pSymbol = i;
			}
		}
	}

	if (params & DT_CENTER)
	{

	}

	return pSymbol;
}

BOOL TextEdit::ApplyMessage(LPVOID COGUIWndProc)
{
	if (COGUIWndProc == NULL)
		return FALSE;

	BOOL capslock = (GetKeyState(VK_CAPITAL) & 1);
	BOOL shift = HIWORD(GetKeyState(VK_SHIFT));

	pfnCOGUIWndProc lpCOGUIWndProc = ReCa<pfnCOGUIWndProc>(COGUIWndProc);

	BOOL curInField = io.CursorInField({ x, y },
		{ x + width, y + height });

	bool uppercase;

	if (capslock && shift)
		uppercase = false;
	else if (capslock && !shift)
		uppercase = true;
	else if (!capslock && shift)
		uppercase = true;

	switch (io.keys[VK_LBUTTON][1])
	{
	case DOWN:
	{
		if (!curInField && COGUI::GetActiveElementID() == ID)
			COGUI::SetActiveElementID(-1);

		if (curInField)
		{
			if (COGUI::GetActiveElementID() != ID)
				COGUI::SetActiveElementID(ID);

			selection.selecting = true;
			selection.startSymbolPos = selection.endSymbolPos = cursor.Pos = GetNearestSymbol();
		}
	}
	break;
	case UP:
	{
		if (COGUI::GetActiveElementID() == ID)
		{
			selection.selecting = false;
			selection.endSymbolPos = cursor.Pos = GetNearestSymbol();
		}
	}
		break;
	default:
		break;
	}

	if (COGUI::GetActiveElementID() != ID)
		return TRUE;

	if (selection.selecting)
		selection.endSymbolPos = cursor.Pos = GetNearestSymbol();

	// Handle backspace input
	if (io.keys[VK_BACK][1] == InputStates::DOWN)
		delete_selection();
	
	// Handle ' ' character
	if (io.keys[VK_SPACE][1] == InputStates::DOWN)
		if (selection.startSymbolPos != selection.endSymbolPos)
		{
			delete_selection();
			add_chr(VK_SPACE, cursor.Pos, uppercase);
		}
		else
			add_chr(VK_SPACE, cursor.Pos, uppercase);

	// Handle '.' character
	if (io.keys[VK_OEM_PERIOD][1] == InputStates::DOWN)
		if (selection.startSymbolPos != selection.endSymbolPos)
		{
			delete_selection();
			add_chr('.', cursor.Pos, uppercase);
		}
		else
			add_chr('.', cursor.Pos, uppercase);

	for (UINT i = 0x30; i <= 0x39; i++)
		if (io.keys[i][1] == InputStates::DOWN)
		{
			if (selection.startSymbolPos != selection.endSymbolPos)
			{
				delete_selection();
				add_chr(i, cursor.Pos, uppercase);
			}
			else
				add_chr(i, cursor.Pos, uppercase);
		}

	for (UINT i = 0x41; i <= 0x5A; i++)
		if (io.keys[i][1] == InputStates::DOWN)
		{
			if (selection.startSymbolPos != selection.endSymbolPos)
			{
				delete_selection();
				add_chr(i, cursor.Pos, uppercase);
			}
			else
				add_chr(i, cursor.Pos, uppercase);
		}

	return TRUE;
}

BOOL TextEdit::Render()
{
	BOOL curInField = io.CursorInField({ x, y },
		{ x + width, y + height });

	draw.Rectangle({ x, y }, width, height, D2D1_COLOR_F(D2D1::ColorF(1.0f, 1.0f, 0.5f)));
	draw.RectangleBordered({ x, y }, width, height, 1, D2D1_COLOR_F(D2D1::ColorF(0, 0, 0.5f)));

	if (selection.startSymbolPos != selection.endSymbolPos)
	{
		Vector2D startPos = draw.GetTextSize(wchElementName.c_str(), selection.startSymbolPos, FONT_NAME, FONT_SIZE);
		Vector2D endPos = draw.GetTextSize(wchElementName.c_str(), selection.endSymbolPos, FONT_NAME, FONT_SIZE);

		if (selection.startSymbolPos > selection.endSymbolPos)
			draw.Rectangle({ x + endPos.x, y }, startPos.x - endPos.x, height, COGUI::COGUI_COLOR(0, 0, 255, 50));
		else
			draw.Rectangle({ x + startPos.x, y }, endPos.x - startPos.x, height, COGUI::COGUI_COLOR(0, 0, 255, 50));
	}

	draw.String({ x, y }, textInfo, wchElementName.c_str(), wchElementName.length(), FONT_NAME, FONT_SIZE, { width, height });

	if (COGUI::GetActiveElementID() == ID)
	{
		Vector2D curPos = draw.GetTextSize(wchElementName.c_str(), cursor.Pos, FONT_NAME, FONT_SIZE);

		draw.Line({ x + curPos.x, y }, { x + curPos.x, y + height }, 1, COGUI::COGUI_COLOR(255, 0, 255, 255));
	}

	return TRUE;
}

TextEdit::TextEdit()
{
	type = COGUI_TextEdit;

	textInfo.clip = true;
	textInfo.color = COGUI::COGUI_COLOR(255, 255, 255);
	textInfo.multiline = false;
	textInfo.xAlign = 0;
	textInfo.yAlign = 0;
}
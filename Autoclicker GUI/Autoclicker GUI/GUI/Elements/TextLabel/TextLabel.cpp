#include "TextLabel.hpp"

#define FONT_NAME L"Consolas"
constexpr UINT FONT_SIZE = 15;

BOOL TextLabel::ApplyMessage(LPVOID COGUIWndProc)
{

	return TRUE;
}

BOOL TextLabel::Render(COGUIHANDLE lpElement)
{
	BOOL curInField = io.CursorInField({ x, y },
		{ x + width, y + height });

	draw.Rectangle({ x, y }, width, height, D2D1_COLOR_F(D2D1::ColorF(0.2f, 0.2f, 0.2f)));
	
	draw.String({ x, y }, textInfo, wchElementName.c_str(), wchElementName.length(), FONT_NAME, FONT_SIZE, { width, height });

	return TRUE;
}

TextLabel::TextLabel()
{
	elementID = COGUI_TextLabel;

	textInfo.clip = true;
	textInfo.color = COGUI::COGUI_COLOR(255, 255, 255);
	textInfo.multiline = false;
	textInfo.xAlign = 2;
	textInfo.yAlign = 2;
}
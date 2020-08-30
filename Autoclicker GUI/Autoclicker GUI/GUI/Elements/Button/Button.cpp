#include "Button.hpp"

#define FONT_NAME L"Consolas"
constexpr UINT FONT_SIZE = 15;

BOOL Button::ApplyMessage(LPVOID COGUIWndProc)
{
	pfnCOGUIWndProc lpCOGUIWndProc = ReCa<pfnCOGUIWndProc>(COGUIWndProc);

	BOOL curInField = io.CursorInField({ x, y },
		{ x + width, y + height });

	switch (io.keys[VK_LBUTTON][1])
	{
	case DOWN:
	{
		if (curInField)
			pressed = true;
	}
	break;
	case UP:
	{
		if (curInField
			&& pressed)
		{
			lpCOGUIWndProc(ID, NULL, NULL);
			pressed = false;
		}
		else
			pressed = false;
	}
	break;
	default:
		break;
	}

	return TRUE;
}

BOOL Button::Render(COGUIHANDLE lpElement)
{
	BOOL curInField = io.CursorInField({ x, y },
		{ x + width, y + height });

	if (!curInField)
		draw.Rectangle({ x, y }, width, height, COGUI::COGUI_COLOR(50, 50, 50, 200));
	else if (curInField && !pressed)
		draw.Rectangle({ x, y }, width, height, COGUI::COGUI_COLOR(50, 50, 50, 255));
	else if (curInField && pressed)
		draw.Rectangle({ x, y }, width, height, COGUI::COGUI_COLOR(70, 70, 70, 255));

	if (wchElementName.length())
	{
		draw.String({ x, y }, textInfo, wchElementName.c_str(), wchElementName.length(), FONT_NAME, FONT_SIZE, { width, height });
	}

	return TRUE;
}

Button::Button()
{
	elementID = COGUI_Button;

	pressed = false;

	textInfo.clip = true;
	textInfo.color = COGUI::COGUI_COLOR(255, 255, 255);
	textInfo.multiline = false;
	textInfo.xAlign = 2;
	textInfo.yAlign = 2;
}
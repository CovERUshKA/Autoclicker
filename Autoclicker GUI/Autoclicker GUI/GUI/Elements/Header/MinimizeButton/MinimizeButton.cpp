#include "MinimizeButton.hpp"

BOOL MinimizeButton::ApplyMessage(LPVOID COGUIWndProc)
{
	pfnCOGUIWndProc lpCOGUIWndProc = ReCa<pfnCOGUIWndProc>(COGUIWndProc);

	D2D1_SIZE_F size = COGUI::GetWindowSize();

	FLOAT fPosX = size.width - width * 3;
	FLOAT fPosY = 0;

	BOOL curInField = io.CursorInField({ fPosX, fPosY },
		{ fPosX + width, fPosY + height });

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
			ShowWindow(COGUI::GetWindow(), SW_MINIMIZE);
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

BOOL MinimizeButton::Render()
{
	D2D1_SIZE_F size = COGUI::GetWindowSize();

	FLOAT fPosX = size.width - width * 3;
	FLOAT fPosY = 0;

	BOOL curInField = io.CursorInField({ fPosX, fPosY },
		{ fPosX + width, fPosY + height });

	if (curInField && !pressed)
		draw.Rectangle({ fPosX, fPosY }, width, height, COGUI::COGUI_COLOR(50, 50, 50, 255));
	else if (curInField && pressed)
		draw.Rectangle({ fPosX, fPosY }, width, height, COGUI::COGUI_COLOR(60, 60, 60, 255));

	draw.Line({ fPosX + width / 2 - 5, height / 2 }, { fPosX + width / 2 + 5, height / 2 }, 1, COGUI::COGUI_COLOR(255, 255, 255, 170), true);

	return TRUE;
}

MinimizeButton::MinimizeButton()
{
	type = COGUI_MinimizeButton;

	pressed = false;

	width = 30;
	height = 23;
}
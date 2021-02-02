#include "CloseButton.hpp"

BOOL CloseButton::ApplyMessage(LPVOID COGUIWndProc)
{
	pfnCOGUIWndProc lpCOGUIWndProc = ReCa<pfnCOGUIWndProc>(COGUIWndProc);

	D2D1_SIZE_F size = COGUI::GetWindowSize();

	FLOAT fPosX = size.width - width;
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
			DestroyWindow(COGUI::GetWindow());
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

BOOL CloseButton::Render()
{
	D2D1_SIZE_F size = COGUI::GetWindowSize();

	FLOAT fPosX = size.width - width;
	FLOAT fPosY = 0;

	BOOL curInField = io.CursorInField({ fPosX, fPosY },
		{ fPosX + width, fPosY + height });

	if (curInField)
		draw.Rectangle({ fPosX, fPosY }, width, height, COGUI::COGUI_COLOR(200, 60, 60, 255));
	 
	draw.Line({ fPosX + width / 2 - 5, height / 2 - 5 }, { fPosX + width / 2 + 5, height / 2 + 5 }, 1, COGUI::COGUI_COLOR(255, 255, 255, 170), false);
	draw.Line({ fPosX + width / 2 + 5, height / 2 - 5 }, { fPosX + width / 2 - 5, height / 2 + 5 }, 1, COGUI::COGUI_COLOR(255, 255, 255, 170), false);

	return TRUE;
}

CloseButton::CloseButton()
{
	type = COGUI_CloseButton;

	pressed = false;

	width = 30;
	height = 25;
}
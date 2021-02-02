#include "ScaleButton.hpp"

BOOL ScaleButton::ApplyMessage(LPVOID COGUIWndProc)
{
	pfnCOGUIWndProc lpCOGUIWndProc = ReCa<pfnCOGUIWndProc>(COGUIWndProc);

	D2D1_SIZE_F size = COGUI::GetWindowSize();

	FLOAT fPosX = size.width - width * 2;
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
			HWND hWnd = COGUI::GetWindow();

			WINDOWPLACEMENT wndplacement;
			wndplacement.length = sizeof(WINDOWPLACEMENT);
			GetWindowPlacement(hWnd, &wndplacement);

			if (wndplacement.showCmd == SW_SHOWNORMAL)
				ShowWindow(hWnd, SW_SHOWMAXIMIZED);
			else if (wndplacement.showCmd == SW_SHOWMAXIMIZED)
				ShowWindow(hWnd, SW_RESTORE);

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

BOOL ScaleButton::Render()
{
	D2D1_SIZE_F size = COGUI::GetWindowSize();

	FLOAT fPosX = size.width - width * 2;
	FLOAT fPosY = 0;

	BOOL curInField = io.CursorInField({ fPosX, fPosY },
		{ fPosX + width, fPosY + height });

	if (curInField && !pressed)
		draw.Rectangle({ fPosX, fPosY }, width, height, COGUI::COGUI_COLOR(50, 50, 50, 255));
	else if (curInField && pressed)
		draw.Rectangle({ fPosX, fPosY }, width, height, COGUI::COGUI_COLOR(60, 60, 60, 255));

	draw.RectangleBordered({ fPosX + width / 2 - 5, height / 2 - 5 }, 10, 10, 1, COGUI::COGUI_COLOR(255, 255, 255, 170), true);

	return TRUE;
}

ScaleButton::ScaleButton()
{
	type = COGUI_ScaleButton;

	pressed = false;

	width = 30;
	height = 23;
}
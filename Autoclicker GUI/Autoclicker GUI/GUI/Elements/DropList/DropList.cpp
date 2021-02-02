#include "DropList.hpp"

#define FONT_NAME L"Consolas"
constexpr UINT FONT_SIZE = 15;

BOOL DropList::ApplyMessage(LPVOID COGUIWndProc)
{
	pfnCOGUIWndProc lpCOGUIWndProc = ReCa<pfnCOGUIWndProc>(COGUIWndProc);

	BOOL curInField = io.CursorInField({ x, y },
		{ x + width, y + height });

	BOOL curInFieldDropList = io.CursorInField({ x, y + height },
		{ x + width, y + height + fDropListHeight });

	COGUI_DropListReceive dlreceive;

	switch (io.keys[VK_LBUTTON][1])
	{
	case DOWN:
	{
		if (curInField
			&& !opened)
		{
			lpCOGUIWndProc(ID, COGUI_DL_PREOPEN, &dlreceive);

			activeLine = 0;
			opened = true;
			COGUI::SetActiveElementID(ID);
		}
		else if (curInField
			     && opened)
		{
			opened = false;
			COGUI::SetActiveElementID(-1);
			lpCOGUIWndProc(ID, COGUI_DL_POSTCLOSE, &dlreceive);
		}
		else if (curInFieldDropList
				&& opened)
		{
			for (UINT i = 0; i < fDropListHeight / fLineHeight; i++)
			{
				if (activeLine + i == vecLineNames.size())
					break;

				if (io.CursorInField({ x, y + height + (fLineHeight * i) },
					{ x + width, y + height + (fLineHeight * (i + 1)) }))
				{
					dlreceive.pStr = vecLineNames[i];
					selectedLine = i;
					lpCOGUIWndProc(ID, COGUI_DL_CHOOSE, &dlreceive);

					wchElementName = vecLineNames[i];
					
					opened = false;
					COGUI::SetActiveElementID(-1);
					lpCOGUIWndProc(ID, COGUI_DL_POSTCLOSE, &dlreceive);
					break;
				}
			}
		}
	}
	break;
	case UP:
	{
		if (curInField)
		{
			
		}
		else if (curInField)
		{
			
		}

		
	}
	break;
	default:
		break;
	}

	switch (io.mWheel)
	{
	case DOWN:
	{
		if (vecLineNames.size() - activeLine > 1)
		{
			activeLine += 1;
		}
	}
	break;
	case UP:
	{
		if (activeLine != 0)
			activeLine -= 1;
	}
	break;
	default:
		break;
	}

	return TRUE;
}

BOOL DropList::Render()
{
	BOOL curInField = io.CursorInField({ x, y },
		{ x + width, y + height });

	draw.Rectangle({ x, y }, width, height, COGUI::COGUI_COLOR(50, 50, 50));

	if (vecLineNames.size() != 0)
	{
		draw.String({ x, y }, textInfo, vecLineNames[selectedLine].c_str(), vecLineNames[selectedLine].length(), FONT_NAME, FONT_SIZE, { width, height });
	}

	if (!opened)
		return TRUE;

	draw.Rectangle({ x, y + height }, width, fDropListHeight, COGUI::COGUI_COLOR(50, 50, 50));

	draw.Line({ x + 1, y + height }, { x + 1, y + height + fDropListHeight }, 1, COGUI::COGUI_COLOR(80, 80, 80));
	draw.Line({ x + 1, y + height + fDropListHeight }, { x + width, y + height + fDropListHeight }, 1, COGUI::COGUI_COLOR(80, 80, 80));
	draw.Line({ x + width, y + height }, { x + width, y + height + fDropListHeight }, 1, COGUI::COGUI_COLOR(80, 80, 80));

	draw.ClipRectBegin({ x, y + height, x + width, y + height + fDropListHeight });

	for (UINT i = 0; i < fDropListHeight / fLineHeight; i++)
	{
		if (vecLineNames.size() == activeLine + i)
			break;

		draw.String({ x, y + height + (i * fLineHeight) }, textInfo, vecLineNames[activeLine + i].c_str(), vecLineNames[activeLine + i].length(), FONT_NAME, FONT_SIZE, { width, fLineHeight });
	}

	draw.ClipRectEnd();

	return TRUE;
}

DropList::DropList()
{
	type = COGUI_DropList;

	textInfo.clip = true;
	textInfo.color = COGUI::COGUI_COLOR(255, 255, 255);
	textInfo.multiline = false;
	textInfo.xAlign = 2;
	textInfo.yAlign = 2;

	fDropListHeight = 200;
	fLineHeight = 20;

	activeLine = 0;
	selectedLine = 0;
}
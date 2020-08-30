#include "DropList.hpp"

#define FONT_NAME L"Consolas"
constexpr UINT FONT_SIZE = 15;

BOOL DropList::ApplyMessage(LPVOID COGUIWndProc)
{
	pfnCOGUIWndProc lpCOGUIWndProc = ReCa<pfnCOGUIWndProc>(COGUIWndProc);

	BOOL curInField = io.CursorInField({ x, y },
		{ x + width, y + height });

	BOOL curInFieldDropList = io.CursorInField({ x, y + height },
		{ x + width, y + height + dListHeight });

	DropListReceive dlreceive;

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
			for (UINT i = 0; i < dListHeight / lHeight; i++)
			{
				if (activeLine + i == cFileNames)
					break;

				if (io.CursorInField({ x, y + height + (lHeight * i) },
					{ x + width, y + height + (lHeight * (i + 1)) }))
				{
					dlreceive.pStr = pFileNames[i];
					lpCOGUIWndProc(ID, COGUI_DL_CHOOSE, &dlreceive);

					wchElementName = pFileNames[i];
					strLength = lFileNames[i];
					
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
		if (cFileNames - activeLine > 1)
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

	draw.String({ x, y }, textInfo, wchElementName.c_str(), strLength, FONT_NAME, FONT_SIZE, { width, height });

	if (!opened)
		return TRUE;

	draw.Rectangle({ x, y + height }, width, dListHeight, COGUI::COGUI_COLOR(50, 50, 50));

	draw.Line({ x + 1, y + height }, { x + 1, y + height + dListHeight }, 1, COGUI::COGUI_COLOR(80, 80, 80));
	draw.Line({ x + 1, y + height + dListHeight }, { x + width, y + height + dListHeight }, 1, COGUI::COGUI_COLOR(80, 80, 80));
	draw.Line({ x + width, y + height }, { x + width, y + height + dListHeight }, 1, COGUI::COGUI_COLOR(80, 80, 80));

	draw.ClipRectBegin({ x, y + height, x + width, y + height + dListHeight });

	for (UINT i = 0; i < dListHeight / lHeight; i++)
	{
		if (cFileNames == activeLine + i)
			break;

		draw.String({ x, y + height + (i * lHeight) }, textInfo, pFileNames[activeLine + i], lFileNames[activeLine + i], FONT_NAME, FONT_SIZE, { width, lHeight });
	}

	draw.ClipRectEnd();

	return TRUE;
}

DropList::DropList()
{
	elementID = COGUI_DropList;

	dListHeight = 200;
	lHeight = 20;

	textInfo.clip = true;
	textInfo.color = COGUI::COGUI_COLOR(255, 255, 255);
	textInfo.multiline = false;
	textInfo.xAlign = 2;
	textInfo.yAlign = 2;
}
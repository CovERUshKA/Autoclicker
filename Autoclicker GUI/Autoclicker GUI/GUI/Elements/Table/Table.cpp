#include "Table.hpp"

#define FONT_NAME L"Consolas"
#define FONT_SIZE 15

BOOL Table::ApplyMessage()
{
	BOOL curInField = io.CursorInField({ x, y },
		{ x + width, y + height });

	switch (io.keys[VK_LBUTTON][1])
	{
	case DOWN:
	{
		if (curInField
			&& cLines != NULL)
		{
			for (INT i = 0; i < height / lHeight; i++)
			{
				if (io.CursorInField({ x, y + (i * lHeight) }, { x + width, y + ((i + 1) * lHeight) }))
				{
					INT selLine = activeLine + i;
					if (selLine > cLines)
						break;
					else
						selectedLine = selLine;
				}
			}
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
		if (cLines - activeLine > 1)
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

BOOL Table::Render()
{
	BOOL curInField = io.CursorInField({ x, y },
		{ x + width, y + height });

	HRESULT hr = S_OK;

	draw.ClipRectBegin({ x, y, x + width, y + height });

	draw.Rectangle({ x, y }, width, height, COGUI::COGUI_COLOR(50, 50, 50, 255));

	UINT strRendered = 0;
	for (INT i = 0; i < height / lHeight; i++)
	{
		if (activeLine + i + 1 > cLines)
			break;

		if (activeLine + i == selectedLine)
			draw.Rectangle({ x, y + (i * lHeight) }, width, lHeight, COGUI::COGUI_COLOR(0, 0, 255, 100));

		draw.String({ x, y + (i * lHeight) }, textInfo, pwchLineStrings[activeLine + i], strLengths[activeLine + i], FONT_NAME, FONT_SIZE, { width, lHeight });
		strRendered += 1;

		if (strRendered == height / lHeight)
			break;
	}

	draw.ClipRectEnd();

	return TRUE;
}

Table::Table()
{
	textInfo.clip = true;
	textInfo.color = COGUI::COGUI_COLOR(255, 255, 255);
	textInfo.multiline = false;
	textInfo.xAlign = 2;
	textInfo.yAlign = 2;

	activeLine = 0;
	cLines = 0;
	lHeight = 20;
	selectedLine = -1;
	elementID = COGUI_Table;
}
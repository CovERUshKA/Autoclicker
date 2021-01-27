#include "CoGUI.hpp"

pfnCOGUIWndProc lpCOGUIWndProc = NULL;

struct
{
	COGUIHANDLE *lpBuffer = 0;
	UINT count = 0;
	INT activeID = -1;
} elements;

D2D1_SIZE_F wndSize;
HWND hWnd;

D2D1_COLOR_F COGUI::COGUI_COLOR(INT r, INT g, INT  b, INT a)
{
	if (r > 255)
		r = 255;
	else if (r < 0)
		r = 0;

	if (g > 255)
		g = 255;
	else if (g < 0)
		g = 0;

	if (b > 255)
		b = 255;
	else if (b < 0)
		b = 0;

	if (a > 255)
		a = 255;
	else if (a < 0)
		a = 0;

	return D2D1::ColorF((float)r/255, (float)g/255, (float)b/255, (float)a/255);
}

D2D1_COLOR_F COGUI::COGUI_COLOR(FLOAT r, FLOAT g, FLOAT  b, FLOAT a)
{
	return D2D1::ColorF(r, g, b, a);
}

LPBYTE GetElement(INT ID)
{
	for (UINT i = 0; i < elements.count; i++)
	{
		if ((*ReCa<Element*>(elements.lpBuffer[i])).ID == ID)
			return elements.lpBuffer[i];
	}

	return FALSE;
}

INT COGUI::GetID(COGUIHANDLE hElement)
{
	return (*ReCa<Element*>(hElement)).ID;
}

INT COGUI::GetElementID(COGUIHANDLE hElement)
{
	return (*ReCa<Element*>(hElement)).elementID;
}

BOOL COGUI::AddTableColumn(INT elementID, LPWCH pwchName, INT strLength)
{
	LPBYTE element = GetElement(elementID);
	if (element == NULL
		|| strLength == NULL
		|| pwchName == NULL)
		return FALSE;

	Table* table = ReCa<Table*>(element);



	return TRUE;
}

BOOL COGUI::AddTableString(INT elementID, LPWCH pwchName, INT strLength)
{
	LPBYTE element = GetElement(elementID);
	if (element == NULL
		|| strLength == NULL
		|| pwchName == NULL)
		return FALSE;

	Table* table = ReCa<Table*>(element);

	if (table->elementID != COGUI_Table)
		return FALSE;

	table->cLines += 1;

	LPVOID buf = table->pwchLineStrings;
	LPVOID strLengthsBuf = table->strLengths;

	table->pwchLineStrings = (LPWCH*)memory.Alloc(sizeof(LPWCH) * table->cLines);
	table->strLengths = (LPINT)memory.Alloc(sizeof(INT) * table->cLines);

	memcpy_s(table->pwchLineStrings, sizeof(LPWCH) * (table->cLines - 1), buf, sizeof(LPWCH) * (table->cLines - 1));
	memcpy_s(table->strLengths, sizeof(INT) * (table->cLines - 1), strLengthsBuf, sizeof(INT) * (table->cLines - 1));

	table->strLengths[table->cLines - 1] = strLength;

	table->pwchLineStrings[table->cLines - 1] = (LPWCH)memory.Alloc(sizeof(WCHAR) * strLength);
	
	memcpy_s(table->pwchLineStrings[table->cLines - 1], strLength * sizeof(WCHAR), pwchName, strLength * sizeof(WCHAR));

	if (buf && memory.Validate(buf))
		memory.Free(buf);

	if (strLengthsBuf && memory.Validate(strLengthsBuf))
		memory.Free(strLengthsBuf);

	return TRUE;
}

INT COGUI::GetTableSelectedLine(INT elementID)
{
	LPBYTE element = GetElement(elementID);
	if (element == NULL)
		return FALSE;

	Table* table = ReCa<Table*>(element);

	return table->selectedLine;
}

INT COGUI::GetDropListSelectedLine(INT elementID)
{
	LPBYTE element = GetElement(elementID);
	if (element == NULL)
		return FALSE;

	DropList* droplist = ReCa<DropList*>(element);

	return droplist->selectedLine;
}

LPWCH COGUI::GetDropListSelectedLineName(INT elementID)
{
	LPBYTE element = GetElement(elementID);
	if (element == NULL)
		return FALSE;

	DropList* droplist = ReCa<DropList*>(element);

	return droplist->pLineNames[droplist->selectedLine];
}

BOOL COGUI::AddDropListString(INT elementID, LPWCH pStr, INT strLength)
{
	LPBYTE element = GetElement(elementID);
	if (element == NULL)
		return FALSE;

	DropList* droplist = ReCa<DropList*>(element);

	droplist->countLineNames += 1;

	LPWCH* bufpFileNames = droplist->pLineNames;
	LPINT buflFileNames = droplist->pLineNamesLength;

	droplist->pLineNames = (LPWCH*)memory.Alloc(sizeof(LPWCH) * droplist->countLineNames);
	droplist->pLineNamesLength = (LPINT)memory.Alloc(sizeof(INT) * droplist->countLineNames);

	for (UINT i = 0; i < droplist->countLineNames - 1; i++)
	{
		droplist->pLineNames[i] = bufpFileNames[i];
		droplist->pLineNamesLength[i] = buflFileNames[i];
	}

	droplist->pLineNames[droplist->countLineNames - 1] = (LPWCH)memory.Alloc((sizeof(WCHAR) + 1) * strLength);
	ZeroMemory(droplist->pLineNames[droplist->countLineNames - 1], (sizeof(WCHAR) + 1) * strLength);
	memcpy_s(droplist->pLineNames[droplist->countLineNames - 1], strLength * 2, pStr, strLength * 2);

	droplist->pLineNamesLength[droplist->countLineNames - 1] = strLength;

	return TRUE;
}

BOOL COGUI::ClearDropList(INT elementID)
{
	LPBYTE element = GetElement(elementID);
	if (element == NULL)
		return FALSE;

	DropList* droplist = ReCa<DropList*>(element);

	for (UINT i = 0; i < droplist->countLineNames; i++)
		memory.Free(droplist->pLineNames[i]);

	memory.Free(droplist->pLineNamesLength);
	memory.Free(droplist->pLineNames);

	droplist->countLineNames = NULL;

	return TRUE;
}

BOOL COGUI::DeleteTableLine(INT elementID, INT nLine)
{
	LPBYTE element = GetElement(elementID);
	if (element == NULL
		|| nLine == -1)
		return FALSE;

	Table* table = ReCa<Table*>(element);

	if (nLine > table->cLines - 1)
		return FALSE;

	table->cLines -= 1;
	LPWCH* buf = table->pwchLineStrings;
	LPINT strLengthsBuf = table->strLengths;
	table->pwchLineStrings = (LPWCH*)memory.Alloc(sizeof(LPWCH) * table->cLines);
	table->strLengths = (LPINT)memory.Alloc(sizeof(INT) * table->cLines);

	bool deleted = false;

	for (INT i = 0; i < table->cLines + 1; i++)
	{
		if (i == nLine)
		{
			memory.Free(buf[i]);
			deleted = true;
		}
		else if (deleted)
		{
			table->pwchLineStrings[i - 1] = buf[i];
			table->strLengths[i - 1] = strLengthsBuf[i];
		}
		else
		{
			table->pwchLineStrings[i] = buf[i];
			table->strLengths[i] = strLengthsBuf[i];
		}
	}
	memory.Free(strLengthsBuf);

	table->selectedLine = -1;

	return TRUE;
}

BOOL COGUI::ShowElement(INT elementID, bool bVisible)
{
	LPBYTE element = GetElement(elementID);
	if (element == NULL)
		return FALSE;

	bool* visible = ReCa<bool*>(element + sizeof(UINT));

	if (bVisible)
		*visible = true;
	else
		*visible = false;
	

	return TRUE;
}

BOOL COGUI::Render()
{
	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof(tme);
	tme.hwndTrack = hWnd;
	tme.dwFlags = TME_LEAVE;
	if (!TrackMouseEvent(&tme))
	{
		Log("Unable to track mouse event");
	}

	for (UINT i = 0; i < elements.count; i++)
	{
		bool visible = false;
		INT ID = GetID(elements.lpBuffer[i]);
		
		if (!IsVisible(ID, &visible))
			continue;

		if (elements.activeID != -1)
		{
			if (elements.activeID != ID)
				COGUI::Routine(COGUI_RENDER, (LPARAM)elements.lpBuffer[i]);
		}
		else
		{
			if (visible)
				COGUI::Routine(COGUI_RENDER, (LPARAM)elements.lpBuffer[i]);
		}
	}

	if (elements.activeID != -1)
	{
		bool visible = false;
		if (!IsVisible(elements.activeID, &visible))
			return TRUE;

		if (visible)
			COGUI::Routine(COGUI_RENDER, (LPARAM)GetElement(elements.activeID));
	}

	return TRUE;
}

BOOL COGUI::SetElementText(INT elementID, const wchar_t* cBuf, UINT cbSize)
{
	LPBYTE pElement = GetElement(elementID);
	if (pElement == NULL)
		return FALSE;

	Element cElement = *ReCa<Element*>(pElement);

	UINT eSize = 0;

	switch (cElement.elementID)
	{
	case COGUI_Button:
	{
		eSize = sizeof(Button);
	}
	break;
	case COGUI_TextEdit:
	{
		eSize = sizeof(TextEdit);
	}
	break;
	case COGUI_TextLabel:
	{
		eSize = sizeof(TextLabel);

		TextLabel* textLabel = ReCa<TextLabel*>(pElement);

		textLabel->wchElementName = cBuf;
	}
	break;
	default:
		break;
	}

	return TRUE;
}

BOOL COGUI::IsVisible(INT elementID, bool* lpBool)
{
	*lpBool = false;
	LPBYTE element = GetElement(elementID);
	if (element == NULL)
		return FALSE;

	bool visible = *ReCa<bool*>(element + sizeof(UINT));

	*lpBool = visible;

	return TRUE;
}

HWND COGUI::GetWindow()
{
	return hWnd;
}

D2D1_SIZE_F COGUI::GetWindowSize()
{
	return wndSize;
}

INT COGUI::GetActiveElementID()
{
	return elements.activeID;
}

BOOL COGUI::SetActiveElementID(INT eID)
{
	elements.activeID = eID;

	return (elements.activeID == eID);
}

void IO()
{
	if (lpCOGUIWndProc == NULL)
		return;

	if (elements.activeID != -1)
	{
		bool visible = false;

		if (!COGUI::IsVisible(elements.activeID, &visible))
			return;

		if (visible)
			COGUI::Routine(COGUI_INPUT, (LPARAM)GetElement(elements.activeID));

		return;
	}

	for (UINT i = 0; i < elements.count; i++)
	{
		bool visible = false;
		INT elementID = COGUI::GetID(elements.lpBuffer[i]);
		if (elementID == NULL)
			continue;

		if (!COGUI::IsVisible(elementID, &visible))
			continue;

		if (visible)
			COGUI::Routine(COGUI_INPUT, (LPARAM)elements.lpBuffer[i]);
	}
}

LRESULT COGUI::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_LBUTTONDOWN:
		io.keys[VK_LBUTTON][0] = 1;
		io.keys[VK_LBUTTON][1] = InputStates::DOWN;
		break;
	case WM_RBUTTONDOWN:
		io.keys[VK_RBUTTON][0] = 1;
		io.keys[VK_RBUTTON][1] = InputStates::DOWN;
		break;
	case WM_MBUTTONDOWN:
		io.keys[VK_MBUTTON][0] = 1;
		io.keys[VK_MBUTTON][1] = InputStates::DOWN;
		break;
	case WM_XBUTTONDOWN:
	{
		DWORD xButton = HIWORD(wParam) - 1;

		io.keys[VK_XBUTTON1 + xButton][0] = 1;
		io.keys[VK_XBUTTON1 + xButton][1] = InputStates::DOWN;
	}
		break;
	case WM_LBUTTONUP:
		io.keys[VK_LBUTTON][0] = 0;
		io.keys[VK_LBUTTON][1] = InputStates::UP;
		break;
	case WM_RBUTTONUP:
		io.keys[VK_RBUTTON][0] = 0;
		io.keys[VK_RBUTTON][1] = InputStates::UP;
		break;
	case WM_MBUTTONUP:
		io.keys[VK_MBUTTON][0] = 0;
		io.keys[VK_MBUTTON][1] = InputStates::UP;
		break;
	case WM_XBUTTONUP:
	{
		DWORD xButton = HIWORD(wParam) - 1;

		io.keys[VK_XBUTTON1 + xButton][0] = 0;
		io.keys[VK_XBUTTON1 + xButton][1] = InputStates::UP;
	}
		break;
	case WM_KEYDOWN:
		io.keys[wParam][0] = 1;
		io.keys[wParam][1] = InputStates::DOWN;
		break;
	case WM_KEYUP:
		io.keys[wParam][0] = 0;
		io.keys[wParam][1] = InputStates::UP;
		break;
	case WM_MOUSEMOVE:
		io.mousePos.x = (FLOAT)GET_X_LPARAM(lParam);
		io.mousePos.y = (FLOAT)GET_Y_LPARAM(lParam);
		break;
	case WM_MOUSELEAVE:
		io.mousePos = { -1, -1 };
		break;
	case WM_MOUSEWHEEL:
	{
		short delta = HIWORD(wParam);

		if (delta > 0)
		{
			io.mWheel = InputStates::UP;
		}
		else if (delta < 0)
		{
			io.mWheel = InputStates::DOWN;
		}
	}
	break;
	case WM_SIZE:
		wndSize.width = LOWORD(lParam);
		wndSize.height = HIWORD(lParam);
		break;
	default:
		return NULL;
	}

	IO();

	for (UINT i = 0; i < MAX_PATH; i++)
	{
		io.keys[i][1] = NULL;
	}

	io.mWheel = 0;

	return NULL;
}

BOOL AddElement(void* pElement, UINT elementSize)
{
	HRESULT result = FALSE;
	BOOL bRet = FALSE;

	LPBYTE* lpBuffer = elements.lpBuffer;

	elements.lpBuffer = (LPBYTE*)memory.Alloc(sizeof(LPBYTE) * elements.count + 1);
	if (elements.lpBuffer == NULL)
	{
		Log("Unable to memory.Alloc");
		
		goto end;
	}
	
	for (UINT i = 0; i < elements.count; i++)
		elements.lpBuffer[i] = lpBuffer[i];

	elements.lpBuffer[elements.count] = (LPBYTE)memory.Alloc(sizeof(BYTE) * elementSize);
	if (elements.lpBuffer[elements.count] == NULL)
	{
		Log("Unable to memory.Alloc");

		goto end;
	}

	result = memcpy_s(elements.lpBuffer[elements.count], elementSize, pElement, elementSize);
	if (result != NULL)
	{
		Log("Unable to memcpy_s");

		goto end;
	}

	elements.count += 1;

	bRet = TRUE;

end:

	if (!bRet)
	{
		if (elements.lpBuffer)
			memory.Free(elements.lpBuffer);

		elements.lpBuffer = lpBuffer;
	}
	else
	{
		memory.Free(lpBuffer);
	}

	return bRet;
}

BOOL COGUI::CreateElement(UINT elementID, LPCWSTR lpElementName, FLOAT x, FLOAT y, FLOAT nWidth, FLOAT nHeight, UINT uiParams, INT ID)
{
	ElementCreateStruct structere = { elementID, lpElementName, x, y, nWidth, nHeight, uiParams, ID };

	if (ID != -1)
	{
		for (UINT i = 0; i < elements.count; i++)
		{
			Element* pHdr = ReCa<Element*>(elements.lpBuffer[i]);
			if (pHdr->ID == ID)
				return FALSE;
		}
	}

	return Routine(COGUI_CREATE, (LPARAM)&structere);
}

BOOL COGUI::Routine(DWORD dwMessageID, LPARAM lParam)
{
	BOOL bRet = FALSE;

	DWORD dwElementID;

	if (dwMessageID == COGUI_CREATE)
		dwElementID = ReCa<ElementCreateStruct*>(lParam)->elementID;
	else
		dwElementID = COGUI::GetElementID((COGUIHANDLE)lParam);

	switch (dwElementID)
	{
	case COGUI_Button:
	{
		if (dwMessageID == COGUI_CREATE)
		{
			ElementCreateStruct createStruct = *ReCa<ElementCreateStruct*>(lParam);

			Button button;
			button.ID = createStruct.ID;
			button.x = createStruct.x;
			button.y = createStruct.y;
			button.width = createStruct.nWidth;
			button.height = createStruct.nHeight;
			button.params = createStruct.uiParams;
			button.wchElementName = createStruct.lpElementName;
			button.strLength = button.wchElementName.length();

			bRet = AddElement(&button, sizeof(button));
		}
		else
		{
			Button* pButton = ReCa<Button*>(lParam);

			if (dwMessageID == COGUI_RENDER)
			{
				pButton->Render((COGUIHANDLE)pButton);
			}
			else if (dwMessageID == COGUI_INPUT)
			{
				pButton->ApplyMessage(lpCOGUIWndProc);
			}
		}
	}
	break;
	case COGUI_TextEdit:
	{
		if (dwMessageID == COGUI_CREATE)
		{
			ElementCreateStruct createStruct = *ReCa<ElementCreateStruct*>(lParam);

			TextEdit textedit;
			textedit.ID = createStruct.ID;
			textedit.x = createStruct.x;
			textedit.y = createStruct.y;
			textedit.width = createStruct.nWidth;
			textedit.height = createStruct.nHeight;
			textedit.params = createStruct.uiParams;
			textedit.wchElementName = createStruct.lpElementName;
			textedit.strLength = textedit.wchElementName.length();

			bRet = AddElement(&textedit, sizeof(textedit));
		}
		else
		{
			TextEdit* pTextEdit = ReCa<TextEdit*>(lParam);

			if (dwMessageID == COGUI_RENDER)
			{
				pTextEdit->Render((COGUIHANDLE)pTextEdit);
			}
			else if (dwMessageID == COGUI_INPUT)
			{
				pTextEdit->ApplyMessage((COGUIHANDLE)pTextEdit, lpCOGUIWndProc);
			}
		}
	}
	break;
	case COGUI_TextLabel:
	{
		if (dwMessageID == COGUI_CREATE)
		{
			ElementCreateStruct createStruct = *ReCa<ElementCreateStruct*>(lParam);

			TextLabel textlabel;
			textlabel.ID = createStruct.ID;
			textlabel.x = createStruct.x;
			textlabel.y = createStruct.y;
			textlabel.width = createStruct.nWidth;
			textlabel.height = createStruct.nHeight;
			textlabel.params = createStruct.uiParams;
			textlabel.wchElementName = createStruct.lpElementName;
			textlabel.strLength = textlabel.wchElementName.length();

			bRet = AddElement(&textlabel, sizeof(textlabel));
		}
		else
		{
			TextLabel* pTextLabel = ReCa<TextLabel*>(lParam);

			if (dwMessageID == COGUI_RENDER)
			{
				pTextLabel->Render((COGUIHANDLE)pTextLabel);
			}
			else if (dwMessageID == COGUI_INPUT)
			{
				pTextLabel->ApplyMessage(lpCOGUIWndProc);
			}
		}
	}
	break;
	case COGUI_RecordButton:
	{
		if (dwMessageID == COGUI_CREATE)
		{
			ElementCreateStruct createStruct = *ReCa<ElementCreateStruct*>(lParam);

			RecordButton recordbutton;
			recordbutton.ID = createStruct.ID;
			recordbutton.x = createStruct.x;
			recordbutton.y = createStruct.y;
			recordbutton.width = createStruct.nWidth;
			recordbutton.height = createStruct.nHeight;
			recordbutton.params = createStruct.uiParams;
			recordbutton.wchElementName = createStruct.lpElementName;
			recordbutton.strLength = recordbutton.wchElementName.length();

			bRet = AddElement(&recordbutton, sizeof(recordbutton));
		}
		else
		{
			RecordButton* pRecordButton = ReCa<RecordButton*>(lParam);

			if (dwMessageID == COGUI_RENDER)
			{
				pRecordButton->Render((COGUIHANDLE)pRecordButton);
			}
			else if (dwMessageID == COGUI_INPUT)
			{
				pRecordButton->ApplyMessage(lpCOGUIWndProc);
			}
		}
	}
	break;
	case COGUI_Table:
	{
		if (dwMessageID == COGUI_CREATE)
		{
			ElementCreateStruct createStruct = *ReCa<ElementCreateStruct*>(lParam);

			Table table;
			table.ID = createStruct.ID;
			table.x = createStruct.x;
			table.y = createStruct.y;
			table.width = createStruct.nWidth;
			table.height = createStruct.nHeight;
			table.params = createStruct.uiParams;
			table.wchElementName = createStruct.lpElementName;
			table.strLength = table.wchElementName.length();

			bRet = AddElement(&table, sizeof(table));
		}
		else
		{
			Table* pTable = ReCa<Table*>(lParam);

			if (dwMessageID == COGUI_RENDER)
			{
				pTable->Render();
			}
			else if (dwMessageID == COGUI_INPUT)
			{
				pTable->ApplyMessage();
			}
		}
	}
	break;
	case COGUI_DropList:
	{
		if (dwMessageID == COGUI_CREATE)
		{
			ElementCreateStruct createStruct = *ReCa<ElementCreateStruct*>(lParam);

			DropList droplist;
			droplist.ID = createStruct.ID;
			droplist.x = createStruct.x;
			droplist.y = createStruct.y;
			droplist.width = createStruct.nWidth;
			droplist.height = createStruct.nHeight;
			droplist.params = createStruct.uiParams;
			droplist.wchElementName = createStruct.lpElementName;
			droplist.strLength = droplist.wchElementName.length();

			bRet = AddElement(&droplist, sizeof(droplist));
		}
		else
		{
			DropList* pDropList = ReCa<DropList*>(lParam);

			if (dwMessageID == COGUI_RENDER)
			{
				pDropList->Render();
			}
			else if (dwMessageID == COGUI_INPUT)
			{
				pDropList->ApplyMessage(lpCOGUIWndProc);
			}
		}
	}
	break;
	case COGUI_MinimizeButton:
	{
		if (dwMessageID == COGUI_CREATE)
		{
			ElementCreateStruct createStruct = *ReCa<ElementCreateStruct*>(lParam);

			MinimizeButton minimizebutton;

			bRet = AddElement(&minimizebutton, sizeof(minimizebutton));
		}
		else
		{
			MinimizeButton* pMinimizeButton = ReCa<MinimizeButton*>(lParam);

			if (dwMessageID == COGUI_RENDER)
			{
				pMinimizeButton->Render();
			}
			else if (dwMessageID == COGUI_INPUT)
			{
				pMinimizeButton->ApplyMessage(lpCOGUIWndProc);
			}
		}
	}
	break;
	case COGUI_ScaleButton:
	{
		if (dwMessageID == COGUI_CREATE)
		{
			ElementCreateStruct createStruct = *ReCa<ElementCreateStruct*>(lParam);

			ScaleButton scalebutton;

			bRet = AddElement(&scalebutton, sizeof(scalebutton));
		}
		else
		{
			ScaleButton* pScaleButton = ReCa<ScaleButton*>(lParam);

			if (dwMessageID == COGUI_RENDER)
			{
				pScaleButton->Render();
			}
			else if (dwMessageID == COGUI_INPUT)
			{
				pScaleButton->ApplyMessage(lpCOGUIWndProc);
			}
		}
	}
	break;
	case COGUI_CloseButton:
	{
		if (dwMessageID == COGUI_CREATE)
		{
			ElementCreateStruct createStruct = *ReCa<ElementCreateStruct*>(lParam);

			CloseButton closebutton;

			bRet = AddElement(&closebutton, sizeof(closebutton));
		}
		else
		{
			CloseButton* pCloseButton = ReCa<CloseButton*>(lParam);

			if (dwMessageID == COGUI_RENDER)
			{
				pCloseButton->Render();
			}
			else if (dwMessageID == COGUI_INPUT)
			{
				pCloseButton->ApplyMessage(lpCOGUIWndProc);
			}
		}
	}
	break;
	default:
		break;
	}
}

BOOL COGUI::Init(HWND _hWnd, LPVOID lpfnCOGUIWndProc)
{
	if (lpfnCOGUIWndProc == NULL
		|| _hWnd == NULL)
		return FALSE;

	hWnd = _hWnd;
	lpCOGUIWndProc = ReCa<pfnCOGUIWndProc>(lpfnCOGUIWndProc);

	return TRUE;
}
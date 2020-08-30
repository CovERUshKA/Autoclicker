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

BOOL RenderElement(LPBYTE lpElement)
{
	Element* element = ReCa<Element*>(lpElement);

	switch (element->elementID)
	{
	case COGUI_Button:
	{
		Button button = *ReCa<Button*>(lpElement);

		button.Render(lpElement);
	}
		break;
	case COGUI_TextEdit:
	{
		TextEdit textedit = *ReCa<TextEdit*>(lpElement);

		textedit.Render(lpElement);
	}
	break;
	case COGUI_TextLabel:
	{
		TextLabel textlabel = *ReCa<TextLabel*>(lpElement);

		textlabel.Render(lpElement);
	}
	break;
	case COGUI_RecordButton:
	{
		RecordButton recordbutton = *ReCa<RecordButton*>(lpElement);

		recordbutton.Render(lpElement);
	}
	break;
	case COGUI_Table:
	{
		Table table = *ReCa<Table*>(lpElement);

		table.Render();
	}
	break;
	case COGUI_DropList:
	{
		DropList droplist = *ReCa<DropList*>(lpElement);

		droplist.Render();
	}
	break;
	case COGUI_MinimizeButton:
	{
		MinimizeButton minimizebutton = *ReCa<MinimizeButton*>(lpElement);

		minimizebutton.Render();
	}
	break;
	case COGUI_ScaleButton:
	{
		ScaleButton scalebutton = *ReCa<ScaleButton*>(lpElement);

		scalebutton.Render();
	}
	break;
	case COGUI_CloseButton:
	{
		CloseButton closebutton = *ReCa<CloseButton*>(lpElement);

		closebutton.Render();
	}
	break;
	default:
		break;
	}

	return TRUE;
}

LPBYTE GetElement(INT elementID)
{
	for (UINT i = 0; i < elements.count; i++)
	{
		if ((*ReCa<Element*>(elements.lpBuffer[i])).ID == elementID)
			return elements.lpBuffer[i];
	}

	return FALSE;
}

INT COGUI::GetElementID(COGUIHANDLE hElement)
{
	return (*ReCa<Element*>(hElement)).ID;
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

	table->cLines += 1;
	LPWCH* buf = table->pwchLineStrings;
	LPINT strLengthsBuf = table->strLengths;
	table->pwchLineStrings = new LPWCH[table->cLines];
	table->strLengths = new INT[table->cLines];

	memcpy_s(table->pwchLineStrings, sizeof(LPWCH) * (table->cLines - 1), buf, sizeof(LPWCH) * (table->cLines - 1));
	memcpy_s(table->strLengths, sizeof(LPINT) * (table->cLines - 1), strLengthsBuf, sizeof(LPINT) * (table->cLines - 1));

	table->strLengths[table->cLines - 1] = strLength;

	table->pwchLineStrings[table->cLines - 1] = new wchar_t[strLength];
	memcpy_s(table->pwchLineStrings[table->cLines - 1], strLength * 2, pwchName, strLength * 2);

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

BOOL COGUI::AddDropListString(INT elementID, LPWCH pStr, INT strLength)
{
	LPBYTE element = GetElement(elementID);
	if (element == NULL)
		return FALSE;

	DropList* droplist = ReCa<DropList*>(element);

	droplist->cFileNames += 1;

	LPWCH* bufpFileNames = droplist->pFileNames;
	LPINT buflFileNames = droplist->lFileNames;

	droplist->pFileNames = new LPWCH[droplist->cFileNames];
	droplist->lFileNames = new INT[droplist->cFileNames];

	for (UINT i = 0; i < droplist->cFileNames - 1; i++)
	{
		droplist->pFileNames[i] = bufpFileNames[i];
		droplist->lFileNames[i] = buflFileNames[i];
	}

	droplist->pFileNames[droplist->cFileNames - 1] = new WCHAR[strLength];
	memcpy_s(droplist->pFileNames[droplist->cFileNames - 1], strLength * 2, pStr, strLength * 2);

	droplist->lFileNames[droplist->cFileNames - 1] = strLength;

	return TRUE;
}

BOOL COGUI::ClearDropList(INT elementID)
{
	LPBYTE element = GetElement(elementID);
	if (element == NULL)
		return FALSE;

	DropList* droplist = ReCa<DropList*>(element);

	for (UINT i = 0; i < droplist->cFileNames; i++)
		delete[] droplist->pFileNames[i];

	delete[] droplist->lFileNames;
	delete[] droplist->pFileNames;

	droplist->cFileNames = NULL;

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
	table->pwchLineStrings = new LPWCH[table->cLines];
	table->strLengths = new INT[table->cLines];

	bool deleted = false;

	for (INT i = 0; i < table->cLines + 1; i++)
	{
		if (i == nLine)
		{
			delete[] buf[i];
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
	delete[] strLengthsBuf;

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
		INT ID = GetElementID(elements.lpBuffer[i]);
		
		if (!IsVisible(ID, &visible))
			continue;

		if (visible)
			RenderElement(elements.lpBuffer[i]);
	}

	if (elements.activeID != -1)
	{
		bool visible = false;
		if (!IsVisible(elements.activeID, &visible))
			return TRUE;

		if (visible)
			RenderElement(GetElement(elements.activeID));
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

void ApplyMessage(COGUIHANDLE hElement)
{
	BOOL bRet = FALSE;
	Element* pHdr = ReCa<Element*>(hElement);

	switch (pHdr->elementID)
	{
	case COGUI_Button:
	{
		Button* button = ReCa<Button*>(hElement);
		button->ApplyMessage(lpCOGUIWndProc);
	}
		break;
	case COGUI_TextEdit:
	{
		TextEdit* textedit = ReCa<TextEdit*>(hElement);

		textedit->ApplyMessage(hElement, lpCOGUIWndProc);
	}
	break;
	case COGUI_RecordButton:
	{
		RecordButton* recordbutton = ReCa<RecordButton*>(hElement);

		recordbutton->ApplyMessage(lpCOGUIWndProc);
	}
	break;
	case COGUI_Table:
	{
		Table* table = ReCa<Table*>(hElement);

		table->ApplyMessage();
	}
	break;
	case COGUI_DropList:
	{
		DropList* droplist = ReCa<DropList*>(hElement);

		droplist->ApplyMessage(lpCOGUIWndProc);
	}
	break;
	case COGUI_MinimizeButton:
	{
		MinimizeButton* minimizebutton = ReCa<MinimizeButton*>(hElement);

		minimizebutton->ApplyMessage(lpCOGUIWndProc);
	}
	break;
	case COGUI_ScaleButton:
	{
		ScaleButton* scalebutton = ReCa<ScaleButton*>(hElement);

		scalebutton->ApplyMessage(lpCOGUIWndProc);
	}
	break;
	case COGUI_CloseButton:
	{
		CloseButton* closebutton = ReCa<CloseButton*>(hElement);

		closebutton->ApplyMessage(lpCOGUIWndProc);
	}
	break;
	default:
		break;
	}

	return;
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
			ApplyMessage(GetElement(elements.activeID));

		return;
	}

	for (UINT i = 0; i < elements.count; i++)
	{
		bool visible = false;
		INT elementID = COGUI::GetElementID(elements.lpBuffer[i]);
		if (elementID == NULL)
			continue;

		if (!COGUI::IsVisible(elementID, &visible))
			continue;

		if (visible)
			ApplyMessage(elements.lpBuffer[i]);
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

	LPBYTE* lpBuffer = elements.lpBuffer;

	elements.lpBuffer = new LPBYTE[elements.count + 1];
	if (elements.lpBuffer == NULL)
	{
		Log("Unable to new");
		elements.lpBuffer = lpBuffer;
		return FALSE;
	}
	
	for (UINT i = 0; i < elements.count; i++)
		elements.lpBuffer[i] = lpBuffer[i];

	elements.lpBuffer[elements.count] = new BYTE[elementSize];
	if (elements.lpBuffer[elements.count] == NULL)
	{
		Log("Unable to new");
		elements.lpBuffer = lpBuffer;
		delete[] elements.lpBuffer;
		return FALSE;
	}

	result = memcpy_s(elements.lpBuffer[elements.count], elementSize, pElement, elementSize);
	if (result != NULL)
	{
		Log("Unable to memcpy_s");
		elements.lpBuffer = lpBuffer;
		delete[] elements.lpBuffer;
		return FALSE;
	}

	elements.count += 1;
	delete[] lpBuffer;

	return TRUE;
}

BOOL COGUI::CreateElement(UINT elementID, LPCWSTR lpElementName, FLOAT x, FLOAT y, FLOAT nWidth, FLOAT nHeight, UINT uiParams, INT ID)
{
	BOOL bRet = FALSE;

	if (ID != -1)
		for (UINT i = 0; i < elements.count; i++)
		{
			Element* pHdr = ReCa<Element*>(elements.lpBuffer[i]);
			if (pHdr->ID == ID)
				return FALSE;
		}

	switch (elementID)
	{
	case COGUI_Button:
	{
		Button button;
		button.ID = ID;
		button.x = x;
		button.y = y;
		button.width = nWidth;
		button.height = nHeight;
		button.params = uiParams;
		button.wchElementName = lpElementName;
		button.strLength = button.wchElementName.length();

		bRet = AddElement(&button, sizeof(button));
	}
	break;
	case COGUI_TextEdit:
	{
		TextEdit textedit;
		textedit.ID = ID;
		textedit.x = x;
		textedit.y = y;
		textedit.width = nWidth;
		textedit.height = nHeight;
		textedit.params = uiParams;
		textedit.wchElementName = lpElementName;
		textedit.strLength = textedit.wchElementName.length();

		bRet = AddElement(&textedit, sizeof(textedit));
	}
	break;
	case COGUI_TextLabel:
	{
		TextLabel textlabel;
		textlabel.ID = ID;
		textlabel.x = x;
		textlabel.y = y;
		textlabel.width = nWidth;
		textlabel.height = nHeight;
		textlabel.params = uiParams;
		textlabel.wchElementName = lpElementName;
		textlabel.strLength = textlabel.wchElementName.length();

		bRet = AddElement(&textlabel, sizeof(textlabel));
	}
	break;
	case COGUI_RecordButton:
	{
		RecordButton recordbutton;
		recordbutton.ID = ID;
		recordbutton.x = x;
		recordbutton.y = y;
		recordbutton.width = nWidth;
		recordbutton.height = nHeight;
		recordbutton.params = uiParams;
		recordbutton.wchElementName = lpElementName;
		recordbutton.strLength = recordbutton.wchElementName.length();

		bRet = AddElement(&recordbutton, sizeof(recordbutton));
	}
	break;
	case COGUI_Table:
	{
		Table table;
		table.ID = ID;
		table.x = x;
		table.y = y;
		table.width = nWidth;
		table.height = nHeight;
		table.params = uiParams;

		bRet = AddElement(&table, sizeof(table));
	}
	break;
	case COGUI_DropList:
	{
		DropList droplist;
		droplist.ID = ID;
		droplist.x = x;
		droplist.y = y;
		droplist.width = nWidth;
		droplist.height = nHeight;
		droplist.params = uiParams;
		droplist.wchElementName = lpElementName;
		droplist.strLength = droplist.wchElementName.length();

		bRet = AddElement(&droplist, sizeof(droplist));
	}
	break;
	case COGUI_MinimizeButton:
	{
		MinimizeButton minimizebutton;

		bRet = AddElement(&minimizebutton, sizeof(minimizebutton));
	}
	break;
	case COGUI_ScaleButton:
	{
		ScaleButton scalebutton;

		bRet = AddElement(&scalebutton, sizeof(scalebutton));
	}
	break;
	case COGUI_CloseButton:
	{
		CloseButton closebutton;

		bRet = AddElement(&closebutton, sizeof(closebutton));
	}
	break;
	default:
		break;
	}

	return bRet;
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
#include "CoGUI.hpp"

pfnCOGUIWndProc lpCOGUIWndProc = NULL;

struct
{
	vector<Element*> lpBuffer;
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

Element* GetElement(INT ID)
{
	for (UINT i = 0; i < elements.lpBuffer.size(); i++)
	{
		if (elements.lpBuffer[i]->ID == ID)
			return elements.lpBuffer[i];
	}

	return FALSE;
}

template <class T>
T* GetElement(INT ID)
{
	for (UINT i = 0; i < elements.lpBuffer.size(); i++)
	{
		if (elements.lpBuffer[i]->ID == ID)
			return (T*)(elements.lpBuffer[i]);
	}

	return FALSE;
}

INT COGUI::GetID(Element* lpElement)
{
	return lpElement->ID;
}

INT COGUI::GetElementType(Element* lpElement)
{
	return lpElement->type;
}

/*BOOL COGUI::AddTableColumn(INT elementID, LPWCH pwchName, INT strLength)
{
	if (elementID == NULL
		|| strLength == NULL
		|| pwchName == NULL)
		return FALSE;

	Table* lpTable = GetElement<Table>(elementID);
	if (lpTable == NULL)
		return FALSE;



	return TRUE;
}*/

BOOL COGUI::AddTableString(INT elementID, wstring wName)
{
	if (elementID == NULL
		|| wName.length() == NULL)
		return FALSE;

	Table* lpTable = GetElement<Table>(elementID);
	if (lpTable == NULL
		|| (lpTable->type != COGUI_Table))
		return FALSE;

	lpTable->vecLineStrings.push_back(wName);

	return TRUE;
}

INT COGUI::GetTableSelectedLine(INT elementID)
{
	Table* lpTable = GetElement<Table>(elementID);
	if (lpTable == NULL)
		return FALSE;

	return lpTable->selectedLine;
}

INT COGUI::GetDropListSelectedLine(INT elementID)
{
	DropList* lpDropList = GetElement<DropList>(elementID);
	if (lpDropList == NULL)
		return FALSE;

	return lpDropList->selectedLine;
}

wstring COGUI::GetDropListSelectedLineName(INT elementID)
{
	DropList* lpDropList = GetElement<DropList>(elementID);
	if (lpDropList == NULL)
		return wstring();

	return lpDropList->vecLineNames[lpDropList->selectedLine];
}

BOOL COGUI::AddDropListString(INT elementID, wstring wStr)
{
	DropList* lpDropList = GetElement<DropList>(elementID);
	if (lpDropList == NULL)
		return FALSE;

	lpDropList->vecLineNames.push_back(wStr);

	return TRUE;
}

BOOL COGUI::ClearDropList(INT elementID)
{
	DropList* lpDropList = GetElement<DropList>(elementID);
	if (lpDropList == NULL)
		return FALSE;

	lpDropList->vecLineNames.clear();

	return TRUE;
}

// nLine - 0 means first line
BOOL COGUI::DeleteTableLine(INT elementID, INT nLine)
{
	Table* lpTable = GetElement<Table>(elementID);
	if (lpTable == NULL
		|| nLine == -1
		|| nLine > lpTable->vecLineStrings.size() - 1)
		return FALSE;

	lpTable->vecLineStrings.erase(lpTable->vecLineStrings.begin() + nLine);

	lpTable->selectedLine = -1;

	return TRUE;
}

BOOL COGUI::ShowElement(INT elementID, bool bVisible)
{
	Element* lpElement = GetElement(elementID);
	if (lpElement == NULL)
		return FALSE;

	if (bVisible)
		lpElement->visible = true;
	else
		lpElement->visible = false;

	return TRUE;
}

BOOL COGUI::Render()
{
	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof(tme);
	tme.hwndTrack = hWnd;
	tme.dwFlags = TME_LEAVE;
	if (!TrackMouseEvent(&tme))
		Log("Unable to track mouse event");

	for (UINT i = 0; i < elements.lpBuffer.size(); i++)
	{
		bool visible = false;
		INT ID = GetID(elements.lpBuffer[i]);
		
		if (ID != elements.activeID
			&& IsVisible(ID, &visible)
			&& visible)
			COGUI::Routine(COGUI_RENDER, (LPARAM)elements.lpBuffer[i]);
	}

	if (elements.activeID != -1)
	{
		bool visible = false;
		if (IsVisible(elements.activeID, &visible)
			&& visible)
			COGUI::Routine(COGUI_RENDER, (LPARAM)GetElement(elements.activeID));
	}

	return TRUE;
}

BOOL COGUI::SetElementText(INT elementID, wstring wText)
{
	Element* lpElement = GetElement(elementID);
	if (lpElement == NULL)
		return FALSE;

	lpElement->wchElementName = wText;

	return TRUE;
}

// Returns window size
BOOL COGUI::IsVisible(INT elementID, bool* lpBool)
{
	if (!lpBool)
		return FALSE;

	*lpBool = false;

	Element* lpElement = GetElement(elementID);
	if (lpElement == NULL)
		return FALSE;

	*lpBool = lpElement->visible;

	return TRUE;
}

// Returns handle to the window
HWND COGUI::GetWindow()
{
	return hWnd;
}

// Returns window size
D2D1_SIZE_F COGUI::GetWindowSize()
{
	return wndSize;
}

// Returns active element ID
INT COGUI::GetActiveElementID()
{
	return elements.activeID;
}

// Set active element ID
BOOL COGUI::SetActiveElementID(INT eID)
{
	elements.activeID = eID;

	return (elements.activeID == eID);
}

// Handles input for COGUI
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

	for (UINT i = 0; i < elements.lpBuffer.size(); i++)
	{
		bool visible = false;
		INT elementID = COGUI::GetID(elements.lpBuffer[i]);
		if (elementID == NULL
			&& elementID != elements.activeID)
			continue;

		if (!COGUI::IsVisible(elementID, &visible))
			continue;

		if (visible)
			COGUI::Routine(COGUI_INPUT, (LPARAM)elements.lpBuffer[i]);
	}
}

// Processes window messages
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
		io.keys[i][1] = NULL;

	io.mWheel = 0;

	return NULL;
}

// Function to add element to the element array
BOOL AddElement(void* pElement, UINT elementSize)
{
	HRESULT result = FALSE;
	BOOL bRet = FALSE;

	try
	{
		elements.lpBuffer.push_back((Element*)memory.Alloc(sizeof(BYTE) * elementSize));

		result = memcpy_s(elements.lpBuffer[elements.lpBuffer.size() - 1], elementSize, pElement, elementSize);
		if (result != NULL)
			throw exception("Unable to memcpy_s");

		bRet = TRUE;
	}
	catch (const std::exception& e)
	{
		Log(e.what());
	}

	return bRet;
}

// Template function to create element and initialize it
template <class T>
BOOL AddElement(COGUI_ElementCreateStruct createStruct)
{
	T element;
	element.Init(createStruct);

	return AddElement(&element, sizeof(element));
}

BOOL COGUI::CreateElement(UINT elementID, LPCWSTR lpElementName, FLOAT x, FLOAT y, FLOAT nWidth, FLOAT nHeight, UINT uiParams, INT ID)
{
	COGUI_ElementCreateStruct structere = { elementID, lpElementName, x, y, nWidth, nHeight, uiParams, ID };

	if (ID != -1)
	{
		for (UINT i = 0; i < elements.lpBuffer.size(); i++)
		{
			Element* pHdr = ReCa<Element*>(elements.lpBuffer[i]);
			if (pHdr->ID == ID)
				return FALSE;
		}
	}
	
	return Routine(COGUI_CREATE, (LPARAM)&structere);
}

// Routine function, for element creation, rendering and input handling
BOOL COGUI::Routine(DWORD dwMessageID, LPARAM lParam)
{
	BOOL bRet = FALSE;

	DWORD dwElementID;

	if (dwMessageID == COGUI_CREATE)
		dwElementID = ReCa<COGUI_ElementCreateStruct*>(lParam)->elementID;
	else
	{
		Element* lpElement = ReCa<Element*>(lParam);

		switch (dwMessageID)
		{
		case COGUI_RENDER:
			lpElement->Render();
			break;
		case COGUI_INPUT:
			lpElement->ApplyMessage(lpCOGUIWndProc);
			break;
		default:
			break;
		}

		return TRUE;
	}

	COGUI_ElementCreateStruct createStruct = *ReCa<COGUI_ElementCreateStruct*>(lParam);

	switch (dwElementID)
	{
	case COGUI_Button:
		bRet = AddElement<Button>(createStruct);
	break;
	case COGUI_TextEdit:
		bRet = AddElement<TextEdit>(createStruct);
	break;
	case COGUI_TextLabel:
		bRet = AddElement<TextLabel>(createStruct);
	break;
	case COGUI_RecordButton:
		bRet = AddElement<RecordButton>(createStruct);
	break;
	case COGUI_Table:
		bRet = AddElement<Table>(createStruct);
	break;
	case COGUI_DropList:
		bRet = AddElement<DropList>(createStruct);
	break;
	case COGUI_MinimizeButton:
		bRet = AddElement<MinimizeButton>(createStruct);
	break;
	case COGUI_ScaleButton:
		bRet = AddElement<ScaleButton>(createStruct);
	break;
	case COGUI_CloseButton:
		bRet = AddElement<CloseButton>(createStruct);
	break;
	default:
		break;
	}

	return TRUE;
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
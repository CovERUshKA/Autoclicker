#include "RecordButton.hpp"

#define FONT_NAME L"Consolas"
constexpr UINT FONT_SIZE = 15;

int GetKeyName(unsigned char key, LPWCH pwchStr, UINT cchSize)
{
	ZeroMemory(pwchStr, cchSize * 2);

	const wchar_t* name = 0;

	switch (key)
	{
	case VK_LBUTTON:
		name = L"mouse1";
		break;
	case VK_RBUTTON:
		name = L"mouse2";
		break;
	case VK_MBUTTON:
		name = L"mouse3";
		break;
	case VK_XBUTTON1:
		name = L"mouse4";
		break;
	case VK_XBUTTON2:
		name = L"mouse5";
		break;
	default:
		break;
	}

	if (name != NULL)
	{
		memcpy_s(pwchStr, cchSize * 2, name, wcslen(name) * 2);
		return (int)wcslen(name);
	}

	UINT scanCode = MapVirtualKeyA(key, MAPVK_VK_TO_VSC);
	LONG lParamValue = (scanCode << 16);
	int length = GetKeyNameTextW(lParamValue, pwchStr, cchSize);

	return length;
}

BOOL RecordButton::ApplyMessage(LPVOID COGUIWndProc)
{
	pfnCOGUIWndProc lpCOGUIWndProc = ReCa<pfnCOGUIWndProc>(COGUIWndProc);

	BOOL curInField = io.CursorInField({ x, y },
		{ x + width, y + height });

	switch (io.keys[VK_LBUTTON][1])
	{
	case DOWN:
	{
		if (curInField)
		{
			pressed = true;
			return TRUE;
		}
	}
	break;
	case UP:
	{
		if (curInField
			&& pressed
			&& !recording)
		{
			recording = true;
		}
		else if (curInField
			     && pressed
			     && recording)
		{
			recording = false;
		}

		pressed = false;
	}
	break;
	default:
		break;
	}

	if (!recording)
		return TRUE;

	for (UINT i = 1; i < MAX_PATH; i++)
	{
		if (io.keys[i][1] == InputStates::DOWN)
		{
			wchar_t key[MAX_PATH];
			int length = GetKeyName(i, key, MAX_PATH);
			if (length == NULL)
				return TRUE;

			wchElementName = key;

			RecordButtonReceive rbreceive;
			rbreceive.key = i;
			rbreceive.pStr = wchElementName;

			lpCOGUIWndProc(ID, 0, &rbreceive);
			recording = false;
			return TRUE;
		}
	}

	return TRUE;
}

BOOL RecordButton::Render(COGUIHANDLE lpElement)
{
	BOOL curInField = io.CursorInField({ x, y },
		{ x + width, y + height });

	draw.Rectangle({ x, y }, width, height, COGUI::COGUI_COLOR(50, 50, 50));

	if (wchElementName.length() != NULL)
	{
		if (recording)
			draw.String({ x, y }, textInfo, L"..", wcslen(L".."), FONT_NAME, FONT_SIZE, { width, height });
		else
			draw.String({ x, y }, textInfo, wchElementName.c_str(), wchElementName.length(), FONT_NAME, FONT_SIZE, { width, height });
	}

	return TRUE;
}

RecordButton::RecordButton()
{
	elementID = COGUI_RecordButton;

	textInfo.clip = true;
	textInfo.color = COGUI::COGUI_COLOR(255, 255, 255);
	textInfo.multiline = false;
	textInfo.xAlign = 2;
	textInfo.yAlign = 2;
}
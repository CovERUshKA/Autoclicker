#include "Autoclicker.hpp"

BOOL IsMouseButton(CHAR vk)
{
	BOOL bRet = FALSE;

	switch (vk)
	{
	case VK_LBUTTON:
	case VK_RBUTTON:
	case VK_MBUTTON:
	case VK_XBUTTON1:
	case VK_XBUTTON2:
		bRet = TRUE;
		break;
	default:
		break;
	}

	return bRet;
}

BOOL IsKeyboardButton(CHAR vk)
{
	BOOL bRet = TRUE;

	switch (vk)
	{
	case VK_LBUTTON:
	case VK_RBUTTON:
	case VK_MBUTTON:
	case VK_XBUTTON1:
	case VK_XBUTTON2:
		bRet = FALSE;
		break;
	default:
		break;
	}

	return bRet;
}

int GetMouseButtonData(CHAR vk)
{
	switch (vk)
	{
	case VK_XBUTTON1:
		return XBUTTON1;
	case VK_XBUTTON2:
		return XBUTTON2;
	default:
		break;
	}

	return NULL;
}

// direction:
// 0 - UP
// 1 - DOWN
int GetMouseButtonFlag(CHAR vk, BOOL direction)
{
	if (!direction)
	{
		switch (vk)
		{
		case VK_LBUTTON:
			return MOUSEEVENTF_LEFTDOWN;
		case VK_RBUTTON:
			return MOUSEEVENTF_RIGHTDOWN;
		case VK_MBUTTON:
			return MOUSEEVENTF_MIDDLEDOWN;
		case VK_XBUTTON1:
		case VK_XBUTTON2:
			return MOUSEEVENTF_XDOWN;
		default:
			break;
		}
	}
	else
	{
		switch (vk)
		{
		case VK_LBUTTON:
			return MOUSEEVENTF_LEFTUP;
		case VK_RBUTTON:
			return MOUSEEVENTF_RIGHTUP;
		case VK_MBUTTON:
			return MOUSEEVENTF_MIDDLEUP;
		case VK_XBUTTON1:
		case VK_XBUTTON2:
			return MOUSEEVENTF_XUP;
		default:
			break;
		}
	}

	return NULL;
}

// direction:
// 0 - UP
// 1 - DOWN
int GetMouseButtonWindowMessage(CHAR vk, BOOL direction)
{
	if (!direction)
	{
		switch (vk)
		{
		case VK_LBUTTON:
			return WM_LBUTTONUP;
		case VK_RBUTTON:
			return WM_RBUTTONUP;
		case VK_MBUTTON:
			return WM_MBUTTONUP;
		case VK_XBUTTON1:
		case VK_XBUTTON2:
			return WM_XBUTTONUP;
		default:
			break;
		}
	}
	else
	{
		switch (vk)
		{
		case VK_LBUTTON:
			return WM_LBUTTONDOWN;
		case VK_RBUTTON:
			return WM_RBUTTONDOWN;
		case VK_MBUTTON:
			return WM_MBUTTONDOWN;
		case VK_XBUTTON1:
		case VK_XBUTTON2:
			return WM_XBUTTONDOWN;
		default:
			break;
		}
	}

	return NULL;
}

void PressMouseButton(CHAR vk, DWORD method)
{

	switch (method)
	{
	case CM_SendInput:
	{
		INPUT Input = { 0 };
		Input.type = INPUT_MOUSE;
		Input.mi.dwFlags = GetMouseButtonFlag(vk, 1);
		Input.mi.mouseData = GetMouseButtonData(vk);
		SendInput(1, &Input, sizeof(INPUT));
		ZeroMemory(&Input, sizeof(INPUT));
	}
	break;
	case CM_SendMessage:
	{
		HWND frg = GetForegroundWindow();
		DWORD pos = 0x00050005;

		SendMessageA(frg, GetMouseButtonWindowMessage(vk, 1), 0, pos);
	}
	break;
	default:
		break;
	}
}

void ReleaseMouseButton(CHAR vk, DWORD method)
{
	switch (method)
	{
	case CM_SendInput:
	{
		INPUT Input = { 0 };
		Input.type = INPUT_MOUSE;									// Let input know we are using the mouse.
		Input.mi.dwFlags = GetMouseButtonFlag(vk, 0);								// We are setting left mouse button up.
		Input.mi.mouseData = GetMouseButtonData(vk);
		SendInput(1, &Input, sizeof(INPUT));								// Send the input.
		ZeroMemory(&Input, sizeof(INPUT));									// Fills a block of memory with zeros.
	}
	break;
	case CM_SendMessage:
	{
		HWND frg = GetForegroundWindow();
		DWORD pos = 0x00050005;

		SendMessageA(frg, GetMouseButtonWindowMessage(vk, 0), 0, pos);
	}
	break;
	default:
		break;
	}
}

void set_bit(DWORD* num, DWORD number, DWORD position)
{
	int mask = number << position;
	*num = *num | mask;
}

void PressKey(CHAR vk, ClickMethod method)
{
	HWND frg = GetForegroundWindow();

	switch (method)
	{
	case CM_SendInput:
	{
		INPUT key = { NULL };

		key.type = INPUT_KEYBOARD;
		key.ki.wScan = MapVirtualKeyExA(vk, MAPVK_VK_TO_VSC, GetKeyboardLayout(GetWindowThreadProcessId(frg, 0)));
		key.ki.dwFlags = KEYEVENTF_SCANCODE;
		SendInput(1, &key, sizeof(INPUT));
	}
	break;
	case CM_SendMessage:
	{
		DWORD bits = 0x00000000;

		set_bit(&bits, 1, 0);
		set_bit(&bits, MapVirtualKey(vk, MAPVK_VK_TO_VSC), 16);
		set_bit(&bits, 0, 24);
		set_bit(&bits, 0, 29);
		set_bit(&bits, 0, 30);
		set_bit(&bits, 0, 31);

		SendMessageA(frg, WM_KEYDOWN, vk, bits);
	}
	break;
	default:
		break;
	}
}

void ReleaseKey(CHAR vk, ClickMethod method)
{
	HWND frg = GetForegroundWindow();

	switch (method)
	{
	case CM_SendInput:
	{
		INPUT key = { NULL };

		key.type = INPUT_KEYBOARD;
		key.ki.wScan = MapVirtualKeyExA(vk, MAPVK_VK_TO_VSC, GetKeyboardLayout(GetWindowThreadProcessId(frg, 0)));
		key.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
		SendInput(1, &key, sizeof(INPUT));
	}
	break;
	case CM_SendMessage:
	{
		DWORD bits = 0x00000000;

		set_bit(&bits, 1, 0);
		set_bit(&bits, MapVirtualKey(vk, MAPVK_VK_TO_VSC), 16);
		set_bit(&bits, 0, 24);
		set_bit(&bits, 0, 29);
		set_bit(&bits, 1, 30);
		set_bit(&bits, 1, 31);

		SendMessageA(frg, WM_KEYUP, vk, bits);
	}
	break;
	default:
		break;
	}
}

void SendCharKey(CHAR vk)
{
	HWND frg = GetForegroundWindow();
	DWORD bits = 0x00000000;

	set_bit(&bits, 1, 0);
	set_bit(&bits, MapVirtualKey(vk, MAPVK_VK_TO_VSC), 16);
	set_bit(&bits, 0, 24);
	set_bit(&bits, 1, 29);
	set_bit(&bits, 1, 30);
	set_bit(&bits, 1, 31);

	PostMessageA(frg, WM_CHAR, vk, bits);
}

BOOL Execute(ExecuteInfo executeInfo)
{
	if (executeInfo.key)
	{
		if (IsMouseButton(executeInfo.key))
		{
			PressMouseButton(executeInfo.key, executeInfo.clickMethod);
			Sleep(5);
			ReleaseMouseButton(executeInfo.key, executeInfo.clickMethod);
		}
		else
		{
			//PressKey(VK_MENU, CM_SendInput);
			PressKey(executeInfo.key, executeInfo.clickMethod);
			//SendCharKey(executeInfo.key);
			Sleep(5);
			ReleaseKey(executeInfo.key, executeInfo.clickMethod);
			//ReleaseKey(VK_MENU, CM_SendInput);
		}
	}
	else
	{

	}

	return TRUE;
}

DWORD WINAPI Work(LPVOID lpvoid)
{
	ExecuteInfo executeInfo = *(ExecuteInfo*)lpvoid;

	bool hold = false;
	bool toggle = false;

	while (true)
	{
		if (HIWORD(GetKeyState(executeInfo.sButton)))
		{
			switch (executeInfo.repeatMethod)
			{
			case CR_Once:
			{
				if (!hold)
					Execute(executeInfo);
			}
				break;
			case CR_Count:
			{
				if (!hold)
					for (DWORD i = 0; i < executeInfo.dwRepeat; i++)
						Execute(executeInfo);
			}
			break;
			default:
				break;
			}

			if (!hold)
				toggle = !toggle;

			hold = TRUE;
		}
		else
		{
			hold = FALSE;
		}

		switch (executeInfo.repeatMethod)
		{
		case CR_Toggle:
		{
			if (toggle)
				Execute(executeInfo);
		}
			break;
		case CR_WhileHolding:
		{
			if (hold)
				Execute(executeInfo);
		}
			break;
		default:
			break;
		}

		Sleep(5);
	}

	return NULL;
}

BOOL Autoclicker::Add(ExecuteInfo eInfo)
{
	for (DWORD i = 0; i < cActives; i++)
	{
		if (SuspendThread(hThreads[i]) == -1)
			Log("Unable to suspend thread");
	}

	ExecuteInfo* _lpExecuteInfo = lpExecuteInfo;
	
	if (hThreads
		&& memory.Validate(hThreads))
		memory.Free(hThreads);

	lpExecuteInfo = (ExecuteInfo*)memory.Alloc(sizeof(ExecuteInfo) * (cActives + 1));
	hThreads = (HANDLE*)memory.Alloc(sizeof(HANDLE) * (cActives + 1));

	memcpy_s(lpExecuteInfo, sizeof(ExecuteInfo) * cActives, _lpExecuteInfo, sizeof(ExecuteInfo) * cActives);

	lpExecuteInfo[cActives] = eInfo;

	if (_lpExecuteInfo
		&& memory.Validate(_lpExecuteInfo))
		memory.Free(_lpExecuteInfo);

	cActives += 1;

	for (DWORD i = 0; i < cActives; i++)
		hThreads[i] = CreateThread(0, 0, Work, &lpExecuteInfo[i], 0, 0);

	return NULL;
}

BOOL Autoclicker::Remove(DWORD number)
{
	for (DWORD i = 0; i < cActives; i++)
	{
		if (TerminateThread(hThreads[i], 0) == -1)
			Log("Unable to terminate thread");
	}

	ExecuteInfo* _lpExecuteInfo = lpExecuteInfo;

	if (hThreads
		&& memory.Validate(hThreads))
		memory.Free(hThreads);

	lpExecuteInfo = (ExecuteInfo*)memory.Alloc(sizeof(ExecuteInfo) * (cActives - 1));
	hThreads = (HANDLE*)memory.Alloc(sizeof(HANDLE) * (cActives - 1));

	memcpy_s(lpExecuteInfo, sizeof(ExecuteInfo) * number, _lpExecuteInfo, sizeof(ExecuteInfo) * number);

	memcpy_s(lpExecuteInfo + number, (cActives * sizeof(ExecuteInfo)) - (sizeof(ExecuteInfo) * (number + 1)), _lpExecuteInfo, (cActives * sizeof(ExecuteInfo)) - (sizeof(ExecuteInfo) * (number + 1)));

	if (_lpExecuteInfo
		&& memory.Validate(_lpExecuteInfo))
		memory.Free(_lpExecuteInfo);

	cActives -= 1;

	for (DWORD i = 0; i < cActives; i++)
		hThreads[i] = CreateThread(0, 0, Work, &lpExecuteInfo[i], 0, 0);

	return NULL;
}

Autoclicker::Autoclicker()
{
	cActives = 0;

	lpExecuteInfo = 0;
	hThreads = 0;
}

Autoclicker::~Autoclicker()
{
	
}
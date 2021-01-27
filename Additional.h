#pragma once

#pragma comment(lib, "pathcch.lib")

#ifndef ADDITIONAL_FUNCTIONS
#define ADDITIONAL_FUNCTIONS

#include <Windows.h>
#include <iostream>
#include <TlHelp32.h>
#include <pathcch.h>
#include <Psapi.h>

inline void PressMouseButton(DWORD dwFlag, DWORD mouseData)
{
	INPUT Input = { 0 };
	Input.type = INPUT_MOUSE;									// Let input know we are using the mouse.
	Input.mi.dwFlags = dwFlag;							// We are setting left mouse button down.
	Input.mi.mouseData = mouseData;
	SendInput(1, &Input, sizeof(INPUT));								// Send the input.
	ZeroMemory(&Input, sizeof(INPUT));									// Fills a block of memory with zeros.
}

inline void ReleaseMouseButton(DWORD dwFlag, DWORD mouseData)
{
	INPUT Input = { 0 };
	Input.type = INPUT_MOUSE;									// Let input know we are using the mouse.
	Input.mi.dwFlags = dwFlag;								// We are setting left mouse button up.
	Input.mi.mouseData = mouseData;
	SendInput(1, &Input, sizeof(INPUT));								// Send the input.
	ZeroMemory(&Input, sizeof(INPUT));									// Fills a block of memory with zeros.
}

inline void PressKey(CHAR vk)
{
	INPUT key = { NULL };

	key.type = INPUT_KEYBOARD;
	key.ki.wScan = MapVirtualKeyExA(vk, MAPVK_VK_TO_VSC, GetKeyboardLayout(0));
	key.ki.dwFlags = KEYEVENTF_SCANCODE;
	SendInput(1, &key, sizeof(INPUT));
}

inline void ReleaseKey(CHAR vk)
{
	INPUT key = { NULL };

	key.type = INPUT_KEYBOARD;
	key.ki.wScan = MapVirtualKeyExA(vk, MAPVK_VK_TO_VSC, GetKeyboardLayout(0));
	key.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
	SendInput(1, &key, sizeof(INPUT));
}

inline char* toString(unsigned int number)
{
	char buf[MAX_PATH];
	sprintf_s(buf, "%d", number);
	return buf;
}

#ifndef UNICODE

inline DWORD GetProcessIDByName(const char* name)
{
	PROCESSENTRY32 peProcessEntry;
	HANDLE CONST hSnapshot = CreateToolhelp32Snapshot(
		TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE) {
		return NULL;
	}

	peProcessEntry.dwSize = sizeof(PROCESSENTRY32);
	Process32First(hSnapshot, &peProcessEntry);
	do {
		if (strcmp(name, peProcessEntry.szExeFile) == NULL)
		{
			return peProcessEntry.th32ProcessID;
		}
	} while (Process32Next(hSnapshot, &peProcessEntry));

	CloseHandle(hSnapshot);
	return NULL;
}

#else

inline DWORD GetProcessIDByName(const wchar_t* name)
{
	PROCESSENTRY32 peProcessEntry = { NULL };
	HANDLE CONST hSnapshot = CreateToolhelp32Snapshot(
		TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE) {
		return NULL;
	}

	peProcessEntry.dwSize = sizeof(PROCESSENTRY32);
	Process32First(hSnapshot, &peProcessEntry);
	do {
		if (wcscmp(name, peProcessEntry.szExeFile) == NULL)
		{
			return peProcessEntry.th32ProcessID;
		}
	} while (Process32Next(hSnapshot, &peProcessEntry));

	CloseHandle(hSnapshot);
	return NULL;
}

#endif

inline void SetClipboardText(char* text)
{
	// Try opening the clipboard
	if (!OpenClipboard(nullptr))
		throw "Unable to open clipboard";

	HGLOBAL clipbuffer;
	char buffer[MAX_PATH];
	EmptyClipboard();

	clipbuffer = GlobalAlloc(GMEM_DDESHARE, strlen(text) + 1);
	if (!clipbuffer)
		return;

	LPCH prevData = (LPCH)GlobalLock(clipbuffer);
	if (!prevData)
		return;

	strcpy_s(buffer, MAX_PATH, prevData);
	strcpy_s(buffer, MAX_PATH, text);
	GlobalUnlock(clipbuffer);

	SetClipboardData(CF_TEXT, clipbuffer);

	// Release the clipboard
	CloseClipboard();

	return;
}

inline void GetClipboardText(char* buf, rsize_t _SizeInBytes)
{
	// Try opening the clipboard
	if (!OpenClipboard(nullptr))
		throw "Unable to open clipboard";

	  // Get handle of clipboard object for ANSI text
		HANDLE hData = GetClipboardData(CF_TEXT);
	if (hData == nullptr)
		throw "hData == nullptr";

	  // Lock the handle to get the actual text pointer
	char* pszText = static_cast<char*>(GlobalLock(hData));
	if (pszText == nullptr)
		throw "pszText == nullptr";

	if (strlen(pszText) > 0)
	{
		ZeroMemory(buf, sizeof(buf));
		strcpy_s(buf, _SizeInBytes, pszText);
	}

	// Release the lock
	GlobalUnlock(hData);

	// Release the clipboard
	CloseClipboard();

	return;
}

template <typename T = uintptr_t>
inline static auto findPattern(const char* module, std::string pattern, size_t offset = 0) noexcept
{
	MODULEINFO moduleInfo;

	if (GetModuleInformation(GetCurrentProcess(), GetModuleHandleA(module), &moduleInfo, sizeof(moduleInfo))) {
		char* begin = static_cast<char*>(moduleInfo.lpBaseOfDll);
		char* end = begin + moduleInfo.SizeOfImage - pattern.length() + 1;

		for (char* c = begin; c != end; c++) {
			bool matched = true;
			auto it = c;

			if (*(c + pattern.length() - 1) != pattern.back())
				continue;

			for (auto a : pattern) {
				if (a != '?' && *it != a) {
					matched = false;
					break;
				}
				it++;
			}
			if (matched)
				return reinterpret_cast<T>(c + offset);
		}
	}
	MessageBoxW(NULL, L"Failed to find pattern", L"Error", MB_OK | MB_ICONERROR);
	exit(EXIT_FAILURE);
}

inline BOOL GetCurDir(wchar_t* cBuf, DWORD size)
{
	ZeroMemory(cBuf, size);

	HMODULE hModule = NULL;
	if (!GetModuleHandleEx(
		GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
		(LPCTSTR)GetCurDir,
		&hModule))
		return FALSE;

	if (!K32GetModuleFileNameExW(GetCurrentProcess(), hModule, cBuf, size))
		return FALSE;

	if (FAILED(PathCchRemoveFileSpec(cBuf, size)))
		return FALSE;

	return TRUE;
}

#endif
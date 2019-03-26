// AutoclickerEXE.cpp : Определяет точку входа для приложения.
//
#pragma once
#include "stdafx.h"
#include "AutoclickerEXE.h"

#define _CRT_SECURE_NO_WARNINGS

#define MAX_LOADSTRING 100
#define ErrorMessageBox(hwnd, msg) MessageBoxA(hwnd, msg, "Autoclicker - Error", MB_OK | MB_TOPMOST | MB_ICONERROR);
#define CustomMessageBox(hwnd, title, msg) MessageBoxA(hwnd, msg, "Autoclicker - " title, MB_OK | MB_TOPMOST);
#define IDC_ADD_BUTTON 275
#define IDC_MILLLISECONDS_INPUT 276
#define IDC_SELECT_BUTTON 277
#define IDC_BUTTON_TO_INPUT 278
#define IDC_LIST_OF_INPUTS 279
#define IDC_CHOOSE_BUTTON 280
#define IDC_BUTTON_TO_START 281
#define IDC_DELETE_BUTTON 282
#define IDC_BUTTON_CHOOSE_START_BUTTON 283
#define IDC_MACROS_BUTTON 284
#define IDC_EDIT_RECORD_MACROS 285
#define IDC_ADD_MACROS_BUTTON 286
#define IDC_ADD_MACROS_TO_CLICK_BUTTON 287
#define IDC_DELETE_MACROS_BUTTON 288

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
char szTitle[MAX_PATH];                  // Текст строки заголовка
char szWindowClass[MAX_PATH];            // имя класса главного окна
WNDCLASSEXW wc = { 0 };
HWND Mainhwnd, ChooseButtonToClick, RecordMacrosButton, ButtonToInput, AddButton, ListOfInputs, ButtonToStartAutoclicker, DeleteButton, ButtonToChooseStartButton, EditOfMacros, AddMacrosButton, NameOfMacros, ListOfMacross, ChooseMacrosButton, DeleteMacrosButton;
bool hooldingclicks;
DWORD lastbuttontime = NULL;

// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int), IsKeyboardButton(int button);
int					GetdwFlagOfMouseButtonDown(int mousebutton), GetdwFlagOfMouseButtonUp(int mousebutton);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void				RegisterDialogClass(HWND hWnd);
DWORD WINAPI		Main(LPVOID lpParam);
void				ReleaseKey(UINT ch);
void				PressKey(UINT ch);
void				PressMouseButton(DWORD dwflag, DWORD moused);
void				ReleaseMouseButton(DWORD dwflag, DWORD moused);
UINT					GetXButton(char* button);
UINT				GetButton(char* button);
char*				GetMacros(char* nameofmacros);
BOOL				DoMacros(char* macros);

UINT GetXButton(char* button)
{
	if (strcmp(button, "mouse4"))
	{
		return XBUTTON1;
	}
	if (strcmp(button, "mouse5"))
	{
		return XBUTTON2;
	}

	return NULL;
}

BOOL DoMacros(char* macros)
{
	char *buffer[1000], *buffer2[1000], *next[1000], *next2[1000];
	*buffer = strtok_s(macros, "->", next);
	while (true)
	{
		if (*buffer)
		{
			if (atoi(*buffer))
			{
				Sleep(atoi(*buffer));
			}
			else
			{
				if (strchr(*buffer, '^'))
				{
					*buffer2 = strtok_s(*buffer, "^", next2);
					if (IsKeyboardButton(GetButton(*buffer)))
					{
						ReleaseKey(GetButton(*buffer));
					}
					else
					{
						ReleaseMouseButton(GetdwFlagOfMouseButtonUp(GetButton(*buffer)), GetXButton(*buffer));
					}
				}
				else
				{
					if (IsKeyboardButton(GetButton(*buffer)))
					{
						PressKey(GetButton(*buffer));
					}
					else
					{
						PressMouseButton(GetdwFlagOfMouseButtonDown(GetButton(*buffer)), GetXButton(*buffer));
					}
				}
			}
			*buffer = strtok_s(NULL, "->", next);
		}
		else
		{
			return TRUE;
		}
	}
	return FALSE;
}

char* GetMacros(char* nameofmacros)
{
	for (DWORD i = 0; i < (int)(SendMessageA(ListOfMacross, LB_GETCOUNT, NULL, NULL)); i++)
	{
		char buffer1[1000] = { NULL }, *buffer2[1000] = { NULL }, *next[1000] = { NULL };
		SendMessageA(ListOfMacross, LB_GETTEXT, i, (LPARAM)buffer1);

		*buffer2 = strtok_s(buffer1, " ", next);

		if (strcmp(nameofmacros, *buffer2) == NULL)
		{
			*buffer2 = strtok_s(NULL, " ", next);
			return *buffer2;
		}
	}
	return NULL;
}

int GetdwFlagOfMouseButtonDown(int mousebutton)
{
	if (mousebutton == VK_LBUTTON)
	{
		return MOUSEEVENTF_LEFTDOWN;
	}
	if (mousebutton == VK_RBUTTON)
	{
		return MOUSEEVENTF_RIGHTDOWN;
	}
	if (mousebutton == VK_MBUTTON)
	{
		return MOUSEEVENTF_MIDDLEDOWN;
	}
	if (mousebutton == VK_XBUTTON1)
	{
		return MOUSEEVENTF_XDOWN;
	}
	if (mousebutton == VK_XBUTTON2)
	{
		return MOUSEEVENTF_XDOWN;
	}
}

int GetdwFlagOfMouseButtonUp(int mousebutton)
{
	if (mousebutton == VK_LBUTTON)
	{
		return MOUSEEVENTF_LEFTUP;
	}
	if (mousebutton == VK_RBUTTON)
	{
		return MOUSEEVENTF_RIGHTUP;
	}
	if (mousebutton == VK_MBUTTON)
	{
		return MOUSEEVENTF_MIDDLEUP;
	}
	if (mousebutton == VK_XBUTTON1)
	{
		return MOUSEEVENTF_XUP;
	}
	if (mousebutton == VK_XBUTTON2)
	{
		return MOUSEEVENTF_XUP;
	}
}

BOOL IsKeyboardButton(int button)
{
	if (button != VK_LBUTTON && button != VK_RBUTTON && button != VK_MBUTTON && button != VK_XBUTTON1 && button != VK_XBUTTON2)
	{
		return 1;
	}

	return 0;
}

UINT GetButton(char* button)
{
	if (strstr((const char *)button, "mouse1"))
	{
		return VK_LBUTTON;
	}
	if (strstr((const char *)button, "mouse2"))
	{
		return VK_RBUTTON;
	}
	if (strstr((const char *)button, "mouse3"))
	{
		return VK_MBUTTON;
	}
	if (strstr((const char *)button, "Space"))
	{
		return VK_SPACE;
	}
	if (strstr((const char *)button, "mouse4"))
	{
		return VK_XBUTTON1;
	}
	if (strstr((const char *)button, "mouse5"))
	{
		return VK_XBUTTON2;
	}
	if (strstr((const char *)button, "Shift"))
	{
		return VK_SHIFT;
	}
	if (strstr((const char *)button, "Ctrl"))
	{
		return VK_CONTROL;
	}
	if (VkKeyScanA(tolower(button[0])))
	{
		return VkKeyScanA(tolower(button[0]));
	}

	return 0;
}

void PressMouseButton(DWORD dwflag, DWORD moused)
{
	INPUT Input = { 0 };
	Input.type = INPUT_MOUSE;									// Let input know we are using the mouse.
	Input.mi.dwFlags = dwflag;							// We are setting left mouse button down.
	Input.mi.mouseData = moused;
	SendInput(1, &Input, sizeof(INPUT));								// Send the input.
	ZeroMemory(&Input, sizeof(INPUT));									// Fills a block of memory with zeros.
}

void ReleaseMouseButton(DWORD dwflag, DWORD moused)
{
	INPUT Input = { 0 };
	Input.type = INPUT_MOUSE;									// Let input know we are using the mouse.
	Input.mi.dwFlags = dwflag;								// We are setting left mouse button up.
	Input.mi.mouseData = moused;
	SendInput(1, &Input, sizeof(INPUT));								// Send the input.
	ZeroMemory(&Input, sizeof(INPUT));									// Fills a block of memory with zeros.
}

void PressKey(UINT ch)
{
	INPUT key = { 0 };
	key.type = INPUT_KEYBOARD;
	key.ki.wScan = MapVirtualKeyExA(ch, MAPVK_VK_TO_VSC, GetKeyboardLayout(0));
	key.ki.dwFlags = KEYEVENTF_SCANCODE;
	SendInput(1, &key, sizeof(INPUT));
	ZeroMemory(&key, sizeof(INPUT));
}

void ReleaseKey(UINT ch)
{
	INPUT key = { 0 };
	key.type = INPUT_KEYBOARD;
	key.ki.wScan = MapVirtualKeyExA(ch, MAPVK_VK_TO_VSC, GetKeyboardLayout(0));
	key.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
	SendInput(1, &key, sizeof(INPUT));
	ZeroMemory(&key, sizeof(INPUT));
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	if (FindWindowA(NULL, "Autoclicker"))
	{
		ErrorMessageBox(NULL, "Autoclicker already running");
		return 0;
	}

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Разместите код здесь.

    // Инициализация глобальных строк
    LoadStringA(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringA(hInstance, IDC_AUTOCLICKEREXE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Выполнить инициализацию приложения:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_AUTOCLICKEREXE));

    MSG msg;

	CreateThread(NULL, NULL, &Main, GetModuleHandleA(NULL), NULL, NULL);

    // Цикл основного сообщения:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

DWORD WINAPI Main(LPVOID lpParam)
{
	char buffer[MAX_PATH];
	char * buttontoinput[MAX_PATH];
	char * buttontostart[MAX_PATH];
	char * next[1000];

	// Цикл основного сообщения:
	while (true)
	{
		for (int i = 0; i < (int)SendMessageA(ListOfInputs, LB_GETCOUNT, 0, (LPARAM)0); i++)
		{
			SendMessageA(ListOfInputs, LB_GETTEXT, i, (LPARAM)(LPSTR)buffer);
			*buttontostart = strtok_s(buffer, " ", next);
			*buttontoinput = strtok_s(NULL, " ", next);
			if (GetAsyncKeyState(GetButton(*buttontostart)) & 0x8000)
			{
				if (IsKeyboardButton(GetButton(*buttontoinput)))
				{
					if (GetMacros(*buttontoinput))
					{
						DoMacros(GetMacros(*buttontoinput));
					}
					else
					{
						PressKey(GetButton(*buttontoinput));
						ReleaseKey(GetButton(*buttontoinput));
					}
				}
				else
				{
					PressMouseButton(GetdwFlagOfMouseButtonDown(GetButton(*buttontoinput)), GetXButton(*buttontoinput));
					ReleaseMouseButton(GetdwFlagOfMouseButtonUp(GetButton(*buttontoinput)), GetXButton(*buttontoinput));
				}
			}
		}
		Sleep(5);
	}
	//some CPU relief
	Sleep(200);
	return 0;
}



//
//  ФУНКЦИЯ: MyRegisterClass()
//
//  ЦЕЛЬ: Регистрирует класс окна.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXA wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_AUTOCLICKEREXE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEA(IDC_AUTOCLICKEREXE);
    wcex.lpszClassName  = (const char *)szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExA(&wcex);
}

//
//   ФУНКЦИЯ: InitInstance(HINSTANCE, int)
//
//   ЦЕЛЬ: Сохраняет маркер экземпляра и создает главное окно
//
//   КОММЕНТАРИИ:
//
//        В этой функции маркер экземпляра сохраняется в глобальной переменной, а также
//        создается и выводится главное окно программы.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной

   Mainhwnd = CreateWindowA((const char *)szWindowClass, (const char *)szTitle, (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX) &~(WS_THICKFRAME),
	   CW_USEDEFAULT, 0, 800, 400, NULL, NULL, hInstance, NULL);

   if (!Mainhwnd)
   {
      return FALSE;
   }

   ShowWindow(Mainhwnd, nCmdShow);
   UpdateWindow(Mainhwnd);

   return TRUE;
}

//
//  ФУНКЦИЯ: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ЦЕЛЬ: Обрабатывает сообщения в главном окне.
//
//  WM_COMMAND  - обработать меню приложения
//  WM_PAINT    - Отрисовка главного окна
//  WM_DESTROY  - отправить сообщение о выходе и вернуться
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_LBUTTONDOWN:
	{
		if (SendMessageA(ChooseButtonToClick, BM_GETSTATE, 0, 0) == 4)
		{
			SetWindowTextA(ButtonToInput, "mouse1");
			SendMessageA(ChooseButtonToClick, BM_SETSTATE, 0, 0);
		}
		if (SendMessageA(ButtonToChooseStartButton, BM_GETSTATE, 0, 0) == 4)
		{
			SetWindowTextA(ButtonToStartAutoclicker, "mouse1");
			SendMessageA(ButtonToChooseStartButton, BM_SETSTATE, 0, 0);
		}
		if (IsDlgButtonChecked(Mainhwnd, IDC_MACROS_BUTTON))
		{
			char textofedit[1000];
			GetWindowTextA(EditOfMacros, textofedit, 1000);
			if (strlen(textofedit) == NULL)
			{
				lastbuttontime = NULL;
			}
			char text[1000], lasttick[1000];
			DWORD timeMid = GetCurrentTime() - lastbuttontime;
			sprintf_s(lasttick, "%d", timeMid);
			GetWindowTextA(EditOfMacros, text, 1000);
			if (lastbuttontime <= NULL)
			{
				strcat_s(text, "mouse1");
			}
			else
			{
				strcat_s(text, "->");
				strcat_s(text, lasttick);
				strcat_s(text, "->");
				strcat_s(text, "mouse1");
			}
			lastbuttontime = GetCurrentTime();
			SetWindowTextA(EditOfMacros, text);
		}
	}
	break;
	case WM_LBUTTONUP:
	{
		if (IsDlgButtonChecked(Mainhwnd, IDC_MACROS_BUTTON))
		{
			char textofedit[1000];
			GetWindowTextA(EditOfMacros, textofedit, 1000);
			if (strlen(textofedit) == NULL)
			{
				lastbuttontime = NULL;
			}
			char text[1000], lasttick[1000];
			DWORD timeMid = GetCurrentTime() - lastbuttontime;
			sprintf_s(lasttick, "%d", timeMid);
			GetWindowTextA(EditOfMacros, text, 1000);
			if (lastbuttontime <= NULL)
			{
				strcat_s(text, "mouse1");
			}
			else
			{
				strcat_s(text, "->");
				strcat_s(text, lasttick);
				strcat_s(text, "->");
				strcat_s(text, "mouse1");
				strcat_s(text, "^");
			}
			lastbuttontime = GetCurrentTime();
			SetWindowTextA(EditOfMacros, text);
		}
	}
	break;
	case WM_RBUTTONDOWN:
	{
		if (SendMessageA(ChooseButtonToClick, BM_GETSTATE, 0, 0) == 4)
		{
			SetWindowTextA(ButtonToInput, "mouse2");
			SendMessageA(ChooseButtonToClick, BM_SETSTATE, 0, 0);
		}
		if (SendMessageA(ButtonToChooseStartButton, BM_GETSTATE, 0, 0) == 4)
		{
			SetWindowTextA(ButtonToStartAutoclicker, "mouse2");
			SendMessageA(ButtonToChooseStartButton, BM_SETSTATE, 0, 0);
		}
		if (IsDlgButtonChecked(Mainhwnd, IDC_MACROS_BUTTON))
		{
			char textofedit[1000];
			GetWindowTextA(EditOfMacros, textofedit, 1000);
			if (strlen(textofedit) == NULL)
			{
				lastbuttontime = NULL;
			}
			char text[1000], lasttick[1000];
			DWORD timeMid = GetCurrentTime() - lastbuttontime;
			sprintf_s(lasttick, "%d", timeMid);
			GetWindowTextA(EditOfMacros, text, 1000);
			if (lastbuttontime <= NULL)
			{
				strcat_s(text, "mouse2");
			}
			else
			{
				strcat_s(text, "->");
				strcat_s(text, lasttick);
				strcat_s(text, "->");
				strcat_s(text, "mouse2");
			}
			lastbuttontime = GetCurrentTime();
			SetWindowTextA(EditOfMacros, text);
		}
	}
	break;
	case WM_RBUTTONUP:
	{
		if (IsDlgButtonChecked(Mainhwnd, IDC_MACROS_BUTTON))
		{
			char textofedit[1000];
			GetWindowTextA(EditOfMacros, textofedit, 1000);
			if (strlen(textofedit) == NULL)
			{
				lastbuttontime = NULL;
			}
			char text[1000], lasttick[1000];
			DWORD timeMid = GetCurrentTime() - lastbuttontime;
			sprintf_s(lasttick, "%d", timeMid);
			GetWindowTextA(EditOfMacros, text, 1000);
			if (lastbuttontime <= NULL)
			{
				strcat_s(text, "mouse2");
			}
			else
			{
				strcat_s(text, "->");
				strcat_s(text, lasttick);
				strcat_s(text, "->");
				strcat_s(text, "mouse2");
				strcat_s(text, "^");
			}
			lastbuttontime = GetCurrentTime();
			SetWindowTextA(EditOfMacros, text);
		}
	}
	break;
	case WM_MBUTTONDOWN:
	{
		if (SendMessageA(ChooseButtonToClick, BM_GETSTATE, 0, 0) == 4)
		{
			SetWindowTextA(ButtonToInput, "mouse3");
			SendMessageA(ChooseButtonToClick, BM_SETSTATE, 0, 0);
		}
		if (SendMessageA(ButtonToChooseStartButton, BM_GETSTATE, 0, 0) == 4)
		{
			SetWindowTextA(ButtonToStartAutoclicker, "mouse3");
			SendMessageA(ButtonToChooseStartButton, BM_SETSTATE, 0, 0);
		}
		if (IsDlgButtonChecked(Mainhwnd, IDC_MACROS_BUTTON))
		{
			char textofedit[1000];
			GetWindowTextA(EditOfMacros, textofedit, 1000);
			if (strlen(textofedit) == NULL)
			{
				lastbuttontime = NULL;
			}
			char text[1000], lasttick[1000];
			DWORD timeMid = GetCurrentTime() - lastbuttontime;
			sprintf_s(lasttick, "%d", timeMid);
			GetWindowTextA(EditOfMacros, text, 1000);
			if (lastbuttontime <= NULL)
			{
				strcat_s(text, "mouse3");
			}
			else
			{
				strcat_s(text, "->");
				strcat_s(text, lasttick);
				strcat_s(text, "->");
				strcat_s(text, "mouse3");
			}
			lastbuttontime = GetCurrentTime();
			SetWindowTextA(EditOfMacros, text);
		}
	}
	break;
	case WM_MBUTTONUP:
	{
		if (IsDlgButtonChecked(Mainhwnd, IDC_MACROS_BUTTON))
		{
			char textofedit[1000];
			GetWindowTextA(EditOfMacros, textofedit, 1000);
			if (strlen(textofedit) == NULL)
			{
				lastbuttontime = NULL;
			}
			char text[1000], lasttick[1000];
			DWORD timeMid = GetCurrentTime() - lastbuttontime;
			sprintf_s(lasttick, "%d", timeMid);
			GetWindowTextA(EditOfMacros, text, 1000);
			if (lastbuttontime <= NULL)
			{
				strcat_s(text, "mouse3");
			}
			else
			{
				strcat_s(text, "->");
				strcat_s(text, lasttick);
				strcat_s(text, "->");
				strcat_s(text, "mouse3");
				strcat_s(text, "^");
			}
			lastbuttontime = GetCurrentTime();
			SetWindowTextA(EditOfMacros, text);
		}
	}
	break;
	case WM_XBUTTONDOWN:
	{
		if (SendMessageA(ChooseButtonToClick, BM_GETSTATE, 0, 0) == 4)
		{
			switch (HIWORD(wParam))
			{
			case XBUTTON1:
				SetWindowTextA(ButtonToInput, "mouse4");
				SendMessageA(ChooseButtonToClick, BM_SETSTATE, 0, 0);
				break;
			case XBUTTON2:
				SetWindowTextA(ButtonToInput, "mouse5");
				SendMessageA(ChooseButtonToClick, BM_SETSTATE, 0, 0);
				break;
			default:
				break;
			}
		}
		if (SendMessageA(ButtonToChooseStartButton, BM_GETSTATE, 0, 0) == 4)
		{
			switch (HIWORD(wParam))
			{
			case XBUTTON1:
				SetWindowTextA(ButtonToStartAutoclicker, "mouse4");
				SendMessageA(ButtonToChooseStartButton, BM_SETSTATE, 0, 0);
				break;
			case XBUTTON2:
				SetWindowTextA(ButtonToStartAutoclicker, "mouse5");
				SendMessageA(ButtonToChooseStartButton, BM_SETSTATE, 0, 0);
				break;
			default:
				break;
			}
		}
		if (IsDlgButtonChecked(Mainhwnd, IDC_MACROS_BUTTON))
		{
			char textofedit[1000];
			GetWindowTextA(EditOfMacros, textofedit, 1000);
			if (strlen(textofedit) == NULL)
			{
				lastbuttontime = NULL;
			}
			char text[1000], lasttick[1000];
			DWORD timeMid = GetCurrentTime() - lastbuttontime;
			sprintf_s(lasttick, "%d", timeMid);
			GetWindowTextA(EditOfMacros, text, 1000);
			switch (HIWORD(wParam))
			{
			case XBUTTON1:
				if (lastbuttontime <= NULL)
				{
					strcat_s(text, "mouse4");
				}
				else
				{
					strcat_s(text, "->");
					strcat_s(text, lasttick);
					strcat_s(text, "->");
					strcat_s(text, "mouse4");
				}
				lastbuttontime = GetCurrentTime();
				SetWindowTextA(EditOfMacros, text);
				break;
			case XBUTTON2:
				if (lastbuttontime <= NULL)
				{
					strcat_s(text, "mouse5");
				}
				else
				{
					strcat_s(text, "->");
					strcat_s(text, lasttick);
					strcat_s(text, "->");
					strcat_s(text, "mouse5");
				}
				lastbuttontime = GetCurrentTime();
				SetWindowTextA(EditOfMacros, text);
				break;
			default:
				break;
			}
		}
	}
	break;
	case WM_XBUTTONUP:
	{
		if (IsDlgButtonChecked(Mainhwnd, IDC_MACROS_BUTTON))
		{
			char textofedit[1000];
			GetWindowTextA(EditOfMacros, textofedit, 1000);
			if (strlen(textofedit) == NULL)
			{
				lastbuttontime = NULL;
			}
			char text[1000], lasttick[1000];
			DWORD timeMid = GetCurrentTime() - lastbuttontime;
			sprintf_s(lasttick, "%d", timeMid);
			GetWindowTextA(EditOfMacros, text, 1000);
			switch (HIWORD(wParam))
			{
			case XBUTTON1:
				if (lastbuttontime <= NULL)
				{
					strcat_s(text, "mouse4");
				}
				else
				{
					strcat_s(text, "->");
					strcat_s(text, lasttick);
					strcat_s(text, "->");
					strcat_s(text, "mouse4");
					strcat_s(text, "^");
				}
				lastbuttontime = GetCurrentTime();
				SetWindowTextA(EditOfMacros, text);
				break;
			case XBUTTON2:
				if (lastbuttontime <= NULL)
				{
					strcat_s(text, "mouse5");
				}
				else
				{
					strcat_s(text, "->");
					strcat_s(text, lasttick);
					strcat_s(text, "->");
					strcat_s(text, "mouse5");
					strcat_s(text, "^");
				}
				lastbuttontime = GetCurrentTime();
				SetWindowTextA(EditOfMacros, text);
				break;
			default:
				break;
			}
		}
	}
	break;
	case WM_KEYDOWN:
	{
		if (SendMessageA(ButtonToChooseStartButton, BM_GETSTATE, 0, 0) == 4)
		{
			char key[MAX_PATH];
			UINT scanCode = MapVirtualKeyA(LOWORD(wParam), MAPVK_VK_TO_VSC);
			LONG lParamValue = (scanCode << 16);
			GetKeyNameTextA(lParamValue, key, MAX_PATH);
			SetWindowTextA(ButtonToStartAutoclicker, key);
			SendMessageA(ButtonToChooseStartButton, BM_SETSTATE, 0, 0);
		}
		if (SendMessageA(ChooseButtonToClick, BM_GETSTATE, 0, 0) == 4)
		{
			char key[MAX_PATH];
			UINT scanCode = MapVirtualKeyA(LOWORD(wParam), MAPVK_VK_TO_VSC);
			LONG lParamValue = (scanCode << 16);
			GetKeyNameTextA(lParamValue, key, MAX_PATH);
			SetWindowTextA(ButtonToInput, key);
			SendMessageA(ChooseButtonToClick, BM_SETSTATE, 0, 0);
		}
		if (IsDlgButtonChecked(Mainhwnd, IDC_MACROS_BUTTON))
		{
			char textofedit[1000];
			GetWindowTextA(EditOfMacros, textofedit, 1000);
			if (strlen(textofedit) == NULL)
			{
				lastbuttontime = NULL;
			}
			char key[MAX_PATH];
			UINT scanCode = MapVirtualKeyA(LOWORD(wParam), MAPVK_VK_TO_VSC);
			LONG lParamValue = (scanCode << 16);
			GetKeyNameTextA(lParamValue, key, MAX_PATH);
			char text[1000], lasttick[1000];
			DWORD timeMid = GetCurrentTime() - lastbuttontime;
			sprintf_s(lasttick, "%d", timeMid);
			GetWindowTextA(EditOfMacros, text, 1000);
			if (lastbuttontime <= NULL)
			{
				strcat_s(text, key);
			}
			else
			{
				strcat_s(text, "->");
				strcat_s(text, lasttick);
				strcat_s(text, "->");
				strcat_s(text, key);
			}
			lastbuttontime = GetCurrentTime();
			SetWindowTextA(EditOfMacros, text);
		}
	}
	break;
	case WM_KEYUP:
	{
		if (IsDlgButtonChecked(Mainhwnd, IDC_MACROS_BUTTON))
		{
			char textofedit[1000];
			GetWindowTextA(EditOfMacros, textofedit, 1000);
			if (strlen(textofedit) == NULL)
			{
				lastbuttontime = NULL;
			}
			char key[MAX_PATH];
			UINT scanCode = MapVirtualKeyA(LOWORD(wParam), MAPVK_VK_TO_VSC);
			LONG lParamValue = (scanCode << 16);
			GetKeyNameTextA(lParamValue, key, MAX_PATH);
			char text[1000], lasttick[1000];
			DWORD timeMid = GetCurrentTime() - lastbuttontime;
			sprintf_s(lasttick, "%d", timeMid);
			GetWindowTextA(EditOfMacros, text, 1000);
			if (lastbuttontime <= NULL)
			{
				strcat_s(text, key);
			}
			else
			{
				strcat_s(text, "->");
				strcat_s(text, lasttick);
				strcat_s(text, "->");
				strcat_s(text, key);
				strcat_s(text, "^");
			}
			lastbuttontime = GetCurrentTime();
			SetWindowTextA(EditOfMacros, text);
		}
	}
	break;
	case WM_CREATE:
		RegisterDialogClass(hWnd);
		break;
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Разобрать выбор в меню:
		switch (wmId)
		{
		case IDC_DELETE_MACROS_BUTTON:
			if (SendMessage(ListOfMacross, LB_GETCURSEL, 0, 0) >= NULL)
			{
				SendMessageA(ListOfMacross, LB_DELETESTRING, (WPARAM)SendMessage(ListOfMacross, LB_GETCURSEL, 0, (LPARAM)0), (LPARAM)0);
			}
			break;
		case IDC_ADD_MACROS_TO_CLICK_BUTTON:
		{
			char buffer[1000], *buffer2[1000], *next[1000];

			if (SendMessage(ListOfMacross, LB_GETCURSEL, 0, 0) >= NULL)
			{
				SendMessageA(ListOfMacross, LB_GETTEXT, (WPARAM)SendMessage(ListOfMacross, LB_GETCURSEL, 0, (LPARAM)0), (LPARAM)buffer);
				
				if (buffer)
				{
					*buffer2 = strtok_s(buffer, " ", next);

					SetWindowTextA(ButtonToInput, *buffer2);
				}
			}
		}
		break;
		case IDC_ADD_MACROS_BUTTON:
		{
			char buffer[1000], buffer2[1000];

			GetWindowTextA(EditOfMacros, buffer, 1000);
			GetWindowTextA(NameOfMacros, buffer2, 1000);

			strcat_s(buffer2, "   ");
			strcat_s(buffer2, buffer);

			SendMessageA(ListOfMacross, LB_ADDSTRING, NULL, (LPARAM)buffer2);
		}
		break;
		case IDC_MACROS_BUTTON:
		{
			if (!(SendMessageA(ChooseButtonToClick, BM_GETSTATE, 0, 0) == 4) && !(SendMessageA(ButtonToChooseStartButton, BM_GETSTATE, 0, 0) == 4))
			{
				if (IsDlgButtonChecked(Mainhwnd, IDC_MACROS_BUTTON))
				{
					CheckDlgButton(Mainhwnd, IDC_MACROS_BUTTON, 0);
				}
				else
				{
					CheckDlgButton(Mainhwnd, IDC_MACROS_BUTTON, 1);
				}
				SetFocus(Mainhwnd);
			}
		}
		break;
		case IDC_BUTTON_CHOOSE_START_BUTTON:
			if (!IsDlgButtonChecked(Mainhwnd, IDC_MACROS_BUTTON) && !(SendMessageA(ChooseButtonToClick, BM_GETSTATE, 0, 0) == 4))
			{
				SendMessageA(ButtonToChooseStartButton, BM_SETSTATE, BS_PUSHLIKE, 0);
				SetFocus(Mainhwnd);
			}
		break;
		case IDC_DELETE_BUTTON:
		{
			int cur = (int)SendMessageA(ListOfInputs, LB_GETCURSEL, NULL, NULL);
			SendMessageA(ListOfInputs, LB_DELETESTRING, (WPARAM)(int)SendMessageA(ListOfInputs, LB_GETCURSEL, NULL, NULL), 0);
		}
		break;
		case IDC_CHOOSE_BUTTON:
			if (!(SendMessageA(ButtonToChooseStartButton, BM_GETSTATE, 0, 0) == 4) && !IsDlgButtonChecked(Mainhwnd, IDC_MACROS_BUTTON))
			{
				SendMessageA(ChooseButtonToClick, BM_SETSTATE, BS_PUSHLIKE, 0);
				SetFocus(Mainhwnd);
			}
		break;
		case IDC_ADD_BUTTON:
		{
			char buffer1[MAX_PATH], buffer2[MAX_PATH];
			GetWindowTextA(ButtonToInput, buffer1, MAX_PATH);
			GetWindowTextA(ButtonToStartAutoclicker, buffer2, MAX_PATH);
			strcat_s(buffer2, sizeof(buffer2), " ");
			strcat_s(buffer2, sizeof(buffer2), buffer1);
			if (SendMessageA(ListOfInputs, LB_FINDSTRING, (WPARAM)0, (LPARAM)buffer2) == LB_ERR && strlen(buffer1) != NULL && strlen(buffer2) != NULL)
			{
				SendMessageA(ListOfInputs, LB_INSERTSTRING, (WPARAM)0, (LPARAM)buffer2);
			}
		}
		break;
		case IDM_ABOUT:
			CustomMessageBox(Mainhwnd, "Документация", "All symbols available to click/press:\n\r\n\rmouse1 - LMB(Left Mouse Button)\n\rmouse2 - RMB(Right Mouse Button)\n\rmouse3 - MMB(Middle Mouse Button)\n\rmouse4 - SMB1(Side Mouse Button 1)\n\rmouse5 - SMB2(Side Mouse Button 2)\n\rWhole modern latin alphabet(Aa - Zz)\n\rAll numbers(0 - 9)");
			break;
		break;
		case IDM_EXIT:
			PostQuitMessage(0);
			break;
		}
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_QUIT:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProcA(hWnd, message, wParam, lParam);
	}
	return 0;
}

void RegisterDialogClass(HWND hWnd)
{
	RECT rect;
	GetClientRect(hWnd, &rect);

	wc.cbSize = sizeof(WNDCLASSEXW);
	wc.lpfnWndProc = (WNDPROC)WndProc;
	wc.hInstance = hInst;
	wc.hbrBackground = (HBRUSH)1;
	wc.lpszClassName = L"DialogClass";
	RegisterClassExW(&wc);

	ButtonToStartAutoclicker = CreateWindowA("STATIC", "",
		WS_VISIBLE | WS_CHILD | WS_BORDER | ES_CENTER,
		20, 0, 95, 20, hWnd, (HMENU)IDC_BUTTON_TO_START, hInst, NULL);

	ButtonToChooseStartButton = CreateWindowA("BUTTON", "Choose",
		WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		120, 0, 60, 20, hWnd, (HMENU)IDC_BUTTON_CHOOSE_START_BUTTON, hInst, NULL);

	CreateWindowA("STATIC", "Button to start",
		WS_VISIBLE | WS_CHILD | ES_CENTER,
		185, 0, 95, 20, hWnd, (HMENU)0, hInst, NULL);

	ButtonToInput = CreateWindowA("STATIC", "",
		WS_VISIBLE | WS_CHILD | WS_BORDER | ES_CENTER,
		20, 25, 95, 20, hWnd, (HMENU)IDC_BUTTON_TO_INPUT, hInst, NULL);

	ChooseButtonToClick = CreateWindowA("BUTTON", "Choose",
		WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		120, 25, 60, 20, hWnd, (HMENU)IDC_CHOOSE_BUTTON, hInst, NULL);

	CreateWindowA("STATIC", "Button/Macros to click/execute",
		WS_VISIBLE | WS_CHILD | ES_CENTER,
		185, 25, 200, 20, hWnd, (HMENU)0, hInst, NULL);

	AddButton = CreateWindowA("BUTTON", "Add",
		WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		20, 50, 95, 20, hWnd, (HMENU)IDC_ADD_BUTTON, hInst, NULL);

	ListOfInputs = CreateWindowA("LISTBOX", "nothing",
		WS_VISIBLE | WS_CHILD | LBS_STANDARD | LBS_HASSTRINGS,
		20, 112, 200, 200, hWnd, (HMENU)IDC_LIST_OF_INPUTS, hInst, NULL);

	DeleteButton = CreateWindowA("BUTTON", "Delete",
		WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		225, 112, 95, 27, hWnd, (HMENU)IDC_DELETE_BUTTON, hInst, NULL);

	ListOfMacross = CreateWindowA("LISTBOX", NULL,
		WS_VISIBLE | WS_CHILD | LBS_STANDARD | LBS_HASSTRINGS | LBS_DISABLENOSCROLL | WS_HSCROLL | WS_VSCROLL,
		450, 1, 250, 175, hWnd, (HMENU)IDC_LIST_OF_INPUTS, hInst, NULL);
	SendMessageA(ListOfMacross, LB_SETHORIZONTALEXTENT, (WPARAM)5000, NULL);

	ChooseMacrosButton = CreateWindowA("BUTTON", "Choose",
		WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		705, 1, 60, 20, hWnd, (HMENU)IDC_ADD_MACROS_TO_CLICK_BUTTON, hInst, NULL);

	DeleteMacrosButton = CreateWindowA("BUTTON", "Delete",
		WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		705, 26, 60, 20, hWnd, (HMENU)IDC_DELETE_MACROS_BUTTON, hInst, NULL);

	EditOfMacros = CreateWindowA("EDIT", "",
		WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | WS_VSCROLL,
		450, 212, 200, 100, hWnd, (HMENU)IDC_EDIT_RECORD_MACROS, hInst, NULL);

	CreateWindowA("STATIC", "Name",
		WS_VISIBLE | WS_CHILD,
		460, 187, 40, 20, hWnd, (HMENU)0, hInst, NULL);

	NameOfMacros = CreateWindowA("EDIT", "",
		WS_VISIBLE | WS_CHILD | WS_BORDER | ES_CENTER,
		505, 187, 95, 20, hWnd, (HMENU)IDC_EDIT_RECORD_MACROS, hInst, NULL);

	CreateWindowA("STATIC", "Macros",
		WS_VISIBLE | WS_CHILD,
		395, 212, 50, 20, hWnd, (HMENU)0, hInst, NULL);

	RecordMacrosButton = CreateWindowA("BUTTON", "Record",
		WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
		656, 212, 70, 20, hWnd, (HMENU)IDC_MACROS_BUTTON, hInst, NULL);

	AddMacrosButton = CreateWindowA("BUTTON", "Add",
		WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		656, 237, 60, 20, hWnd, (HMENU)IDC_ADD_MACROS_BUTTON, hInst, NULL);

	HFONT hFont1 = CreateFontA(21, NULL, NULL, NULL, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
		CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, VARIABLE_PITCH, "Arial");

	SendMessage(AddButton,             // Handle of edit control
		WM_SETFONT,         // Message to change the font
		(WPARAM)hFont1,     // handle of the font
		MAKELPARAM(TRUE, 0) // Redraw text
	);
}
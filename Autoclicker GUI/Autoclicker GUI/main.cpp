#include "Header.hpp"

constexpr auto ID_CHOOSE_START_BUTTON = 1;
constexpr auto ID_CHOOSE_EXECUTE_BUTTON = 2;
constexpr auto ID_ADD_BUTTON = 3;
constexpr auto ID_ACTIVE_MACROSSES_TABLE = 4;
constexpr auto ID_DELETE_MACROS_BUTTON = 5;

D2D1_DRAW draw;
IO io;
IO overlayIo;

// Global Variables:
HINSTANCE hInst;                                // current instance
constexpr auto szTitle = L"Autoclicker";                  // The title bar text
constexpr auto szWindowClass = L"Autoclicker_exe_gui";                  // The title bar text
HWND mainhWnd;
bool minimized = false;
HKL keyboardLayout = NULL;
HHOOK hooklowLvlKeyboard;

FLOAT pos = 1.0f;
POINT curLast;

struct ExecuteInfo
{
	UINT key = 0;

	wstring wchMacrosName;
};

struct
{
	INT cActives;

	LPCH sButton;
	ExecuteInfo* eInfo;
} actives;

RecordButtonReceive startButton;

DropListReceive executeButton;

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

BOOL COGUIProc(UINT elementID, UINT message, void* pStruct);

BOOL AddActive(CHAR sButton, ExecuteInfo eInfo)
{
	LPCH _sButton = new CHAR[actives.cActives + 1];
	ExecuteInfo* _eInfo = new ExecuteInfo[actives.cActives + 1];

	memcpy_s(_sButton, sizeof(CHAR) * actives.cActives, actives.sButton, sizeof(CHAR) * actives.cActives);
	memcpy_s(_eInfo, sizeof(ExecuteInfo) * actives.cActives, actives.eInfo, sizeof(ExecuteInfo) * actives.cActives);

	_sButton[actives.cActives] = sButton;
	_eInfo[actives.cActives] = eInfo;

	LPCH __sButton = actives.sButton;
	ExecuteInfo* __eInfo = actives.eInfo;

	actives.sButton = _sButton;
	actives.eInfo = _eInfo;

	actives.cActives += 1;

	delete[] __sButton;
	delete[] __eInfo;

	return TRUE;
}

int FPS = 0;
int bufFPS = 0;
LONGLONG lastCheck = 0;

HRESULT Redraw()
{
	if (minimized)
	{
		Sleep(1);
		return NULL;
	}

	if (pos == 100)
	{
		pos = 1;
	}
	else
	{
		pos = 1;
	}

	HRESULT hr = draw.BeginDraw(0.17f, 0.17f, 0.17f);
	if (FAILED(hr))
		return hr;

	D2D1_SIZE_F wndSize = COGUI::GetWindowSize();

	draw.Rectangle({ 0, 0 }, wndSize.width, 22, COGUI::COGUI_COLOR(0.14f, 0.14f, 0.14f));

	COGUI::Render();

	LONGLONG curTime = GetCurrentTimeMilliseconds();

	if (curTime - lastCheck > 1000)
	{
		FPS = bufFPS;
		lastCheck = curTime;
		bufFPS = 0;
	}
	else
	{
		bufFPS += 1;
	}

	wstring buf;
	buf.clear();
	buf.append(to_wstring((DWORD)GetActiveWindow()).c_str());
	buf.append(L" ");
	buf.append(to_wstring((DWORD)GetForegroundWindow()).c_str());
	buf.append(L" ");
	buf.append(to_wstring((DWORD)GetFocus()).c_str());
	buf.append(L" ");
	buf.append(to_wstring((DWORD)GetCapture()).c_str());
	buf.append(L" ");
	buf.append(to_wstring((DWORD)GetTopWindow(NULL)).c_str());
	buf.append(L" ");
	buf.append(to_wstring(io.mousePos.x).c_str());
	buf.append(L" ");
	buf.append(to_wstring(io.mousePos.y).c_str());

	buf.append(L" ");
	buf.append(to_wstring(curLast.x).c_str());
	buf.append(L" ");
	buf.append(to_wstring(curLast.y).c_str());

	POINT pt;
	if (!GetCursorPos(&pt))
	{
		Log("Unable to GetCursorPos");
	}

	buf.append(L" ");
	buf.append(to_wstring(pt.x).c_str());
	buf.append(L" ");
	buf.append(to_wstring(pt.y).c_str());

	TextInformation textInfo;

	textInfo.clip = true;
	textInfo.color = COGUI::COGUI_COLOR(255, 255, 255);
	textInfo.multiline = false;
	textInfo.xAlign = 0;
	textInfo.yAlign = 0;
	
	draw.String({ pos, wndSize.height - 30 }, textInfo, buf.c_str(), buf.length(), L"Consolas", 13);
	//draw.String({ 1.0f, 20.0f }, { 1, 1, 1, 1 }, buf.c_str(), buf.length(), L"Consolas", 13, 0);

	hr = draw.EndDraw();
	if (FAILED(hr))
		return hr;

	return hr;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Place code here.
	Log("Autoclicker started");

	HANDLE hToken;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
	{
		Log("Unable to query process token");
		return FALSE;
	}

	TOKEN_ELEVATION tElevation = { 0 };
	DWORD dwLength;

	if (!GetTokenInformation(hToken, TokenElevation, &tElevation, sizeof(tElevation), &dwLength))
	{
		Log("Unable to get token information");
		CloseHandle(hToken);
		return FALSE;
	}

	if (!tElevation.TokenIsElevated)
	{
		Log("Not elevated");
		CloseHandle(hToken);
		return FALSE;
	}

	Log("Token is elevated");

	DWORD UIAccess;

	if (!GetTokenInformation(hToken, TokenUIAccess, &UIAccess, sizeof(UIAccess), &dwLength))
	{
		Log("Unable to get token information about UIAccess");
		CloseHandle(hToken);
		return FALSE;
	}

	if (!UIAccess)
		Log("No UIAccess");
	else
		Log("UIAccess granted");

	DWORD sID;

	if (!GetTokenInformation(hToken, TokenSessionId, &sID,
		sizeof(sID), &dwLength))
	{
		Log("GetTokenInformation Error");
		return FALSE;
	}
	if (!sID)
	{
		Log("0 Session ID error");
		CloseHandle(hToken);
		return FALSE;
	}

	Log("Not 0 session ID");

	CloseHandle(hToken);
	
	if (Overlay::AlreadyRunning())
	{
		Log("Autoclicker already started");
		return FALSE;
	}

	if (UIAccess && Overlay::Init(hInstance))
	{
		Log("Overlay initialized");
		return Overlay::Work();
	}

	if (FindWindowW(szWindowClass, NULL))
		return FALSE;

	if (!MyRegisterClass(hInstance))
	{
		Log("Unable to register window class");
		return FALSE;
	}

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
		return FALSE;
	
	if (FAILED(draw.Initialize(mainhWnd)))
	{
		Log("Unable to initialize draw class");
		return FALSE;
	}

	if (!COGUI::Init(mainhWnd, COGUIProc))
	{
		Log("Unable to initialize CoGUI");
		return FALSE;
	}

	COGUI::CreateElement(COGUI_MinimizeButton, L"", 0, 0, 0, 0, 0, -1);
	COGUI::CreateElement(COGUI_ScaleButton, L"", 0, 0, 0, 0, 0, -1);
	COGUI::CreateElement(COGUI_CloseButton, L"", 0, 0, 0, 0, 0, -1);
	
	COGUI::CreateElement(COGUI_RecordButton, L"Start", 25, 25, 100, 25, 0, ID_CHOOSE_START_BUTTON);
	COGUI::CreateElement(COGUI_DropList, L"Execute", 25, 60, 100, 25, 0, ID_CHOOSE_EXECUTE_BUTTON);
	COGUI::CreateElement(COGUI_Button, L"Add", 25, 110, 100, 25, 0, ID_ADD_BUTTON);
	COGUI::CreateElement(COGUI_Table, L"", 25, 145, 200, 130, 0, ID_ACTIVE_MACROSSES_TABLE);
	COGUI::CreateElement(COGUI_Button, L"Delete", 240, 145, 100, 25, 0, ID_DELETE_MACROS_BUTTON);

	keyboardLayout = LoadKeyboardLayoutA("00000400", KLF_SETFORPROCESS);

	Redraw();

	SetTimer(mainhWnd,
		1,
		1000 / 70,
		(TIMERPROC)NULL);

	RAWINPUTDEVICE Rid[2];

	Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
	Rid[0].usUsage = HID_USAGE_GENERIC_KEYBOARD;
	Rid[0].dwFlags = RIDEV_INPUTSINK;
	Rid[0].hwndTarget = mainhWnd;

	Rid[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
	Rid[1].usUsage = HID_USAGE_GENERIC_MOUSE;
	Rid[1].dwFlags = RIDEV_INPUTSINK;
	Rid[1].hwndTarget = mainhWnd;

	if (RegisterRawInputDevices(Rid, 2, sizeof(Rid[0])) == FALSE)
	{
		Log("Unable to register raw input devices.");
		return FALSE;
	}

	MSG msg;
	ZeroMemory(&msg, sizeof(msg));

	while (GetMessageA(&msg, nullptr, 0, 0))
	{
		DispatchMessage(&msg);
	}

	if (hooklowLvlKeyboard && !UnhookWindowsHookEx(hooklowLvlKeyboard))
	{
		Log("Unable to unhook Low Level Keyboard Hook");
		return EXIT_FAILURE;
	}

	return (int)msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = NULL;

	return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	mainhWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		300, 300, 500, 500, nullptr, nullptr, hInstance, nullptr);

	if (!mainhWnd)
	{
		Log("Unable to create window");
		return FALSE;
	}

	ShowWindow(mainhWnd, nCmdShow);
	UpdateWindow(mainhWnd);

	return TRUE;
}

BOOL COGUIProc(UINT elementID, UINT message, void* pStruct)
{
	switch (elementID)
	{
	case ID_CHOOSE_START_BUTTON:
	{
		RecordButtonReceive rbreceive = *ReCa<RecordButtonReceive*>(pStruct);
		startButton = rbreceive;
	}
		break;
	case ID_CHOOSE_EXECUTE_BUTTON:
	{
		DropListReceive dlreceive = *ReCa<DropListReceive*>(pStruct);
		switch (message)
		{
		case COGUI_DL_PREOPEN:
		{
			WIN32_FIND_DATAW data;
			HANDLE hFind;

			wchar_t lpCurDir[MAX_PATH];
			GetCurDir(lpCurDir, MAX_PATH);

			wstring path(lpCurDir);
			path.append(L"\\*");

			hFind = FindFirstFileW(path.c_str(), &data);

			if (hFind == INVALID_HANDLE_VALUE)
			{
				Log("Unable to find first file");
				return FALSE;
			}

			do {
				if (wcscmp(data.cFileName, L".") == NULL
					|| wcscmp(data.cFileName, L"..") == NULL)
					continue;

				wstring fileName = wstring(data.cFileName).substr(0, wcslen(data.cFileName) - 4).c_str();

				if (wcslen(data.cFileName) > 4 && wstring(data.cFileName).compare(wcslen(data.cFileName) - 4, 4, L".txt") == NULL)
					COGUI::AddDropListString(ID_CHOOSE_EXECUTE_BUTTON, (LPWCH)fileName.c_str(), fileName.length());

			} while (FindNextFileW(hFind, &data) != 0);
			FindClose(hFind);
		}
			break;
		case COGUI_DL_POSTCLOSE:
		{
			COGUI::ClearDropList(ID_CHOOSE_EXECUTE_BUTTON);
		}
			break;
		case COGUI_DL_CHOOSE:
		{
			executeButton = dlreceive;
		}
		break;
		default:
			break;
		}
	}
		break;
	case ID_ADD_BUTTON:
	{
		wstring buf;
		buf.append(startButton.pStr);
		buf.append(L" ");
		buf.append(executeButton.pStr);
		COGUI::AddTableString(ID_ACTIVE_MACROSSES_TABLE, (LPWCH)buf.c_str(), buf.length());

		AddActive(startButton.key, { NULL, executeButton.pStr });
	}
		break;
	case ID_DELETE_MACROS_BUTTON:
	{
		COGUI::DeleteTableLine(ID_ACTIVE_MACROSSES_TABLE, COGUI::GetTableSelectedLine(ID_ACTIVE_MACROSSES_TABLE));
	}
		break;
	default:
		break;
	}

	return FALSE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRet = 0;

	COGUI::WndProc(hWnd, message, wParam, lParam);

	switch (message)
	{
	case WM_INPUT:
	{
		RAWINPUT rw;
		UINT sz = sizeof(rw);

		if (!GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &rw, &sz, sizeof(RAWINPUTHEADER)))
			break;

		if (rw.header.dwType == RIM_TYPEMOUSE)
		{
			curLast.x = rw.data.mouse.lLastX;
			curLast.x = rw.data.mouse.lLastY;
		}

		if (rw.header.dwType == RIM_TYPEKEYBOARD)
		{
			switch (rw.data.keyboard.Message)
			{
			default:
				break;
			}
		}
	}
	break;
	case WM_CREATE:
	{
		if (FAILED(D2D1CreateFactory(
			D2D1_FACTORY_TYPE_SINGLE_THREADED,
			&draw.pFactory)))
			return -1;  // Fail CreateWindowInBandEx.
	}
	break;
	case WM_NCHITTEST:
	{
		POINT mousePos = { LOWORD(lParam), HIWORD(lParam) };

		ScreenToClient(hWnd, &mousePos);

		if (mousePos.y < 23
			&& mousePos.x < COGUI::GetWindowSize().width - 90)
			return HTCAPTION;
		else
			return HTCLIENT;
	}
		break;
	case WM_TIMER:
	{
		switch (wParam)
		{
		case 1:
			Redraw();
			break;
		default:
			break;
		}
	}
		break;
	case WM_ERASEBKGND:
		return 1;
	case WM_NCCALCSIZE:
	{
		if (wParam)
		{
			LPNCCALCSIZE_PARAMS ncalcsize = (LPNCCALCSIZE_PARAMS)lParam;
			NCCALCSIZE_PARAMS calcsizebuf;

			calcsizebuf.rgrc[0] = ncalcsize->rgrc[0];
			calcsizebuf.rgrc[1] = ncalcsize->rgrc[1];
			calcsizebuf.rgrc[2] = ncalcsize->rgrc[2];

			if (calcsizebuf.rgrc[0].left < 0)
			{
				calcsizebuf.rgrc[0].left = 0;
				calcsizebuf.rgrc[0].top = 0;
				calcsizebuf.rgrc[0].right -= 8;
				calcsizebuf.rgrc[0].bottom -= 8;
			}

			ncalcsize->rgrc[0] = calcsizebuf.rgrc[0];
			ncalcsize->rgrc[1] = calcsizebuf.rgrc[0];
			ncalcsize->rgrc[2] = calcsizebuf.rgrc[0];

			lRet = (HRESULT)ncalcsize;
		}
	}
	break;
	case WM_SIZE:
		if (wParam != SIZE_MINIMIZED)
		{
			draw.Resize(lParam);
			Redraw();
		}

		if (wParam == SIZE_MINIMIZED)
		{
			minimized = true;
			io.bFullscreen = false;
		}
		else if (wParam == SIZE_RESTORED)
		{
			minimized = false;
			io.bFullscreen = false;
		}
		else if (wParam == SIZE_MAXIMIZED)
		{
			io.bFullscreen = true;
		}
		break;
	case WM_NCMOUSEMOVE:
	{
		POINT mousePos = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

		ScreenToClient(hWnd, &mousePos);

		io.mousePos.x = (FLOAT)mousePos.x;
		io.mousePos.y = (FLOAT)mousePos.y;
	}
		break;
	case WM_DISPLAYCHANGE:
		Redraw();
		break;
	case WM_DESTROY:
		Overlay::Destroy();
		draw.CleanupDeviceD2D();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProcW(hWnd, message, wParam, lParam);
	}

	return lRet;
}
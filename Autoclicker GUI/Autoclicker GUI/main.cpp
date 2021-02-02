#include "Header.hpp"

constexpr auto ID_CLICK_METHOD_DROPLIST = 1;
constexpr auto ID_CHOOSE_START_BUTTON = 2;
constexpr auto ID_CHOOSE_EXECUTE_BUTTON = 3;
constexpr auto ID_SWITCH_METHOD_BUTTON = 4;
constexpr auto ID_CLICK_REPEAT_DROPLIST = 5;
constexpr auto ID_CLICK_COUNT_TEXTEDIT = 6;
constexpr auto ID_ADD_BUTTON = 7;
constexpr auto ID_ACTIVE_MACROSSES_TABLE = 8;
constexpr auto ID_DELETE_MACROS_BUTTON = 9;

D2D1_DRAW draw;
IO io;
IO overlayIo;
Memory memory;
Autoclicker autoclicker;

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

RecordButtonReceive startButton;
RecordButtonReceive executeButton;

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

BOOL COGUIProc(UINT elementID, UINT message, void* pStruct);

int FPS = 0;
int bufFPS = 0;
LONGLONG lastCheck = 0;

HRESULT Redraw()
{
	if (minimized)
		return NULL;

	if (pos == 100)
		pos = 1;
	else
		pos = 1;

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

	Log("Autoclicker started");

	HANDLE hToken;
	TOKEN_ELEVATION tElevation = { 0 };
	DWORD dwLength, UIAccess, sID;
	RAWINPUTDEVICE Rid[2];

	MSG msg;
	ZeroMemory(&msg, sizeof(msg));

	try
	{
		if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
			throw exception("Unable to query process token");

		if (!GetTokenInformation(hToken, TokenElevation, &tElevation, sizeof(tElevation), &dwLength))
			throw exception("Unable to get token information");

		if (!tElevation.TokenIsElevated)
			throw exception("Not elevated");

		Log("Token is elevated");

		if (!GetTokenInformation(hToken, TokenUIAccess, &UIAccess, sizeof(UIAccess), &dwLength))
			throw exception("Unable to get token information about UIAccess");

		if (!UIAccess)
			Log("No UIAccess");
		else
			Log("UIAccess granted");

		if (!GetTokenInformation(hToken, TokenSessionId, &sID,
			sizeof(sID), &dwLength))
			throw exception("GetTokenInformation Error");
		if (!sID)
			throw exception("0 Session ID error");

		Log("Not 0 session ID");

		CloseHandle(hToken);

		if (Overlay::AlreadyRunning())
			throw exception("Autoclicker already started");

		if (UIAccess && Overlay::Init(hInstance))
		{
			Log("Overlay initialized");
			return Overlay::Work();
		}

		if (FindWindowW(szWindowClass, NULL))
			return FALSE;

		if (!MyRegisterClass(hInstance))
			throw exception("Unable to register window class");

		// Perform application initialization:
		if (!InitInstance(hInstance, nCmdShow))
			return FALSE;

		// Initialize draw class
		if (FAILED(draw.Initialize(mainhWnd)))
			throw exception("Unable to initialize draw class");

		// Initialize COGUI
		if (!COGUI::Init(mainhWnd, COGUIProc))
			throw exception("Unable to initialize CoGUI");

		COGUI::CreateElement(COGUI_MinimizeButton, L"", 0, 0, 0, 0, 0, -1);
		COGUI::CreateElement(COGUI_ScaleButton, L"", 0, 0, 0, 0, 0, -1);
		COGUI::CreateElement(COGUI_CloseButton, L"", 0, 0, 0, 0, 0, -1);

		COGUI::CreateElement(COGUI_DropList, L"", 25, 25, 100, 25, 0, ID_CLICK_METHOD_DROPLIST);
		COGUI::CreateElement(COGUI_RecordButton, L"Start", 25, 60, 100, 25, 0, ID_CHOOSE_START_BUTTON);
		COGUI::CreateElement(COGUI_RecordButton, L"Execute", 25, 95, 100, 25, 0, ID_CHOOSE_EXECUTE_BUTTON);
		COGUI::CreateElement(COGUI_DropList, L"", 25, 130, 100, 25, 0, ID_CLICK_REPEAT_DROPLIST);
		COGUI::CreateElement(COGUI_TextEdit, L"", 135, 130, 100, 25, 0, ID_CLICK_COUNT_TEXTEDIT);
		COGUI::CreateElement(COGUI_Button, L"Add", 25, 165, 100, 25, 0, ID_ADD_BUTTON);
		COGUI::CreateElement(COGUI_Table, L"", 25, 200, 300, 130, 0, ID_ACTIVE_MACROSSES_TABLE);
		COGUI::CreateElement(COGUI_Button, L"Delete", 340, 200, 100, 25, 0, ID_DELETE_MACROS_BUTTON);

		COGUI::AddDropListString(ID_CLICK_METHOD_DROPLIST, L"SendInput");
		COGUI::AddDropListString(ID_CLICK_METHOD_DROPLIST, L"SendMessage");

		COGUI::AddDropListString(ID_CLICK_REPEAT_DROPLIST, L"Once");
		COGUI::AddDropListString(ID_CLICK_REPEAT_DROPLIST, L"Count");
		COGUI::AddDropListString(ID_CLICK_REPEAT_DROPLIST, L"Toggle");
		COGUI::AddDropListString(ID_CLICK_REPEAT_DROPLIST, L"WhileHolding");

		keyboardLayout = LoadKeyboardLayoutA("00000400", KLF_SETFORPROCESS);

		Redraw();

		if (!SetTimer(mainhWnd,
			1,
			1000 / 70,
			0))
			throw exception("Unable to create timer");

		Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
		Rid[0].usUsage = HID_USAGE_GENERIC_KEYBOARD;
		Rid[0].dwFlags = RIDEV_INPUTSINK;
		Rid[0].hwndTarget = mainhWnd;

		Rid[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
		Rid[1].usUsage = HID_USAGE_GENERIC_MOUSE;
		Rid[1].dwFlags = RIDEV_INPUTSINK;
		Rid[1].hwndTarget = mainhWnd;

		if (RegisterRawInputDevices(Rid, 2, sizeof(Rid[0])) == FALSE)
			throw exception("Unable to register raw input devices.");

		while (GetMessageA(&msg, nullptr, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (hooklowLvlKeyboard && !UnhookWindowsHookEx(hooklowLvlKeyboard))
			throw exception("Unable to unhook Low Level Keyboard Hook");
	}
	catch (const std::exception& e)
	{
		Log(e.what());
		if (hToken) CloseHandle(hToken);
		return FALSE;
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
		CW_USEDEFAULT, 0, 500, 500, nullptr, nullptr, hInstance, nullptr);

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
		RecordButtonReceive rbreceive = *ReCa<RecordButtonReceive*>(pStruct);
		executeButton = rbreceive;
	}
		break;
	case ID_ADD_BUTTON:
	{
		if (startButton.strLength == NULL
			|| executeButton.strLength == NULL)
			break;
		
		wstring buf;
		buf.append(COGUI::GetDropListSelectedLineName(ID_CLICK_METHOD_DROPLIST));
		buf.append(L" ");
		buf.append(startButton.pStr);
		buf.append(L" ");
		buf.append(executeButton.pStr);
		buf.append(L" ");
		buf.append(COGUI::GetDropListSelectedLineName(ID_CLICK_REPEAT_DROPLIST));
		COGUI::AddTableString(ID_ACTIVE_MACROSSES_TABLE, (LPWCH)buf.c_str());

		autoclicker.Add({ startButton.key, executeButton.key, 0, (ClickMethod)COGUI::GetDropListSelectedLine(ID_CLICK_METHOD_DROPLIST), (ClickRepeat)COGUI::GetDropListSelectedLine(ID_CLICK_REPEAT_DROPLIST), 1});
	}
		break;
	case ID_DELETE_MACROS_BUTTON:
	{
		DWORD lNum = COGUI::GetTableSelectedLine(ID_ACTIVE_MACROSSES_TABLE);
		
		if (lNum != -1)
		{
			COGUI::DeleteTableLine(ID_ACTIVE_MACROSSES_TABLE, lNum);

			autoclicker.Remove(lNum);
		}
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
			curLast.y = rw.data.mouse.lLastY;
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
	case WM_TIMER:
		Redraw();
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
		draw.CleanupDeviceD2D();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProcW(hWnd, message, wParam, lParam);
	}

	return lRet;
}
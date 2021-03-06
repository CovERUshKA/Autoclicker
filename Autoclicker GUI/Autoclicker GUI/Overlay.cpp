#include "Overlay.hpp"

#define ErrorMessageBox(msg) MessageBoxA(overlayhWnd, msg, "Autoclicker - Error", MB_OK | MB_TOPMOST | MB_ICONERROR);

constexpr auto ID_CHOOSE_START_BUTTON = 1;
constexpr auto ID_CHOOSE_EXECUTE_BUTTON = 2;
constexpr auto ID_ADD_BUTTON = 3;
constexpr auto ID_ACTIVE_MACROSSES_TABLE = 4;
constexpr auto ID_DELETE_MACROS_BUTTON = 5;

// Global Variables:
HINSTANCE ohInst; // current instance
HMODULE pInputHookDLL;
constexpr auto oszTitle = L"Autoclicker Overlay";                  // The title bar text
constexpr auto oszWindowClass = L"Autoclicker_overlay";                  // The title bar text
HWND overlayhWnd;
HWND inputhWnd;
BOOL activate = FALSE;
BOOL quit = FALSE;
HWINSTA winSta;
HANDLE hThread;
DWORD dwThreadID;

ID3D11Device* g_pd3dDevice;
IDXGIDevice4* g_pdxgiDevice;
IDXGIFactory5* dxFactory;
ID2D1Factory2* d2Factory;
IDXGISurface2* surface;
ID2D1DeviceContext* dc;
ID2D1Device* d2Device;
IDCompositionVisual* visual;
IDCompositionDesktopDevice* m_device;

IDCompositionTarget*  m_target;
IDCompositionVisual2* m_visual;
IDCompositionSurface* m_surface;
D2D_SIZE_F            m_size;
D2D_POINT_2F          m_dpi;

HHOOK hookLowLvlKeyboard;
HHOOK hookLowLvlMouse;

POINT curPos = { 0, 0 };
POINT retCurPos = { 0, 0 };
POINT ocurLast;

DWORD tID;
DWORD hWndThreadID;

LARGE_INTEGER pFreq;
LARGE_INTEGER pCount;
DWORD rawFPS;
DWORD FPS;

BOOL CleanupDevices();

template <class T>
void  SafeReleasee(T** ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}

bool show_demo_window = true;
bool show_another_window = false;
FLOAT clear_color[4] = { 0.00f, 0.00f, 0.00f, 0.30f } ;
char buf[MAX_PATH];

HRESULT Overlay::Redraw()
{
	HRESULT hr = S_OK;
	wstring buf;

	try
	{
		if (!activate)
			return hr;

		POINT offset = {};
		hr = m_surface->BeginDraw(nullptr,
			__uuidof(dc),
			reinterpret_cast<void**>(&dc),
			&offset);
		if (FAILED(hr))
			throw exception("Unable to begin draw");

		draw.pRenderTarget = dc;

		dc->SetDpi(m_dpi.x,
			m_dpi.y);

		dc->SetTransform(D2D1::Matrix3x2F::Translation(offset.x * 96 / m_dpi.x,
			offset.y * 96 / m_dpi.y));

		dc->Clear();

		COGUI::Render();

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
		buf.append(to_wstring(ocurLast.x).c_str());
		buf.append(L" ");
		buf.append(to_wstring(ocurLast.y).c_str());

		POINT pt;
		if (!GetCursorPos(&pt))
			Log("Unable to GetCursorPos");

		buf.append(L" ");
		buf.append(to_wstring(pt.x).c_str());
		buf.append(L" ");
		buf.append(to_wstring(pt.y).c_str());

		CURSORINFO curInfo;

		curInfo.cbSize = sizeof(CURSORINFO);

		GetCursorInfo(&curInfo);

		LARGE_INTEGER bufpCounter;

		if (!pCount.QuadPart)
			QueryPerformanceCounter(&pCount);

		QueryPerformanceCounter(&bufpCounter);

		if ((FLOAT)(bufpCounter.QuadPart - pCount.QuadPart) / (pFreq.QuadPart / 1000) < 1000)
			rawFPS += 1;
		else
		{
			pCount = bufpCounter;
			FPS = rawFPS;
			rawFPS = 0;
		}

		buf.append(L" ");
		buf.append(to_wstring(FPS).c_str());

		TextInformation textInfo;

		textInfo.clip = true;
		textInfo.color = COGUI::COGUI_COLOR(255, 255, 255);
		textInfo.multiline = false;
		textInfo.xAlign = 0;
		textInfo.yAlign = 0;

		draw.String({ 50, 50 }, textInfo, buf.c_str(), buf.length(), L"Consolas", 13);

		if (curInfo.flags != CURSOR_SHOWING)
			draw.Circle({ (FLOAT)pt.x, (FLOAT)pt.y }, 5, D2D1::ColorF(D2D1::ColorF::Red));

		hr = m_surface->EndDraw();
		if (FAILED(hr))
			throw exception("Unable to m_surface->EndDraw()");

		hr = m_device->Commit();
		if (FAILED(hr))
			throw exception("Unable to m_device->Commit()");
	}
	catch (const std::exception& e)
	{
		Log(e.what());
	}

	return hr;
}

HRESULT Initialize()
{
	RECT rect;
	HRESULT hr;
	unsigned x;
	unsigned y;
	HMONITOR monitor;

	try
	{
		hr = D3D11CreateDevice(nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			D3D11_CREATE_DEVICE_BGRA_SUPPORT,
			nullptr, 0,
			D3D11_SDK_VERSION,
			&g_pd3dDevice,
			nullptr,
			nullptr);
		if (FAILED(hr)) throw exception("Unable to D3D11CreateDevice");

		hr = g_pd3dDevice->QueryInterface(&g_pdxgiDevice);
		if (FAILED(hr)) throw exception("Unable to g_pd3dDevice->QueryInterface");

		hr = D2D1CreateDevice(g_pdxgiDevice,
			nullptr,
			&d2Device);
		if (FAILED(hr)) throw exception("Unable to D2D1CreateDevice");

		hr = DCompositionCreateDevice3(
			d2Device,
			__uuidof(m_device),
			reinterpret_cast<void**>(&m_device));
		if (FAILED(hr)) throw exception("Unable to DCompositionCreateDevice3");

		hr = m_device->CreateTargetForHwnd(overlayhWnd,
			false,
			&m_target);
		if (FAILED(hr)) throw exception("Unable to m_device->CreateTargetForHwnd");

		hr = m_device->CreateVisual(&m_visual);
		if (FAILED(hr)) throw exception("Unable to m_device->CreateVisual");

		if (!GetClientRect(overlayhWnd, &rect))
			throw exception("Unable to GetClientRect");

		hr = m_device->CreateSurface(rect.right - rect.left,
			rect.bottom - rect.top,
			DXGI_FORMAT_B8G8R8A8_UNORM,
			DXGI_ALPHA_MODE_PREMULTIPLIED,
			&m_surface);
		if (FAILED(hr)) throw exception("Unable to m_device->CreateSurface");

		hr = m_visual->SetContent(m_surface);
		if (FAILED(hr)) throw exception("Unable to m_visual->SetContent");

		hr = m_target->SetRoot(m_visual);
		if (FAILED(hr)) throw exception("Unable to target->SetRoot");

		hr = d2Device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
			&dc);
		if (FAILED(hr)) throw exception("Unable to d2Device->CreateDeviceContext");

		monitor = MonitorFromWindow(overlayhWnd,
			MONITOR_DEFAULTTONEAREST);

		hr = GetDpiForMonitor(monitor,
			MDT_EFFECTIVE_DPI,
			&x,
			&y);
		if (FAILED(hr)) throw exception("Unable to GetDpiForMonitor");

		m_dpi.x = static_cast<float>(x);
		m_dpi.y = static_cast<float>(y);
		m_size.width = (rect.right - rect.left) * 96 / m_dpi.x;
		m_size.height = (rect.bottom - rect.top) * 96 / m_dpi.y;
	}
	catch (const std::exception& e)
	{
		Log(e.what());
		CleanupDevices();
	}

	return hr;
}

LRESULT CALLBACK Overlay::LowLevelKeyboardProc(
	_In_ int    nCode,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
)
{
	if (nCode < HC_ACTION)
		return CallNextHookEx(hookLowLvlKeyboard, nCode, wParam, lParam);

	LPKBDLLHOOKSTRUCT pKeyboard = (LPKBDLLHOOKSTRUCT)lParam;

	switch (nCode)
	{
	case HC_ACTION:
	{
		switch (wParam)
		{
		case WM_KEYUP:
		{
			if (!activate)
				break;

			if (pKeyboard->vkCode == VK_ESCAPE)
			{
				Overlay::Toggle();
				return 1;
			}
		}
		case WM_KEYDOWN:
		case WM_CHAR:
		case WM_SYSKEYUP:
		{
			if (pKeyboard->vkCode == VK_LMENU
				|| pKeyboard->vkCode == VK_RMENU
				|| pKeyboard->vkCode == VK_SNAPSHOT)
			{
				break;
			}

			if (!activate)
				break;

			return 1;
		}
			break;
		case WM_SYSKEYDOWN:
		{
			if (pKeyboard->vkCode == VK_LMENU
				|| pKeyboard->vkCode == VK_RMENU)
			{
				break;
			}

			if (pKeyboard->vkCode == 'X')
			{
				Overlay::Toggle();
				return 1;
			}

			if (!activate)
				break;

			if (pKeyboard->vkCode == VK_LSHIFT || pKeyboard->vkCode == VK_RSHIFT)
				break;
			
			if (pKeyboard->vkCode == VK_TAB)
			{
				Overlay::Toggle();
				break;
			}

			return 1;
		}
		break;
		default:
			break;
		}
	}
	break;
	default:
		break;
	}

	return CallNextHookEx(hookLowLvlKeyboard, nCode, wParam, lParam);
}

void TimerProc(
	HWND Arg1,
	UINT Arg2,
	UINT_PTR Arg3,
	DWORD Arg4
)
{
	Overlay::Redraw();
}

BOOL Overlay::Init(HINSTANCE hInstance)
{
	ohInst = hInstance;

	QueryPerformanceFrequency(&pFreq);

	Log("Overlay initializing...");

	try
	{
		ATOM pClass = OverlayRegisterClass(hInstance);
		if (!pClass)
			throw exception("Unable to register overlay class");

		Log("Window class initialized");

		if (!ImmDisableIME(-1))
			throw exception("Unable to disable IMM");

		Log("IME disabled");

		// Perform application initialization:
		if (!InitInstance(hInstance, pClass, SW_SHOWNOACTIVATE))
			return FALSE;

		Log("Instance initialized");

		hookLowLvlKeyboard = SetWindowsHookExA(WH_KEYBOARD_LL, LowLevelKeyboardProc, hInstance, NULL);
		if (!hookLowLvlKeyboard)
			throw exception("Unable to hook Low Lvl Keyboard");

		if (!SetTimer(overlayhWnd,
			1,
			1000 / 70,
			(TIMERPROC)TimerProc))
			throw exception("Unable to create timer");

		Log("Timer initialized");

		RAWINPUTDEVICE Rid[2];

		Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
		Rid[0].usUsage = HID_USAGE_GENERIC_KEYBOARD;
		Rid[0].dwFlags = RIDEV_INPUTSINK;
		Rid[0].hwndTarget = overlayhWnd;

		Rid[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
		Rid[1].usUsage = HID_USAGE_GENERIC_MOUSE;
		Rid[1].dwFlags = RIDEV_INPUTSINK;
		Rid[1].hwndTarget = overlayhWnd;

		if (RegisterRawInputDevices(Rid, 2, sizeof(Rid[0])) == FALSE)
			throw exception("Unable to register raw input devices.");

		Log("Raw input devices registered");
	}
	catch (const std::exception& e)
	{
		Log(e.what());
		return FALSE;
	}

	return TRUE;
}

BOOL Overlay::Work()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));

	tID = GetCurrentThreadId();

	while (true)
	{
		if (GetMessageA(&msg, nullptr, 0, 0))
			DispatchMessage(&msg);
	}

	if (hookLowLvlKeyboard && !UnhookWindowsHookEx(hookLowLvlKeyboard))
	{
		Log("Unable to unhook Low Level Keyboard Hook");
		return EXIT_FAILURE;
	}

	if (hookLowLvlMouse && !UnhookWindowsHookEx(hookLowLvlMouse))
	{
		Log("Unable to unhook Low Level Mouse Hook");
		return EXIT_FAILURE;
	}

	CleanupDevices();

	Log("Devices cleaned");

	return EXIT_SUCCESS;
}

BOOL Overlay::Toggle()
{
	HRESULT hr = S_OK;

	activate = !activate;

	try
	{
		if (activate)
		{
			RECT cliprc = { retCurPos.x, retCurPos.y, retCurPos.x, retCurPos.y };
			HWND frg = GetForegroundWindow();

			D2D1_RECT_F rectf = D2D1::RectF(0, 0, m_size.width, m_size.height);
			m_visual->SetClip(rectf);

			SetWindowLongPtrW(overlayhWnd, GWL_EXSTYLE, GetWindowLongPtrW(overlayhWnd, GWL_EXSTYLE) & ~WS_EX_TRANSPARENT);

			hWndThreadID = GetWindowThreadProcessId(frg, 0);

			HDESK hDesktop = OpenInputDesktop(0, FALSE, GENERIC_ALL);

			if (!hDesktop)
				Log("Unable to OpenInputDesktop");

			if (hDesktop && hDesktop != GetThreadDesktop(GetCurrentThreadId()))
				SetThreadDesktop(hDesktop);

			GetCursorPos(&retCurPos);

			if (!AttachThreadInput(tID, hWndThreadID, true))
				Log("Unable to AttachThreadInput");
		}
		else
		{
			D2D1_RECT_F rectf = D2D1::RectF(0, 0, 0, 0);
			hr = m_visual->SetClip(rectf);
			if (FAILED(hr))
				throw exception("Unable to m_visual->SetClip(rectf)");

			hr = m_device->Commit();
			if (FAILED(hr))
				throw exception("Unable to m_device->Commit()");

			HDESK hDesktop = OpenInputDesktop(0, FALSE, GENERIC_ALL);

			if (!hDesktop)
				Log("Unable to OpenInputDesktop");

			if (hDesktop && hDesktop != GetThreadDesktop(GetCurrentThreadId()))
				SetThreadDesktop(hDesktop);

			SetCursorPos(retCurPos.x, retCurPos.y);

			if (!AttachThreadInput(tID, hWndThreadID, false))
				Log("Unable to de AttachThreadInput");

			SetWindowLongPtrW(overlayhWnd, GWL_EXSTYLE, GetWindowLongPtrW(overlayhWnd, GWL_EXSTYLE) | WS_EX_TRANSPARENT);
		}
	}
	catch (const std::exception& e)
	{
		Log(e.what());
	}

	return TRUE;
}

void Overlay::Destroy()
{
	quit = true;

	return;
}

HWND Overlay::GetWindow()
{
	return overlayhWnd;
}

BOOL Overlay::AlreadyRunning()
{
	if (FindWindowW(oszWindowClass, NULL))
		return TRUE;

	return FALSE;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM Overlay::OverlayRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEXW);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)COLOR_DESKTOP;
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = oszWindowClass;
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
BOOL Overlay::InitInstance(HINSTANCE hInstance, ATOM aClass, int nCmdShow)
{
	RECT desktop;

	try
	{
		if (!GetWindowRect(GetDesktopWindow(), &desktop))
			throw exception("Unable to get dektop window size");

		const auto hpath = LoadLibraryW(L"user32.dll");
		CreateWindowInBandEx pCreateWindowInBandEx = CreateWindowInBandEx(GetProcAddress(hpath, "CreateWindowInBandEx"));
		if (!pCreateWindowInBandEx)
			throw exception("Unable to get CreateWindowInBandEx");

		Log("CreateWindowInBandEx founded");

		overlayhWnd = pCreateWindowInBandEx(WS_EX_NOACTIVATE | WS_EX_NOREDIRECTIONBITMAP | WS_EX_LAYERED | WS_EX_TRANSPARENT,
			aClass,
			oszTitle,
			WS_POPUP,
			0, 0, desktop.right, desktop.bottom,
			NULL,
			NULL,
			hInstance,
			LPVOID(aClass),
			ZBID_UIACCESS,
			0);

		if (!overlayhWnd)
			throw exception("Unable to create overlay window");

		Log("Overlay window created");

		if (FAILED(Initialize()))
			throw exception("Unable to initialize");

		Log("Drawing initialized");

		if (FAILED(draw.Initialize(overlayhWnd)))
			throw exception("Unable to initialize draw class");

		if (!COGUI::Init(overlayhWnd, COGUIProc))
			throw exception("Unable to initialize CoGUI");

		COGUI::CreateElement(COGUI_RecordButton, L"Start", 25, 25, 100, 25, 0, ID_CHOOSE_START_BUTTON);
		COGUI::CreateElement(COGUI_DropList, L"Execute", 25, 60, 100, 25, 0, ID_CHOOSE_EXECUTE_BUTTON);
		COGUI::CreateElement(COGUI_Button, L"Add", 25, 110, 100, 25, 0, ID_ADD_BUTTON);
		COGUI::CreateElement(COGUI_Table, L"", 25, 145, 200, 130, 0, ID_ACTIVE_MACROSSES_TABLE);
		COGUI::CreateElement(COGUI_Button, L"Delete", 240, 145, 100, 25, 0, ID_DELETE_MACROS_BUTTON);

		UpdateWindow(overlayhWnd);

		ShowWindow(overlayhWnd, nCmdShow);
	}
	catch (const std::exception& e)
	{
		Log(e.what());
		return FALSE;
	}

	return TRUE;
}

BOOL CleanupDevices()
{
	SafeReleasee(&dc);
	SafeReleasee(&m_surface);
	SafeReleasee(&m_visual);
	SafeReleasee(&m_target);
	SafeReleasee(&m_device);
	SafeReleasee(&d2Device);
	SafeReleasee(&g_pdxgiDevice);
	SafeReleasee(&g_pd3dDevice);

	return TRUE;
}

BOOL Overlay::COGUIProc(UINT elementID, UINT message, void* pStruct)
{
	switch (elementID)
	{
	case ID_CHOOSE_EXECUTE_BUTTON:
	{
		COGUI_DropListReceive dlreceive = *ReCa<COGUI_DropListReceive*>(pStruct);
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

			if ((hFind = FindFirstFileW(path.c_str(), &data)) != INVALID_HANDLE_VALUE) 
			{
				do
				{
					// Idk what is this
					if (wcscmp(data.cFileName, L".") == NULL
						|| wcscmp(data.cFileName, L"..") == NULL)
						continue;

					wstring fileName = wstring(data.cFileName).substr(0, wcslen(data.cFileName) - 4).c_str();

					if (wcslen(data.cFileName) > 4 && wstring(data.cFileName).compare(wcslen(data.cFileName) - 4, 4, L".txt") == NULL)
						COGUI::AddDropListString(ID_CHOOSE_EXECUTE_BUTTON, fileName);

				} while (FindNextFileW(hFind, &data) != 0);
				FindClose(hFind);
			}
		}
		break;
		case COGUI_DL_POSTCLOSE:
		{
			COGUI::ClearDropList(ID_CHOOSE_EXECUTE_BUTTON);
		}
		break;
		default:
			break;
		}
	}
	break;
	case ID_ADD_BUTTON:
	{
		COGUI::AddTableString(ID_ACTIVE_MACROSSES_TABLE, L"Hello");
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
LRESULT CALLBACK Overlay::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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
			ocurLast.x = rw.data.mouse.lLastX;
			ocurLast.y = rw.data.mouse.lLastY;
		}
		
		if (rw.header.dwType == RIM_TYPEKEYBOARD)
		{
			switch (rw.data.keyboard.Message)
			{
			case WM_KEYDOWN:
			{
				if (!activate)
					break;

				if (rw.data.keyboard.VKey == VK_ESCAPE)
				{
					DestroyWindow(overlayhWnd);
					return 1;
				}
			}
				break;
			case WM_SYSKEYDOWN:
				if (rw.data.keyboard.VKey == 18)
					break;

				if (rw.data.keyboard.VKey == 'X')
				{
					if (hookLowLvlKeyboard && !UnhookWindowsHookEx(hookLowLvlKeyboard))
					{
						Log("Unable to unhook Low Level Keyboard Hook");
						return EXIT_FAILURE;
					}

					hookLowLvlKeyboard = SetWindowsHookExA(WH_KEYBOARD_LL, LowLevelKeyboardProc, ohInst, NULL);
					if (!hookLowLvlKeyboard)
					{
						Log("Unable to hook Low Lvl Keyboard");
						return FALSE;
					}

					Overlay::Toggle();
					break;
				}

				if (!activate)
					break;

				if (rw.data.keyboard.VKey == VK_TAB)
				{
					Overlay::Toggle();
					break;
				}

				break;
			default:
				break;
			}
		}
	}
		break;
	case WM_SETCURSOR:
		SetCursor(LoadCursor(NULL, IDC_CROSS));
		return 1;
	case WM_NCHITTEST:
		return HTCLIENT;
	case WM_MOUSEACTIVATE:
		return MA_NOACTIVATE;
	case WM_HOTKEY:
	{
		switch (wParam)
		{
		case 1:
		{
			Overlay::Toggle();
		}
			break;
		default:
			break;
		}
	}
		break;
	case WM_ERASEBKGND:
		return 1;
	case WM_SYSKEYDOWN:
	{
		switch (wParam)
		{
		case VK_F4:
		{
			Toggle();
		}
			break;
		default:
			break;
		}
	}
		break;
	case WM_MOUSEMOVE:
		curPos.x = GET_X_LPARAM(lParam);
		curPos.y = GET_Y_LPARAM(lParam);
		break;
	case WM_NCMOUSEMOVE:
	{
		POINT mousePos = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

		ScreenToClient(hWnd, &mousePos);

		overlayIo.mousePos.x = (FLOAT)mousePos.x;
		overlayIo.mousePos.y = (FLOAT)mousePos.y;
	}
	break;
	case WM_DISPLAYCHANGE:
		Redraw();
		break;
	case WM_DESTROY:
		Log("Window destroyed");
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProcW(hWnd, message, wParam, lParam);
	}

	return lRet;
}
#include "Overlay.hpp"

#define ErrorMessageBox(msg) MessageBoxA(overlayhWnd, msg, "Autoclicker - Error", MB_OK | MB_TOPMOST | MB_ICONERROR);

enum ZBID
{
	ZBID_DEFAULT = 0,
	ZBID_DESKTOP = 1,
	ZBID_UIACCESS = 2,
	ZBID_IMMERSIVE_IHM = 3,
	ZBID_IMMERSIVE_NOTIFICATION = 4,
	ZBID_IMMERSIVE_APPCHROME = 5,
	ZBID_IMMERSIVE_MOGO = 6,
	ZBID_IMMERSIVE_EDGY = 7,
	ZBID_IMMERSIVE_INACTIVEMOBODY = 8,
	ZBID_IMMERSIVE_INACTIVEDOCK = 9,
	ZBID_IMMERSIVE_ACTIVEMOBODY = 10,
	ZBID_IMMERSIVE_ACTIVEDOCK = 11,
	ZBID_IMMERSIVE_BACKGROUND = 12,
	ZBID_IMMERSIVE_SEARCH = 13,
	ZBID_GENUINE_WINDOWS = 14,
	ZBID_IMMERSIVE_RESTRICTED = 15,
	ZBID_SYSTEM_TOOLS = 16,
	ZBID_LOCK = 17,
	ZBID_ABOVELOCK_UX = 18,
};

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
FLOAT opos = 0;
HWINSTA winSta;

ID3D11Device* g_pd3dDevice;
ID3D11DeviceContext* g_pd3dDeviceContext;
ID3D11RenderTargetView* g_mainRenderTargetView;
IDXGIDevice4* g_pdxgiDevice;
IDXGIFactory5* dxFactory;
IDXGISwapChain4* g_pSwapChain;
ID2D1Factory2* d2Factory;
IDXGISurface2* surface;
ID2D1DeviceContext* dc;
ID2D1Device* d2Device;
ID2D1Bitmap* bitmap;
IDCompositionVisual* visual;
IDCompositionDevice* dcompDevice;
IDCompositionDesktopDevice* m_device;
IDCompositionTarget* target;
ID2D1SolidColorBrush* brush;

IDCompositionTarget*  m_target;
IDCompositionVisual2* m_visual;
IDCompositionSurface* m_surface;
D2D_SIZE_F                   m_size;
D2D_POINT_2F                 m_dpi;

HHOOK hookLowLvlKeyboard;
HHOOK hookLowLvlMouse;

POINT curPos = { 0, 0 };
POINT retCurPos = { 0, 0 };

BOOL CleanupDevices();

template <class T> void  SafeReleasee(T** ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}

void HR(HRESULT const result)
{
	if (S_OK != result)
	{
		Log("HR Error");
	}
}

bool show_demo_window = true;
bool show_another_window = false;
FLOAT clear_color[4] = { 0.00f, 0.00f, 0.00f, 0.30f } ;
char buf[MAX_PATH];

HRESULT Overlay::Redraw()
{
	HRESULT hr = S_OK;

	if (!activate)
		return 0;

	POINT offset = {};
	HR(m_surface->BeginDraw(nullptr,
		__uuidof(dc),
		reinterpret_cast<void**>(&dc),
		&offset));

	draw.pRenderTarget = dc;

	dc->SetDpi(m_dpi.x,
		m_dpi.y);
	
	dc->SetTransform(D2D1::Matrix3x2F::Translation(offset.x * 96 / m_dpi.x,
		offset.y * 96 / m_dpi.y));

	dc->Clear();

	COGUI::Render();

	if (opos == 1000)
	{
		opos = 0;
	}

	opos += 1;

	HR(m_surface->EndDraw());

	HR(m_device->Commit());

	return hr;
}

HRESULT Initialize()
{
	HRESULT hr = S_OK;

	if (FAILED(D3D11CreateDevice(nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		D3D11_CREATE_DEVICE_BGRA_SUPPORT,
		nullptr, 0,
		D3D11_SDK_VERSION,
		&g_pd3dDevice,
		nullptr,
		nullptr)))
	{
		Log("Unable to D3D11CreateDevice");
		return -1;
	}

	if (FAILED(g_pd3dDevice->QueryInterface(&g_pdxgiDevice)))
	{
		Log("Unable to g_pd3dDevice->QueryInterface");
		return -1;
	}
	
	if (FAILED(D2D1CreateDevice(g_pdxgiDevice,
		nullptr,
		&d2Device)))
	{
		Log("Unable to D2D1CreateDevice");
		return -1;
	}

	if (FAILED(DCompositionCreateDevice3(
		d2Device,
		__uuidof(m_device),
		reinterpret_cast<void**>(&m_device))))
	{
		Log("Unable to DCompositionCreateDevice3");
		return -1;
	}

	if (FAILED(m_device->CreateTargetForHwnd(overlayhWnd,
		false,
		&m_target)))
	{
		Log("Unable to m_device->CreateTargetForHwnd");
		return -1;
	}

	if (FAILED(m_device->CreateVisual(&m_visual)))
	{
		Log("Unable to m_device->CreateVisual");
		return -1;
	}

	RECT rect = {};
	if (!GetClientRect(overlayhWnd,
		&rect))
	{
		Log("Unable to GetClientRect");
		return -1;
	}

	if (FAILED(m_device->CreateSurface(rect.right - rect.left,
		rect.bottom - rect.top,
		DXGI_FORMAT_B8G8R8A8_UNORM,
		DXGI_ALPHA_MODE_PREMULTIPLIED,
		&m_surface)))
	{
		Log("Unable to m_device->CreateSurface");
		return -1;
	}

	if (FAILED(m_visual->SetContent(m_surface)))
	{
		Log("Unable to m_visual->SetContent");
		return -1;
	}

	if (FAILED(m_target->SetRoot(m_visual)))
	{
		Log("Unable to m_target->SetRoot");
		return -1;
	}

	if (FAILED(d2Device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
		&dc)))
	{
		Log("Unable to d2Device->CreateDeviceContext");
		return -1;
	}

	D2D_COLOR_F const color = D2D1::ColorF(0.26f,
		0.56f,
		0.87f,
		0.5f);

	if (FAILED(dc->CreateSolidColorBrush(color,
		&brush)))
	{
		Log("Unable to dc->CreateSolidColorBrush");
		return -1;
	}

	HMONITOR const monitor = MonitorFromWindow(overlayhWnd,
		MONITOR_DEFAULTTONEAREST);
	unsigned x = 0;
	unsigned y = 0;
	
	if (FAILED(GetDpiForMonitor(monitor,
		MDT_EFFECTIVE_DPI,
		&x,
		&y)))
	{
		Log("Unable to GetDpiForMonitor");
		return -1;
	}

	m_dpi.x = static_cast<float>(x);
	m_dpi.y = static_cast<float>(y);
	m_size.width = (rect.right - rect.left) * 96 / m_dpi.x;
	m_size.height = (rect.bottom - rect.top) * 96 / m_dpi.y;

	return TRUE;
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

	Log("Overlay initializing...");

	ATOM pClass = OverlayRegisterClass(hInstance);
	if (!pClass)
	{
		Log("Unable to register overlay class");
		return FALSE;
	}

	Log("Window class initialized");

	if (!ImmDisableIME(0))
	{
		Log("Unable to disable IMM");
		return FALSE;
	}

	Log("IME disabled");

	// Perform application initialization:
	if (!InitInstance(hInstance, pClass, SW_SHOW))
	{
		return FALSE;
	}

	Log("Instance initialized");

	hookLowLvlKeyboard = SetWindowsHookExA(WH_KEYBOARD_LL, LowLevelKeyboardProc, hInstance, NULL);
	if (!hookLowLvlKeyboard)
	{
		Log("Unable to hook Low Lvl Keyboard");
		return FALSE;
	}

	if (!SetTimer(overlayhWnd,
		1,
		1000 / 70,
		(TIMERPROC)TimerProc))
	{
		Log("Unable to create timer");
		return FALSE;
	}

	Log("Timer initialized");
	
	RAWINPUTDEVICE Rid[1];

	Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
	Rid[0].usUsage = HID_USAGE_GENERIC_KEYBOARD;
	Rid[0].dwFlags = RIDEV_INPUTSINK;
	Rid[0].hwndTarget = overlayhWnd;

	if (RegisterRawInputDevices(Rid, 1, sizeof(Rid[0])) == FALSE)
	{
		Log("Unable to register raw input devices.");
		return FALSE;
	}

	Log("Raw input devices registered");

	return TRUE;
}

BOOL Overlay::Work()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));

	while (GetMessage(&msg, nullptr, 0, 0))
	{
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
	activate = !activate;

	if (activate)
	{
		RECT cliprc = { retCurPos.x, retCurPos.y, retCurPos.x, retCurPos.y };

		D2D1_RECT_F rectf = D2D1::RectF(0, 0, m_size.width, m_size.height);
		m_visual->SetClip(rectf);
		
		SetWindowLongPtrW(overlayhWnd, GWL_EXSTYLE, GetWindowLongPtrW(overlayhWnd, GWL_EXSTYLE) & ~WS_EX_TRANSPARENT);
	}
	else
	{
		D2D1_RECT_F rectf = D2D1::RectF(0, 0, 0, 0);
		HR(m_visual->SetClip(rectf));

		HR(m_device->Commit());

		SetWindowLongPtrW(overlayhWnd, GWL_EXSTYLE, GetWindowLongPtrW(overlayhWnd, GWL_EXSTYLE) | WS_EX_TRANSPARENT);
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
	GetWindowRect(GetDesktopWindow(), &desktop);

	const auto hpath = LoadLibraryW(L"user32.dll");
	CreateWindowInBandEx pCreateWindowInBandEx = CreateWindowInBandEx(GetProcAddress(hpath, "CreateWindowInBandEx"));
	if (!pCreateWindowInBandEx)
	{
		Log("Unable to get CreateWindowInBandEx");
		return FALSE;
	}

	Log("CreateWindowInBandEx founded");

	overlayhWnd = pCreateWindowInBandEx(WS_EX_TOPMOST | WS_EX_NOACTIVATE | WS_EX_NOREDIRECTIONBITMAP | WS_EX_LAYERED | WS_EX_TRANSPARENT,
		aClass,
		oszTitle,
		WS_POPUP | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
		0, 0, desktop.right, desktop.bottom,
		NULL,
		NULL,
		hInstance,
		LPVOID(aClass),
		ZBID_UIACCESS,
		0);
	
	if (!overlayhWnd)
	{
		Log("Unable to create overlay window");
		return FALSE;
	}

	Log("Overlay window created");

	if (FAILED(Initialize()))
	{
		Log("Unable to initialize");
		return FALSE;
	}

	Log("Drawing initialized");

	if (FAILED(draw.Initialize(overlayhWnd)))
	{
		Log("Unable to initialize draw class");
		return FALSE;
	}

	if (!COGUI::Init(overlayhWnd, COGUIProc))
	{
		Log("Unable to initialize CoGUI");
		return FALSE;
	}

	COGUI::CreateElement(COGUI_RecordButton, L"Start", 25, 25, 100, 25, 0, ID_CHOOSE_START_BUTTON);
	COGUI::CreateElement(COGUI_DropList, L"Execute", 25, 60, 100, 25, 0, ID_CHOOSE_EXECUTE_BUTTON);
	COGUI::CreateElement(COGUI_Button, L"Add", 25, 110, 100, 25, 0, ID_ADD_BUTTON);
	COGUI::CreateElement(COGUI_Table, L"", 25, 145, 200, 130, 0, ID_ACTIVE_MACROSSES_TABLE);
	COGUI::CreateElement(COGUI_Button, L"Delete", 240, 145, 100, 25, 0, ID_DELETE_MACROS_BUTTON);

	UpdateWindow(overlayhWnd);

	ShowWindow(overlayhWnd, nCmdShow);

	return TRUE;
}

BOOL CleanupDevices()
{
	SafeReleasee(&brush);
	SafeReleasee(&target);
	SafeReleasee(&dcompDevice);
	SafeReleasee(&visual);
	SafeReleasee(&bitmap);
	SafeReleasee(&d2Device);
	SafeReleasee(&dc);
	SafeReleasee(&surface);
	SafeReleasee(&d2Factory);
	SafeReleasee(&dxFactory);
	SafeReleasee(&g_pSwapChain);
	SafeReleasee(&g_pdxgiDevice);
	SafeReleasee(&g_mainRenderTargetView);
	SafeReleasee(&g_pd3dDeviceContext);
	SafeReleasee(&g_pd3dDevice);

	return TRUE;
}

BOOL Overlay::COGUIProc(UINT elementID, UINT message, void* pStruct)
{
	switch (elementID)
	{
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

			if ((hFind = FindFirstFileW(path.c_str(), &data)) != INVALID_HANDLE_VALUE) {
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
		COGUI::AddTableString(ID_ACTIVE_MACROSSES_TABLE, (LPWCH)L"Hello", 5);
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
			switch (rw.data.mouse.usButtonFlags)
			{
			case RI_MOUSE_BUTTON_1_DOWN:
				return 1;
				break;
			default:
				break;
			}
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
	case WM_NCHITTEST:
		return HTCLIENT;
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
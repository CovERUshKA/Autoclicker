#include "Service.hpp"

SERVICE_STATUS          gSvcStatus;
SERVICE_STATUS_HANDLE   gSvcStatusHandle;
HANDLE                  ghSvcStopEvent = NULL;
HHOOK                   hookLowLvlMouse = NULL;

VOID                ReportSvcStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint);
VOID                SvcInit(DWORD dwArgc, LPTSTR* lpszArgv);
VOID                SvcReportEvent(LPTSTR szFunction);
VOID                WINAPI SvcCtrlHandler(DWORD dwCtrl);

//
// Purpose: 
//   Entry point for the service
//
// Parameters:
//   dwArgc - Number of arguments in the lpszArgv array
//   lpszArgv - Array of strings. The first string is the name of
//     the service and subsequent strings are passed by the process
//     that called the StartService function to start the service.
// 
// Return value:
//   None.
//
VOID WINAPI SvcMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
	// Register the handler function for the service

	gSvcStatusHandle = RegisterServiceCtrlHandlerW(
		SERVICE_INSTALL_NAME,
		SvcCtrlHandler);

	if (!gSvcStatusHandle)
	{
		SvcReportEvent((LPTSTR)"RegisterServiceCtrlHandler");
		return;
	}

	// These SERVICE_STATUS members remain as set here

	gSvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	gSvcStatus.dwServiceSpecificExitCode = 0;

	// Report initial status to the SCM

	ReportSvcStatus(SERVICE_START_PENDING, NO_ERROR, 3000);

	// Perform service-specific initialization and work.

	SvcInit(dwArgc, lpszArgv);
}

BOOL StartupWithToken()
{
	DWORD dwBytesNeeded = NULL;
	DWORD dwResult = NULL;
	DWORD sID = NULL;

	DWORD dwUIAccess = 1;

	HANDLE hToken;
	HANDLE hToken2;
	HANDLE hUserToken;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken))
	{
		Log("OpenProcessToken error");
		return FALSE;
	}

	if (!WTSQueryUserToken(WTSGetActiveConsoleSessionId(), &hUserToken))
	{
		Log("WTSQueryUserToken error");
		return FALSE;
	}
	
	if (!DuplicateTokenEx(hToken, MAXIMUM_ALLOWED, NULL, SecurityIdentification, TokenPrimary, &hToken2))
	{
		Log("DuplicateTokenEx error");
		CloseHandle(hToken);
		return FALSE;
	}

	if (!GetTokenInformation(hUserToken, TokenSessionId, &sID, sizeof(sID), &dwBytesNeeded))
	{
		Log("GetTokenInformation error");
		CloseHandle(hToken2);
		CloseHandle(hToken);
		return FALSE;
	}
	if (!sID)
	{
		Log("TokenSessionId error");
		CloseHandle(hToken2);
		CloseHandle(hToken);
		return FALSE;
	}

	CloseHandle(hUserToken);

	if (!SetTokenInformation(hToken2, TokenSessionId, &sID, sizeof(sID)))
	{
		Log("SetTokenInformation error");
		CloseHandle(hToken2);
		CloseHandle(hToken);
		return FALSE;
	}

	if (!ImpersonateLoggedOnUser(hToken2))
	{
		Log("ImpersonateLoggedOnUser error");
		CloseHandle(hToken2);
		CloseHandle(hToken);
		return FALSE;
	}
	
	if (!SetTokenInformation(hToken2, TokenUIAccess, &dwUIAccess, sizeof(dwUIAccess)))
	{
		Log("SetTokenInformation error");
		CloseHandle(hToken2);
		CloseHandle(hToken);
		return FALSE;
	}
	
	LPVOID pEnvBlock = NULL;
	if (!CreateEnvironmentBlock(&pEnvBlock, hToken2, FALSE))
	{
		Log("CreateEnvironmentBlock error");
		CloseHandle(hToken2);
		CloseHandle(hToken);
		return FALSE;
	}
	
	wchar_t buf[] = L"winsta0\\default";
	STARTUPINFOW si;
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.lpDesktop = buf;

	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));

	wchar_t wchFullPath[MAX_PATH];

	if (!GetCurDir(wchFullPath, MAX_PATH))
	{
		Log("GetCurDir error");

		DestroyEnvironmentBlock(pEnvBlock);
		CloseHandle(hToken2);
		CloseHandle(hToken);
		return FALSE;
	}

	wstring wsFullPath(wchFullPath);
	wsFullPath.append(L"\\Autoclicker GUI.exe");
	
	BOOL bSuccess = CreateProcessAsUserW(
		hToken2,           // client's access token
		wsFullPath.c_str(),     // file to execute
		NULL,              // command line
		NULL,              // pointer to process SECURITY_ATTRIBUTES
		NULL,              // pointer to thread SECURITY_ATTRIBUTES
		FALSE,             // handles are not inheritable
		NORMAL_PRIORITY_CLASS | CREATE_UNICODE_ENVIRONMENT,   // creation flags
		pEnvBlock,         // pointer to new environment block 
		NULL,              // name of current directory 
		&si,               // pointer to STARTUPINFO structure
		&pi                // receives information about new process
	);
	if (!bSuccess)
	{
		Log("CreateProcessAsUserW error");

		DestroyEnvironmentBlock(pEnvBlock);
		CloseHandle(hToken2);
		CloseHandle(hToken);
		return FALSE;
	}
	
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	DestroyEnvironmentBlock(pEnvBlock);
	CloseHandle(hToken2);
	CloseHandle(hToken);

	return TRUE;
}

//
// Purpose: 
//   The service code
//
// Parameters:
//   dwArgc - Number of arguments in the lpszArgv array
//   lpszArgv - Array of strings. The first string is the name of
//     the service and subsequent strings are passed by the process
//     that called the StartService function to start the service.
// 
// Return value:
//   None
//
VOID SvcInit(DWORD dwArgc, LPTSTR* lpszArgv)
{
	// TO_DO: Declare and set any required variables.
	//   Be sure to periodically call ReportSvcStatus() with 
	//   SERVICE_START_PENDING. If initialization fails, call
	//   ReportSvcStatus with SERVICE_STOPPED.

	// Create an event. The control handler function, SvcCtrlHandler,
	// signals this event when it receives the stop control code.

	ghSvcStopEvent = CreateEvent(
		NULL,    // default security attributes
		TRUE,    // manual reset event
		FALSE,   // not signaled
		NULL);   // no name

	if (ghSvcStopEvent == NULL)
	{
		ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
		return;
	}

	Log("Service started");

	// Report running status when initialization is complete.

	ReportSvcStatus(SERVICE_RUNNING, NO_ERROR, 0);
	Log("Service status changed to SERVICE_RUNNING");

	StartupWithToken();

	ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
	
	return;
}

//
// Purpose: 
//   Sets the current service status and reports it to the SCM.
//
// Parameters:
//   dwCurrentState - The current state (see SERVICE_STATUS)
//   dwWin32ExitCode - The system error code
//   dwWaitHint - Estimated time for pending operation, 
//     in milliseconds
// 
// Return value:
//   None
//
VOID ReportSvcStatus(DWORD dwCurrentState,
	DWORD dwWin32ExitCode,
	DWORD dwWaitHint)
{
	static DWORD dwCheckPoint = 1;

	// Fill in the SERVICE_STATUS structure.

	gSvcStatus.dwCurrentState = dwCurrentState;
	gSvcStatus.dwWin32ExitCode = dwWin32ExitCode;
	gSvcStatus.dwWaitHint = dwWaitHint;

	if (dwCurrentState == SERVICE_START_PENDING)
		gSvcStatus.dwControlsAccepted = 0;
	else gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

	if ((dwCurrentState == SERVICE_RUNNING) ||
		(dwCurrentState == SERVICE_STOPPED))
		gSvcStatus.dwCheckPoint = 0;
	else gSvcStatus.dwCheckPoint = dwCheckPoint++;

	// Report the status of the service to the SCM.
	SetServiceStatus(gSvcStatusHandle, &gSvcStatus);
}

//
// Purpose: 
//   Called by SCM whenever a control code is sent to the service
//   using the ControlService function.
//
// Parameters:
//   dwCtrl - control code
// 
// Return value:
//   None
//
VOID WINAPI SvcCtrlHandler(DWORD dwCtrl)
{
	// Handle the requested control code. 

	switch (dwCtrl)
	{
	case SERVICE_CONTROL_STOP:
		ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);

		// Signal the service to stop.

		SetEvent(ghSvcStopEvent);
		ReportSvcStatus(gSvcStatus.dwCurrentState, NO_ERROR, 0);

		return;

	case SERVICE_CONTROL_INTERROGATE:
		break;

	default:
		break;
	}

}

//
// Purpose: 
//   Logs messages to the event log
//
// Parameters:
//   szFunction - name of function that failed
// 
// Return value:
//   None
//
// Remarks:
//   The service must have an entry in the Application event log.
//
VOID SvcReportEvent(LPTSTR szFunction)
{
	HANDLE hEventSource;
	LPCWSTR lpszStrings[2];
	wchar_t Buffer[80];

	hEventSource = RegisterEventSourceW(NULL, SERVICE_INSTALL_NAME);

	if (NULL != hEventSource)
	{
		StringCchPrintfW(Buffer, 80, L"%s failed with %d", szFunction, GetLastError());

		lpszStrings[0] = SERVICE_INSTALL_NAME;
		lpszStrings[1] = Buffer;

		ReportEventW(hEventSource,        // event log handle
			EVENTLOG_ERROR_TYPE, // event type
			0,                   // event category
			SVC_ERROR,           // event identifier
			NULL,                // no security identifier
			2,                   // size of lpszStrings array
			0,                   // no binary data
			lpszStrings,         // array of strings
			NULL);               // no binary data

		DeregisterEventSource(hEventSource);
	}
}
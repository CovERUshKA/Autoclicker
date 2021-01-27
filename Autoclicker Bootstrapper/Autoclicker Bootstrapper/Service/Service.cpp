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
	BOOL bRet, bSuccess;
	DWORD dwBytesNeeded = NULL;
	DWORD dwResult = NULL;
	DWORD dwSessionId = NULL;

	DWORD dwUIAccess = 1;

	HANDLE hToken;
	HANDLE hToken2 = NULL;
	HANDLE hUserToken = NULL;
	LPVOID pEnvBlock = NULL;
	wstring wsFullPath;
	STARTUPINFOW si;
	wchar_t chWinStaName[] = L"winsta0\\default";
	wchar_t wchFullPath[MAX_PATH];
	PROCESS_INFORMATION pi{};

	bRet = FALSE;

	// idk if this is a good idea to use this, if you have an idea how to make exception handler better please create an issue
	try
	{
		if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken))
			throw exception("OpenProcessToken error");

		if (!WTSQueryUserToken(WTSGetActiveConsoleSessionId(), &hUserToken))
			throw exception("WTSQueryUserToken error");

		if (!DuplicateTokenEx(hToken, MAXIMUM_ALLOWED, NULL, SecurityIdentification, TokenPrimary, &hToken2))
			throw exception("DuplicateTokenEx error");

		if (!GetTokenInformation(hUserToken, TokenSessionId, &dwSessionId, sizeof(dwSessionId), &dwBytesNeeded))
			throw exception("GetTokenInformation error");
		if (!dwSessionId)
			throw exception("TokenSessionId error");

		if (!SetTokenInformation(hToken2, TokenSessionId, &dwSessionId, sizeof(dwSessionId)))
			throw exception("SetTokenInformation error");

		if (!SetTokenInformation(hToken2, TokenUIAccess, &dwUIAccess, sizeof(dwUIAccess)))
			throw exception("SetTokenInformation error");

		if (!CreateEnvironmentBlock(&pEnvBlock, hToken2, FALSE))
			throw exception("CreateEnvironmentBlock error");

		ZeroMemory(&si, sizeof(STARTUPINFO));
		si.cb = sizeof(STARTUPINFO);
		si.lpDesktop = chWinStaName;

		ZeroMemory(&pi, sizeof(pi));

		if (!GetCurDir(wchFullPath, MAX_PATH))
			throw exception("GetCurDir error");

		wsFullPath.assign(wchFullPath);
		wsFullPath.append(L"\\Autoclicker GUI.exe");

		bSuccess = CreateProcessAsUserW(
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
			throw exception("CreateProcessAsUserW error");

		bRet = TRUE;
	}
	catch (const std::exception& e)
	{
		Log(e.what());
	}

	if (pi.hProcess) CloseHandle(pi.hProcess);
	if (pi.hThread) CloseHandle(pi.hThread);
	if (pEnvBlock) DestroyEnvironmentBlock(pEnvBlock);
	if (hUserToken) CloseHandle(hUserToken);
	if (hToken2) CloseHandle(hToken2);
	if (hToken) CloseHandle(hToken);

	return bRet;
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
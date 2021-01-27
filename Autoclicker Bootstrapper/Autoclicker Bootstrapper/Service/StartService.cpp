#include "StartService.hpp"

BOOL CheckSvc(SC_HANDLE schSCManager), InstallSvc(SC_HANDLE schSCManager);

BOOL StartupSvc()
{
	BOOL bRet;
	SC_HANDLE schSCManager;
	SC_HANDLE schService = 0;
	SERVICE_STATUS_PROCESS ssStatus;
	ULONGLONG dwStartTickCount;
	DWORD dwBytesNeeded, dwWaitTime, dwOldCheckPoint;

	bRet = FALSE;

	try
	{
		// Get a handle to the SCM database.
		schSCManager = OpenSCManager(
			NULL,                    // local computer
			NULL,                    // servicesActive database 
			SC_MANAGER_ALL_ACCESS);  // full access rights 

		if (!schSCManager)
			throw exception("OpenSCManager failed");

		Log("SCManager opened");

		// Get a handle to the service.

		schService = OpenService(
			schSCManager,         // SCM database 
			SERVICE_INSTALL_NAME, // name of service 
			SERVICE_ALL_ACCESS);  // full access 

		if (!schService)
		{
			DWORD lastError = GetLastError();

			switch (lastError)
			{
			case ERROR_SERVICE_DOES_NOT_EXIST:
			{
				Log("Service not exists");
				
				if (!InstallSvc(schSCManager))
					throw NULL; // Here i just need to get out of try{}, please help me how to improve this
			}
			break;
			default:
				throw exception("OpenService failed");
			}
		}
		else
		{
			Log("Service opened");

			if (!CheckSvc(schService))
				throw NULL; // Here i just need to get out of try{}, please help me how to improve this
		}

		// Check the status in case the service is not stopped. 
		if (!QueryServiceStatusEx(
			schService,                       // handle to service 
			SC_STATUS_PROCESS_INFO,           // information level
			(LPBYTE)&ssStatus,                // address of structure
			sizeof(SERVICE_STATUS_PROCESS),   // size of structure
			&dwBytesNeeded))                  // size needed if buffer is too small
			throw exception("QueryServiceStatusEx failed");

		// Check if the service is already running. It would be possible 
		// to stop the service here, but for simplicity this example just returns. 

		if (ssStatus.dwCurrentState != SERVICE_STOPPED && ssStatus.dwCurrentState != SERVICE_STOP_PENDING)
			throw exception("Service already running");

		// Save the tick count and initial checkpoint.

		dwStartTickCount = GetTickCount64();
		dwOldCheckPoint = ssStatus.dwCheckPoint;

		// Wait for the service to stop before attempting to start it.

		while (ssStatus.dwCurrentState == SERVICE_STOP_PENDING)
		{
			// Do not wait longer than the wait hint. A good interval is 
			// one-tenth of the wait hint but not less than 1 second  
			// and not more than 10 seconds. 

			dwWaitTime = ssStatus.dwWaitHint / 10;

			if (dwWaitTime < 1000)
				dwWaitTime = 1000;
			else if (dwWaitTime > 10000)
				dwWaitTime = 10000;

			Sleep(dwWaitTime);

			// Check the status until the service is no longer stop pending. 

			if (!QueryServiceStatusEx(
				schService,                     // handle to service 
				SC_STATUS_PROCESS_INFO,         // information level
				(LPBYTE)&ssStatus,             // address of structure
				sizeof(SERVICE_STATUS_PROCESS), // size of structure
				&dwBytesNeeded))              // size needed if buffer is too small
				throw exception("QueryServiceStatusEx failed");

			if (ssStatus.dwCheckPoint > dwOldCheckPoint)
			{
				// Continue to wait and check.

				dwStartTickCount = GetTickCount64();
				dwOldCheckPoint = ssStatus.dwCheckPoint;
			}
			else
			{
				if (GetTickCount64() - dwStartTickCount > ssStatus.dwWaitHint)
					throw exception("Timeout waiting for service to stop");
			}
		}

		Log("Starting service...");

		// Attempt to start the service.

		if (!StartService(
			schService,  // handle to service 
			0,           // number of arguments 
			NULL))      // no arguments 
			throw exception("StartService failed");

		bRet = TRUE;
	}
	catch (const std::exception& e)
	{
		Log(e.what());
	}
	catch (...) {}

	if (schService) CloseServiceHandle(schService);
	if (schSCManager) CloseServiceHandle(schService);

	return bRet;
}

BOOL InstallSvc(SC_HANDLE schSCManager)
{
	BOOL bRet;
	SC_HANDLE schService = 0;
	TCHAR szPath[MAX_PATH];

	bRet = FALSE;
	
	try
	{
		Log("Installing service...");

		// Get full path of the current process and put it into the szPath variable
		if (!GetModuleFileNameW(NULL, szPath, MAX_PATH))
			throw exception("GetModuleFileName failed");

		// Create the service
		schService = CreateServiceW(
			schSCManager,              // SCM database
			SERVICE_INSTALL_NAME,                   // name of service
			SERVICE_DISPLAY_NAME,                   // service name to display
			SERVICE_ALL_ACCESS,        // desired access
			SERVICE_WIN32_OWN_PROCESS, // service type
			SERVICE_DEMAND_START,      // start type
			SERVICE_ERROR_NORMAL,      // error control type
			szPath,                    // path to service's binary
			NULL,                      // no load ordering group
			NULL,                      // no tag identifier
			NULL,                      // no dependencies
			NULL,                      // LocalSystem account
			NULL);                     // no password

		if (schService == NULL)
			throw exception("CreateService failed");

		Log("Service installed");

		bRet = TRUE;
	}
	catch (const std::exception& e)
	{
		// Log the exception text
		Log(e.what());
	}

	// Close all handles
	if (schService) CloseServiceHandle(schService);
	if (schSCManager) CloseServiceHandle(schSCManager);

	return bRet;
}

BOOL CheckSvc(SC_HANDLE schService)
{
	BOOL bRet;
	DWORD dwError, dwBytesNeeded;
	LPQUERY_SERVICE_CONFIG lpConfig = 0;
	LSTATUS lStatus;

	wchar_t curPath[MAX_PATH];
	ZeroMemory(curPath, sizeof(curPath));

	bRet = FALSE;

	try
	{
		if (!schService)
			throw exception("Service handle is empty");

		Log("Service executable path checking...");

		if (!QueryServiceConfigW(
			schService,
			NULL,
			NULL,
			&dwBytesNeeded))
		{
			dwError = GetLastError();
			if (dwError == ERROR_INSUFFICIENT_BUFFER)
			{
				lpConfig = (LPQUERY_SERVICE_CONFIG)LocalAlloc(LMEM_FIXED, dwBytesNeeded);
				if (!lpConfig)
					throw exception("Unable to allocate memory for LPQUERY_SERVICE_CONFIG");

				if (!QueryServiceConfigW(
					schService,
					lpConfig,
					dwBytesNeeded,
					&dwBytesNeeded))
					throw exception("Unable to QueryServiceConfig");
			}
			else
				throw exception("Unable to QueryServiceConfig bytes needed");
		}
		else
			throw exception("Unable to QueryServiceConfig bytes needed");

		if (!GetModuleFileNameExW(GetCurrentProcess(), NULL, curPath, MAX_PATH))
			throw exception("Unable to GetModuleFileNameExW");

		if (wcscmp(curPath, lpConfig->lpBinaryPathName) != NULL)
		{
			Log("Executable path incorrect");

			Log("Executable path changing...");

			lStatus = RegSetKeyValueW(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\AutoclickerBootstrapper", L"ImagePath", REG_MULTI_SZ, curPath, sizeof(curPath));
			if (lStatus != ERROR_SUCCESS)
				throw exception("Unable to RegSetKeyValueW");

			Log("Executable path changed");
		}
		else
			Log("Executable path correct");

		bRet = TRUE;
	}
	catch (const std::exception& e)
	{
		Log(e.what());
	}

	LocalFree(lpConfig);

	return bRet;
}
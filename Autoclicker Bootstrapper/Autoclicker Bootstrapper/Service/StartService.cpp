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

	// Get a handle to the SCM database.

	schSCManager = OpenSCManager(
		NULL,                    // local computer
		NULL,                    // servicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 

	if (!schSCManager)
	{
		Log("OpenSCManager failed");

		goto end;
	}

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
				goto end;
		}
			break;
		default:
			Log("OpenService failed");
			break;
		}
	}
	else
	{
		Log("Service opened");

		if (!CheckSvc(schService))
			goto end;
	}

	// Check the status in case the service is not stopped. 
	if (!QueryServiceStatusEx(
		schService,                       // handle to service 
		SC_STATUS_PROCESS_INFO,           // information level
		(LPBYTE)&ssStatus,                // address of structure
		sizeof(SERVICE_STATUS_PROCESS),   // size of structure
		&dwBytesNeeded))                  // size needed if buffer is too small
	{
		Log("QueryServiceStatusEx failed");

		goto end;
	}

	// Check if the service is already running. It would be possible 
	// to stop the service here, but for simplicity this example just returns. 

	if (ssStatus.dwCurrentState != SERVICE_STOPPED && ssStatus.dwCurrentState != SERVICE_STOP_PENDING)
	{
		Log("Service already running");

		goto end;
	}

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
		{
			Log("QueryServiceStatusEx failed");

			goto end;
		}

		if (ssStatus.dwCheckPoint > dwOldCheckPoint)
		{
			// Continue to wait and check.

			dwStartTickCount = GetTickCount64();
			dwOldCheckPoint = ssStatus.dwCheckPoint;
		}
		else
		{
			if (GetTickCount64() - dwStartTickCount > ssStatus.dwWaitHint)
			{
				Log("Timeout waiting for service to stop");

				goto end;
			}
		}
	}

	Log("Starting service...");

	// Attempt to start the service.

	if (!StartService(
		schService,  // handle to service 
		0,           // number of arguments 
		NULL))      // no arguments 
	{
		Log("StartService failed");

		goto end;
	}

	bRet = TRUE;
end:
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

	Log("Installing service...");

	if (!GetModuleFileNameW(NULL, szPath, MAX_PATH))
	{
		Log("GetModuleFileName failed");

		goto end;
	}

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
	{
		Log("CreateService failed");

		goto end;
	}

	Log("Service installed");

	bRet = TRUE;
end:
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

	bRet = FALSE;

	if (!schService)
		goto end;

	// Get a handle to the service.

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
			{
				Log("Unable to allocate memory for LPQUERY_SERVICE_CONFIG");

				goto end;
			}

			if (!QueryServiceConfigW(
				schService,
				lpConfig,
				dwBytesNeeded,
				&dwBytesNeeded))
			{
				Log("Unable to QueryServiceConfig");

				goto end;
			}
		}
		else
		{
			Log("Unable to QueryServiceConfig bytes needed");

			goto end;
		}
	}
	else
	{
		Log("Unable to QueryServiceConfig bytes needed");

		goto end;
	}

	wchar_t curPath[MAX_PATH];

	if (!GetModuleFileNameExW(GetCurrentProcess(), NULL, curPath, MAX_PATH))
	{
		Log("Unable to GetModuleFileNameExW");

		goto end;
	}

	if (wcscmp(curPath, lpConfig->lpBinaryPathName) != NULL)
	{
		Log("Executable path incorrect");

		Log("Executable path changing...");

		lStatus = RegSetKeyValueW(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\AutoclickerBootstrapper", L"ImagePath", REG_MULTI_SZ, curPath, sizeof(curPath));
		if (lStatus != ERROR_SUCCESS)
		{
			Log("Unable to RegSetKeyValueW");

			goto end;
		}

		Log("Executable path changed");
	}
	else
		Log("Executable path correct");

	bRet = TRUE;
end:
	LocalFree(lpConfig);

	return bRet;
}
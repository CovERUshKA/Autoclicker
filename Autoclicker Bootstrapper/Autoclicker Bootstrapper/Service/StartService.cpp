#include "StartService.hpp"

BOOL CheckSvc();

BOOL StartupSvc()
{
	if (!CheckSvc())
		return FALSE;

	SC_HANDLE schSCManager;
	SC_HANDLE schService;
	SERVICE_STATUS_PROCESS ssStatus;
	ULONGLONG dwStartTickCount;
	DWORD dwBytesNeeded, dwWaitTime, dwOldCheckPoint;

	// Get a handle to the SCM database.

	schSCManager = OpenSCManager(
		NULL,                    // local computer
		NULL,                    // servicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 

	if (!schSCManager)
	{
		Log("OpenSCManager failed");
		return FALSE;
	}

	Log("SCManager opened");

	// Get a handle to the service.

	schService = OpenService(
		schSCManager,         // SCM database 
		SERVICE_INSTALL_NAME, // name of service 
		SERVICE_ALL_ACCESS);  // full access 

	if (!schService)
	{
		Log("OpenService failed");
		CloseServiceHandle(schSCManager);
		return FALSE;
	}

	Log("Service opened");

	// Check the status in case the service is not stopped. 
	if (!QueryServiceStatusEx(
		schService,                       // handle to service 
		SC_STATUS_PROCESS_INFO,           // information level
		(LPBYTE)&ssStatus,                // address of structure
		sizeof(SERVICE_STATUS_PROCESS),   // size of structure
		&dwBytesNeeded))                  // size needed if buffer is too small
	{
		Log("QueryServiceStatusEx failed");
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		return FALSE;
	}

	// Check if the service is already running. It would be possible 
	// to stop the service here, but for simplicity this example just returns. 

	if (ssStatus.dwCurrentState != SERVICE_STOPPED && ssStatus.dwCurrentState != SERVICE_STOP_PENDING)
	{
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		return TRUE;
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
			CloseServiceHandle(schService);
			CloseServiceHandle(schSCManager);
			return FALSE;
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
				CloseServiceHandle(schService);
				CloseServiceHandle(schSCManager);
				return FALSE;
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
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		return FALSE;
	}

	// Check the status until the service is no longer start pending. 

	if (!QueryServiceStatusEx(
		schService,                     // handle to service 
		SC_STATUS_PROCESS_INFO,         // info level
		(LPBYTE)&ssStatus,             // address of structure
		sizeof(SERVICE_STATUS_PROCESS), // size of structure
		&dwBytesNeeded))              // if buffer too small
	{
		Log("QueryServiceStatusEx failed");
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		return FALSE;
	}

	// Save the tick count and initial checkpoint.

	dwStartTickCount = GetTickCount64();
	dwOldCheckPoint = ssStatus.dwCheckPoint;

	while (ssStatus.dwCurrentState == SERVICE_START_PENDING)
	{
		// Do not wait longer than the wait hint. A good interval is 
		// one-tenth the wait hint, but no less than 1 second and no 
		// more than 10 seconds. 

		dwWaitTime = ssStatus.dwWaitHint / 10;

		if (dwWaitTime < 1000)
			dwWaitTime = 1000;
		else if (dwWaitTime > 10000)
			dwWaitTime = 10000;

		Sleep(dwWaitTime);

		// Check the status again. 

		if (!QueryServiceStatusEx(
			schService,             // handle to service 
			SC_STATUS_PROCESS_INFO, // info level
			(LPBYTE)&ssStatus,             // address of structure
			sizeof(SERVICE_STATUS_PROCESS), // size of structure
			&dwBytesNeeded))              // if buffer too small
		{
			Log("QueryServiceStatusEx failed");
			break;
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
				// No progress made within the wait hint.
				break;
			}
		}
	}

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);

	return TRUE;
}

BOOL InstallSvc(SC_HANDLE schSCManager)
{
	SC_HANDLE schService;
	TCHAR szPath[MAX_PATH];

	Log("Installing service...");

	if (!GetModuleFileNameW(NULL, szPath, MAX_PATH))
	{
		Log("GetModuleFileName failed");
		return FALSE;
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
		CloseServiceHandle(schSCManager);
		return FALSE;
	}

	Log("Service installed");

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);

	return TRUE;
}

BOOL CheckSvc()
{
	SC_HANDLE schSCManager;
	SC_HANDLE schService;
	DWORD dwError, dwBytesNeeded;
	LPQUERY_SERVICE_CONFIG lpConfig;
	LSTATUS lStatus;

	Log("Service checking...");

	// Get a handle to the SCM database. 

	schSCManager = OpenSCManager(
		NULL,                    // local computer
		NULL,                    // ServicesActive database 
		STANDARD_RIGHTS_EXECUTE);  // full access rights 

	if (!schSCManager)
	{
		Log("OpenSCManager failed");
		return FALSE;
	}

	// Get a handle to the service.

	schService = OpenServiceW(
		schSCManager,          // SCM database
		SERVICE_INSTALL_NAME,  // name of service
		SERVICE_ALL_ACCESS);   // need query config access

	if (!schService)
	{
		Log("Service not exists");

		if (!InstallSvc(schSCManager))
		{
			CloseServiceHandle(schSCManager);
			return FALSE;
		}
	}
	else
	{
		Log("Service exists");

		Log("Executable path checking...");

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
					CloseServiceHandle(schService);
					CloseServiceHandle(schSCManager);
					return FALSE;
				}

				if (!QueryServiceConfigW(
					schService,
					lpConfig,
					dwBytesNeeded,
					&dwBytesNeeded))
				{
					Log("Unable to QueryServiceConfig");
					LocalFree(lpConfig);
					CloseServiceHandle(schService);
					CloseServiceHandle(schSCManager);
					return FALSE;
				}
			}
			else
			{
				Log("Unable to QueryServiceConfig bytes needed");
				CloseServiceHandle(schService);
				CloseServiceHandle(schSCManager);
				return FALSE;
			}
		}
		else
		{
			Log("Unable to QueryServiceConfig bytes needed");
			CloseServiceHandle(schService);
			CloseServiceHandle(schSCManager);
			return FALSE;
		}

		wchar_t curPath[MAX_PATH];

		if (!GetModuleFileNameExW(GetCurrentProcess(), NULL, curPath, MAX_PATH))
		{
			Log("Unable to GetModuleFileNameExA");
			LocalFree(lpConfig);
			return FALSE;
		}

		if (wcscmp(curPath, lpConfig->lpBinaryPathName) != NULL)
		{
			Log("Executable path incorrect");

			Log("Executable path changing...");

			lStatus = RegSetKeyValueW(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\AutoclickerBootstrapper", L"ImagePath", REG_MULTI_SZ, curPath, sizeof(curPath));
			if (lStatus != ERROR_SUCCESS)
			{
				Log("Unable to RegSetKeyValueW");
				LocalFree(lpConfig);
				return FALSE;
			}

			Log("Executable path changed");
		}
		else
			Log("Executable path correct");

		LocalFree(lpConfig);
	}

	Log("Service ready for work");

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);

	return TRUE;
}
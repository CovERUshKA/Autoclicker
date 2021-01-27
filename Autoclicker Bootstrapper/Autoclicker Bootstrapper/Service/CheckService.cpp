#include "CheckService.hpp"

BOOL
WINAPI
LaunchedAsApplication(DWORD* pdwIsApplication)
{
	BOOL bRet;
	DWORD dwBytesNeeded = NULL;
	DWORD dwSessionId = NULL;

	HANDLE hToken;

	bRet = FALSE;

	try
	{
		if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken))
			throw exception("OpenProcessToken error");

		if (!GetTokenInformation(hToken, TokenSessionId, &dwSessionId, sizeof(dwSessionId), &dwBytesNeeded))
			throw exception("GetTokenInformation error");

		*pdwIsApplication = dwSessionId;

		bRet = TRUE;
	}
	catch (const std::exception& e)
	{
		Log(e.what());
	}

	if (hToken) CloseHandle(hToken);

	return bRet;
}
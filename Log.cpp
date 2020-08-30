#include "Log.hpp"

void LogTime(wofstream* log, DWORD lastError)
{
	time_t _time = time(NULL);

	tm ltm;
	ZeroMemory(&ltm, sizeof(ltm));

	localtime_s(&ltm, &_time);

	*log << L"[";

	if (ltm.tm_mday < 10)
		*log << L"0";

	*log << ltm.tm_mday;

	*log << L".";

	if (1 + ltm.tm_mon < 10)
		*log << L"0";

	*log << 1 + ltm.tm_mon;

	*log << L".";

	*log << 1900 + ltm.tm_year;

	*log << L" ";

	if (ltm.tm_hour < 10)
		*log << L"0";

	*log << ltm.tm_hour;

	*log << L":";

	if (ltm.tm_min < 10)
		*log << L"0";

	*log << ltm.tm_min;

	*log << L":";

	if (ltm.tm_sec < 10)
		*log << L"0";

	*log << ltm.tm_sec;

	*log << L" LastError: ";

	*log << lastError;

	if (lastError)
	{
		*log << L"(";

		// Retrieve the system error message for the last-error code

		LPVOID lpMsgBuf;

		FormatMessageW(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			lastError,
			MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
			(LPWSTR)&lpMsgBuf,
			0, NULL);

		for (size_t i = 0; i < LocalSize(lpMsgBuf); i++)
			if (((LPWSTR)lpMsgBuf)[i] == L'\n'
				|| ((LPWSTR)lpMsgBuf)[i] == L'\r')
				((LPWSTR)lpMsgBuf)[i] = 0x00;

		*log << (LPWSTR)lpMsgBuf << L")";

		LocalFree(lpMsgBuf);
	}

	*log << L"] ";

	return;
}

void Log(const char* text)
{
	size_t nocConverted;

	wchar_t* pwText = new wchar_t[strlen(text) + 1];
	ZeroMemory(pwText, sizeof(pwText));

	mbstowcs_s(&nocConverted, pwText, strlen(text) + 1, text, strlen(text));

	Log(pwText);

	delete[] pwText;

	return;
}

void Log(const wchar_t* text)
{
	DWORD lastError = GetLastError();

	wchar_t path[MAX_PATH];
	if (!GetCurDir(path, MAX_PATH))
		return;

	std::wstring fullpath(path);
	fullpath.append(L"\\error.log");

	std::wofstream log(fullpath.c_str(), std::ios::app);

	if (!log.is_open())
		return;

	LogTime(&log, lastError);

	log << text;

	log << L"\n";

	log.close();

	return;
}
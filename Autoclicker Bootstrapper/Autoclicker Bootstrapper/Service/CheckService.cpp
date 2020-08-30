#include "CheckService.hpp"

BOOL          getParentProcessName(wchar_t* name);
BOOL WINAPI   getSelfProcessEntry(PROCESSENTRY32& entry);
BOOL WINAPI   getParentProcessEntry(PROCESSENTRY32& entry);
BOOL WINAPI   getProcessEntry(PROCESSENTRY32& entry, DWORD processID);

BOOL
WINAPI
wasLaunchedAsService()
{
	wchar_t name[MAX_PATH];
	if (!getParentProcessName(name)) {
		return false;
	}

	return wcscmp(name, SERVICE_LAUNCHER) == 0;
}

BOOL
getParentProcessName(wchar_t* name)
{
	PROCESSENTRY32 parentEntry;
	if (!getParentProcessEntry(parentEntry)) {
		return false;
	}
	wcscpy_s(name, MAX_PATH, parentEntry.szExeFile);
	return true;
}

BOOL
WINAPI
getSelfProcessEntry(PROCESSENTRY32& entry)
{
	// get entry from current PID
	return getProcessEntry(entry, GetCurrentProcessId());
}

BOOL
WINAPI
getParentProcessEntry(PROCESSENTRY32& entry)
{
	// get the current process, so we can get parent PID
	PROCESSENTRY32 selfEntry;
	if (!getSelfProcessEntry(selfEntry)) {
		return FALSE;
	}

	// get entry from parent PID
	return getProcessEntry(entry, selfEntry.th32ParentProcessID);
}

BOOL
WINAPI
getProcessEntry(PROCESSENTRY32& entry, DWORD processID)
{
	// first we need to take a snapshot of the running processes
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snapshot == INVALID_HANDLE_VALUE) {
		Log("could not get process snapshot");
		return FALSE;
	}

	entry.dwSize = sizeof(PROCESSENTRY32);

	// get the first process, and if we can't do that then it 
	// unlikely we can go any further
	BOOL gotEntry = Process32First(snapshot, &entry);
	if (!gotEntry) {
		Log("could not get first process entry");
		return FALSE;
	}

	while (gotEntry) {

		if (entry.th32ProcessID == processID) {
			// found current process
			return TRUE;
		}

		// now move on to the next entry (when we reach end, loop will stop)
		gotEntry = Process32Next(snapshot, &entry);
	}

	return FALSE;
}
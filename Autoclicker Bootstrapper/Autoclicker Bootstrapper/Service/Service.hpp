#pragma once
#include <Windows.h>
#include <wtsapi32.h>
#include <userenv.h>
#include <sddl.h>

#include "../../../Additional.h"
#include "CheckService.hpp"
#include "StartService.hpp"
#include "../Tools.hpp"

VOID WINAPI SvcMain(DWORD dwArgc, LPTSTR* lpszArgv);
VOID SvcReportEvent(LPTSTR szFunction);

using namespace std;
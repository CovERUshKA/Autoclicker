#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include "../../../Log.hpp"

#define SERVICE_LAUNCHER L"services.exe"

BOOL WINAPI wasLaunchedAsService();
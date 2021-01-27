#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include "../../../Log.hpp"

BOOL WINAPI LaunchedAsApplication(DWORD* pdwSessionId);
#pragma once
#include <Windows.h>
#include <fstream>
#include <string>
#include <Psapi.h>
#include <Shlwapi.h>
#include <time.h>

#include "Additional.h"

void Log(const char* text);
void Log(const wchar_t* text);

using namespace std;
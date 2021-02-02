#pragma once

#include <Windows.h>

#include "../GUI/Memory/Memory.hpp"
#include "../Tools.hpp"

enum ClickMethod
{
	CM_SendInput,
	CM_SendMessage
};

enum ClickRepeat
{
	CR_Once,
	CR_Count,
	CR_Toggle,
	CR_WhileHolding
};

struct ExecuteInfo
{
	CHAR sButton = 0;

	CHAR key = 0;

	LPWCH wchMacrosName;

	ClickMethod clickMethod;

	ClickRepeat	repeatMethod;

	DWORD dwRepeat;
};

class Autoclicker
{
public:
	Autoclicker();
	~Autoclicker();

	BOOL Add(ExecuteInfo eInfo);
	BOOL Remove(DWORD number);

private:
	DWORD cActives;

	ExecuteInfo* lpExecuteInfo;
	HANDLE* hThreads;
};

extern Autoclicker autoclicker;
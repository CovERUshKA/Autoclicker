#pragma once

#include <Windows.h>
#include "../../Tools.hpp"

class Memory
{
public:
	Memory();
	~Memory();

	LPVOID Alloc(DWORD dwBytes);
	LPVOID ReAlloc(LPVOID lpMem, DWORD dwBytes);

	BOOL Validate(LPVOID lpMem);

	BOOL Free(LPVOID lpMem);

private:
	HANDLE hHeap;
};

extern Memory memory;
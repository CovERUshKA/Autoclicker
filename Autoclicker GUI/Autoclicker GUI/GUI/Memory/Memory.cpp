#include "Memory.hpp"

LPVOID Memory::Alloc(DWORD dwBytes)
{
	return HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwBytes);
}

LPVOID Memory::ReAlloc(LPVOID lpMem, DWORD dwBytes)
{
	return HeapReAlloc(hHeap, HEAP_ZERO_MEMORY, lpMem, dwBytes);
}

BOOL Memory::Validate(LPVOID lpMem)
{
	return HeapValidate(hHeap, 0, lpMem);
}

BOOL Memory::Free(LPVOID lpMem)
{
	return HeapFree(hHeap, 0, lpMem);
}

Memory::Memory()
{
	hHeap = GetProcessHeap();
	if (!hHeap)
		Log("Unable to create heap");
}

Memory::~Memory()
{
	HeapDestroy(hHeap);
}
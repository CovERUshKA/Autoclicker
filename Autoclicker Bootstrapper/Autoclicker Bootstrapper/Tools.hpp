#pragma once

#include <string>
#include <TlHelp32.h>
#include <tchar.h>
#include <strsafe.h>
#include <Psapi.h>
#include "../../Log.hpp"
#include "Def.hpp"

#define ReCa reinterpret_cast
#define StCa static_cast

#ifndef TOOLS_H
#define TOOLS_H

inline BOOL GetProcessBinaryType(DWORD PID, DWORD* buf)
{
	*buf = 0;

	HANDLE handle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, PID);
	if (handle == NULL)
	{
		Log("Unable to open process");
		return FALSE;
	}

	char exepath[MAX_PATH];
	ZeroMemory(exepath, MAX_PATH);

	if (K32GetModuleFileNameExA(handle, NULL, exepath, MAX_PATH) == NULL)
	{
		CloseHandle(handle);
		return FALSE;
	}

	DWORD dwBinaryType = 0;

	if (!GetBinaryTypeA(exepath, &dwBinaryType))
	{
		CloseHandle(handle);
		return FALSE;
	}

	if (dwBinaryType == SCS_64BIT_BINARY)
	{
		*buf = 64;
		CloseHandle(handle);
		return TRUE;
	}
	else if (dwBinaryType == SCS_32BIT_BINARY)
	{
		*buf = 86;
		CloseHandle(handle);
		return TRUE;
	}
	CloseHandle(handle);
	return FALSE;
}

inline BOOL CompareProcessBinaryTypes(DWORD PID, DWORD SubPID)
{
	DWORD Binary, subBinary;

	if (!GetProcessBinaryType(PID, &Binary) || !GetProcessBinaryType(SubPID, &subBinary))
		return FALSE;

	if (Binary == subBinary && Binary != NULL)
		return TRUE;
	else
		return FALSE;
}

inline HINSTANCE GetDllInProcess(HANDLE hTargetProc, const wchar_t* dllName)
{
	HMODULE hMod[1024];
	DWORD cbNeeded;
	wchar_t nameofDLL[MAX_PATH];

	if (K32EnumProcessModules(hTargetProc, hMod, sizeof(hMod),
		&cbNeeded) != NULL)
	{
		for (int i = 0; i < (sizeof(hMod) / sizeof(HMODULE)); i++)
		{
			if (K32GetModuleBaseNameW(hTargetProc, hMod[i], nameofDLL, MAX_PATH) != NULL)
			{
				if (wcscmp(nameofDLL, dllName) == NULL)
				{
					return hMod[i];
				}
			}
		}
	}
	else
	{
		Log("Unable to enum process modules");
		return FALSE;
	}

	// Release the handle to the process.

	return FALSE;
}

inline bool GetProcAddressEx(HANDLE hTargetProc, HINSTANCE hModule, const char* szProcName, void*& pOut);

inline bool GetProcAddressEx(HANDLE hTargetProc, const TCHAR* szModuleName, const char* szProcName, void*& pOut)
{
	return GetProcAddressEx(hTargetProc, GetDllInProcess(hTargetProc, szModuleName), szProcName, pOut);
}

inline bool GetProcAddressEx(HANDLE hTargetProc, HINSTANCE hModule, const char* szProcName, void*& pOut)
{
	BYTE* modBase = ReCa<BYTE*>(hModule);

	if (!modBase)
	{
		Log("Unable to get mod Base");
		return false;
	}

	BYTE* pe_header = new BYTE[0x1000];
	if (!pe_header)
	{
		Log("Unable to allocate 1000 BYTE");
		return false;
	}

	if (!ReadProcessMemory(hTargetProc, modBase, pe_header, 0x1000, nullptr))
	{
		Log("Unable to read header of modBase");
		delete[] pe_header;

		return false;
	}

	auto* pNT = ReCa<IMAGE_NT_HEADERS*>(pe_header + ReCa<IMAGE_DOS_HEADER*>(pe_header)->e_lfanew);
	auto* pExportEntry = &pNT->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
	auto ExportSize = pExportEntry->Size;
	auto ExportDirRVA = pExportEntry->VirtualAddress;

	if (!ExportSize)
	{
		Log("Unable to get export size");
		delete[] pe_header;

		return false;
	}

	BYTE* export_data = new BYTE[ExportSize];
	if (!export_data)
	{
		Log("Unable to get export data");
		delete[] pe_header;

		return false;
	}

	if (!ReadProcessMemory(hTargetProc, modBase + ExportDirRVA, export_data, ExportSize, nullptr))
	{
		Log("Unable to read process memory");
		delete[] export_data;
		delete[] pe_header;

		return false;
	}

	BYTE* localBase = export_data - ExportDirRVA;
	auto pExportDir = ReCa<IMAGE_EXPORT_DIRECTORY*>(export_data);

	auto Forward = [&](DWORD FuncRVA) -> bool
	{
		char pFullExport[MAX_PATH]{ 0 };
		size_t len_out = 0;

		StringCchLengthA(ReCa<char*>(localBase + FuncRVA), sizeof(pFullExport), &len_out);
		if (!len_out)
			return false;

		memcpy(pFullExport, ReCa<char*>(localBase + FuncRVA), len_out);

		char* pFuncName = strchr(pFullExport, '.');
		*(pFuncName++) = '\0';
		if (*pFuncName == '#')
			pFuncName = ReCa<char*>(LOWORD(atoi(++pFuncName)));

#ifdef UNICODE

		TCHAR ModNameW[MAX_PATH + 1]{ 0 };
		size_t SizeOut = 0;

		if (mbstowcs_s(&SizeOut, ModNameW, pFullExport, MAX_PATH))
			return GetProcAddressEx(hTargetProc, ModNameW, pFuncName, pOut);
		else
			return false;

#else

		return GetProcAddressEx(hTargetProc, pFullExport, pFuncName, pOut);

#endif
	};

	if ((ReCa<ULONG_PTR>(szProcName) & 0xFFFFFF) <= MAXWORD)
	{
		WORD Base = LOWORD(pExportDir->Base - 1);
		WORD Ordinal = LOWORD(szProcName) - Base;
		DWORD FuncRVA = ReCa<DWORD*>(localBase + pExportDir->AddressOfFunctions)[Ordinal];

		delete[] export_data;
		delete[] pe_header;

		if (FuncRVA >= ExportDirRVA && FuncRVA < ExportDirRVA + ExportSize)
		{
			return Forward(FuncRVA);
		}

		return modBase + FuncRVA;
	}

	DWORD max = pExportDir->NumberOfNames - 1;
	DWORD min = 0;
	DWORD FuncRVA = 0;

	while (min <= max)
	{
		DWORD mid = (min + max) / 2;

		DWORD CurrNameRVA = ReCa<DWORD*>(localBase + pExportDir->AddressOfNames)[mid];
		char* szName = ReCa<char*>(localBase + CurrNameRVA);

		int cmp = strcmp(szName, szProcName);
		if (cmp < 0)
			min = mid + 1;
		else if (cmp > 0)
			max = mid - 1;
		else
		{
			WORD Ordinal = ReCa<WORD*>(localBase + pExportDir->AddressOfNameOrdinals)[mid];
			FuncRVA = ReCa<DWORD*>(localBase + pExportDir->AddressOfFunctions)[Ordinal];

			break;
		}
	}

	delete[] export_data;
	delete[] pe_header;

	if (!FuncRVA)
	{
		Log("Unable to get FuncRVA");
		return false;
	}

	if (FuncRVA >= ExportDirRVA && FuncRVA < ExportDirRVA + ExportSize)
	{
		return Forward(FuncRVA);
	}

	pOut = modBase + FuncRVA;

	return true;
}

#endif
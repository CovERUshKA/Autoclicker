#pragma once

#include <Windows.h>
#include <string>

class Element
{
public:
	INT elementID;
	bool visible;
	INT ID;

	FLOAT x, y;
	FLOAT width, height;

	UINT params = 0;

	std::wstring wchElementName;
	UINT strLength;

	Element();
};
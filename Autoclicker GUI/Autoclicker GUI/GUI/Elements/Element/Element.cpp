#include "Element.hpp"

void Element::Init(COGUI_ElementCreateStruct createStruct)
{
	ID = createStruct.ID;
	x = createStruct.x;
	y = createStruct.y;
	width = createStruct.nWidth;
	height = createStruct.nHeight;
	params = createStruct.uiParams;
	wchElementName = createStruct.lpElementName;
}

Element::Element()
{
	type = -1;
	visible = true;
	ID = -1;

	x = y = 0;
	width = height = 0;

	wchElementName = L"";
}
#include "IO.hpp"

BOOL IO::CursorInField(Vector2D startPos, Vector2D endPos)
{
	if (mousePos.x >= startPos.x
		&& mousePos.x < endPos.x
		&& mousePos.y >= startPos.y
		&& mousePos.y < endPos.y)
		return TRUE;

	return FALSE;
}

IO::IO()
{
	mousePos.x = 0;
	mousePos.y = 0;

	ZeroMemory(keys, sizeof(keys));
}
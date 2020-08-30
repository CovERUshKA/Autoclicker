#pragma once

#include <Windows.h>
#include "../../Tools.hpp"

enum InputStates {
	DOWN = 1,
	UP
};

class IO
{
public:
	struct
	{
		FLOAT x, y;
	} mousePos;

	// [*][0] - holding
	// [*][1] - once pressing InputStates::DOWN\UP
	UINT keys[MAX_PATH][2];

	// [*][0] - holding
	// [*][1] - once pressing InputStates::DOWN\UP
	UINT rawKeys[MAX_PATH][2];

	// [*][1] - once rotating InputStates::DOWN\UP
	UINT mWheel;

	BOOL bFullscreen;

	IO();

	BOOL CursorInField(Vector2D startPos, Vector2D endPos);
};

extern IO io;
extern IO overlayIo;
#pragma once

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <d2d1.h>
#include <string>
#include <dwmapi.h>
#include <sddl.h>
#pragma comment(lib, "Shcore.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "dcomp.lib")

#include "GUI/CoGUI.hpp"
#include "GUI/Draw/Draw.hpp"
#include "GUI/IO/IO.hpp"

#include "Overlay.hpp"

using namespace std;
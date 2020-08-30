#pragma once

#include <Windows.h>
#include <d2d1.h>
#include <dwrite.h>

#include "../../../Tools.hpp"

struct TextInformation
{
	//0 - Left
	//1 - Right
	//2 - Center
	DWORD xAlign;
	
	//0 - Top
	//1 - Center
	//2 - Bottom
	DWORD yAlign;

	D2D1_COLOR_F color;

	bool multiline;
	bool clip;
};

class D2D1_DRAW
{
public:
	ID2D1Factory* pFactory;
	ID2D1PathGeometry* pGeometry;
	ID2D1SolidColorBrush* pBrush;
	ID2D1GeometrySink* pSink;
	ID2D1RenderTarget* pRenderTarget;
	ID2D1HwndRenderTarget* pHwndRenderTarget;
	IDWriteFactory* pIDWFactory;
	IDWriteTextFormat* pTextFormat;
	IDWriteTextLayout* pTextLayout;

	D2D1_ELLIPSE ellipse;

	D2D1_SIZE_U WndSize;

	D2D1_DRAW();

	HRESULT Initialize(HWND hWnd);
	VOID CleanupDeviceD2D();
	
	VOID Resize(LPARAM lParam);
	HRESULT CreateGraphicsResources();
	VOID DiscardGraphicsResources();

	HRESULT BeginDraw(FLOAT r, FLOAT g, FLOAT b, FLOAT a = 1.0f);
	HRESULT EndDraw();

	VOID Line(Vector2D startPos, Vector2D endPos, FLOAT width, D2D1_COLOR_F color, bool antialias = false);
	VOID RectangleBordered(Vector2D Pos, FLOAT width, FLOAT height, FLOAT widthpx, D2D1_COLOR_F color, bool antialias = false);
	VOID Circle(Vector2D Pos, DWORD Radius, D2D1_COLOR_F color);
	VOID DrawBitmap(BYTE* pBuf, DWORD width, DWORD height);
	VOID String(Vector2D Pos,TextInformation tInfo, LPCWSTR str, UINT strLength, LPCWSTR pFont, UINT fontSize, Vector2D rectSize = { 0, 0 });
	VOID String(Vector2D Pos,TextInformation tInfo, LPCSTR str, UINT strLength, LPCSTR pFont, UINT fontSize, Vector2D rectSize = { 0, 0 });
	Vector2D GetTextSize(LPCWSTR str, UINT strLength, LPCWSTR pFont, UINT fontSize);
	Vector2D GetTextSize(LPCSTR str, UINT strLength, LPCSTR pFont, UINT fontSize);
	VOID Triangle(Vector2D* Vertices, D2D1_COLOR_F color);
	VOID Rectangle(Vector2D Pos, FLOAT width, FLOAT height, D2D1_COLOR_F color);
	VOID ClipRectBegin(D2D1_RECT_F rect);
	VOID ClipRectEnd();

private:
	HWND m_hwnd;
	PAINTSTRUCT ps;
};
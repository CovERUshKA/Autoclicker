#include "D2D1_DRAW.hpp"
using namespace D2D1;

template <class T> void  SafeRelease(T** ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}

VOID D2D1_DRAW::Circle(Vector2D Pos, DWORD Radius, D2D1_COLOR_F color)
{
	HRESULT hr;

	ellipse = D2D1::Ellipse(D2D1::Point2F(Pos.x, Pos.y), (FLOAT)Radius, (FLOAT)Radius);

	hr = pRenderTarget->CreateSolidColorBrush(color, &pBrush);

	if (FAILED(hr))
		return;

	pRenderTarget->FillEllipse(ellipse, pBrush);

	SafeRelease(&pBrush);

	return;
}

Vector2D D2D1_DRAW::GetTextSize(LPCWSTR str, UINT strLength, LPCWSTR pFont, UINT fontSize)
{
	HRESULT hr = S_OK;
	RECT rc = { 0, 0, 0, 0 };

	RECT rect;
	GetClientRect(m_hwnd, &rect);

	// Create a DirectWrite text format object.
	hr = pIDWFactory->CreateTextFormat(
		pFont,
		NULL,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		(FLOAT)fontSize,
		L"", //locale
		&pTextFormat
	);

	if (FAILED(hr))
		return { 0, 0 };

	// Center the text horizontally and vertically.
	pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);

	pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

	hr = pIDWFactory->CreateTextLayout(
		str,      // The string to be laid out and formatted.
		strLength,  // The length of the string.
		pTextFormat,  // The text format to apply to the string (contains font information, etc).
		(FLOAT)rect.right,         // The width of the layout box.
		(FLOAT)rect.bottom,        // The height of the layout box.
		&pTextLayout  // The IDWriteTextLayout interface pointer.
	);

	DWRITE_TEXT_METRICS metrics;
	hr = pTextLayout->GetMetrics(&metrics);
	if (FAILED(hr))
	{
		return { 0, 0 };
	}

	SafeRelease(&pTextFormat);
	SafeRelease(&pTextLayout);

	return { StCa<float>(metrics.width), StCa<float>(metrics.height) };
}

Vector2D D2D1_DRAW::GetTextSize(LPCSTR str, UINT strLength, LPCSTR pFont, UINT fontSize)
{
	HRESULT hr = S_OK;
	size_t nocConverted;

	wchar_t* pwFont = new wchar_t[strlen(pFont) + 1];
	ZeroMemory(pwFont, sizeof(pwFont));
	
	mbstowcs_s(&nocConverted, pwFont, strlen(pFont) + 1, pFont, strlen(pFont));

	wchar_t* pwStr = new wchar_t[strLength + 1];
	ZeroMemory(pwStr, sizeof(pwStr));

	mbstowcs_s(&nocConverted, pwStr, strLength + 1, str, strLength);
	
	Vector2D size = GetTextSize(pwStr, strLength, pwFont, fontSize);

	delete[] pwFont;
	delete[] pwStr;
	pwFont = 0;
	pwStr = 0;

	return size;
}

VOID D2D1_DRAW::Line(Vector2D startPos, Vector2D endPos, FLOAT width, D2D1_COLOR_F color, bool antialias)
{
	HRESULT hr = S_OK;

	hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(color.r, color.g, color.b, color.a), &pBrush);

	if (FAILED(hr))
	{
		Log("Unable to create solid color brush");
		return;
	}

	if (antialias)
		pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
	else
		pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

	pRenderTarget->DrawLine({ startPos.x, startPos.y }, { endPos.x, endPos.y }, pBrush, width);

	SafeRelease(&pBrush);

	return;
}

VOID D2D1_DRAW::RectangleBordered(Vector2D Pos, FLOAT width, FLOAT height, FLOAT widthpx, D2D1_COLOR_F color, bool antialias)
{
	HRESULT hr = S_OK;

	hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(color.r, color.g, color.b, color.a), &pBrush);

	if (FAILED(hr))
		return;

	if (antialias)
		pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
	else
		pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

	pRenderTarget->DrawRectangle({ Pos.x, Pos.y, Pos.x + width, Pos.y + height }, pBrush, widthpx);

	SafeRelease(&pBrush);
	
	return;
}

VOID D2D1_DRAW::String(Vector2D Pos, TextInformation tInfo, LPCSTR str, UINT strLength, LPCSTR pFont, UINT fontSize, Vector2D rectSize)
{
	size_t nocConverted = NULL;

	wchar_t* pwFont = new wchar_t[strlen(pFont) + 1];
	ZeroMemory(pwFont, sizeof(pwFont));

	mbstowcs_s(&nocConverted, pwFont, strlen(pFont) + 1, pFont, strlen(pFont));

	wchar_t* pwStr = new wchar_t[strLength + 1];
	ZeroMemory(pwStr, sizeof(pwStr));

	mbstowcs_s(&nocConverted, pwStr, strLength + 1, str, strLength);

	String(Pos, tInfo, pwStr, strLength, pwFont, fontSize, rectSize);
	
	delete[] pwFont;
	delete[] pwStr;
	pwFont = 0;
	pwStr = 0;

	return;
}

VOID D2D1_DRAW::String(Vector2D Pos, TextInformation tInfo, LPCWSTR str, UINT strLength, LPCWSTR pFont, UINT fontSize, Vector2D rectSize)
{
	HRESULT hr = S_OK;

	if (rectSize.x < 0
		|| rectSize.y < 0
		|| strLength == NULL)
		return;

	if (rectSize.x == 0
		&& rectSize.y == 0)
	{
		D2D1_SIZE_F wndSize = pRenderTarget->GetSize();

		rectSize.x = wndSize.width;
		rectSize.y = wndSize.height;
	}

	// Create a DirectWrite text format object.
	hr = pIDWFactory->CreateTextFormat(
		pFont,
		NULL,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		(FLOAT)fontSize,
		L"", //locale
		&pTextFormat
	);

	if (FAILED(hr))
		return;
	
	pTextFormat->SetTextAlignment((DWRITE_TEXT_ALIGNMENT)tInfo.yAlign);

	pTextFormat->SetParagraphAlignment((DWRITE_PARAGRAPH_ALIGNMENT)tInfo.xAlign);

	pTextFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);

	hr = pRenderTarget->CreateSolidColorBrush(tInfo.color, &pBrush);
	if (FAILED(hr))
		return;

	hr = pIDWFactory->CreateTextLayout(
		str,      // The string to be laid out and formatted.
		strLength,  // The length of the string.
		pTextFormat,  // The text format to apply to the string (contains font information, etc).
		rectSize.x,         // The width of the layout box.
		rectSize.y,        // The height of the layout box.
		&pTextLayout  // The IDWriteTextLayout interface pointer.
	);
	if (FAILED(hr))
		return;

	pRenderTarget->DrawTextLayout({ Pos.x, Pos.y }, pTextLayout, pBrush, D2D1_DRAW_TEXT_OPTIONS_CLIP);

	SafeRelease(&pBrush);
	SafeRelease(&pTextFormat);
	SafeRelease(&pTextLayout);

	return;
}

VOID D2D1_DRAW::ClipRectBegin(D2D1_RECT_F rect)
{
	HRESULT hr;

	// Create a layer.
	ID2D1Layer* pLayer = NULL;
	hr = pRenderTarget->CreateLayer(NULL, &pLayer);

	if (FAILED(hr))
		return;

	// Push the layer.
	pRenderTarget->PushLayer(
		D2D1::LayerParameters(rect),
		pLayer
	);

	SafeRelease(&pLayer);

	return;
}

VOID D2D1_DRAW::ClipRectEnd()
{
	// Push the layer.
	pRenderTarget->PopLayer();

	return;
}

VOID D2D1_DRAW::Triangle(Vector2D* Vertices, D2D1_COLOR_F color)
{
	HRESULT hr;

	hr = pFactory->CreatePathGeometry(&pGeometry);

	if (FAILED(hr))
		return;

	hr = pGeometry->Open(&pSink);

	if (FAILED(hr))
		return;

	pSink->SetFillMode(D2D1_FILL_MODE_WINDING);

	pSink->BeginFigure(
		D2D1::Point2F(Vertices[0].x, Vertices[0].y),
		D2D1_FIGURE_BEGIN_FILLED
	);
	D2D1_POINT_2F points[] = {
	   D2D1::Point2F(Vertices[1].x, Vertices[1].y),
	   D2D1::Point2F(Vertices[2].x, Vertices[2].y),
	   D2D1::Point2F(Vertices[0].x, Vertices[0].y)
	};
	pSink->AddLines(points, ARRAYSIZE(points));
	pSink->EndFigure(D2D1_FIGURE_END_CLOSED);

	hr = pSink->Close();

	if (FAILED(hr))
		return;

	hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(color.r, color.g, color.b, color.a), &pBrush);

	if (FAILED(hr))
		return;

	pRenderTarget->DrawGeometry(pGeometry, pBrush);

	pRenderTarget->FillGeometry(pGeometry, pBrush);

	SafeRelease(&pSink);
	SafeRelease(&pGeometry);
	SafeRelease(&pBrush);

	return;
}

VOID D2D1_DRAW::Rectangle(Vector2D Pos, FLOAT width, FLOAT height, D2D1_COLOR_F d2dColor)
{
	HRESULT hr;

	// Draw two rectangles.
	D2D1_RECT_F rectangle1 = D2D1::RectF(
		Pos.x,
		Pos.y,
		Pos.x + width,
		Pos.y + height
	);

	hr = pRenderTarget->CreateSolidColorBrush(d2dColor, &pBrush);

	if (FAILED(hr))
		return;

	pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);

	// Draw a filled rectangle.
	pRenderTarget->FillRectangle(&rectangle1, pBrush);

	SafeRelease(&pBrush);

	return;
}

VOID D2D1_DRAW::DrawBitmap(BYTE* pBuf, DWORD width, DWORD height)
{
	HRESULT hr;

	if (pRenderTarget == NULL)
		return;

	ID2D1Bitmap* pBitmap;
	hr = pRenderTarget->CreateBitmap(D2D1::SizeU(width, height),
		D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM,
			D2D1_ALPHA_MODE_IGNORE)), &pBitmap);
	if (FAILED(hr))
		return;

	hr = pBitmap->CopyFromMemory(nullptr, pBuf, width * 4);
	if (FAILED(hr))
		return;

	pRenderTarget->DrawBitmap(pBitmap, //the bitmap to draw [a portion of],
		D2D1::RectF(0.0f, 0.0f, (FLOAT)width, (FLOAT)height) //destination rectangle
		);

	SafeRelease(&pBitmap);
}

HRESULT D2D1_DRAW::EndDraw()
{
	HRESULT hr = S_OK;
	hr = pRenderTarget->EndDraw();
	if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
	{
		DiscardGraphicsResources();
	}

	EndPaint(m_hwnd, &ps);

	return hr;
}

HRESULT D2D1_DRAW::BeginDraw(D2D1_COLOR_F color)
{
	HRESULT hr = CreateGraphicsResources();
	if (FAILED(hr))
		return hr;

	BeginPaint(m_hwnd, &ps);

	pRenderTarget->BeginDraw();
	//pRenderTarget->GetSize().width;
	pRenderTarget->SetTransform(D2D1::IdentityMatrix());

	pRenderTarget->Clear(color);

	return hr;
}

HRESULT D2D1_DRAW::BeginDraw(FLOAT r, FLOAT g, FLOAT b, FLOAT a)
{
	HRESULT hr = CreateGraphicsResources();
	if (FAILED(hr))
		return hr;

	BeginPaint(m_hwnd, &ps);

	pRenderTarget->BeginDraw();
	//pRenderTarget->GetSize().width;
	pRenderTarget->SetTransform(D2D1::IdentityMatrix());

	pRenderTarget->Clear(D2D1::ColorF(r, g, b, a));

	return hr;
}

VOID D2D1_DRAW::Resize(LPARAM lParam)
{
	if (pRenderTarget == NULL)
		return;

	WndSize = D2D1::SizeU(LOWORD(lParam), HIWORD(lParam));

	pHwndRenderTarget->Resize(WndSize);
}

HRESULT D2D1_DRAW::CreateGraphicsResources()
{
	HRESULT hr = S_OK;
	if (pRenderTarget != NULL)
		return hr;

	if (pFactory == NULL)
		return E_FAIL;

	RECT rc;
	if (GetClientRect(m_hwnd, &rc) == NULL)
		return E_FAIL;
	
	D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

	hr = pFactory->CreateHwndRenderTarget(
		D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT,
			D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED)),
		D2D1::HwndRenderTargetProperties(m_hwnd, size), // D2D1_PRESENT_OPTIONS_IMMEDIATELY
		&pHwndRenderTarget);

	pRenderTarget = pHwndRenderTarget;

	if (FAILED(hr))
		return hr;

	return hr;
}

VOID D2D1_DRAW::DiscardGraphicsResources()
{
	SafeRelease(&pRenderTarget);
}

VOID D2D1_DRAW::CleanupDeviceD2D()
{
	DiscardGraphicsResources();
	SafeRelease(&pIDWFactory);
	SafeRelease(&pFactory);
}

HRESULT D2D1_DRAW::Initialize(HWND hWnd)
{
	HRESULT hr = S_OK;

	m_hwnd = hWnd;

	// Create a DirectWrite factory.
	hr = DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(pIDWFactory),
		reinterpret_cast<IUnknown**>(&pIDWFactory)
	);

	return hr;
}

D2D1_DRAW::D2D1_DRAW()
{
	pFactory = NULL;
	pRenderTarget = NULL;
	pBrush = NULL;
	m_hwnd = NULL;
	pGeometry = NULL;
	pSink = NULL;
	pIDWFactory = NULL;
	pTextFormat = NULL;
	pTextLayout = NULL;

	ZeroMemory(&ps, sizeof(PAINTSTRUCT));
}
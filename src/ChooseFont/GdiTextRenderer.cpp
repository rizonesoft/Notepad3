// encoding: UTF-8
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
//----------------------------------------------------------------------------

#include "ChooseFont.h"
#include "GdiTextRenderer.h"



/******************************************************************
*                                                                 *
* GdiTextRenderer::GdiTextRenderer                                *
*                                                                 *
* Destruct the object.                                            *
*                                                                 *
******************************************************************/

GdiTextRenderer::~GdiTextRenderer()
{
    SafeRelease(&m_renderTarget);
    SafeRelease(&m_renderingParams);
}


/******************************************************************
*                                                                 *
* GdiTextRenderer::GetDC                                          *
*                                                                 *
* Return a DC with the DIB surface selected into it.  The client  *
* can use this to initialize the background before drawing and to *
* get the result after drawing.                                   *
*                                                                 *
* The DC should not be released or deleted.  Ownership is         *
* retained by the GdiTextRenderer object.                         *
*                                                                 *
******************************************************************/

HDC GdiTextRenderer::GetDC()
{
    return m_renderTarget->GetMemoryDC();
}


/******************************************************************
*                                                                 *
* GdiTextRenderer::Initialize                                     *
*                                                                 *
* Create a bitmap (DIB) render target and setup default rendering *
* parameters.                                                     *
*                                                                 *
******************************************************************/

HRESULT GdiTextRenderer::Initialize(HWND referenceHwnd, HDC referenceDC, UINT width, UINT height)
{
    HRESULT hr;
    IDWriteGdiInterop* gdiInterop = nullptr;

    hr = g_dwrite->GetGdiInterop(&gdiInterop);
    if (SUCCEEDED(hr)) {
        hr = gdiInterop->CreateBitmapRenderTarget(referenceDC, width, height, &m_renderTarget);
    }
    if (SUCCEEDED(hr)) {
        hr = g_dwrite->CreateMonitorRenderingParams(
                 MonitorFromWindow(referenceHwnd, MONITOR_DEFAULTTONULL),
                 &m_renderingParams);
    }

    SafeRelease(&gdiInterop);

    return hr;
}


/******************************************************************
*                                                                 *
* GdiTextRenderer::DrawGlyphRun                                   *
*                                                                 *
* Defer to IDWriteBitmapRenderTarget to draw a series of glyphs.  *
*                                                                 *
******************************************************************/

HRESULT STDMETHODCALLTYPE GdiTextRenderer::DrawGlyphRun(
    void* /* clientDrawingContext */,
    FLOAT baselineOriginX,
    FLOAT baselineOriginY,
    DWRITE_MEASURING_MODE measuringMode,
    DWRITE_GLYPH_RUN const* glyphRun,
    DWRITE_GLYPH_RUN_DESCRIPTION const* /* glyphRunDescription */,
    IUnknown* /* clientDrawingEffect */)
{
    m_renderTarget->DrawGlyphRun(
        baselineOriginX,
        baselineOriginY,
        measuringMode,
        glyphRun,
        m_renderingParams,
        GetSysColor(COLOR_BTNTEXT));

    return S_OK;
}


/******************************************************************
*                                                                 *
* GdiTextRenderer::DrawUnderline                                  *
*                                                                 *
* This sample does not draw underlines.                           *
*                                                                 *
******************************************************************/

HRESULT STDMETHODCALLTYPE GdiTextRenderer::DrawUnderline(
    void* /* clientDrawingContext */,
    FLOAT /* baselineOriginX */,
    FLOAT /* baselineOriginY */,
    DWRITE_UNDERLINE const* /* underline */,
    IUnknown* /* clientDrawingEffect */)
{
    return S_OK;
}


/******************************************************************
*                                                                 *
* GdiTextRenderer::DrawStrikethrough                              *
*                                                                 *
* This sample does not draw strikethroughs                        *
*                                                                 *
******************************************************************/

HRESULT STDMETHODCALLTYPE GdiTextRenderer::DrawStrikethrough(
    void* /* clientDrawingContext */,
    FLOAT /* baselineOriginX */,
    FLOAT /* baselineOriginY */,
    DWRITE_STRIKETHROUGH const* /* strikethrough */,
    IUnknown* /* clientDrawingEffect */)
{
    return S_OK;
}


/******************************************************************
*                                                                 *
* GdiTextRenderer::DrawInlineObject                               *
*                                                                 *
* This sample does not use inline objects.                        *
*                                                                 *
******************************************************************/

HRESULT STDMETHODCALLTYPE GdiTextRenderer::DrawInlineObject(
    void* /* clientDrawingContext */,
    FLOAT /* originX */,
    FLOAT /* originY */,
    IDWriteInlineObject* /* inlineObject */,
    BOOL /* isSideways */,
    BOOL /* isRightToLeft */,
    IUnknown* /* clientDrawingEffect */)
{
    return S_OK;
}


/******************************************************************
*                                                                 *
* GdiTextRenderer::IsPixelSnapped                                 *
*                                                                 *
* Leave pixel snapping on.                                        *
*                                                                 *
******************************************************************/

HRESULT STDMETHODCALLTYPE GdiTextRenderer::IsPixelSnappingDisabled(
    void* /* clientDrawingContext */,
    BOOL* isDisabled)
{
    *isDisabled = false;
    return S_OK;
}


/******************************************************************
*                                                                 *
* GdiTextRenderer::GetCurrentTransform                            *
*                                                                 *
* This sample does not draw scaled and/or rotated text.           *
*                                                                 *
******************************************************************/

HRESULT STDMETHODCALLTYPE GdiTextRenderer::GetCurrentTransform(
    void* /* clientDrawingContext */,
    DWRITE_MATRIX* transform)
{
    static const DWRITE_MATRIX identity = {1.0f, 0.0f,
                                           0.0f, 1.0f,
                                           0.0f, 0.0f
                                          };

    *transform = identity;
    return S_OK;
}


/******************************************************************
*                                                                 *
* GdiTextRenderer::GetPixelsPerDip                                *
*                                                                 *
* Adjust the font size for the underlying DPI of the reference DC *
*                                                                 *
******************************************************************/

HRESULT STDMETHODCALLTYPE GdiTextRenderer::GetPixelsPerDip(
    void* /* clientDrawingContext */,
    FLOAT* pixelsPerDip)
{
    *pixelsPerDip = m_renderTarget->GetPixelsPerDip();
    return S_OK;
}


/******************************************************************
*                                                                 *
* GdiTextRenderer::QueryInterface                                 *
*                                                                 *
* COM boilerplate                                                 *
*                                                                 *
******************************************************************/

HRESULT STDMETHODCALLTYPE GdiTextRenderer::QueryInterface(
    REFIID riid,
    void **ppvObject)
{
    *ppvObject = nullptr;

    if (riid == __uuidof(IDWriteTextRenderer) ||
            riid == __uuidof(IUnknown)) {
        AddRef();
        *ppvObject = this;
        return S_OK;
    }

    return E_NOINTERFACE;
}


/******************************************************************
*                                                                 *
* GdiTextRenderer::AddRef                                         *
*                                                                 *
* COM boilerplate
*                                                                 *
******************************************************************/

ULONG STDMETHODCALLTYPE GdiTextRenderer::AddRef()
{
    return InterlockedIncrement(&m_refs);
}


/******************************************************************
*                                                                 *
* GdiTextRenderer::Release                                        *
*                                                                 *
* COM boilerplate                                                 *
*                                                                 *
******************************************************************/

ULONG STDMETHODCALLTYPE GdiTextRenderer::Release()
{
    LONG refs = InterlockedDecrement(&m_refs);
    if (refs == 0) {
        delete this;
    }
    return refs;
}

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
//----------------------------------------------------------------------------

/******************************************************************
*                                                                 *
* GdiTextRenderer                                                 *
*                                                                 *
* Encapsulate the rendering callbacks needed for DirectWrite to   *
* draw onto a GDI DIB surface                                     *
*                                                                 *
******************************************************************/

class DECLSPEC_UUID("70d1bcc3-2fcf-4b42-bfce-e3cd4db9d316") GdiTextRenderer : public IDWriteTextRenderer
{
public:

    GdiTextRenderer();
    ~GdiTextRenderer();

    HDC GetDC();

    HRESULT Initialize(HWND referenceHwnd, HDC referenceDC, UINT width, UINT height);

private:

    IDWriteBitmapRenderTarget*   m_renderTarget;
    IDWriteRenderingParams*      m_renderingParams;
    volatile LONG                m_refs;
    
    HRESULT STDMETHODCALLTYPE DrawGlyphRun(
        void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_MEASURING_MODE measuringMode,
        DWRITE_GLYPH_RUN const* glyphRun,
        DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
        IUnknown* clientDrawingEffect);

    HRESULT STDMETHODCALLTYPE DrawUnderline(
        void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_UNDERLINE const* underline,
        IUnknown* clientDrawingEffect);

    HRESULT STDMETHODCALLTYPE DrawStrikethrough(
        void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_STRIKETHROUGH const* strikethrough,
        IUnknown* clientDrawingEffect);

    HRESULT STDMETHODCALLTYPE DrawInlineObject(
        void* clientDrawingContext,
        FLOAT originX,
        FLOAT originY,
        IDWriteInlineObject* /* inlineObject */,
        BOOL isSideways,
        BOOL isRightToLeft,
        IUnknown* clientDrawingEffect);

    HRESULT STDMETHODCALLTYPE IsPixelSnappingDisabled(
        void* clientDrawingContext,
        OUT BOOL* isDisabled);

    HRESULT STDMETHODCALLTYPE GetCurrentTransform(
        void* clientDrawingContext,
        OUT DWRITE_MATRIX* transform);

    HRESULT STDMETHODCALLTYPE GetPixelsPerDip(
        void* clientDrawingContext,
        OUT FLOAT* pixelsPerDip);
    
public:

    HRESULT STDMETHODCALLTYPE QueryInterface( 
        REFIID riid,
        void **ppvObject);

    ULONG STDMETHODCALLTYPE AddRef();

    ULONG STDMETHODCALLTYPE Release();
};

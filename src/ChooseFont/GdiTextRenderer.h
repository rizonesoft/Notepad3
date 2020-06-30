// encoding: UTF-8
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

    GdiTextRenderer() = default;
    virtual ~GdiTextRenderer();

    HDC GetDC();

    HRESULT Initialize(HWND referenceHwnd, HDC referenceDC, UINT width, UINT height);

private:

    volatile LONG                m_refs = 0;
    IDWriteBitmapRenderTarget*   m_renderTarget = nullptr;
    IDWriteRenderingParams*      m_renderingParams = nullptr;
    
    HRESULT STDMETHODCALLTYPE DrawGlyphRun(
        void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_MEASURING_MODE measuringMode,
        DWRITE_GLYPH_RUN const* glyphRun,
        DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
        IUnknown* clientDrawingEffect) override;

    HRESULT STDMETHODCALLTYPE DrawUnderline(
        void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_UNDERLINE const* underline,
        IUnknown* clientDrawingEffect) override;

    HRESULT STDMETHODCALLTYPE DrawStrikethrough(
        void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_STRIKETHROUGH const* strikethrough,
        IUnknown* clientDrawingEffect) override;

    HRESULT STDMETHODCALLTYPE DrawInlineObject(
        void* clientDrawingContext,
        FLOAT originX,
        FLOAT originY,
        IDWriteInlineObject* /* inlineObject */,
        BOOL isSideways,
        BOOL isRightToLeft,
        IUnknown* clientDrawingEffect) override;

    HRESULT STDMETHODCALLTYPE IsPixelSnappingDisabled(
        void* clientDrawingContext,
        OUT BOOL* isDisabled) override;

    HRESULT STDMETHODCALLTYPE GetCurrentTransform(
        void* clientDrawingContext,
        OUT DWRITE_MATRIX* transform) override;

    HRESULT STDMETHODCALLTYPE GetPixelsPerDip(
        void* clientDrawingContext,
        OUT FLOAT* pixelsPerDip) override;

public:

    HRESULT STDMETHODCALLTYPE QueryInterface( 
        REFIID riid,
        void **ppvObject) override;

    ULONG STDMETHODCALLTYPE AddRef() override;

    ULONG STDMETHODCALLTYPE Release() override;
};

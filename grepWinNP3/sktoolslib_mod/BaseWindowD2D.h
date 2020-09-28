// sktoolslib - common files for SK tools

// Copyright (C) 2020 - Stefan Kueng

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//

#pragma once
#include "BaseWindow.h"

#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <math.h>

#include <d2d1.h>
#include <d2d1effects.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>
#include <dxgi1_3.h>
#include <d3d11_2.h>
#include <d2d1_2.h>
#include <d2d1_2helper.h>
#include <dcomp.h>
#include <wrl.h>
using namespace Microsoft::WRL;

struct ComException
{
    HRESULT result;
    ComException(HRESULT const value)
        : result(value)
    {
    }
};
void HR(HRESULT const result);

/**
 * \ingroup Utils
 * A base window class with Direct2D support.
 */
class CWindowD2D : public CWindow
{
public:
    virtual bool Create() override;
    virtual bool Create(DWORD dwStyles, HWND hParent = nullptr, RECT* rect = nullptr) override;
    virtual bool CreateEx(DWORD dwExStyles, DWORD dwStyles, HWND hParent = nullptr, RECT* rect = nullptr, LPCWSTR classname = nullptr, HMENU hMenu = nullptr) override;

protected:
    //constructor
    CWindowD2D(HINSTANCE hInst, CONST WNDCLASSEX* wcx = nullptr)
        : CWindow(hInst, wcx)
        , m_window_width(0)
        , m_window_height(0)
    {
    }
    virtual ~CWindowD2D()
    {
    }

    virtual LRESULT CALLBACK WinMsgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
    virtual HRESULT          OnRender(ID2D1DeviceContext* dc)                               = 0;
    virtual HRESULT          CreateDeviceResources()                                        = 0;
    virtual HRESULT          DiscardDeviceResources()                                       = 0;

protected:
    ComPtr<ID3D11Device>        m_direct3dDevice;
    ComPtr<IDXGIDevice>         m_dxgiDevice;
    ComPtr<IDXGIFactory2>       m_dxFactory;
    ComPtr<IDWriteFactory>      m_writeFactory;
    ComPtr<IDXGISwapChain1>     m_swapChain;
    ComPtr<ID2D1Factory2>       m_d2Factory;
    ComPtr<ID2D1Device1>        m_d2Device;
    ComPtr<ID2D1DeviceContext>  m_dc;
    ComPtr<IDXGISurface2>       m_surface;
    ComPtr<IDCompositionDevice> m_compositionDevice;
    ComPtr<IDCompositionTarget> m_compositionTarget;
    ComPtr<IDCompositionVisual> m_compositionVisual;
    ComPtr<ID2D1Bitmap1>        m_d2dBitmap;

    UINT m_window_width, m_window_height;

private:
    LRESULT CALLBACK WinMsgHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    void             Resize(UINT width, UINT height);
};

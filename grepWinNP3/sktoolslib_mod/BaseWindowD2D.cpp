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

#include "stdafx.h"
#include "BaseWindowD2D.h"
#include <memory>
#include <Shlwapi.h>

#pragma comment(lib, "dxgi")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d2d1")
#pragma comment(lib, "dcomp")
#pragma comment(lib, "Dwrite")
#pragma comment(lib, "dxguid")

void HR(HRESULT const result)
{
    if (S_OK != result)
    {
        throw ComException(result);
    }
}

bool CWindowD2D::Create()
{
    // Create the window
    RECT rect;

    rect.top    = 0;
    rect.left   = 0;
    rect.right  = 600;
    rect.bottom = 400;

    return CreateEx(WS_EX_NOREDIRECTIONBITMAP, WS_OVERLAPPEDWINDOW | WS_VISIBLE, nullptr, &rect);
}

bool CWindowD2D::Create(DWORD dwStyles, HWND hParent /* = nullptr */, RECT* rect /* = nullptr */)
{
    return CreateEx(WS_EX_NOREDIRECTIONBITMAP, dwStyles, hParent, rect);
}

bool CWindowD2D::CreateEx(DWORD dwExStyles, DWORD dwStyles, HWND hParent, RECT* rect, LPCWSTR classname, HMENU hMenu)
{
    auto ret = __super::CreateEx(dwExStyles, dwStyles, hParent, rect, classname, hMenu);
    if (ret)
    {
        D2D1_FACTORY_OPTIONS const options = {D2D1_DEBUG_LEVEL_NONE};
        HR(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,
                             options,
                             m_d2Factory.GetAddressOf()));
        HR(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(m_writeFactory), reinterpret_cast<IUnknown**>(m_writeFactory.GetAddressOf())));

        m_dc             = nullptr;
        m_d2Device       = nullptr;
        m_dxFactory      = nullptr;
        m_dxgiDevice     = nullptr;
        m_direct3dDevice = nullptr;
        HR(D3D11CreateDevice(nullptr,
                             D3D_DRIVER_TYPE_HARDWARE,
                             nullptr,
                             D3D11_CREATE_DEVICE_BGRA_SUPPORT,
                             nullptr,
                             0,
                             D3D11_SDK_VERSION,
                             &m_direct3dDevice,
                             nullptr,
                             nullptr));
        HR(m_direct3dDevice.As(&m_dxgiDevice));
        HR(CreateDXGIFactory2(0, __uuidof(m_dxFactory), reinterpret_cast<void**>(m_dxFactory.GetAddressOf())));
        HR(m_d2Factory->CreateDevice(m_dxgiDevice.Get(), m_d2Device.GetAddressOf()));
        HR(m_d2Device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, m_dc.GetAddressOf()));
        if (rect)
            Resize(rect->right - rect->left, rect->bottom - rect->top);
    }
    return ret;
}

LRESULT CALLBACK CWindowD2D::WinMsgHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CREATE:
            m_hwnd = hwnd;
            break;
        case WM_ERASEBKGND:
            return TRUE;
        case WM_SIZE:
            Resize((unsigned)lParam & 0xFFFF, (unsigned)lParam >> 16);
            [[fallthrough]];
        case WM_PAINT:
        {
            if (!m_dc || !m_d2dBitmap)
                return 0;
            m_dc->BeginDraw();
            m_dc->Clear();
            OnRender(m_dc.Get());
            HR(m_dc->EndDraw());
            HR(m_swapChain->Present(1, 0));
            HR(m_compositionDevice->Commit());
            ValidateRect(*this, nullptr);
            return 0;
        }
        break;
        case WM_DISPLAYCHANGE:
        {
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }

        break;
        default:
            return WinMsgProc(hwnd, uMsg, wParam, lParam);
    }

    return WinMsgProc(hwnd, uMsg, wParam, lParam);
}
void CWindowD2D::Resize(UINT width, UINT height)
{
    DiscardDeviceResources();
    m_window_width  = width;
    m_window_height = height;
    if (m_window_width == 0 || m_window_height == 0)
    {
        return;
    }
    DXGI_SWAP_CHAIN_DESC1 sc_description = {};
    sc_description.Format                = DXGI_FORMAT_B8G8R8A8_UNORM;
    sc_description.BufferUsage           = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sc_description.SwapEffect            = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    sc_description.BufferCount           = 2;
    sc_description.SampleDesc.Count      = 1;
    sc_description.AlphaMode             = DXGI_ALPHA_MODE_PREMULTIPLIED;
    sc_description.Width                 = m_window_width;
    sc_description.Height                = m_window_height;
    m_swapChain                          = nullptr;
    HR(m_dxFactory->CreateSwapChainForComposition(m_dxgiDevice.Get(),
                                                  &sc_description,
                                                  nullptr,
                                                  m_swapChain.GetAddressOf()));
    m_compositionDevice = nullptr;
    HR(DCompositionCreateDevice(m_dxgiDevice.Get(),
                                __uuidof(m_compositionDevice),
                                reinterpret_cast<void**>(m_compositionDevice.GetAddressOf())));

    m_compositionTarget = nullptr;
    HR(m_compositionDevice->CreateTargetForHwnd(*this, true, m_compositionTarget.GetAddressOf()));

    m_compositionVisual = nullptr;
    HR(m_compositionDevice->CreateVisual(m_compositionVisual.GetAddressOf()));
    HR(m_compositionVisual->SetContent(m_swapChain.Get()));
    HR(m_compositionTarget->SetRoot(m_compositionVisual.Get()));

    m_surface = nullptr;
    HR(m_swapChain->GetBuffer(0, __uuidof(m_surface), reinterpret_cast<void**>(m_surface.GetAddressOf())));
    D2D1_BITMAP_PROPERTIES1 properties = {};
    properties.pixelFormat.alphaMode   = D2D1_ALPHA_MODE_PREMULTIPLIED;
    properties.pixelFormat.format      = DXGI_FORMAT_B8G8R8A8_UNORM;
    properties.bitmapOptions           = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;

    m_d2dBitmap = nullptr;
    HR(m_dc->CreateBitmapFromDxgiSurface(m_surface.Get(),
                                         properties,
                                         m_d2dBitmap.GetAddressOf()));
    m_dc->SetTarget(m_d2dBitmap.Get());
    CreateDeviceResources();
};

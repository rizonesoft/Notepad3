// sktoolslib - common files for SK tools

// Copyright (C) 2012 - Stefan Kueng

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
#include "HotKeyCtrl.h"
#include <CommCtrl.h>


#define PROP_OBJECT_PTR         MAKEINTATOM(ga.atom)
#define PROP_ORIGINAL_PROC      MAKEINTATOM(ga.atom)

/*
 * typedefs
 */
class CGlobalAtom
{
public:
    CGlobalAtom(void)
    { atom = GlobalAddAtom(TEXT("_HotKeyCtrl_Object_Pointer_")
             TEXT("\\{07B14ED2-C35E-438a-9B39-F4BAFE4E59EB}")); }
    ~CGlobalAtom(void)
    { DeleteAtom(atom); }

    ATOM atom;
};

/*
 * Local variables
 */
static CGlobalAtom ga;


CHotKeyCtrl::CHotKeyCtrl(void)
    : m_hWnd(NULL)
    , m_pfnOrigCtlProc(NULL)
    , controldown(false)
    , shiftdown(false)
    , menudown(false)
    , lwindown(false)
{
}

CHotKeyCtrl::~CHotKeyCtrl(void)
{
}


BOOL CHotKeyCtrl::ConvertEditToHotKeyCtrl(HWND hwndCtl)
{
    // Subclass the existing control.
    m_pfnOrigCtlProc = (WNDPROC)GetWindowLong(hwndCtl, GWLP_WNDPROC);
    SetProp(hwndCtl, PROP_OBJECT_PTR, (HANDLE) this);
    SetWindowLongPtr(hwndCtl, GWLP_WNDPROC, (LONG_PTR)(WNDPROC)_HotKeyProc);

    kb_hook = SetWindowsHookEx(WH_KEYBOARD, _KeyboardProc, NULL, GetCurrentThreadId());
    m_hWnd = hwndCtl;

    return TRUE;
}

BOOL CHotKeyCtrl::ConvertEditToHotKeyCtrl(HWND hwndParent, UINT uiCtlId)
{
    return ConvertEditToHotKeyCtrl(GetDlgItem(hwndParent, uiCtlId));
}

void CHotKeyCtrl::SetHKText(WORD hk)
{
    TCHAR buf[MAX_PATH] = {0};
    TCHAR result[MAX_PATH] = {0};
    BYTE h = hk >> 8;
    if (h & HOTKEYF_CONTROL)
    {
        LONG sc = MapVirtualKey(VK_CONTROL, MAPVK_VK_TO_VSC);
        sc <<= 16;
        GetKeyNameText(sc, buf, _countof(buf));
        _tcscat_s(result, _countof(result), buf);
    }
    if (h & HOTKEYF_SHIFT)
    {
        if (result[0])
            _tcscat_s(result, _countof(result), _T(" + "));
        LONG sc = MapVirtualKey(VK_SHIFT, MAPVK_VK_TO_VSC);
        sc <<= 16;
        GetKeyNameText(sc, buf, _countof(buf));
        _tcscat_s(result, _countof(result), buf);
    }
    if (h & HOTKEYF_ALT)
    {
        if (result[0])
            _tcscat_s(result, _countof(result), _T(" + "));
        LONG sc = MapVirtualKey(VK_MENU, MAPVK_VK_TO_VSC);
        sc <<= 16;
        GetKeyNameText(sc, buf, _countof(buf));
        _tcscat_s(result, _countof(result), buf);
    }
    if (h & HOTKEYF_EXT)
    {
        if (result[0])
            _tcscat_s(result, _countof(result), _T(" + "));
        LONG sc = MapVirtualKey(VK_LWIN, MAPVK_VK_TO_VSC);
        sc |= 0x0100;
        sc <<= 16;
        GetKeyNameText(sc, buf, _countof(buf));
        _tcscat_s(result, _countof(result), buf);
    }
    if (result[0])
        _tcscat_s(result, _countof(result), _T(" + "));

    LONG nScanCode = MapVirtualKey(LOBYTE(hk), MAPVK_VK_TO_VSC);
    switch(LOBYTE(hk))
    {
        // Keys which are "extended" (except for Return which is Numeric Enter as extended)
    case VK_INSERT:
    case VK_DELETE:
    case VK_HOME:
    case VK_END:
    case VK_NEXT:  // Page down
    case VK_PRIOR: // Page up
    case VK_LEFT:
    case VK_RIGHT:
    case VK_UP:
    case VK_DOWN:
    case VK_SNAPSHOT:
        nScanCode |= 0x0100; // Add extended bit
    }
    nScanCode <<= 16;
    GetKeyNameText(nScanCode, buf, _countof(buf));


    _tcscat_s(result, _countof(result), buf);
    ::SetWindowText(m_hWnd, result);
}

LRESULT CALLBACK CHotKeyCtrl::_KeyboardProc(int code, WPARAM wParam, LPARAM lParam)
{
    HWND hWndFocus = ::GetFocus();
    if (hWndFocus)
    {
        CHotKeyCtrl *pHyperLink = (CHotKeyCtrl *)GetProp(hWndFocus, PROP_OBJECT_PTR);
        if ((pHyperLink) && (pHyperLink->m_hWnd == hWndFocus))
        {
            if (lParam & 0x80000000)
            {
                // WM_KEYUP
                if (wParam == VK_CONTROL)
                {
                    pHyperLink->controldown = false;
                }
                else if (wParam == VK_SHIFT)
                {
                    pHyperLink->shiftdown = false;
                }
                else if (wParam == VK_MENU)
                {
                    pHyperLink->menudown = false;
                }
                else if (wParam == VK_LWIN)
                {
                    pHyperLink->lwindown = false;
                }
                else
                {
                    WORD hk = 0;
                    if (pHyperLink->controldown)
                    {
                        hk |= HOTKEYF_CONTROL;
                    }
                    if (pHyperLink->shiftdown)
                    {
                        hk |= HOTKEYF_SHIFT;
                    }
                    if (pHyperLink->menudown)
                    {
                        hk |= HOTKEYF_ALT;
                    }
                    if (pHyperLink->lwindown)
                    {
                        hk |= HOTKEYF_EXT;
                    }

                    pHyperLink->hotkey = MAKEWORD(wParam, hk);
                    pHyperLink->SetHKText(pHyperLink->hotkey);
                    return 1;//we processed it
                }
            }
            else
            {
                // WM_KEYDOWN
                if (wParam == VK_CONTROL)
                {
                    pHyperLink->controldown = true;
                }
                else if (wParam == VK_SHIFT)
                {
                    pHyperLink->shiftdown = true;
                }
                else if (wParam == VK_MENU)
                {
                    pHyperLink->menudown = true;
                }
                else if (wParam == VK_LWIN)
                {
                    pHyperLink->lwindown = true;
                }
                else
                {
                    WORD hk = 0;
                    if (pHyperLink->controldown)
                    {
                        hk |= HOTKEYF_CONTROL;
                    }
                    if (pHyperLink->shiftdown)
                    {
                        hk |= HOTKEYF_SHIFT;
                    }
                    if (pHyperLink->menudown)
                    {
                        hk |= HOTKEYF_ALT;
                    }
                    if (pHyperLink->lwindown)
                    {
                        hk |= HOTKEYF_EXT;
                    }
                    pHyperLink->hotkey = MAKEWORD(wParam, hk);
                    pHyperLink->SetHKText(pHyperLink->hotkey);
                    return 1;//we processed it
                }
            }
            return CallNextHookEx(pHyperLink->kb_hook, code, wParam, lParam);
        }
    }
    return 0;
}

LRESULT CALLBACK CHotKeyCtrl::_HotKeyProc(HWND hwnd, UINT message,
                                           WPARAM wParam, LPARAM lParam)
{
    CHotKeyCtrl *pHyperLink = (CHotKeyCtrl *)GetProp(hwnd, PROP_OBJECT_PTR);

    switch (message)
    {
    case WM_SHOWWINDOW:
        pHyperLink->SetHKText(pHyperLink->hotkey);
        break;
    case WM_GETDLGCODE:
            return DLGC_WANTALLKEYS;
    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
        {
            return 0;
        }
    case WM_SYSKEYUP:
    case WM_KEYUP:
        {
            return 0;
        }
    case WM_CHAR:
        return 0;
    case WM_DESTROY:
        {
            SetWindowLongPtr(hwnd, GWLP_WNDPROC,
                            (LONG_PTR)pHyperLink->m_pfnOrigCtlProc);

            RemoveProp(hwnd, PROP_OBJECT_PTR);
            break;
        }
    }

    return CallWindowProc(pHyperLink->m_pfnOrigCtlProc, hwnd, message,
                          wParam, lParam);
}

void CHotKeyCtrl::SetHotKey(WPARAM hk)
{
    hotkey = hk;
    SetHKText(hk);
}

WPARAM CHotKeyCtrl::GetHotKey()
{
    return hotkey;
}

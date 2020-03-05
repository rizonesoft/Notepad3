// Scintilla source code edit control
/** @file PlatWin.h
 ** Implementation of platform facilities on Windows.
 **/
// Copyright 1998-2011 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.
#pragma once
#ifndef PLATWIN_H
#define PLATWIN_H

// sdkddkver.h
#ifndef _WIN32_WINNT_VISTA
#define _WIN32_WINNT_VISTA				0x0600
#endif
#ifndef _WIN32_WINNT_WIN7
#define _WIN32_WINNT_WIN7				0x0601
#endif
#ifndef _WIN32_WINNT_WIN8
#define _WIN32_WINNT_WIN8				0x0602
#endif
#ifndef _WIN32_WINNT_WINBLUE
#define _WIN32_WINNT_WINBLUE			0x0603
#endif
#ifndef _WIN32_WINNT_WIN10
#define _WIN32_WINNT_WIN10				0x0A00
#endif

#if !defined(DISABLE_D2D)
#define USE_D2D 1
#endif

#if defined(USE_D2D)
#if defined(_MSC_BUILD) && (VER_PRODUCTVERSION_W <= _WIN32_WINNT_WIN7)
#pragma warning(push)
#pragma warning(disable: 4458)
// d2d1helper.h(677,19): warning C4458:  declaration of 'a' hides class member
#endif
#include <d2d1.h>
#include <dwrite.h>
#if defined(_MSC_BUILD) && (VER_PRODUCTVERSION_W <= _WIN32_WINNT_WIN7)
#pragma warning(pop)
#endif
#endif


// force compile C as CPP
#define NP2_FORCE_COMPILE_C_AS_CPP	0

#if NP2_FORCE_COMPILE_C_AS_CPP
extern int GetSystemMetricsDPIScaledX(HWND hwnd, int nIndex);
extern int GetSystemMetricsDPIScaledY(HWND hwnd, int nIndex);
#else
extern "C" int GetSystemMetricsDPIScaledX(HWND hwnd, int nIndex);
extern "C" int GetSystemMetricsDPIScaledY(HWND hwnd, int nIndex);
#endif


namespace Scintilla {

extern void Platform_Initialise(void *hInstance) noexcept;
extern void Platform_Finalise(bool fromDllMain) noexcept;

constexpr RECT RectFromPRectangle(PRectangle prc) noexcept {
	RECT rc = { static_cast<LONG>(prc.left), static_cast<LONG>(prc.top),
		static_cast<LONG>(prc.right), static_cast<LONG>(prc.bottom) };
	return rc;
}

constexpr POINT POINTFromPoint(Point pt) noexcept {
	return POINT{ static_cast<LONG>(pt.x), static_cast<LONG>(pt.y) };
}

constexpr Point PointFromPOINT(POINT pt) noexcept {
	return Point::FromInts(pt.x, pt.y);
}

constexpr HWND HwndFromWindowID(WindowID wid) noexcept {
	return static_cast<HWND>(wid);
}

inline HWND HwndFromWindow(const Window &w) noexcept {
	return HwndFromWindowID(w.GetID());
}

#if defined(USE_D2D)
extern bool LoadD2D() noexcept;
extern ID2D1Factory *pD2DFactory;
extern IDWriteFactory *pIDWriteFactory;
#endif

}

#endif

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include <SDKDDKVer.h>

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <windowsx.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>

#include <commctrl.h>
#include <shlwapi.h>

#include "Language.h"
#include "SimpleIni.h"

#define BOOST_REGEX_BLOCKSIZE        4096
#define BOOST_REGEX_MAX_BLOCKS       (1024 * 32)
#define BOOST_REGEX_MAX_CACHE_BLOCKS (16 * 32)

extern HINSTANCE    g_hInst;
extern bool         bPortable;
extern CSimpleIni   g_iniFile;
extern std::wstring g_iniPath;

#define DEBUGOUTPUTREGPATH L"Software\\grepWinNP3\\DebugOutput"

#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
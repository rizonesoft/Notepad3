// encoding: UTF-8
/******************************************************************************
*                                                                             *
*                                                                             *
* MiniPath - Notepad3 Explorer Plugin                                         *
*                                                                             *
* version.h                                                                   *
*   MiniPath version information                                              *
*   Based on code from metapath, (c) Florian Balmer 1996-2011                 *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2020   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/
#include "VersionEx.h"

#ifndef _T
#if !defined(ISPP_INVOKED) && (defined(UNICODE) || defined(_UNICODE))
#define _T(text) L##text
#else
#define _T(text) text
#endif
#endif

#define DO_STRINGIFY(x) _T(#x)
#define STRINGIFY(x)    DO_STRINGIFY(x)

#define VERSION_FILEVERSION_NUM      VERSION_MAJOR,VERSION_MINOR,VERSION_REV,VERSION_BUILD
#define VERSION_FILEVERSION_SHORT    STRINGIFY(VERSION_MAJOR) "." STRINGIFY(VERSION_MINOR) "." \
                                     STRINGIFY(VERSION_REV) "." STRINGIFY(VERSION_BUILD)
#define VERSION_LEGALCOPYRIGHT_SHORT L"Copyright © 2008-2020"
#define VERSION_LEGALCOPYRIGHT_LONG  L"Copyright © Rizonesoft 2008-2020"

#define VERSION_INTERNALNAME         L"MiniPath"
#define VERSION_ORIGINALFILENAME     L"minipath.exe"
#define VERSION_AUTHORNAME           L"Florian Balmer et al. ( metapath )"
#define VERSION_WEBPAGEDISPLAY       L"https://www.rizonesoft.com"
#define VERSION_FBWEBPAGEDISPLAY     L"http://www.flos-freeware.ch"

#if defined(_WIN64)
#define VERSION_FILEVERSION_LONG  L"MiniPath (x64) " STRINGIFY(VERSION_MAJOR) L" Build " \
                                     STRINGIFY(VERSION_BUILD)
#else
#define VERSION_FILEVERSION_LONG  L"MiniPath (x86) " STRINGIFY(VERSION_MAJOR) L" Build " \
                                     STRINGIFY(VERSION_BUILD)
#endif

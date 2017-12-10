/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* Version.h                                                                   *
*   Based on code from Notepad2-mod, (c) XhmikosR 2010-2015                   *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2016   *
*                                                 https://www.rizonesoft.com  *
*                                                                             *
*                                                                             *
*******************************************************************************/

#ifndef NOTEPAD3_VERSION_H
#define NOTEPAD3_VERSION_H

#include "VersionEx.h"

#ifndef _T
#if !defined(ISPP_INVOKED) && (defined(UNICODE) || defined(_UNICODE))
#define _T(text) L##text
#else
#define _T(text) text
#endif
#endif

#define DO_STRINGIFYA(x)    #x
#define DO_STRINGIFYW(x) _T(#x)
#define STRG(x)         DO_STRINGIFYA(x)
#define STRINGIFY(x)    DO_STRINGIFYW(x)

#define MY_APPNAME                   L"Notepad3"
#define VERSION_FILEVERSION_NUM      VERSION_MAJOR,VERSION_MINOR,VERSION_REV,VERSION_BUILD
#define VERSION_FILEVERSION          STRINGIFY(VERSION_MAJOR) "." STRINGIFY(VERSION_MINOR) "." \
                                     STRINGIFY(VERSION_REV) "." STRINGIFY(VERSION_BUILD)
#define VERSION_LEGALCOPYRIGHT       L"Copyright © 2008-2017 Rizonesoft"
//#define VERSION_LEGALCOPYRIGHT_LONG  L"© Rizonesoft 2008-2017"
#define VERSION_AUTHORNAME           L"© Rizonesoft"
#define VERSION_WEBPAGEDISPLAY       L"https://www.rizonesoft.com"
#define VERSION_COMPANYNAME          L"© Rizonesoft"
#define VERSION_MODPAGEDISPLAY       L"https://xhmikosr.github.io/notepad2-mod/"
#define VERSION_WEBPAGE2DISPLAY      L"http://www.flos-freeware.ch"
#define VERSION_SCIVERSION           L"Scintilla Library (RegEx:DeelX) Version: " STRINGIFY(SCINTILLA_VER)

#if defined(_WIN64)
//   #define VERSION_FILEVERSION_LONG  L"Notepad3 (64-bit) " STRINGIFY(VERSION_MAJOR) L" Build " \
//                                     STRINGIFY(VERSION_BUILD)
   #define VERSION_FILEVERSION_LONG  L"Notepad3 (64-bit) v." VERSION_FILEVERSION

#else
//   #define VERSION_FILEVERSION_LONG  L"Notepad3 v." STRINGIFY(VERSION_MAJOR) L" Build " \
//                                     STRINGIFY(VERSION_BUILD)
   #define VERSION_FILEVERSION_LONG  L"Notepad3 v." VERSION_FILEVERSION
#endif



// Compiler specific
#if defined(_MSC_VER)
    #if (_MSC_VER >= 1912)
        #if(_MSC_FULL_VER >= 191225830)
            #define VER_CPL     "Microsoft Visual C++ 2017  Version 15.5"
        #endif    
    #elif (_MSC_VER >= 1911)
        #if((_MSC_FULL_VER >= 191125542) && (_MSC_FULL_VER <= 191125547))
           #define VER_CPL     "Microsoft Visual C++ 2017  Version 15.4"
        #elif((_MSC_FULL_VER >= 191125506) && (_MSC_FULL_VER <= 191125508))
           #define VER_CPL     "Microsoft Visual C++ 2017  Version 15.3"
        #else
           #define VER_CPL     "Microsoft Visual C++ 2017"
        #endif
    #elif (_MSC_VER >= 1910)
        #if ((_MSC_FULL_VER >= 191025017) && (_MSC_FULL_VER <= 191025019))
           #define VER_CPL     "Microsoft Visual C++ 2017  Version 15.2"
        #else
            #define VER_CPL    "Microsoft Visual C++ 2017  RC"
        #endif
    #elif (_MSC_VER == 1900)
        #if (_MSC_FULL_VER == 190024210) || (_MSC_FULL_VER == 190024215)
            #define VER_CPL    "Microsoft Visual C++ 2015  Update 3"
        #elif (_MSC_FULL_VER == 190023918)
            #define VER_CPL    "Microsoft Visual C++ 2015  Update 2"
        #elif (_MSC_FULL_VER == 190023506)
            #define VER_CPL    "Microsoft Visual C++ 2015  Update 1"
        #elif (_MSC_FULL_VER == 190023506)
            #define VER_CPL    "Microsoft Visual C++ 2015  Update 1"
        #else
            #define VER_CPL    "Microsoft Visual C++  (version unknown)"
        #endif
    #else
        #define VER_CPL        "Microsoft Visual C++  (version unknown)"
    #endif
#else
    #define VER_CPL            "(Unknown compiler)"
#endif

#define VERSION_COMPILER  STRINGIFY(VER_CPL)

#pragma message("Compiler Version: " VER_CPL "  (#" STRG(_MSC_FULL_VER) ")")

#endif // NOTEPAD3_VERSION_H

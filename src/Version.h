// encoding: UTF-8
/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* Version.h                                                                   *
*   Based on code from Notepad2-mod, (c) XhmikosR 2010-2015                   *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2026   *
*                                                     https://rizonesoft.com  *
*                                                                             *
*                                                                             *
*******************************************************************************/

#ifndef NOTEPAD3_VERSION_H
#define NOTEPAD3_VERSION_H

#define __CC(p,s) p ## s
#define _V(s)  __CC(v,s)
#define _W(s)  __CC(L,s)

#define _STRINGIFY(s) #s
#define _STRG(s)  _STRINGIFY(s)

// ----------------------------------------------------------------------------

#define VERSION_FILEVERSION          VERSION_MAJOR.VERSION_MINOR.VERSION_REV.VERSION_BUILD
#define VERSION_FILEVERSION_NUM      VERSION_MAJOR,VERSION_MINOR,VERSION_REV,VERSION_BUILD

#if defined(_WIN64)
#define VERSION_FILEVERSION_LONG     APPNAME (x64)  VERSION_FILEVERSION  VERSION_PATCH
#else
#define VERSION_FILEVERSION_LONG     APPNAME (x86)  VERSION_FILEVERSION  VERSION_PATCH
#endif


#if (defined(_DEBUG) || defined(DEBUG)) && !defined(NDEBUG)
#pragma message("Debug Build: " _STRG(VERSION_FILEVERSION_LONG))
#else
#pragma message("Release Build: " _STRG(VERSION_FILEVERSION_LONG))
#endif

#define VERSION_LEGALCOPYRIGHT         "Copyright © 2008-2026 Rizonesoft"
//#define VERSION_LEGALCOPYRIGHT_LONG  "© Rizonesoft 2008-2026"
#define VERSION_AUTHORNAME             "© Rizonesoft"
#define VERSION_WEBPAGEDISPLAY         "https://rizonesoft.com"
#define VERSION_COMPANYNAME            "© Rizonesoft"
//#define VERSION_MODPAGEDISPLAY       "https://xhmikosr.github.io/notepad2-mod/"
//#define VERSION_WEBPAGE2DISPLAY      "https://www.flos-freeware.ch"

#ifdef _DLL
#define _SCI_BUILD                     L"Scintilla(dll) "
#define _LXI_BUILD                     L"Lexilla(dll) "
#else
#define _SCI_BUILD                     L"Scintilla "
#define _LXI_BUILD                     L"Lexilla "
#endif

#define VERSION_SCIVERSION             _SCI_BUILD _W(_STRG(_V(SCINTILLA_VER)))
#define VERSION_LXIVERSION             _LXI_BUILD _W(_STRG(_V(LEXILLA_VER)))
#define VERSION_ONIGURUMA              L"Oniguruma " _W(_STRG(_V(ONIGURUMA_REGEX_VER)))
#define VERSION_UCHARDET               L"UChardet " _W(_STRG(_V(UCHARDET_VER)))
#define VERSION_TINYEXPR               L"TinyExpr " _W(_STRG(_V(TINYEXPR_VER)))
#define VERSION_UTHASH                 L"UTHash " _W(_STRG(_V(UTHASH_VER)))

// ============================================================================

#define VERSION_UPDATE_CHECK           L"https://rizonesoft.com/downloads/notepad3/update/?version=" _W(_STRG(VERSION_FILEVERSION))

// ============================================================================

// ----------------------------------------------------------------------------
// https://docs.microsoft.com/en-us/windows/release-health  (Windows releases health)
// https://docs.microsoft.com/en-us/windows/release-health/release-information  (Windows 10)
// https://docs.microsoft.com/en-us/windows/release-health/windows11-release-information  (Windows 11)
// https://docs.microsoft.com/en-us/windows/release-health/windows-server-release-info  (Windows Server)
// https://docs.microsoft.com/en-us/windows-insider/flight-hub  (Windows Insider Preview Builds)
// https://en.wikipedia.org/wiki/Windows_10 (Wikipedia: Windows 10)
// https://en.wikipedia.org/wiki/Windows_11 (Wikipedia: Windows 11)
// ----------------------------------------------------------------------------

inline LPCWSTR _Win10BuildToReleaseId(const DWORD build)
{
    static LPCWSTR lpcReleaseID = L"unknown";

    if (build >= 22000) { // Win11

        if (build >= 26200)
        {
            lpcReleaseID = L"25H2";
        }
        else if (build >= 26100)
        {
            lpcReleaseID = L"24H2";
        }
        else if (build >= 22631)
        {
            lpcReleaseID = L"23H2";
        }
        else if (build >= 22621)
        {
            lpcReleaseID = L"22H2";
        }
        else {
            lpcReleaseID = L"21H2";
        }
    }
    else { // Win10

        if (build >= 19045) {
            lpcReleaseID = L"22H2";
        }
        else if (build >= 19044) {
            lpcReleaseID = L"21H2";
        }
        else if (build >= 19043) {
            lpcReleaseID = L"21H1";
        }
        else if (build >= 19042) {
            lpcReleaseID = L"20H2";
        }
        else if (build >= 19041) {
            lpcReleaseID = L"20H1";
        }
        else if (build >= 18363) {
            lpcReleaseID = L"19H2";
        }
        else if (build >= 18362) {
            lpcReleaseID = L"19H1";
        }
        else if (build >= 17763) {
            lpcReleaseID = L"1809";
        }
        else if (build >= 17134) {
            lpcReleaseID = L"1803";
        }
        else if (build >= 16299) {
            lpcReleaseID = L"1709";
        }
        else if (build >= 15063) {
            lpcReleaseID = L"1703";
        }
        else if (build >= 14393) {
            lpcReleaseID = L"1607";
        }
        else if (build >= 10586) {
            lpcReleaseID = L"1511";
        }
        else if (build >= 10240) {
            lpcReleaseID = L"1507";
        }
    }
    return lpcReleaseID;
}

// ============================================================================

// Compiler specific

#undef VER_CPL

#if defined(_MSC_VER)
    #if (_MSC_VER == 1950)
        #if (_MSC_FULL_VER >= 195035724)
            #define VER_CPL     MS Visual C++ 2026 v18.3.(0-1)
        #elif (_MSC_FULL_VER >= 195035719)
            #define VER_CPL     MS Visual C++ 2026 v18.(1-2)
        #elif (_MSC_FULL_VER >= 195035717)
            #define VER_CPL     MS Visual C++ 2026 v18.0.0
        #endif
    #elif (_MSC_VER == 1944)
        #if (_MSC_FULL_VER >= 194435222)
            #define VER_CPL     MS Visual C++ 2022 v17.14.(22-26)
        #elif (_MSC_FULL_VER >= 194435221)
            #define VER_CPL     MS Visual C++ 2022 v17.14.21
        #elif (_MSC_FULL_VER >= 194435220)
            #define VER_CPL     MS Visual C++ 2022 v17.14.20
        #elif (_MSC_FULL_VER >= 194435219)
            #define VER_CPL     MS Visual C++ 2022 v17.14.(18-19)
        #elif (_MSC_FULL_VER >= 194435217)
            #define VER_CPL     MS Visual C++ 2022 v17.14.(15-17)
        #elif (_MSC_FULL_VER >= 194435216)
            #define VER_CPL     MS Visual C++ 2022 v17.14.14
        #elif (_MSC_FULL_VER >= 194435215)
            #define VER_CPL     MS Visual C++ 2022 v17.14.13
        #elif (_MSC_FULL_VER >= 194435214)
            #define VER_CPL     MS Visual C++ 2022 v17.14.(11-12)
        #elif (_MSC_FULL_VER >= 194435213)
            #define VER_CPL     MS Visual C++ 2022 v17.14.(9-10)
        #elif (_MSC_FULL_VER >= 194435211)
            #define VER_CPL     MS Visual C++ 2022 v17.14.(7-8)
        #elif (_MSC_FULL_VER >= 194435210)
            #define VER_CPL     MS Visual C++ 2022 v17.14.(5-6)
        #elif (_MSC_FULL_VER >= 194435209)
            #define VER_CPL     MS Visual C++ 2022 v17.14.4
        #elif (_MSC_FULL_VER >= 194435208)
            #define VER_CPL     MS Visual C++ 2022 v17.14.3
        #elif (_MSC_FULL_VER >= 194435207)
            #define VER_CPL     MS Visual C++ 2022 v17.14.(0-2)
        #endif
    #elif (_MSC_VER == 1943)
        #if (_MSC_FULL_VER >= 194334810)
            #define VER_CPL     MS Visual C++ 2022 v17.13.(6-7)
        #elif (_MSC_FULL_VER >= 194334809)
            #define VER_CPL     MS Visual C++ 2022 v17.13.(3-5)
        #elif (_MSC_FULL_VER >= 194334808)
            #define VER_CPL     MS Visual C++ 2022 v17.13.(0-2)
        #endif
    #elif (_MSC_VER == 1942)
        #if (_MSC_FULL_VER >= 194234436)
            #define VER_CPL     MS Visual C++ 2022 v17.12.4
        #elif (_MSC_FULL_VER >= 194234435)
            #define VER_CPL     MS Visual C++ 2022 v17.12.(2-3)
        #elif (_MSC_FULL_VER >= 194234433)
            #define VER_CPL     MS Visual C++ 2022 v17.12.(0-1)
        #endif
    #elif (_MSC_VER == 1941)
        #if (_MSC_FULL_VER >= 194134123)
            #define VER_CPL     MS Visual C++ 2022 v17.11.5
        #elif (_MSC_FULL_VER >= 194134120)
            #define VER_CPL     MS Visual C++ 2022 v17.11.(0-4)
        #endif
    #elif (_MSC_VER == 1940)
        #if (_MSC_FULL_VER >= 194033813)
            #define VER_CPL     MS Visual C++ 2022 v17.10.5
        #elif (_MSC_FULL_VER >= 194033812)
            #define VER_CPL     MS Visual C++ 2022 v17.10.4
        #elif (_MSC_FULL_VER >= 194033811)
            #define VER_CPL     MS Visual C++ 2022 v17.10.(1-3)
        #elif (_MSC_FULL_VER >= 194033808)
            #define VER_CPL     MS Visual C++ 2022 v17.10.0
        #endif
    #elif (_MSC_VER == 1939)
        #if (_MSC_FULL_VER >= 193933523)
            #define VER_CPL     MS Visual C++ 2022 v17.9.(4-7)
        #elif (_MSC_FULL_VER >= 193933522)
            #define VER_CPL     MS Visual C++ 2022 v17.9.3
        #elif (_MSC_FULL_VER >= 193933521)
            #define VER_CPL     MS Visual C++ 2022 v17.9.2
        #elif (_MSC_FULL_VER >= 193933520)
            #define VER_CPL     MS Visual C++ 2022 v17.9.1
        #elif (_MSC_FULL_VER >= 193933519)
            #define VER_CPL     MS Visual C++ 2022 v17.9.0
        #endif
    #elif (_MSC_VER == 1938)
        #if (_MSC_FULL_VER >= 193833135)
            #define VER_CPL     MS Visual C++ 2022 v17.8.6
        #elif (_MSC_FULL_VER >= 193833134)
            #define VER_CPL     MS Visual C++ 2022 v17.8.(4-5)
        #elif (_MSC_FULL_VER >= 193833133)
            #define VER_CPL     MS Visual C++ 2022 v17.8.3
        #elif (_MSC_FULL_VER >= 193833130)
            #define VER_CPL     MS Visual C++ 2022 v17.8.(0-2)
        #endif
    #elif (_MSC_VER == 1937)
        #if (_MSC_FULL_VER >= 193732825)
            #define VER_CPL     MS Visual C++ 2022 v17.7.(5-6)
        #elif (_MSC_FULL_VER >= 193732824)
            #define VER_CPL     MS Visual C++ 2022 v17.7.4
        #elif (_MSC_FULL_VER >= 193732822)
            #define VER_CPL     MS Visual C++ 2022 v17.7.(0-3)
        #endif
    #elif (_MSC_VER == 1936)
        #if (_MSC_FULL_VER >= 193632537)
            #define VER_CPL     MS Visual C++ 2022 v17.6.5
        #elif (_MSC_FULL_VER >= 193632535)
            #define VER_CPL     MS Visual C++ 2022 v17.6.4
        #elif (_MSC_FULL_VER >= 193632534)
            #define VER_CPL     MS Visual C++ 2022 v17.6.3
        #elif (_MSC_FULL_VER >= 193632532)
            #define VER_CPL     MS Visual C++ 2022 v17.6.(0-2)
        #endif
    #elif (_MSC_VER == 1935)
        #if (_MSC_FULL_VER >= 193532217)
            #define VER_CPL     MS Visual C++ 2022 v17.5.(4-5)
        #elif (_MSC_FULL_VER >= 193532216)
            #define VER_CPL     MS Visual C++ 2022 v17.5.3
        #elif (_MSC_FULL_VER >= 193532215)
            #define VER_CPL     MS Visual C++ 2022 v17.5.(0-2)
        #endif
    #elif (_MSC_VER == 1934)
        #if (_MSC_FULL_VER >= 193431942)
            #define VER_CPL     MS Visual C++ 2022 v17.4.5
        #elif (_MSC_FULL_VER >= 193431937)
            #define VER_CPL     MS Visual C++ 2022 v17.4.(3-4)
        #elif (_MSC_FULL_VER >= 193431935)
            #define VER_CPL     MS Visual C++ 2022 v17.4.2
        #elif (_MSC_FULL_VER >= 193431933)
            #define VER_CPL     MS Visual C++ 2022 v17.4.(0-1)
        #endif
    #elif (_MSC_VER == 1933)
        #if (_MSC_FULL_VER >= 193331630)
            #define VER_CPL     MS Visual C++ 2022 v17.3.(4-6)
        #elif (_MSC_FULL_VER >= 193331629)
            #define VER_CPL     MS Visual C++ 2022 v17.3.(0-3)
        #endif
    #elif (_MSC_VER == 1932)
        #if (_MSC_FULL_VER >= 193231332)
            #define VER_CPL     MS Visual C++ 2022 v17.2.(5-6)
        #elif (_MSC_FULL_VER >= 193231329)
            #define VER_CPL     MS Visual C++ 2022 v17.2.(1-4)
        #elif (_MSC_FULL_VER >= 193231328)
            #define VER_CPL     MS Visual C++ 2022 v17.2.0
        #endif
    #elif (_MSC_VER == 1931)
        #if (_MSC_FULL_VER >= 193131107)
            #define VER_CPL     MS Visual C++ 2022 v17.1.(5-6)
        #elif (_MSC_FULL_VER >= 193131106)
            #define VER_CPL     MS Visual C++ 2022 v17.1.4
        #elif (_MSC_FULL_VER >= 193131105)
            #define VER_CPL     MS Visual C++ 2022 v17.1.(2-3)
        #elif (_MSC_FULL_VER >= 193131104)
            #define VER_CPL     MS Visual C++ 2022 v17.1.(0-1)
        #endif
    #elif (_MSC_VER == 1930)
        #if (_MSC_FULL_VER >= 193030709)
            #define VER_CPL     MS Visual C++ 2022 v17.0.(5-6)
        #elif (_MSC_FULL_VER >= 193030706)
            #define VER_CPL     MS Visual C++ 2022 v17.0.(2-4)
        #elif (_MSC_FULL_VER >= 193030705)
            #define VER_CPL     MS Visual C++ 2022 v17.0.(0-1)
        #endif
    #elif (_MSC_VER == 1929)
        #if (_MSC_FULL_VER >= 192930154)
            #define VER_CPL     MS Visual C++ 2019 v16.11.(34-36)
        #elif (_MSC_FULL_VER >= 192930153)
            #define VER_CPL     MS Visual C++ 2019 v16.11.33
        #elif (_MSC_FULL_VER >= 192930152)
            #define VER_CPL     MS Visual C++ 2019 v16.11.(30-32)
        #elif (_MSC_FULL_VER >= 192930151)
            #define VER_CPL     MS Visual C++ 2019 v16.11.29
        #elif (_MSC_FULL_VER >= 192930150)
            #define VER_CPL     MS Visual C++ 2019 v16.11.28
        #elif (_MSC_FULL_VER >= 192930149)
            #define VER_CPL     MS Visual C++ 2019 v16.11.(26-27)
        #elif (_MSC_FULL_VER >= 192930148)
            #define VER_CPL     MS Visual C++ 2019 v16.11.(24-25)
        #elif (_MSC_FULL_VER >= 192930147)
            #define VER_CPL MS Visual C++ 2019 v16 .11.(21-23)
        #elif (_MSC_FULL_VER >= 192930146)
            #define VER_CPL     MS Visual C++ 2019 v16.11.(17-20)
        #elif (_MSC_FULL_VER >= 192930145)
            #define VER_CPL     MS Visual C++ 2019 v16.11.(15-16)
        #elif (_MSC_FULL_VER >= 192930144)
            #define VER_CPL     MS Visual C++ 2019 v16.11.14
        #elif (_MSC_FULL_VER >= 192930143)
            #define VER_CPL     MS Visual C++ 2019 v16.11.13
        #elif (_MSC_FULL_VER >= 192930142)
            #define VER_CPL     MS Visual C++ 2019 v16.11.12
        #elif (_MSC_FULL_VER >= 192930141)
            #define VER_CPL     MS Visual C++ 2019 v16.11.11
        #elif (_MSC_FULL_VER >= 192930140)
            #define VER_CPL     MS Visual C++ 2019 v16.11.10
        #elif (_MSC_FULL_VER >= 192930139)
            #define VER_CPL     MS Visual C++ 2019 v16.11.9
        #elif (_MSC_FULL_VER >= 192930138)
            #define VER_CPL     MS Visual C++ 2019 v16.11.8
        #elif (_MSC_FULL_VER >= 192930137)
            #define VER_CPL     MS Visual C++ 2019 v16.11.(6-7)
        #elif (_MSC_FULL_VER >= 192930136)
            #define VER_CPL     MS Visual C++ 2019 v16.11.(4-5)
        #elif (_MSC_FULL_VER >= 192930133)
            #define VER_CPL     MS Visual C++ 2019 v16.11.(0-3)
        #elif (_MSC_FULL_VER >= 192930040)
            #define VER_CPL     MS Visual C++ 2019 v16.10.4
        #elif (_MSC_FULL_VER >= 192930038)
            #define VER_CPL     MS Visual C++ 2019 v16.10.3
        #elif (_MSC_FULL_VER >= 192930037)
            #define VER_CPL     MS Visual C++ 2019 v16.10.(0-2)
        #endif
    #elif (_MSC_VER == 1928)
        #if (_MSC_FULL_VER >= 192829915)
            #define VER_CPL     MS Visual C++ 2019 v16.9.(5-6)
        #elif (_MSC_FULL_VER >= 192829914)
            #define VER_CPL     MS Visual C++ 2019 v16.9.4
        #elif (_MSC_FULL_VER >= 192829913)
            #define VER_CPL     MS Visual C++ 2019 v16.9.(2-3)
        #elif (_MSC_FULL_VER >= 192829912)
            #define VER_CPL     MS Visual C++ 2019 v16.9.1
        #elif (_MSC_FULL_VER >= 192829910)
            #define VER_CPL     MS Visual C++ 2019 v16.9.0
        #elif (_MSC_FULL_VER >= 192829337)
            #define VER_CPL     MS Visual C++ 2019 v16.8.(5-6)
        #elif (_MSC_FULL_VER >= 192829336)
            #define VER_CPL     MS Visual C++ 2019 v16.8.4
        #elif (_MSC_FULL_VER >= 192829335)
            #define VER_CPL     MS Visual C++ 2019 v16.8.3
        #elif (_MSC_FULL_VER >= 192829334)
            #define VER_CPL     MS Visual C++ 2019 v16.8.2
        #elif (_MSC_FULL_VER >= 192829333)
            #define VER_CPL     MS Visual C++ 2019 v16.8.1
        #elif (_MSC_FULL_VER > 192800000)
            #define VER_CPL     MS Visual C++ 2019 v16.8.0
        #endif
    #elif (_MSC_VER == 1927)
        #if (_MSC_FULL_VER >= 192729112)
            #define VER_CPL     MS Visual C++ 2019 v16.7.(5-7)
        #elif (_MSC_FULL_VER >= 192729111)
            #define VER_CPL     MS Visual C++ 2019 v16.7.(1-4)
        #elif (_MSC_FULL_VER >= 192729110)
            #define VER_CPL     MS Visual C++ 2019 v16.7.0
        #endif
    #elif (_MSC_VER == 1926)
        #if (_MSC_FULL_VER >= 192628806)
            #define VER_CPL     MS Visual C++ 2019 v16.6.(1-5)
        #elif (_MSC_FULL_VER >= 192628805)
            #define VER_CPL     MS Visual C++ 2019 v16.6.0
        #endif
    #elif (_MSC_VER == 1925)
        #if (_MSC_FULL_VER >= 192528614)
            #define VER_CPL     MS Visual C++ 2019 v16.5.(4-5)
        #elif (_MSC_FULL_VER >= 192528612)
            #define VER_CPL     MS Visual C++ 2019 v16.5.(2-3)
        #elif (_MSC_FULL_VER >= 192528611)
            #define VER_CPL     MS Visual C++ 2019 v16.5.1
        #elif (_MSC_FULL_VER >= 192528610)
            #define VER_CPL     MS Visual C++ 2019 v16.5.0
        #endif
    #elif (_MSC_VER == 1924)
        #if (_MSC_FULL_VER >= 192428319)
            #define VER_CPL     MS Visual C++ 2019 v16.4.6
        #elif (_MSC_FULL_VER >= 192428316)
            #define VER_CPL     MS Visual C++ 2019 v16.4.(4-5)
        #elif (_MSC_FULL_VER >= 192428315)
            #define VER_CPL     MS Visual C++ 2019 v16.4.3
        #elif (_MSC_FULL_VER >= 192428314)
            #define VER_CPL     MS Visual C++ 2019 v16.4.(0-2)
        #endif
    #elif (_MSC_VER == 1923)
        #if (_MSC_FULL_VER >= 192328107)
            #define VER_CPL     MS Visual C++ 2019 v16.3.(9-10)
        #elif (_MSC_FULL_VER >= 192328106)
            #define VER_CPL     MS Visual C++ 2019 v16.3.(3-8)
        #elif (_MSC_FULL_VER >= 192328105)
            #define VER_CPL     MS Visual C++ 2019 v16.3.(0-2)
        #endif
    #elif (_MSC_VER == 1922)
        #if (_MSC_FULL_VER >= 192227905)
            #define VER_CPL     MS Visual C++ 2019 v16.2.(0-5)
        #endif
    #elif (_MSC_VER == 1921)
        #if (_MSC_FULL_VER >= 192127702)
            #define VER_CPL     MS Visual C++ 2019 v16.1.(0-6)
        #endif
    #elif (_MSC_VER == 1920)
        #if (_MSC_FULL_VER >= 192027508)
            #define VER_CPL     MS Visual C++ 2019 v16.0.(0-4)
        #elif (_MSC_FULL_VER >= 192027027)
            #define VER_CPL     MS Visual C++ 2019 v16.0.Prev(1-4)
        #endif
    #elif (_MSC_VER == 1916)
        #if (_MSC_FULL_VER >= 191627034)
            #define VER_CPL     MS Visual C++ 2017 v15.9.(16-17)
        #elif (_MSC_FULL_VER >= 191627032)
            #define VER_CPL     MS Visual C++ 2017 v15.9.(14-15)
        #elif (_MSC_FULL_VER >= 191627031)
            #define VER_CPL     MS Visual C++ 2017 v15.9.(12-13)
        #elif (_MSC_FULL_VER >= 191627030)
            #define VER_CPL     MS Visual C++ 2017 v15.9.11
        #elif (_MSC_FULL_VER >= 191627027)
            #define VER_CPL     MS Visual C++ 2017 v15.9.(7-10)
        #elif (_MSC_FULL_VER >= 191627026)
            #define VER_CPL     MS Visual C++ 2017 v15.9.(5-6)
        #elif (_MSC_FULL_VER >= 191627025)
            #define VER_CPL     MS Visual C++ 2017 v15.9.4
        #elif (_MSC_FULL_VER >= 191627024)
            #define VER_CPL     MS Visual C++ 2017 v15.9.(2-3)
        #elif (_MSC_FULL_VER >= 191627023)
            #define VER_CPL     MS Visual C++ 2017 v15.9.(0-1)
        #endif
    #elif (_MSC_VER == 1915)
        #if (_MSC_FULL_VER >= 191526732)
            #define VER_CPL     MS Visual C++ 2017 v15.8.(8-9)
        #elif (_MSC_FULL_VER >= 191526730)
            #define VER_CPL     MS Visual C++ 2017 v15.8.(5-7)
        #elif (_MSC_FULL_VER >= 191526729)
            #define VER_CPL     MS Visual C++ 2017 v15.8.4
        #elif (_MSC_FULL_VER >= 191526726)
            #define VER_CPL     MS Visual C++ 2017 v15.8.(0-3)
        #endif
    #elif (_MSC_VER == 1914)
        #if (_MSC_FULL_VER >= 191426433)
            #define VER_CPL     MS Visual C++ 2017 v15.7.(5-6)
        #elif (_MSC_FULL_VER >= 191426431)
            #define VER_CPL     MS Visual C++ 2017 v15.7.4
        #elif (_MSC_FULL_VER >= 191426430)
            #define VER_CPL     MS Visual C++ 2017 v15.7.3
        #elif (_MSC_FULL_VER >= 191426429)
            #define VER_CPL     MS Visual C++ 2017 v15.7.2
        #elif (_MSC_FULL_VER >= 191426428)
            #define VER_CPL     MS Visual C++ 2017 v15.7.(0-1)
        #endif
    #elif (_MSC_VER == 1913)
        #if (_MSC_FULL_VER >= 191326132)
            #define VER_CPL     MS Visual C++ 2017 v15.6.7
        #elif (_MSC_FULL_VER >= 191326131)
            #define VER_CPL     MS Visual C++ 2017 v15.6.6
        #elif (_MSC_FULL_VER >= 191326129)
            #define VER_CPL     MS Visual C++ 2017 v15.6.(3-5)
        #elif (_MSC_FULL_VER >= 191326128)
            #define VER_CPL     MS Visual C++ 2017 v15.6.(0-2)
        #endif
    #elif (_MSC_VER == 1912)
        #if (_MSC_FULL_VER >= 191225835)
            #define VER_CPL     MS Visual C++ 2017 v15.5.(5-7)
        #elif (_MSC_FULL_VER >= 191225834)
            #define VER_CPL     MS Visual C++ 2017 v15.5.(3-4)
        #elif (_MSC_FULL_VER >= 191225831)
            #define VER_CPL     MS Visual C++ 2017 v15.5.2
        #elif (_MSC_FULL_VER >= 191225830)
            #define VER_CPL     MS Visual C++ 2017 v15.5.(0-1)
        #else
            #define VER_CPL     MS Visual C++ 2017 (version unknown)
        #endif
    #elif (_MSC_VER == 1911)
        #if ((_MSC_FULL_VER >= 191125542) && (_MSC_FULL_VER <= 191125547))
            #define VER_CPL     MS Visual C++ 2017 v15.4
        #elif ((_MSC_FULL_VER >= 191125506) && (_MSC_FULL_VER <= 191125508))
            #define VER_CPL     MS Visual C++ 2017 v15.3
        #else
            #define VER_CPL     MS Visual C++ 2017 (version unknown)
        #endif
    #elif (_MSC_VER == 1910)
        #if ((_MSC_FULL_VER >= 191025017) && (_MSC_FULL_VER <= 191025019))
            #define VER_CPL     MS Visual C++ 2017 v15.(0-2)
        #else
            #define VER_CPL     MS Visual C++ 2017 RC
        #endif
    #elif (_MSC_VER == 1900)
        #if (_MSC_FULL_VER >= 190024210) || (_MSC_FULL_VER == 190024215)
            #define VER_CPL     MS Visual C++ 2015 Update 3
        #elif (_MSC_FULL_VER >= 190023918)
            #define VER_CPL     MS Visual C++ 2015 Update 2
        #elif (_MSC_FULL_VER >= 190023506)
            #define VER_CPL     MS Visual C++ 2015 Update 1
        #elif (_MSC_FULL_VER >= 190023506)
            #define VER_CPL     MS Visual C++ 2015 Update 1
        #elif (_MSC_FULL_VER >= 190023026)
            #define VER_CPL     MS Visual C++ 2015
        #else
            #define VER_CPL     MS Visual C++ (version unknown)
        #endif
    #else
        #ifdef _MSC_VER
            #define VER_CPL     MS Visual C++ (version unknown)
        #endif
    #endif
#endif

#ifndef VER_CPL
    #define VER_CPL (Unknown Compiler)
#endif

#pragma message("Compiler " _STRG(VER_CPL) "  (v" _STRG(_MSC_FULL_VER) ")")

#define VERSION_COMPILER  L"Compiler: " _W(_STRG(VER_CPL)) L" (VC v" _W(_STRG(_MSC_VER)) L")"


#endif // NOTEPAD3_VERSION_H

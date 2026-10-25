// encoding: UTF-8
/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* EncodingDetection.cpp                                                       *
*   Interface to Encoding Detector  (CED or UCHARDET)                         *
*                                                                             *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2026   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/
#include <sdkddkver.h>
#if !defined(WINVER)
#define WINVER _WIN32_WINNT_WIN10
#endif
#if !defined(_WIN32_WINNT)
#define _WIN32_WINNT _WIN32_WINNT_WIN10
#endif
#if !defined(NTDDI_VERSION)
#define NTDDI_VERSION NTDDI_WIN10_RS5
#endif

#if (defined(_DEBUG) || defined(DEBUG)) && !defined(NDEBUG)
#define DBG_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__)
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
// allocations to be of _CLIENT_BLOCK type
#else
#define DBG_NEW new
#endif

#define VC_EXTRALEAN 1
#define WIN32_LEAN_AND_MEAN 1
#define NOMINMAX 1
#include <windows.h>

#define STRSAFE_NO_CB_FUNCTIONS
#define STRSAFE_NO_DEPRECATE      // don't allow deprecated functions
#include <strsafe.h>
#include <string_view>

//~#include <future>                 // async detection

extern "C" {
#include "Helpers.h"
#include "PathLib.h"
#include "Encoding.h"
#include "SciCall.h"
}

// CED - Compact Encoding Detection (by Google)
//~#include "compact_enc_det/compact_enc_det.h" ~ deprecated

// UCHARDET - Universal Character Detection (by Mozilla)
#include "uchardet/uchardet/src/uchardet.h"

#include "resource.h"

//=============================================================================

extern "C" void Style_SetMultiEdgeLine(const int colVec[], const size_t count);

//=============================================================================

static WCHAR wchEncodingInfo[MIDSZ_BUFFER] = { L'\0' };

static void _SetEncodingTitleInfo(const ENC_DET_T* pEncDetInfo);

extern "C" const WCHAR* Encoding_GetTitleInfo()
{
    return wchEncodingInfo;
}

extern "C" const char* Encoding_GetTitleInfoA()
{
    static char chEncodingInfo[MIDSZ_BUFFER] = { '\0' };
    ::WideCharToMultiByte(CP_ACP, 0, wchEncodingInfo, -1, chEncodingInfo, (int)COUNTOF(chEncodingInfo), NULL, NULL);
    return chEncodingInfo;
}


//=============================================================================

//#define MIN_CONFIDENCE_ANSI_NOT_UTF8 0.98f // if confidence is above this threshold, we consider it ANSI, even if it could be UTF-8
#define MIN_CONFIDENCE_ANSI_NOT_UTF8 0.995f // if confidence is above this threshold, we consider it ANSI, even if it could be UTF-8

#define ENC_PARSE_NAM_ASCII                ",ASCII,ascii,"
#define ENC_PARSE_NAM_ANSI                 ",ANSI,ansi,SYSTEM,system" ENC_PARSE_NAM_ASCII
#define ENC_PARSE_NAM_OEM                  ",OEM,oem,"
#define ENC_PARSE_NAM_UTF16LEBOM           ",UTF-16LE-BOM,"
#define ENC_PARSE_NAM_UTF16BEBOM           ",UTF-16BE-BOM,"
#define ENC_PARSE_NAM_UTF16LE              ",UTF-16LE,UTF-16,utf16,utf16le,unicode,"
#define ENC_PARSE_NAM_UTF16BE              ",UTF-16BE,utf16be,unicodebe,"
#define ENC_PARSE_NAM_UTF8                 ",UTF-8,utf8,"
#define ENC_PARSE_NAM_UTF8SIG              ",UTF-8-SIG,utf8sig,UTF-8-BOM,utf8bom,"
#define ENC_PARSE_NAM_UTF7                 ",UTF-7,utf7,"
#define ENC_PARSE_NAM_DOS_720              ",DOS-720,dos720,"
#define ENC_PARSE_NAM_ISO_8859_6           ",ISO-8859-6,iso88596,arabic,csisolatinarabic,ecma114,isoir127,"
#define ENC_PARSE_NAM_MAC_ARABIC           ",x-mac-arabic,xmacarabic,mac-arabic,macarabic,"
#define ENC_PARSE_NAM_WIN_1256             ",Windows-1256,windows1256,CP-1256,cp1256,ansiarabic"
#define ENC_PARSE_NAM_DOS_775              ",CP-500,cp500,ibm775,"
#define ENC_PARSE_NAM_ISO_8859_4           ",ISO-8859-4,iso88594,csisolatin4,isoir110,l4,latin4,"
#define ENC_PARSE_NAM_WIN_1257             ",Windows-1257,windows1257,CP-1257,cp1257,ansibaltic,"
#define ENC_PARSE_NAM_DOS_852              ",CP-852,cp852,ibm852,ISO-8859-16,iso885916"
#define ENC_PARSE_NAM_ISO_8859_2           ",ISO-8859-2,iso88592,csisolatin2,isoir101,latin2,l2,"
#define ENC_PARSE_NAM_MAC_CENTRAL_EUROP    ",x-mac-ce,xmacce,mac-ce,mac-centraleurope,xmaccentraleurope,maccentraleurope,"
#define ENC_PARSE_NAM_WIN_1250             ",Windows-1250,windows1250,CP-1250,cp1250,xcp1250,"
#define ENC_PARSE_NAM_GBK_936              ",CP-936,cp936,gb,gbk,gbk-936,chinese,cngb,cngbk,chinese_gb,chinese_gbk,"
#define ENC_PARSE_NAM_GB2312_80            ",gb2312,csgb2312,EUC-CN,euccn,gb2312-80,gb231280,gb231280,csgb231280,"
#define ENC_PARSE_NAM_MAC_ZH_CN            ",x-mac-chinesesimp,xmacchinesesimp,mac-chinesesimp,macchinesesimp,"
#define ENC_PARSE_NAM_BIG5                 ",big5,cnbig5,csbig5,xxbig5,chinese_big5,"
#define ENC_PARSE_NAM_MAC_ZH_TW            ",x-mac-chinesetrad,xmacchinesetrad,mac-chinesetrad,macchinesetrad,"
#define ENC_PARSE_NAM_MAC_CROATIAN         ",x-mac-croatian,xmaccroatian,mac-croatian,maccroatian,"
#define ENC_PARSE_NAM_DOS_866              ",CP-866,cp866,ibm866,"
#define ENC_PARSE_NAM_ISO_8859_5           ",ISO-8859-5,iso88595,csisolatin5,csisolatincyrillic,cyrillic,isoir144,"
#define ENC_PARSE_NAM_KOI8_R               ",KOI8-R,koi8r,cskoi8r,koi,koi8,"
#define ENC_PARSE_NAM_KOI8_U               ",KOI8-U,koi8u,koi8ru,"
#define ENC_PARSE_NAM_MAC_CYRILLIC         ",x-mac-cyrillic,xmaccyrillic,mac-cyrillic,maccyrillic,"
#define ENC_PARSE_NAM_WIN_1251             ",Windows-1251,windows1251,CP-1251,cp1251,xcp1251,"
#define ENC_PARSE_NAM_ISO_8859_13          ",ISO-8859-13,iso885913,"
#define ENC_PARSE_NAM_DOS_863              ",CP-863,cp863,ibm863,"
#define ENC_PARSE_NAM_DOS_737              ",CP-737,cp737,ibm737,"
#define ENC_PARSE_NAM_ISO_8859_7           ",ISO-8859-7,iso88597,csisolatingreek,ecma118,elot928,greek,greek8,isoir126,"
#define ENC_PARSE_NAM_MAC_GREEK            ",x-mac-greek,xmacgreek,mac-greek,macgreek,"
#define ENC_PARSE_NAM_WIN_1253             ",Windows-1253,windows1253,CP-1253,cp1253,"
#define ENC_PARSE_NAM_DOS_869              ",CP-869,cp869,ibm869,"
#define ENC_PARSE_NAM_DOS_862              ",DOS-862,dos862,"
#define ENC_PARSE_NAM_ISO_8859_8_I         ",ISO-8859-8-I,iso88598i,logical,"
#define ENC_PARSE_NAM_ISO_8859_8           ",ISO-8859-8,iso88598,csisolatinhebrew,hebrew,isoir138,visual,"
#define ENC_PARSE_NAM_MAC_HEBREW           ",x-mac-hebrew,xmachebrew,mac-hebrew,machebrew,"
#define ENC_PARSE_NAM_WIN_1255             ",Windows-1255,windows1255,CP-1255,cp1255,"
#define ENC_PARSE_NAM_DOS_861              ",CP-861,cp861,ibm861,"
#define ENC_PARSE_NAM_MAC_ICELANDIC        ",x-mac-icelandic,xmacicelandic,mac-icelandic,macicelandic,"
#define ENC_PARSE_NAM_MAC_JAPANESE         ",x-mac-japanese,xmacjapanese,mac-japanese,macjapanese,"
#define ENC_PARSE_NAM_SHIFT_JIS            ",CP-932,cp932,shift-jis,shift_jis,shiftjis,shiftjs,csshiftjis,cswindows31j,mskanji,xmscp932,xsjis,"
#define ENC_PARSE_NAM_MAC_KOREAN           ",x-mac-korean,xmackorean,mac-korean,mackorean,"
#define ENC_PARSE_NAM_WIN_949              ",Windows-949,windows949,uhc,EUC-KR,euckr,CP-949,cp949,ksx1001,ksc56011987,csksc5601,isoir149,korean,ksc56011989,"  // ANSI/OEM Korean (Unified Hangul Code)
#define ENC_PARSE_NAM_ISO_8859_3           ",ISO-8859-3,iso88593,latin3,isoir109,l3,"
#define ENC_PARSE_NAM_ISO_8859_15          ",ISO-8859-15,iso885915,latin9,l9,"
#define ENC_PARSE_NAM_DOS_865              ",CP-865,cp865,ibm865,"
#define ENC_PARSE_NAM_DOS_437              ",CP-437,cp437,ibm437,437,codepage437,cspc8,"
#define ENC_PARSE_NAM_DOS_858              ",CP-858,cp858,ibm858,ibm00858,"
#define ENC_PARSE_NAM_DOS_860              ",CP-860,cp860,ibm860,"
#define ENC_PARSE_NAM_MAC_ROMANIAN         ",x-mac-romanian,xmacromanian,mac-romanian,macromanian,"
#define ENC_PARSE_NAM_MAC_THAI             ",x-mac-thai,xmacthai,mac-thai,macthai,"
#define ENC_PARSE_NAM_WIN_874              ",Windows-874,windows874,dos874,CP-874,cp874,ISO-8859-11,iso885911,TIS-620,tis620,isoir166,"
#define ENC_PARSE_NAM_DOS_857              ",CP-857,cp857,ibm857,"
#define ENC_PARSE_NAM_ISO_8859_9           ",ISO-8859-9,iso88599,latin5,isoir148,l5,"
#define ENC_PARSE_NAM_MAC_TURKISH          ",x-mac-turkish,xmacturkish,mac-turkish,macturkish,"
#define ENC_PARSE_NAM_WIN_1254             ",Windows-1254,windows1254,CP-1254,cp1254,"
#define ENC_PARSE_NAM_MAC_UKRAINIAN        ",x-mac-ukrainian,xmacukrainian,mac-ukrainian,macukrainian,"
#define ENC_PARSE_NAM_WIN_1258             ",Windows-1258,windows1258,CP-1258,cp1258,VISCII,viscii,csviscii,ansivietnamese,"
#define ENC_PARSE_NAM_DOS_850              ",CP-850,cp850,ibm850,"
#define ENC_PARSE_NAM_ISO_8859_1           ",ISO-8859-1,iso88591,CP-819,cp819,latin1,ibm819,isoir100,latin1,l1,"
#define ENC_PARSE_NAM_MAC_WESTERN_EUROP    ",macintosh,macintosh,"
#define ENC_PARSE_NAM_WIN_1252             ",Windows-1252,windows1252,CP-1252,cp1252,CP-367,cp367,ibm367,us,xansi,"
#define ENC_PARSE_NAM_IBM_EBCDIC_US        ",ebcdic-cp-us,ebcdiccpus,ebcdiccpca,ebcdiccpwt,ebcdiccpnl,ibm037,cp037,"
#define ENC_PARSE_NAM_IBM_EBCDIC_INT       ",x-ebcdic-International,xebcdicinternational,"
#define ENC_PARSE_NAM_IBM_EBCDIC_GR        ",x-ebcdic-GreekModern,xebcdicgreekmodern,"
#define ENC_PARSE_NAM_IBM_EBCDIC_LAT_5     ",CP-1026,cp1026,ibm1026,csibm1026,"
#define ENC_PARSE_NAM_GB18030              ",GB-18030,gb18030,"
#define ENC_PARSE_NAM_EUC_JAPANESE         ",euc-jp,euc_jp,eucjp,xeuc,xeucjp,"
#define ENC_PARSE_NAM_EUC_KOREAN           ",euc-kr,euckr,cseuckr,"
#define ENC_PARSE_NAM_ISO_2022_CN          ",ISO-2022-CN,iso2022cn,"
#define ENC_PARSE_NAM_HZ_GB2312            ",HZ-GB-2312,hzgb2312,hz,"
#define ENC_PARSE_NAM_ISO_2022_JP          ",ISO-2022-JP,iso2022jp,"
#define ENC_PARSE_NAM_ISO_2022_KR          ",ISO-2022-KR,iso2022kr,csiso2022kr,"
#define ENC_PARSE_NAM_X_CHINESE_CNS        ",X-CHINESE-CNS,xchinesecns,EUC-TW,euctw,"
#define ENC_PARSE_NAM_JOHAB                ",johab,"
#define ENC_PARSE_NAM_BIG5_HKSCS           ",big5hkscs,cnbig5hkscs,xxbig5hkscs,"
#define ENC_PARSE_NAM_ISO_8859_10          ",ISO-8859-10,iso885910,Windows-28600,windows28600,CP-28600,cp28600,"
#define ENC_PARSE_NAM_DOS_855              ",CP-855,cp855,ibm855,"

//=============================================================================

// Missing ICONV Strings:
// -----------------------
// "UTF-32BE / UTF-32LE / X-ISO-10646-UCS-4-34121 / X-ISO-10646-UCS-4-21431"
// "Bulgarian"
// "ISO-8859-10"
// "ISO-8859-11"

extern "C" NP2ENCODING g_Encodings[] = {
    /* 000 */{ NCP_ASCII_7BIT | NCP_ANSI | NCP_RECODE,              CP_ACP,   ENC_PARSE_NAM_ANSI,              IDS_ENC_ANSI,              L"" }, // CPI_ANSI_DEFAULT       0
    /* 001 */{ NCP_ASCII_7BIT | NCP_OEM | NCP_RECODE,               CP_OEMCP, ENC_PARSE_NAM_OEM,               IDS_ENC_OEM,               L"" }, // CPI_OEM                1
    /* 002 */{ NCP_UNICODE | NCP_UNICODE_BOM,                       CP_UTF8,  ENC_PARSE_NAM_UTF16LEBOM,        IDS_ENC_UTF16LEBOM,        L"" }, // CPI_UNICODEBOM         2
    /* 003 */{ NCP_UNICODE | NCP_UNICODE_REVERSE | NCP_UNICODE_BOM, CP_UTF8,  ENC_PARSE_NAM_UTF16BEBOM,        IDS_ENC_UTF16BEBOM,        L"" }, // CPI_UNICODEBEBOM       3
    /* 004 */{ NCP_UNICODE | NCP_RECODE,                            CP_UTF8,  ENC_PARSE_NAM_UTF16LE,           IDS_ENC_UTF16LE,           L"" }, // CPI_UNICODE            4
    /* 005 */{ NCP_UNICODE | NCP_UNICODE_REVERSE | NCP_RECODE,      CP_UTF8,  ENC_PARSE_NAM_UTF16BE,           IDS_ENC_UTF16BE,           L"" }, // CPI_UNICODEBE          5
    /* 006 */{ NCP_ASCII_7BIT | NCP_UNICODE | NCP_UTF8 | NCP_RECODE,CP_UTF8,  ENC_PARSE_NAM_UTF8,              IDS_ENC_UTF8,              L"" }, // CPI_UTF8               6
    /* 007 */{ NCP_UNICODE | NCP_UTF8 | NCP_UTF8_SIGN,              CP_UTF8,  ENC_PARSE_NAM_UTF8SIG,           IDS_ENC_UTF8SIG,           L"" }, // CPI_UTF8SIGN           7
    /* 008 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     CP_UTF7,  ENC_PARSE_NAM_UTF7,              IDS_ENC_UTF7,              L"" }, // CPI_UTF7               8
    /* 009 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     720,      ENC_PARSE_NAM_DOS_720,           IDS_ENC_DOS_720,           L"" },
    /* 010 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     28596,    ENC_PARSE_NAM_ISO_8859_6,        IDS_ENC_ISO_8859_6,        L"" },
    /* 011 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     10004,    ENC_PARSE_NAM_MAC_ARABIC,        IDS_ENC_MAC_ARABIC,        L"" },
    /* 012 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     1256,     ENC_PARSE_NAM_WIN_1256,          IDS_ENC_WIN_1256,          L"" },
    /* 013 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     775,      ENC_PARSE_NAM_DOS_775,           IDS_ENC_DOS_775,           L"" },
    /* 014 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     28594,    ENC_PARSE_NAM_ISO_8859_4,        IDS_ENC_ISO_8859_4,        L"" },
    /* 015 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     1257,     ENC_PARSE_NAM_WIN_1257,          IDS_ENC_WIN_1257,          L"" },
    /* 016 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     852,      ENC_PARSE_NAM_DOS_852,           IDS_ENC_DOS_852,           L"" },
    /* 017 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     28592,    ENC_PARSE_NAM_ISO_8859_2,        IDS_ENC_ISO_8859_2,        L"" },
    /* 018 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     10029,    ENC_PARSE_NAM_MAC_CENTRAL_EUROP, IDS_ENC_MAC_CENTRAL_EUROP, L"" },
    /* 019 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     1250,     ENC_PARSE_NAM_WIN_1250,          IDS_ENC_WIN_1250,          L"" },
    /* 020 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     936,      ENC_PARSE_NAM_GBK_936,           IDS_ENC_GBK_936,           L"" },
    /* 021 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     10008,    ENC_PARSE_NAM_MAC_ZH_CN,         IDS_ENC_MAC_ZH_CN,         L"" },
    /* 022 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     950,      ENC_PARSE_NAM_BIG5,              IDS_ENC_BIG5,              L"" },
    /* 023 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     10002,    ENC_PARSE_NAM_MAC_ZH_TW,         IDS_ENC_MAC_ZH_TW,         L"" },
    /* 024 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     10082,    ENC_PARSE_NAM_MAC_CROATIAN,      IDS_ENC_MAC_CROATIAN,      L"" },
    /* 025 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     866,      ENC_PARSE_NAM_DOS_866,           IDS_ENC_DOS_866,           L"" },
    /* 026 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     28595,    ENC_PARSE_NAM_ISO_8859_5,        IDS_ENC_ISO_8859_5,        L"" },
    /* 027 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     20866,    ENC_PARSE_NAM_KOI8_R,            IDS_ENC_KOI8_R,            L"" },
    /* 028 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     21866,    ENC_PARSE_NAM_KOI8_U,            IDS_ENC_KOI8_U,            L"" },
    /* 029 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     10007,    ENC_PARSE_NAM_MAC_CYRILLIC,      IDS_ENC_MAC_CYRILLIC,      L"" },
    /* 030 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     1251,     ENC_PARSE_NAM_WIN_1251,          IDS_ENC_WIN_1251,          L"" },
    /* 031 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     28603,    ENC_PARSE_NAM_ISO_8859_13,       IDS_ENC_ISO_8859_13,       L"" },
    /* 032 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     863,      ENC_PARSE_NAM_DOS_863,           IDS_ENC_DOS_863,           L"" },
    /* 033 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     737,      ENC_PARSE_NAM_DOS_737,           IDS_ENC_DOS_737,           L"" },
    /* 034 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     28597,    ENC_PARSE_NAM_ISO_8859_7,        IDS_ENC_ISO_8859_7,        L"" },
    /* 035 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     10006,    ENC_PARSE_NAM_MAC_GREEK,         IDS_ENC_MAC_GREEK,         L"" },
    /* 036 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     1253,     ENC_PARSE_NAM_WIN_1253,          IDS_ENC_WIN_1253,          L"" },
    /* 037 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     869,      ENC_PARSE_NAM_DOS_869,           IDS_ENC_DOS_869,           L"" },
    /* 038 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     862,      ENC_PARSE_NAM_DOS_862,           IDS_ENC_DOS_862,           L"" },
    /* 039 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     38598,    ENC_PARSE_NAM_ISO_8859_8_I,      IDS_ENC_ISO_8859_8_I,      L"" },
    /* 040 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     28598,    ENC_PARSE_NAM_ISO_8859_8,        IDS_ENC_ISO_8859_8,        L"" },
    /* 041 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     10005,    ENC_PARSE_NAM_MAC_HEBREW,        IDS_ENC_MAC_HEBREW,        L"" },
    /* 042 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     1255,     ENC_PARSE_NAM_WIN_1255,          IDS_ENC_WIN_1255,          L"" },
    /* 043 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     861,      ENC_PARSE_NAM_DOS_861,           IDS_ENC_DOS_861,           L"" },
    /* 044 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     10079,    ENC_PARSE_NAM_MAC_ICELANDIC,     IDS_ENC_MAC_ICELANDIC,     L"" },
    /* 045 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     10001,    ENC_PARSE_NAM_MAC_JAPANESE,      IDS_ENC_MAC_JAPANESE,      L"" },
    /* 046 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     932,      ENC_PARSE_NAM_SHIFT_JIS,         IDS_ENC_SHIFT_JIS,         L"" },
    /* 047 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     10003,    ENC_PARSE_NAM_MAC_KOREAN,        IDS_ENC_MAC_KOREAN,        L"" },
    /* 048 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     949,      ENC_PARSE_NAM_WIN_949,           IDS_ENC_WIN_949,           L"" },
    /* 049 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     28593,    ENC_PARSE_NAM_ISO_8859_3,        IDS_ENC_ISO_8859_3,        L"" },
    /* 050 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     28605,    ENC_PARSE_NAM_ISO_8859_15,       IDS_ENC_ISO_8859_15,       L"" },
    /* 051 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     865,      ENC_PARSE_NAM_DOS_865,           IDS_ENC_DOS_865,           L"" },
    /* 052 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     437,      ENC_PARSE_NAM_DOS_437,           IDS_ENC_DOS_437,           L"" },
    /* 053 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     858,      ENC_PARSE_NAM_DOS_858,           IDS_ENC_DOS_858,           L"" },
    /* 054 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     860,      ENC_PARSE_NAM_DOS_860,           IDS_ENC_DOS_860,           L"" },
    /* 055 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     10000,    ENC_PARSE_NAM_MAC_WESTERN_EUROP, IDS_ENC_MAC_WESTERN_EUROP, L"" },
    /* 056 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     10021,    ENC_PARSE_NAM_MAC_THAI,          IDS_ENC_MAC_THAI,          L"" },
    /* 057 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     874,      ENC_PARSE_NAM_WIN_874,           IDS_ENC_WIN_874,           L"" },
    /* 058 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     857,      ENC_PARSE_NAM_DOS_857,           IDS_ENC_DOS_857,           L"" },
    /* 059 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     28599,    ENC_PARSE_NAM_ISO_8859_9,        IDS_ENC_ISO_8859_9,        L"" },
    /* 060 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     10081,    ENC_PARSE_NAM_MAC_TURKISH,       IDS_ENC_MAC_TURKISH,       L"" },
    /* 061 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     1254,     ENC_PARSE_NAM_WIN_1254,          IDS_ENC_WIN_1254,          L"" },
    /* 062 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     10017,    ENC_PARSE_NAM_MAC_UKRAINIAN,     IDS_ENC_MAC_UKRAINIAN,     L"" },
    /* 063 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     1258,     ENC_PARSE_NAM_WIN_1258,          IDS_ENC_WIN_1258,          L"" },
    /* 064 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     850,      ENC_PARSE_NAM_DOS_850,           IDS_ENC_DOS_850,           L"" },
    /* 065 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     28591,    ENC_PARSE_NAM_ISO_8859_1,        IDS_ENC_ISO_8859_1,        L"" },
    /* 066 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     10010,    ENC_PARSE_NAM_MAC_ROMANIAN,      IDS_ENC_MAC_ROMANIAN,      L"" },
    /* 067 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     1252,     ENC_PARSE_NAM_WIN_1252,          IDS_ENC_WIN_1252,          L"" },
    /* 068 */{ NCP_EXTERNAL_8BIT | NCP_RECODE,                      37,       ENC_PARSE_NAM_IBM_EBCDIC_US,     IDS_ENC_IBM_EBCDIC_US,     L"" },
    /* 069 */{ NCP_EXTERNAL_8BIT | NCP_RECODE,                      500,      ENC_PARSE_NAM_IBM_EBCDIC_INT,    IDS_ENC_IBM_EBCDIC_INT,    L"" },
    /* 070 */{ NCP_EXTERNAL_8BIT | NCP_RECODE,                      875,      ENC_PARSE_NAM_IBM_EBCDIC_GR,     IDS_ENC_IBM_EBCDIC_GR,     L"" },
    /* 071 */{ NCP_EXTERNAL_8BIT | NCP_RECODE,                      1026,     ENC_PARSE_NAM_IBM_EBCDIC_LAT_5,  IDS_ENC_IBM_EBCDIC_LAT_5,  L"" },
    /* 072 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     20936,    ENC_PARSE_NAM_GB2312_80,         IDS_ENC_GB2312_80,         L"" }, // Chinese Simplified (GB2312-80)
    /* 073 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     54936,    ENC_PARSE_NAM_GB18030,           IDS_ENC_GB18030,           L"" }, // Chinese Simplified (GB18030)
    /* 074 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     20932,    ENC_PARSE_NAM_EUC_JAPANESE,      IDS_ENC_EUC_JAPANESE,      L"" }, // Japanese (EUC)
    /* 075 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     51949,    ENC_PARSE_NAM_EUC_KOREAN,        IDS_ENC_EUC_KOREAN,        L"" }, // Korean (EUC)
    /* 076 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     50229,    ENC_PARSE_NAM_ISO_2022_CN,       IDS_ENC_ISO_2022_CN,       L"" }, // Chinese Traditional (ISO-2022-CN)
    /* 077 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     52936,    ENC_PARSE_NAM_HZ_GB2312,         IDS_ENC_HZ_GB2312,         L"" }, // Chinese Simplified (HZ-GB2312)
    /* 078 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     50220,    ENC_PARSE_NAM_ISO_2022_JP,       IDS_ENC_ISO_2022_JP,       L"" }, // Japanese (JIS)
    /* 079 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     50225,    ENC_PARSE_NAM_ISO_2022_KR,       IDS_ENC_ISO_2022_KR,       L"" }, // Korean (ISO-2022-KR)
    /* 080 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     20000,    ENC_PARSE_NAM_X_CHINESE_CNS,     IDS_ENC_X_CHINESE_CNS,     L"" }, // Chinese Traditional (CNS)
    /* 081 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     1361,     ENC_PARSE_NAM_JOHAB,             IDS_ENC_JOHAB,             L"" }, // Korean (Johab)
    // may need special codepage installation on some
    /* 082 */{ NCP_EXTERNAL_8BIT | NCP_RECODE,                      951,      ENC_PARSE_NAM_BIG5_HKSCS,        IDS_ENC_BIG5_HKSCS,        L"" }, // Chinese (Hong Kong Supplementary Character Set)
    /* 083 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     28600,    ENC_PARSE_NAM_ISO_8859_10,       IDS_ENC_ISO_8859_10,       L"" }, // Nordic (ISO 8859-10)
    /* 084 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     855,      ENC_PARSE_NAM_DOS_855,           IDS_ENC_DOS_855,           L"" }  // Cyrillic (CP-855)


#if 0
    NP2ENCODING mEncoding[] = {
        { NCP_8BIT | NCP_RECODE, 38596, "ISO-8859-6-I,ISO88596I,", 61011, L"" },// Arabic (ISO 8859-6-I Logical)
        { NCP_8BIT | NCP_RECODE, 28604, "ISO-8859-14,ISO885914,Windows-28604,windows28604,", 61055, L"" },// Celtic (ISO 8859-14)
        { NCP_8BIT | NCP_RECODE, 28606, "ISO-8859-16,ISO885916,Windows-28606,windows28606", 61054, L"" },// Latin-10 (ISO 8859-16)
    };
#endif

};

constexpr cpi_enc_t _CountOfEncodings()
{
    return static_cast<cpi_enc_t>(ARRAYSIZE(g_Encodings));
}

extern "C" cpi_enc_t Encoding_CountOf()
{
    return _CountOfEncodings();
}

extern "C" void ChangeEncodingCodePage(const cpi_enc_t cpi, UINT newCP)
{
    if (Encoding_IsValidIdx(cpi)) {
        g_Encodings[cpi].uCodePage = newCP;
    }
}

//=============================================================================

constexpr float clampf(float x, float lower, float upper)
{
    return (x < lower) ? lower : ((x > upper) ? upper : x);
}

//=============================================================================

// ============================================================================

static bool IsValidUTF16(const char* pBuffer, size_t len, bool bBigEndian)
{
    bool bHighSurrogate = false;
    for (size_t i = 0; i + 1 < len; i += 2) {
        const unsigned char* const p = (const unsigned char*)pBuffer + i;
        unsigned int const unit = bBigEndian ? ((p[0] << 8) | p[1]) : ((p[1] << 8) | p[0]);
        if (bHighSurrogate) {
            if (unit < 0xDC00 || unit > 0xDFFF) {
                return false;
            }
            bHighSurrogate = false;
        } else if (unit >= 0xD800 && unit <= 0xDBFF) {
            bHighSurrogate = true;
        } else if (unit >= 0xDC00 && unit <= 0xDFFF) {
            return false;
        }
    }
    return !bHighSurrogate;
}

// Supplementary BOM-less UTF-16 detection via null-byte distribution analysis.
// This heuristic counts null bytes at even vs odd byte positions.
// In UTF-16 LE, ASCII chars produce nulls at odd positions (e.g. 'A' = 41 00).
// In UTF-16 BE, ASCII chars produce nulls at even positions (e.g. 'A' = 00 41).
//
// A trailing odd byte (truncated file) is ignored. A few nulls at the opposite position
// (including surrogate units with zero low bytes) are tolerated. Strong evidence
// requires at least 10% dominant-position NULs and valid surrogate pairing.
//
static cpi_enc_t DetectUTF16ByNullDistribution(const char* pBuffer, size_t len, bool* pbStrong)
{
    *pbStrong = false;
    if (!pBuffer || len < 2) {
        return CPI_NONE;
    }

    size_t const cb = min_s(len & ~(size_t)1, 4096LL);
    const unsigned char* p = (const unsigned char*)pBuffer;

    size_t nullsEven = 0;  // null bytes at even positions (0, 2, 4, ...)
    size_t nullsOdd = 0;   // null bytes at odd positions (1, 3, 5, ...)

    for (size_t i = 0; i < cb; i += 2) {
        if (p[i] == 0)     { ++nullsEven; }
        if (p[i + 1] == 0) { ++nullsOdd; }
    }

    // In valid UTF-16, nulls appear (almost) exclusively at one byte position (even XOR odd).
    // We need a minimum count to avoid false positives from stray null bytes in binary data.
    size_t const minNulls = max_s(2, cb / 256LL);
    size_t const units = cb / 2;

    if (nullsOdd >= minNulls && nullsEven <= (nullsOdd >> 5) && IsValidUTF16(pBuffer, len, false)) {
        *pbStrong = (nullsOdd * 10 >= units);
        return CPI_UNICODE;     // UTF-16 LE: nulls at odd positions
    }
    if (nullsEven >= minNulls && nullsOdd <= (nullsEven >> 5) && IsValidUTF16(pBuffer, len, true)) {
        *pbStrong = (nullsEven * 10 >= units);
        return CPI_UNICODEBE;   // UTF-16 BE: nulls at even positions
    }
    return CPI_NONE;
}

// ============================================================================

static cpi_enc_t DetectUTF16ByWindows(const char* pBuffer, size_t len)
{
    if (len < 2 || (len & 1)) {
        return CPI_NONE;
    }
    int const tests = IS_TEXT_UNICODE_UNICODE_MASK | IS_TEXT_UNICODE_REVERSE_MASK |
                      IS_TEXT_UNICODE_NOT_UNICODE_MASK | IS_TEXT_UNICODE_NOT_ASCII_MASK;
    int flags = tests;
    // The Boolean result can be false even when reverse-order evidence is present.
    IsTextUnicode(pBuffer, (int)min_s(len, 4096LL), &flags);
    if (flags == tests || (flags & IS_TEXT_UNICODE_ILLEGAL_CHARS)) {
        return CPI_NONE;
    }
    bool const bLE = (flags & IS_TEXT_UNICODE_UNICODE_MASK) != 0;
    bool const bBE = (flags & IS_TEXT_UNICODE_REVERSE_MASK) != 0;
    if (bLE != bBE && IsValidUTF16(pBuffer, len, bBE)) {
        return bBE ? CPI_UNICODEBE : CPI_UNICODE;
    }
    return CPI_NONE;
}

// ============================================================================

// BOM-less UTF-32 detection: every 32-bit unit must be a code point <= U+10FFFF
// (the 2 bytes at the most significant end are 0x00 0x00..0x10).
//
static bool DetectUTF32ByRange(const char* pBuffer, size_t len, bool* pbBigEndian)
{
    if (!pBuffer || len < 8 || (len & 3)) {
        return false;
    }
    size_t const cb = min_s(len, 4096LL) & ~(size_t)3;
    const unsigned char* p = (const unsigned char*)pBuffer;

    bool bLE = true;
    bool bBE = true;
    bool bAnyNonZero = false;

    for (size_t i = 0; i < cb && (bLE || bBE); i += 4) {
        bLE = bLE && (p[i + 3] == 0) && (p[i + 2] <= 0x10);
        bBE = bBE && (p[i] == 0) && (p[i + 1] <= 0x10);
        bAnyNonZero = bAnyNonZero || p[i] || p[i + 1] || p[i + 2] || p[i + 3];
    }
    if (!bAnyNonZero || (bLE == bBE)) {
        return false; // all zero or ambiguous
    }
    *pbBigEndian = bBE;
    return true;
}
// ============================================================================

constexpr cpi_enc_t _MapStdEncodingString2CPI(const char* encStrg, float* pConfidence)
{
    float const confidence = *pConfidence;

    cpi_enc_t cpiEncoding = CPI_NONE;

    if (encStrg && (encStrg[0] != '\0')) {
        // preprocessing: special cases
        if (StrCmpICA(encStrg, "ascii") == 0) {
            cpiEncoding = CPI_ASCII_7BIT;
        } else if (StrCmpICA(encStrg, "utf-8") == 0) {
            cpiEncoding = CPI_UTF8;
        } else {
            cpiEncoding = Encoding_MatchA(encStrg);
        }
        // check for default ANSI
        if (cpiEncoding > CPI_ANSI_DEFAULT) {
            if (g_Encodings[cpiEncoding].uCodePage == g_Encodings[CPI_ANSI_DEFAULT].uCodePage) {
                cpiEncoding = CPI_ANSI_DEFAULT;
            }
        }
    }
    *pConfidence = Encoding_IsNONE(cpiEncoding) ? 0.0f : confidence;
    return cpiEncoding;
}
// ============================================================================

// ============================================================================
//   UCHARDET (Universal CharacterSet Detector)
// ============================================================================

cpi_enc_t AnalyzeText_UCHARDET(
    const char* const text, const size_t len,
    float* pConfidence, char* encodingStrg, int cch)
{
    cpi_enc_t cpiEncoding = CPI_NONE;
    float confidence = 0.0f;

    uchardet_t hUcharDet = uchardet_new();

    uchardet_set_language_filter(hUcharDet, static_cast<unsigned int>(Settings2.UchardetLanguageFilter));

    int const result = uchardet_handle_data(hUcharDet, text, len);

    uchardet_data_end(hUcharDet); // transfer report

    switch (result) {
    case HANDLE_DATA_RESULT_NEED_MORE_DATA:  // need more data is a result too
    case HANDLE_DATA_RESULT_DETECTED: {
        const char* charset = uchardet_get_charset(hUcharDet);
        StringCchCopyA(encodingStrg, cch, charset);  // UCHARDET

        confidence = uchardet_get_confidence(hUcharDet);
        cpiEncoding = _MapStdEncodingString2CPI(charset, &confidence);
    }
    break;

    case HANDLE_DATA_RESULT_ERROR:
    default:
        cpiEncoding = CPI_NONE;
        confidence = 0.0f;
        break;
    }

    uchardet_delete(hUcharDet);

    *pConfidence = clampf(confidence, 0.0f, 1.0f);
    return cpiEncoding;
}


// ============================================================================

// ESC (ISO-2022-xx) or "~{" (HZ-GB-2312) in 7-bit data may announce a stateful encoding
static bool _MayBeStateful7Bit(const char* text, size_t len)
{
    if (memchr(text, 0x1B, len)) {
        return true;
    }
    for (const char* p = (const char*)memchr(text, '~', len); p; ) {
        size_t const rest = len - (size_t)(p - text);
        if ((rest > 1) && (p[1] == '{')) {
            return true;
        }
        p = (rest > 1) ? (const char*)memchr(p + 1, '~', rest - 1) : NULL;
    }
    return false;
}

// ============================================================================

void Encoding_AnalyzeText(const char* const text, const size_t len,
                          ENC_DET_T* pEncDetInfo, const cpi_enc_t encodingHint)
{
    pEncDetInfo->bIsAnalyzed = true;

    if (len == 0) {
        pEncDetInfo->analysisSkipReason = L"empty";
        pEncDetInfo->analyzedEncoding = CPI_NONE;
        pEncDetInfo->confidence = 0.0f;
        return;
    }

    float confidence_UCD = 0.0f;
    cpi_enc_t cpiEncoding_UCD = CPI_NONE;

    // Pure 7-bit ASCII is a certain result - skip the expensive uchardet run, unless the
    // text may be a 7-bit stateful encoding (ISO-2022-xx: ESC sequences, HZ: "~{").
    if (pEncDetInfo->bPureASCII7Bit && !Flags.bDevDebugMode && !_MayBeStateful7Bit(text, len)) {
        pEncDetInfo->analysisSkipReason = L"ASCII";
        StringCchCopyA(pEncDetInfo->encodingStrg, COUNTOF(pEncDetInfo->encodingStrg), "ASCII");
        pEncDetInfo->confidence = 1.0f;
        pEncDetInfo->analyzedEncoding = CPI_ASCII_7BIT;
        return;
    }

    pEncDetInfo->bUCDRan = true;
    cpiEncoding_UCD = AnalyzeText_UCHARDET(text, len, &confidence_UCD, pEncDetInfo->encodingStrg, COUNTOF(pEncDetInfo->encodingStrg));

    // Hint and locale are preferences, not independent evidence: apply one bonus.
    if (Encoding_IsValid(cpiEncoding_UCD) &&
        (cpiEncoding_UCD == encodingHint || cpiEncoding_UCD == CPI_ANSI_DEFAULT)) {
        confidence_UCD += (1.0f - confidence_UCD) * Settings2.LocaleAnsiCodePageAnalysisBonus;
    }

    pEncDetInfo->confidence = confidence_UCD;
    pEncDetInfo->analyzedEncoding = cpiEncoding_UCD;

}
// ============================================================================


//=============================================================================
//
//  _SetEncodingTitleInfo()
//
static const WCHAR* _EncodingDebugName(cpi_enc_t encoding, WCHAR* buffer, size_t count)
{
    switch (encoding) {
    case CPI_NONE: return L"none";
    case CPI_ASCII_7BIT: return L"ASCII";
    case CPI_UTF8: return L"UTF-8";
    case CPI_UTF8SIGN: return L"UTF-8+BOM";
    case CPI_UNICODE: return L"UTF-16LE";
    case CPI_UNICODEBE: return L"UTF-16BE";
    case CPI_UNICODEBOM: return L"UTF-16LE+BOM";
    case CPI_UNICODEBEBOM: return L"UTF-16BE+BOM";
    default:
        StringCchPrintf(buffer, count, L"CP%u", Encoding_GetCodePage(encoding));
        return buffer;
    }
}

static void _SetEncodingTitleInfo(const ENC_DET_T* result)
{
    WCHAR name[32];
    const WCHAR* const utf8 = (result->bIsUTF32 && result->bHasBOM) ? L"not checked" :
        result->bPureASCII7Bit ? L"ASCII" : result->bValidUTF8 ? L"valid" :
        result->bUTF8WithNulls ? L"valid+NUL" : L"invalid";
    StringCchPrintf(wchEncodingInfo, COUNTOF(wchEncodingInfo), L"ENC=%s <- %s | U8=%s",
                    _EncodingDebugName(result->Encoding, name, COUNTOF(name)), result->selectionReason, utf8);
    WCHAR part[192];
    if (!Encoding_IsNONE(result->unicodeAnalysis)) {
        StringCchPrintf(part, COUNTOF(part), L" | U16=%s/%s",
                        _EncodingDebugName(result->unicodeAnalysis, name, COUNTOF(name)),
                        wcscmp(result->unicodeReason, L"BOM") == 0 ? L"BOM" : L"valid");
        StringCchCat(wchEncodingInfo, COUNTOF(wchEncodingInfo), part);
    }
    if (result->bUCDRan) {
        WCHAR candidate[64] = L"";
        MultiByteToWideChar(CP_ACP, 0, result->encodingStrg, -1, candidate, COUNTOF(candidate));
        StringCchPrintf(part, COUNTOF(part), L" | UCD(%.1f%%)=%s",
                        result->confidence * 100.0, candidate[0] ? candidate : L"none");
    } else {
        StringCchPrintf(part, COUNTOF(part), L" | UCD=skipped(%s)", result->analysisSkipReason);
    }
    StringCchCat(wchEncodingInfo, COUNTOF(wchEncodingInfo), part);
}



//=============================================================================
//
//  _ParseVimModeline()
//
//  Recognizes the Vim modeline forms:
//    [text] vim: option=value option=value :    (also vi: / ex:)
//    [text] vim:set option=value option=value:  (with optional "set" keyword)
//    [text] vim:option=value:option=value       (colon-separated form)
//  The marker must be preceded by whitespace or be at line start.
//  Honored options:
//    ts/tabstop, sw/shiftwidth, et/expandtab, noet/noexpandtab,
//    wrap/nowrap, tw/textwidth, ft/filetype
//
static void _ParseVimModeline(const char* buffer, LPFILEVARS lpfv)
{
    static const char* const markers[] = { "vim:", "ex:", "vi:" };
    const char* mlStart = NULL;
    for (int m = 0; m < (int)COUNTOF(markers) && !mlStart; ++m) {
        size_t const mLen = StringCchLenA(markers[m], 0);
        const char* p = StrStrIA_UTF8(buffer, markers[m]);
        while (p) {
            if (p == buffer || p[-1] == ' ' || p[-1] == '\t') {
                mlStart = p + mLen;
                break;
            }
            p = StrStrIA_UTF8(p + 1, markers[m]);
        }
    }
    if (!mlStart) {
        return;
    }

    const char* mlEnd = mlStart;
    while (*mlEnd && *mlEnd != '\r' && *mlEnd != '\n') {
        ++mlEnd;
    }

    const char* p = mlStart;
    while (p < mlEnd && (*p == ' ' || *p == '\t')) {
        ++p;
    }

    bool bSetForm = false;
    if ((p + 4 <= mlEnd) && (_strnicmp(p, "set ", 4) == 0)) {
        bSetForm = true;
        p += 4;
    } else if ((p + 3 <= mlEnd) && (_strnicmp(p, "se ", 3) == 0)) {
        bSetForm = true;
        p += 3;
    }

    char nameBuf[32];
    char valBuf[64];
    while (p < mlEnd) {
        while (p < mlEnd && (*p == ' ' || *p == '\t' || *p == ':')) {
            ++p;
        }
        if (p >= mlEnd) {
            break;
        }

        const char* nameStart = p;
        while (p < mlEnd && *p != '=' && *p != ':' && *p != ' ' && *p != '\t') {
            ++p;
        }
        size_t nameLen = (size_t)(p - nameStart);
        if (nameLen == 0) {
            break;
        }
        if (nameLen >= COUNTOF(nameBuf)) {
            nameLen = COUNTOF(nameBuf) - 1;
        }
        memcpy(nameBuf, nameStart, nameLen);
        nameBuf[nameLen] = '\0';

        valBuf[0] = '\0';
        if (p < mlEnd && *p == '=') {
            ++p;
            const char* valStart = p;
            while (p < mlEnd && *p != ':' && (!bSetForm || (*p != ' ' && *p != '\t'))) {
                ++p;
            }
            size_t valLen = (size_t)(p - valStart);
            if (valLen >= COUNTOF(valBuf)) {
                valLen = COUNTOF(valBuf) - 1;
            }
            memcpy(valBuf, valStart, valLen);
            valBuf[valLen] = '\0';
        }

        if ((_stricmp(nameBuf, "ts") == 0) || (_stricmp(nameBuf, "tabstop") == 0)) {
            int const i = atoi(valBuf);
            if (i > 0) {
                lpfv->iTabWidth = clampi(i, 1, 256);
                lpfv->mask |= FV_TABWIDTH;
            }
        } else if ((_stricmp(nameBuf, "sw") == 0) || (_stricmp(nameBuf, "shiftwidth") == 0)) {
            int const i = atoi(valBuf);
            lpfv->iIndentWidth = clampi(i, 0, 256);
            lpfv->mask |= FV_INDENTWIDTH;
        } else if ((_stricmp(nameBuf, "et") == 0) || (_stricmp(nameBuf, "expandtab") == 0)) {
            lpfv->bTabsAsSpaces = true;
            lpfv->mask |= FV_TABSASSPACES;
        } else if ((_stricmp(nameBuf, "noet") == 0) || (_stricmp(nameBuf, "noexpandtab") == 0)) {
            lpfv->bTabsAsSpaces = false;
            lpfv->mask |= FV_TABSASSPACES;
        } else if (_stricmp(nameBuf, "wrap") == 0) {
            lpfv->bWordWrap = true;
            lpfv->mask |= FV_WORDWRAP;
        } else if (_stricmp(nameBuf, "nowrap") == 0) {
            lpfv->bWordWrap = false;
            lpfv->mask |= FV_WORDWRAP;
        } else if ((_stricmp(nameBuf, "tw") == 0) || (_stricmp(nameBuf, "textwidth") == 0)) {
            int const i = atoi(valBuf);
            if (i > 0) {
                WCHAR wbuf[16];
                StringCchPrintf(wbuf, COUNTOF(wbuf), L"%d", i);
                StringCchCopy(lpfv->wchMultiEdgeLines, COUNTOF(lpfv->wchMultiEdgeLines), wbuf);
                lpfv->mask |= FV_LONGLINESLIMIT;
            }
        } else if ((_stricmp(nameBuf, "ft") == 0) || (_stricmp(nameBuf, "filetype") == 0)) {
            if (valBuf[0] && !(lpfv->mask & FV_MODE)) {
                StringCchCopyA(lpfv->chMode, COUNTOF(lpfv->chMode), valBuf);
                lpfv->mask |= FV_MODE;
            }
        }
    }
}


//=============================================================================
//
//  _CopyScanBuffer()
//
//  Copies up to (dstSize-1) bytes; embedded NUL bytes are replaced by blanks so
//  that file variables behind a stray NUL are still found.
//
static void _CopyScanBuffer(char* dst, size_t dstSize, const char* src, size_t cnt)
{
    size_t const n = min_s(cnt, dstSize - 1);
    for (size_t i = 0; i < n; ++i) {
        dst[i] = src[i] ? src[i] : ' ';
    }
    dst[n] = '\0';
}

//=============================================================================
//
//  _SetFileVars()
//
static void _SetFileVars(const char* buffer, bool bHasSignature, LPFILEVARS lpfv)
{
    // every file variable / encoding tag / modeline needs a ':' or '=' separator
    if (!strpbrk(buffer, ":=")) {
        return;
    }

    bool bEnableFileVar = !Flags.NoFileVariables;

    if (bEnableFileVar) {
        int i;
        if (FileVars_ParseInt(buffer, "enable-local-variables", &i) && (!i)) {
            bEnableFileVar = false;
        }
        if (bEnableFileVar) {

            if (FileVars_ParseInt(buffer, "tab-width", &i)) {
                lpfv->iTabWidth = clampi(i, 1, 256);
                lpfv->mask |= FV_TABWIDTH;
            }

            if (FileVars_ParseInt(buffer, "c-basic-indent", &i)) {
                lpfv->iIndentWidth = clampi(i, 0, 256);
                lpfv->mask |= FV_INDENTWIDTH;
            }

            if (FileVars_ParseInt(buffer, "indent-tabs-mode", &i)) {
                lpfv->bTabsAsSpaces = (i) ? false : true;
                lpfv->mask |= FV_TABSASSPACES;
            }

            if (FileVars_ParseInt(buffer, "c-tab-always-indent", &i)) {
                lpfv->bTabIndents = (i) ? true : false;
                lpfv->mask |= FV_TABINDENTS;
            }

            if (FileVars_ParseInt(buffer, "truncate-lines", &i)) {
                lpfv->bWordWrap = (i) ? false : true;
                lpfv->mask |= FV_WORDWRAP;
            }

            char columns[SMALL_BUFFER];
            if (FileVars_ParseStr(buffer, "fill-column", columns, COUNTOF(columns))) {
                NormalizeColumnVector(columns, lpfv->wchMultiEdgeLines, COUNTOF(lpfv->wchMultiEdgeLines));
                lpfv->mask |= FV_LONGLINESLIMIT;
            }

            if (FileVars_ParseStr(buffer, "mode", lpfv->chMode, COUNTOF(lpfv->chMode))) {
                lpfv->mask |= FV_MODE;
            }

            // Vim modelines (`vim:` / `vi:` / `ex:`) — parsed alongside Emacs file variables.
            _ParseVimModeline(buffer, lpfv);
        }
    }

    // ========  Encoding Tags  ========

    if (!Settings.NoEncodingTags) {

        // Signature override Encoding Tag
        if (!bHasSignature) {
            if (FileVars_ParseStr(buffer, "coding", lpfv->chEncoding, COUNTOF(lpfv->chEncoding))) {
                lpfv->mask |= FV_ENCODING;
            } else if (FileVars_ParseStr(buffer, "encoding", lpfv->chEncoding, COUNTOF(lpfv->chEncoding))) {
                lpfv->mask |= FV_ENCODING;
            } else if (FileVars_ParseStr(buffer, "charset", lpfv->chEncoding, COUNTOF(lpfv->chEncoding))) {
                lpfv->mask |= FV_ENCODING;
            }
        }
    }
    if (lpfv->mask & FV_ENCODING) {
        lpfv->iEncoding = Encoding_MatchA(lpfv->chEncoding);
    }

}

//=============================================================================
//
//  FileVars_Init()
//
extern "C" bool FileVars_GetFromData(const char* lpData, size_t cbData, LPFILEVARS lpfv)
{
    ZeroMemory(lpfv, sizeof(FILEVARS));
    lpfv->bTabIndents = Settings.TabIndents;
    lpfv->bTabsAsSpaces = Settings.TabsAsSpaces;
    lpfv->bWordWrap = Settings.WordWrap;
    lpfv->iTabWidth = Settings.TabWidth;
    lpfv->iIndentWidth = Settings.IndentWidth;
    lpfv->iEncoding = Settings.DefaultEncoding;
    StringCchCopy(lpfv->wchMultiEdgeLines, COUNTOF(lpfv->wchMultiEdgeLines), Settings.MultiEdgeLines);

    if ((Flags.NoFileVariables && Settings.NoEncodingTags) || !lpData || !cbData) {
        return true;
    }

    size_t const scanBytes = (size_t)clampi(Settings2.FileVarScanBytes, MIDSZ_BUFFER, XHUGE_BUFFER);
    char tmpbuf[XHUGE_BUFFER];

    // an encoding tag must not override a signature (BOM) at the very start of the file
    bool const bHasSignature = (cbData >= 3 && IsUTF8Signature(lpData)) || Has_UTF16_LE_BOM(lpData, cbData) || Has_UTF16_BE_BOM(lpData, cbData);

    _CopyScanBuffer(tmpbuf, scanBytes, lpData, cbData);
    _SetFileVars(tmpbuf, bHasSignature, lpfv);

    // if no file vars found, look at EOF
    if ((lpfv->mask == 0) && (cbData > scanBytes)) {
        _CopyScanBuffer(tmpbuf, scanBytes, lpData + cbData - (scanBytes - 1), scanBytes - 1);
        _SetFileVars(tmpbuf, bHasSignature, lpfv);
    }

    return true;
}


//=============================================================================
//
//  FileVars_Apply()
//
extern "C" bool FileVars_Apply(LPFILEVARS lpfv)
{
    int const _iTabWidth = (lpfv->mask & FV_TABWIDTH) ? lpfv->iTabWidth : Settings.TabWidth;
    SciCall_SetTabWidth(_iTabWidth);

    int const _iIndentWidth = (lpfv->mask & FV_INDENTWIDTH) ? lpfv->iIndentWidth : ((lpfv->mask & FV_TABWIDTH) ? 0 : Settings.IndentWidth);
    SciCall_SetIndent(_iIndentWidth);

    bool const _bTabsAsSpaces = (lpfv->mask & FV_TABSASSPACES) ? lpfv->bTabsAsSpaces : Settings.TabsAsSpaces;
    SciCall_SetUseTabs(!_bTabsAsSpaces);

    bool const _bTabIndents = (lpfv->mask & FV_TABINDENTS) ? lpfv->bTabIndents : Settings.TabIndents;
    SciCall_SetTabIndents(_bTabIndents);
    SciCall_SetBackSpaceUnIndents(Settings.BackspaceUnindents);

    Globals.fvCurFile.bWordWrap = (lpfv->mask & FV_WORDWRAP) ? lpfv->bWordWrap : Settings.WordWrap;
    Sci_SetWrapModeEx(GET_WRAP_MODE());

    int          edgeColumns[EDGELINE_NUM_LIMIT];
    size_t const cnt = ReadVectorFromString(lpfv->wchMultiEdgeLines, edgeColumns, COUNTOF(edgeColumns), 0, LONG_LINES_MARKER_LIMIT, 0, true);
    Style_SetMultiEdgeLine(edgeColumns, cnt);

    return true;
}


//=============================================================================
//
//  FileVars_ParseInt()
//
extern "C" bool FileVars_ParseInt(const char* pszData, const char* pszName, int* piValue)
{

    char* pvStart = StrStrIA_UTF8(pszData, pszName);
    while (pvStart) {
        char chPrev = (pvStart > pszData) ? *(pvStart - 1) : 0;
        if (!IsCharAlphaNumericA(chPrev) && chPrev != '-' && chPrev != '_') {
            pvStart += StringCchLenA(pszName, 0);
            while (*pvStart == ' ') {
                pvStart++;
            }
            if (*pvStart == ':' || *pvStart == '=') {
                break;
            }
        } else {
            pvStart += StringCchLenA(pszName, 0);
        }
        pvStart = StrStrIA_UTF8(pvStart, pszName); // next
    }

    if (pvStart) {

        while (*pvStart && StrChrIA(":=\"' \t", *pvStart)) {
            pvStart++;
        }
        char tch[32] = { L'\0' };
        StringCchCopyNA(tch, COUNTOF(tch), pvStart, COUNTOF(tch));

        char* pvEnd = tch;
        while (*pvEnd && IsCharAlphaNumericA(*pvEnd)) {
            pvEnd++;
        }
        *pvEnd = 0;
        StrTrimUTF8(tch, " \t:=\"'");

        int itok = sscanf_s(tch, "%i", piValue);
        if (itok == 1) {
            return true;
        }
        if (tch[0] == 't') {
            *piValue = 1;
            return true;
        }
        if (tch[0] == 'n' || tch[0] == 'f') {
            *piValue = 0;
            return true;
        }
    }
    return false;
}


//=============================================================================
//
//  FileVars_ParseStr()
//
extern "C" bool FileVars_ParseStr(const char* pszData, const char* pszName, char* pszValue, int cchValue)
{

    const char* pvStart = StrStrIA_UTF8(pszData, pszName);
    while (pvStart) {
        char chPrev = (pvStart > pszData) ? *(pvStart - 1) : 0;
        if (!IsCharAlphaNumericA(chPrev) && chPrev != '-' && chPrev != '_') {
            pvStart += StringCchLenA(pszName, 0);
            while (*pvStart == ' ') {
                pvStart++;
            }
            if (*pvStart == ':' || *pvStart == '=') {
                break;
            }
        } else {
            pvStart += StringCchLenA(pszName, 0);
        }
        pvStart = StrStrIA_UTF8(pvStart, pszName);  // next
    }

    if (pvStart) {
        bool bQuoted = false;
        while (*pvStart && StrChrIA(":=\"' \t", *pvStart)) {
            if (*pvStart == '\'' || *pvStart == '"') {
                bQuoted = true;
            }
            pvStart++;
        }

        char tch[32] = { L'\0' };
        StringCchCopyNA(tch, COUNTOF(tch), pvStart, COUNTOF(tch));

        char* pvEnd = tch;
        while (*pvEnd && (IsCharAlphaNumericA(*pvEnd) || StrChrIA("+-/_", *pvEnd) || (bQuoted && *pvEnd == ' '))) {
            pvEnd++;
        }
        *pvEnd = 0;

        StrTrimUTF8(tch, " \t:=\"'");

        StringCchCopyNA(pszValue, cchValue, tch, COUNTOF(tch));

        return true;
    }
    return false;
}


//=============================================================================
//
//  FileVars_IsUTF8()
//
extern "C" bool FileVars_IsUTF8(LPFILEVARS lpfv)
{
    if (lpfv->mask & FV_ENCODING) {
        if (StrCmpIA(lpfv->chEncoding, "utf-8") == 0 || StrCmpIA(lpfv->chEncoding, "utf8") == 0) {
            return true;
        }
    }
    return false;
}


//=============================================================================
//
//  FileVars_IsValidEncoding()
//
extern "C" bool FileVars_IsValidEncoding(LPFILEVARS lpfv)
{
    CPINFO cpi;
    if (lpfv->mask & FV_ENCODING && Encoding_IsValidIdx(lpfv->iEncoding)) {
        if ((Encoding_IsINTERNAL(lpfv->iEncoding)) ||
                (IsValidCodePage(Encoding_GetCodePage(lpfv->iEncoding)) &&
                 GetCPInfo(Encoding_GetCodePage(lpfv->iEncoding), &cpi))) {
            return true;
        }
    }
    return false;
}


//=============================================================================
//
//  FileVars_GetEncoding()
//
extern "C" cpi_enc_t FileVars_GetEncoding(LPFILEVARS lpfv)
{
    if (lpfv->mask & FV_ENCODING) {
        return(lpfv->iEncoding);
    }
    return CPI_NONE;
}

//=============================================================================
//=============================================================================

__forceinline static cpi_enc_t _SetAnsiDefaultEncoding(const ENC_DET_T* const encDetRes)
{
    // Pure ASCII (7-bit) - use UTF-8 if configured, otherwise ANSI default
    if (Settings.LoadASCIIasUTF8 && encDetRes->bPureASCII7Bit) {
        return CPI_UTF8;
    }
    if (Encoding_IsValid(Settings.DefaultEncoding)) {
        if (Settings.DefaultEncoding == CPI_UTF8) {
            if (encDetRes->bValidUTF8 || encDetRes->bUTF8WithNulls) {
                return Settings.DefaultEncoding;
            }
        }
        else if (Encoding_IsEXTERNAL_8BIT(Settings.DefaultEncoding)) {
            return Settings.DefaultEncoding;
        }
    }
    return CPI_ANSI_DEFAULT;
}


static void _SetResultingEncoding(ENC_DET_T* encDetRes, bool bBOM_LE, bool bBOM_BE, bool bStrongUTF16)
{
    if (encDetRes->bIsUTF8Sig) {
        encDetRes->Encoding = CPI_UTF8SIGN;
        encDetRes->selectionReason = L"BOM";
    }
    else if (bBOM_LE || bBOM_BE) {
        encDetRes->Encoding = bBOM_LE ? CPI_UNICODEBOM : CPI_UNICODEBEBOM;
        encDetRes->bIsReverse = bBOM_BE;
        encDetRes->selectionReason = L"BOM";
    }
    else if (bStrongUTF16) {
        encDetRes->Encoding = encDetRes->unicodeAnalysis;
        encDetRes->selectionReason = encDetRes->unicodeReason;
    }
    else if (Encoding_IsValid(encDetRes->analyzedEncoding) && encDetRes->bIsAnalysisReliable) {
        // Valid UTF-8 byte sequence (no null bytes) prefer UTF-8 over
        // analyzed encoding. ASCII-only files are also valid UTF-8.
        if ((encDetRes->bValidUTF8) && (encDetRes->confidence < MIN_CONFIDENCE_ANSI_NOT_UTF8)) {
            encDetRes->Encoding = CPI_UTF8;
            encDetRes->selectionReason = L"UTF-8(valid)";
        }
        else {
            encDetRes->Encoding = encDetRes->analyzedEncoding;
            encDetRes->selectionReason = L"UCD";
        }
    }
    else if (Encoding_IsUNICODE(encDetRes->unicodeAnalysis)) {
        // unicodeAnalysis confirms Unicode structure.
        // Only prefer analyzedEncoding if it's also Unicode (e.g., UCHARDET
        // identified specific UTF-16 variant). Otherwise use unicodeAnalysis.
        if (Encoding_IsValid(encDetRes->analyzedEncoding) && Encoding_IsUNICODE(encDetRes->analyzedEncoding)) {
            encDetRes->Encoding = encDetRes->analyzedEncoding;
            encDetRes->selectionReason = L"UCD";
        }
        else {
            encDetRes->Encoding = encDetRes->unicodeAnalysis;
            encDetRes->selectionReason = encDetRes->unicodeReason;
        }
    }
    
    // Non-ASCII data where all bytes form valid UTF-8 multi-byte sequences.
    // When UCHARDET was skipped or returned unreliable results, valid
    // multi-byte UTF-8 is a strong structural signal — prefer UTF-8 over
    // blind ANSI fallback.
    if (Encoding_IsNONE(encDetRes->Encoding))
    {
        if (Settings.UseDefaultForFileEncoding) {
            encDetRes->Encoding = _SetAnsiDefaultEncoding(encDetRes);
            encDetRes->selectionReason = L"DEF";
        }
        else if (((encDetRes->bValidUTF8 || encDetRes->bUTF8WithNulls) && !encDetRes->bPureASCII7Bit) ||
                 (encDetRes->bPureASCII7Bit && Settings.LoadASCIIasUTF8)) {
            // Promote to UTF-8 when either:
            //  - the file has multi-byte valid UTF-8 sequences (always preferred), or
            //  - it's pure ASCII AND the user opted in via LoadASCIIasUTF8.
            // Splitting the disjuncts is required: pure ASCII implies bValidUTF8, so
            // a flat `bValidUTF8 || (bPureASCII7Bit && LoadASCIIasUTF8)` makes the
            // setting dead — pure-ASCII files would always become UTF-8.
            encDetRes->Encoding = CPI_UTF8;
            encDetRes->selectionReason = encDetRes->bPureASCII7Bit ? L"ASCII-PREF" : L"UTF-8(valid)";
        }
        else {
            cpi_enc_t const weak = Encoding_SrcWeak(CPI_GET);
            if (Encoding_IsValid(weak)) {
                if ((weak == CPI_UTF8) && encDetRes->bValidUTF8) {
                    // weak UTF-8 hint and data is valid UTF-8 - use weak hint
                    encDetRes->Encoding = weak;
                    encDetRes->selectionReason = L"HINT";
                }
                else if (Encoding_IsEXTERNAL_8BIT(weak)) {
                    encDetRes->Encoding = weak;
                    encDetRes->selectionReason = L"HINT";
                }
            }
        }
    }

    // Final fallback / last resort: if still no encoding, but valid UTF-8, use UTF-8.
    if (Encoding_IsNONE(encDetRes->Encoding)) {
        encDetRes->Encoding = (encDetRes->bValidUTF8) ? CPI_UTF8 : CPI_ANSI_DEFAULT;
        encDetRes->selectionReason = encDetRes->bValidUTF8 ? L"UTF-8(valid)" : L"ANSI-FB";
    }
}

//=============================================================================
//
//  GetFileEncoding()
//

// Large files: analyze head, middle and tail chunks (cut at line breaks) instead of the head only,
// because the characteristic non-ASCII content may start late.
static char* _BuildAnalysisSample(const char* lpData, size_t cbData, size_t* pcbSample)
{
    constexpr size_t HEAD = 100000, MID = 50000, TAIL = 50000;

    auto lineStart = [&](size_t pos) -> size_t { // first position after the next LF within 512 bytes
        size_t const lim = min_s(pos + 512, cbData);
        for (size_t i = pos; i < lim; ++i) {
            if (lpData[i] == '\n') {
                return i + 1;
            }
        }
        return pos;
    };
    auto lineEnd = [&](size_t pos, size_t floor) -> size_t { // last position after an LF within 512 bytes before pos
        size_t const lim = (pos > floor + 512) ? (pos - 512) : floor;
        for (size_t i = pos; i > lim; --i) {
            if (lpData[i - 1] == '\n') {
                return i;
            }
        }
        return pos;
    };

    char* const buf = (char*)AllocMem(HEAD + MID + TAIL + 1, HEAP_ZERO_MEMORY);
    if (!buf) {
        return NULL;
    }
    size_t n = 0;
    size_t const headEnd = lineEnd(HEAD, 0);
    memcpy(buf, lpData, headEnd);
    n = headEnd;

    size_t const midBeg = lineStart((cbData / 2) - (MID / 2));
    size_t const midEnd = lineEnd(midBeg + MID, midBeg);
    memcpy(buf + n, lpData + midBeg, midEnd - midBeg);
    n += (midEnd - midBeg);

    size_t const tailBeg = lineStart(cbData - TAIL);
    memcpy(buf + n, lpData + tailBeg, cbData - tailBeg);
    n += (cbData - tailBeg);

    *pcbSample = n;
    return buf;
}

// ============================================================================

extern "C" ENC_DET_T Encoding_DetectEncoding(const HPATHL hpath,
                                             const char* lpData, const size_t cbData,
                                             cpi_enc_t iAnalyzeHint,
                                             bool bSkipUTFDetection,
                                             bool bSkipANSICPDetection,
                                             bool bForceEncDetection)
{
    ENC_DET_T encDetRes = INIT_ENC_DET_T;

    #define IS_ENC_ENFORCED() (!Encoding_IsNONE(encDetRes.forcedEncoding))

    FileVars_GetFromData(lpData, cbData, &Globals.fvCurFile);

    bool const bBOM_LE = Has_UTF16_LE_BOM(lpData, cbData);
    bool const bBOM_BE = Has_UTF16_BE_BOM(lpData, cbData);
    bool const bUTF8Sig = (cbData >= 3) ? IsUTF8Signature(lpData) : false;

    // --- 1st check for force encodings ---

    LPCWSTR    lpszExt = Path_FindExtension(hpath);
    bool const bNfoDizDetected = (lpszExt && ((StringCchCompareXI(lpszExt, L".nfo") == 0) || (StringCchCompareXI(lpszExt, L".diz") == 0)));

    encDetRes.forcedEncoding = (Settings.LoadNFOasOEM && bNfoDizDetected) ? Globals.DOSEncoding : Encoding_Forced(CPI_GET);
    if (IS_ENC_ENFORCED()) {
        encDetRes.selectionReason = (Settings.LoadNFOasOEM && bNfoDizDetected) ? L"NFO/OEM" : L"OVR";
    }

    if (!IS_ENC_ENFORCED()) {
        // force file vars ?
        encDetRes.fileVarEncoding = (FileVars_IsValidEncoding(&Globals.fvCurFile)) ? FileVars_GetEncoding(&Globals.fvCurFile) : CPI_NONE;
        if (Encoding_IsValid(encDetRes.fileVarEncoding) && (Globals.fvCurFile.mask & FV_ENCODING)) {
            encDetRes.forcedEncoding = encDetRes.fileVarEncoding;
            encDetRes.selectionReason = L"F-TAG";
        }
    }

    // Input signatures describe actual bytes, never the requested save encoding.
    encDetRes.bHasBOM = bBOM_LE || bBOM_BE;
    encDetRes.bIsReverse = bBOM_BE;
    encDetRes.bIsUTF8Sig = bUTF8Sig;

    if (!IS_ENC_ENFORCED() && Has_UTF32_BOM(lpData, cbData)) {
        bool const bBE = Has_UTF32_BE_BOM(lpData, cbData);
        encDetRes.bIsUTF32 = true;
        encDetRes.bHasBOM = true;
        encDetRes.bIsReverse = bBE;
        encDetRes.Encoding = CPI_UTF8SIGN; // converted to UTF-8 on load
        encDetRes.selectionReason = bBE ? L"BOM(U32BE->U8)" : L"BOM(U32LE->U8)";
        encDetRes.analysisSkipReason = L"UTF-32";
        if (Flags.bDevDebugMode) {
            _SetEncodingTitleInfo(&encDetRes);
        }
        return encDetRes;
    }
    // One scan yields strict validity, pure-ASCII, NUL presence and "valid UTF-8 except for NUL bytes".
    encDetRes.bValidUTF8 = IsValidUTF8(lpData, cbData, &(encDetRes.bPureASCII7Bit), &(encDetRes.bHasNullBytes), &(encDetRes.bUTF8WithNulls));

    // --- 2nd Use Encoding Analysis if applicable

    size_t const cbNbytes4Analysis = min_s(cbData, 200000LL);

    encDetRes.confidence = 0.0f;
    bool bStrongUTF16 = false;

    if (!IS_ENC_ENFORCED() || bForceEncDetection) {

        // --- BOM-less UTF-32
        bool bUTF32BE = false;
        if (!IS_ENC_ENFORCED() && !bSkipUTFDetection && !encDetRes.bHasBOM && !bUTF8Sig && encDetRes.bHasNullBytes &&
            DetectUTF32ByRange(lpData, cbData, &bUTF32BE)) {
            encDetRes.bIsUTF32 = true;
            encDetRes.bIsReverse = bUTF32BE;
            encDetRes.Encoding = CPI_UTF8; // converted to UTF-8 on load
            encDetRes.selectionReason = bUTF32BE ? L"UTF-32BE(range->U8)" : L"UTF-32LE(range->U8)";
            encDetRes.analysisSkipReason = L"UTF-32";
            if (Flags.bDevDebugMode) {
                _SetEncodingTitleInfo(&encDetRes);
            }
            return encDetRes;
        }

        // --- Unicode analysis: BOM check + null-byte distribution for BOM-less UTF-16
        if (!bSkipUTFDetection) {

            if (bBOM_LE || bBOM_BE) {
                encDetRes.bHasBOM = true;
                encDetRes.bIsReverse = bBOM_BE;
                encDetRes.unicodeAnalysis = bBOM_BE ? CPI_UNICODEBEBOM : CPI_UNICODEBOM;
                encDetRes.unicodeReason = L"BOM";
            }

            // BOM-less UTF-16 detection (also handles CJK-heavy content)
            else if (encDetRes.bHasNullBytes) {
                cpi_enc_t const nullDist = DetectUTF16ByNullDistribution(lpData, cbData, &bStrongUTF16);
                if (Encoding_IsUNICODE(nullDist)) {
                    encDetRes.unicodeAnalysis = nullDist;
                    encDetRes.bIsReverse = Encoding_IsUNICODE_REVERSE(nullDist);
                    encDetRes.unicodeReason = bStrongUTF16 ? L"UTF-16(NUL-S)" : L"UTF-16(NUL-W)";
                }
                // Valid UTF-8 containing stray NULs must not be reclassified by
                // Windows' statistical heuristic. ASCII UTF-16 is handled above.
                if (!bStrongUTF16 && !encDetRes.bUTF8WithNulls) {
                    cpi_enc_t const windowsUTF16 = DetectUTF16ByWindows(lpData, cbData);
                    if (Encoding_IsUNICODE(windowsUTF16) &&
                        (Encoding_IsNONE(nullDist) || windowsUTF16 == nullDist)) {
                        encDetRes.unicodeAnalysis = windowsUTF16;
                        encDetRes.bIsReverse = Encoding_IsUNICODE_REVERSE(windowsUTF16);
                        bStrongUTF16 = true;
                        encDetRes.unicodeReason = L"UTF-16(WIN)";
                    }
                }
            }
        }

        // a BOM decides the encoding anyway - the (expensive) analysis is only needed for diagnostics / forced detection
        // (the same holds for strong BOM-less UTF-16 evidence: uchardet must not overrule it)
        bool const bHasUnicodeEvidence = bUTF8Sig || bBOM_LE || bBOM_BE || bStrongUTF16;
        encDetRes.analysisSkipReason = bSkipANSICPDetection ? L"disabled" : bStrongUTF16 ? L"strong UTF-16" : L"BOM";
        if (!bSkipANSICPDetection && (!bHasUnicodeEvidence || bForceEncDetection || Flags.bDevDebugMode)) {
            // ---------------------------------------------------------------------------
            size_t cbSample = 0;
            char* const lpSample = (cbData > 2 * 200000LL) ? _BuildAnalysisSample(lpData, cbData, &cbSample) : NULL;
            if (lpSample) {
                Encoding_AnalyzeText(lpSample, cbSample, &encDetRes, iAnalyzeHint);
                FreeMem(lpSample);
            } else {
                Encoding_AnalyzeText(lpData, cbNbytes4Analysis, &encDetRes, iAnalyzeHint);
            }
            // ---------------------------------------------------------------------------
        }

        if (bForceEncDetection && !IS_ENC_ENFORCED() && !bHasUnicodeEvidence) {
            if (Encoding_IsValid(encDetRes.analyzedEncoding)) {
                // no bIsReliable check (forced unreliable detection)
                encDetRes.forcedEncoding = encDetRes.analyzedEncoding;
                encDetRes.selectionReason = L"UCD(FORCE)";
            }
            else if (Encoding_IsValid(encDetRes.unicodeAnalysis)) {
                encDetRes.forcedEncoding = encDetRes.unicodeAnalysis;
                encDetRes.selectionReason = L"UTF-16(FORCE)";
            }
        }
    } else {
        encDetRes.analysisSkipReason = L"override";
    }

    // --------------------------------------------------------------------------
    // ---  choose best encoding guess  ----
    // --------------------------------------------------------------------------

    encDetRes.bIsAnalysisReliable = (encDetRes.confidence >= ((float)Settings.AnalyzeReliableConfidenceLevel / 100.0f));

    // init resulting encoding
    encDetRes.Encoding = CPI_NONE;

    if (IS_ENC_ENFORCED()) {
        encDetRes.Encoding = encDetRes.forcedEncoding;
        encDetRes.bIsReverse = Encoding_IsUNICODE_REVERSE(encDetRes.forcedEncoding);
        encDetRes.bHasBOM = encDetRes.bIsReverse ? bBOM_BE : bBOM_LE;
    }
    else {
        _SetResultingEncoding(&encDetRes, bBOM_LE, bBOM_BE, bStrongUTF16);
    }
    if (Flags.bDevDebugMode) {
        _SetEncodingTitleInfo(&encDetRes);
    }
    return encDetRes;
}

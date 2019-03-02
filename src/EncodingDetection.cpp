/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* EncodingDetection.cpp                                                       *
*   Interface to Encoding Detector  (CED or UCHARDET)                         *
*                                                                             *
*                                                                             *
*                                                  (c) Rizonesoft 2015-2019   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/

#if !defined(WINVER)
#define WINVER 0x601  /*_WIN32_WINNT_WIN7*/
#endif
#if !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x601  /*_WIN32_WINNT_WIN7*/
#endif
#if !defined(NTDDI_VERSION)
#define NTDDI_VERSION 0x06010000  /*NTDDI_WIN7*/
#endif
#define VC_EXTRALEAN 1
#define WIN32_LEAN_AND_MEAN 1
#define NOMINMAX 1
#include <windows.h>

#define STRSAFE_NO_CB_FUNCTIONS
#define STRSAFE_NO_DEPRECATE      // don't allow deprecated functions
#include <strsafe.h>


#include "resource.h"

extern "C" {
#include "Encoding.h"
}

// CED - Compact Encoding Detection (by Google)
#include "compact_enc_det/compact_enc_det.h"

// UCHARDET - Universal Character Detection (by Mozilla)
#include "uchardet/src/uchardet.h"


//=============================================================================

#define ENC_PARSE_NAM_ASCII                ",ASCII,ascii,"
#define ENC_PARSE_NAM_ANSI                 ",ANSI,ansi,SYSTEM,system" ENC_PARSE_NAM_ASCII
#define ENC_PARSE_NAM_OEM                  ",OEM,oem,"
#define ENC_PARSE_NAM_UTF16LEBOM           ",,"
#define ENC_PARSE_NAM_UTF16BEBOM           ",,"
#define ENC_PARSE_NAM_UTF16LE              ",UTF-16,utf16,UTF-16LE,utf16le,unicode,"
#define ENC_PARSE_NAM_UTF16BE              ",UTF-16BE,utf16be,unicodebe,"
#define ENC_PARSE_NAM_UTF8                 ",UTF-8,utf8,"
#define ENC_PARSE_NAM_UTF8SIG              ",UTF-8-SIG,utf8sig,"
#define ENC_PARSE_NAM_UTF7                 ",UTF-7,utf7,"
#define ENC_PARSE_NAM_DOS_720              ",DOS-720,dos720,"
#define ENC_PARSE_NAM_ISO_8859_6           ",ISO-8859-6,iso88596,arabic,csisolatinarabic,ecma114,isoir127,"
#define ENC_PARSE_NAM_MAC_ARABIC           ",x-mac-arabic,xmacarabic,mac-arabic,"
#define ENC_PARSE_NAM_WIN_1256             ",Windows-1256,windows1256,CP-1256,cp1256,ansiarabic"
#define ENC_PARSE_NAM_DOS_775              ",CP-500,cp500,ibm775,"
#define ENC_PARSE_NAM_ISO_8859_4           ",ISO-8859-4,iso88594,csisolatin4,isoir110,l4,latin4,"
#define ENC_PARSE_NAM_WIN_1257             ",Windows-1257,windows1257,CP-1257,cp1257,ansibaltic,"
#define ENC_PARSE_NAM_DOS_852              ",CP-852,cp852,ibm852,"
#define ENC_PARSE_NAM_ISO_8859_2           ",ISO-8859-2,iso88592,csisolatin2,isoir101,latin2,l2,"
#define ENC_PARSE_NAM_MAC_CENTRAL_EUROP    ",x-mac-ce,xmacce,mac-ce,"
#define ENC_PARSE_NAM_WIN_1250             ",Windows-1250,windows1250,CP-1250,cp1250,xcp1250,"
#define ENC_PARSE_NAM_GBK_2312             ",gb2312,chinese,cngb,csgb2312,csgb231280,gb231280,gbk,"
#define ENC_PARSE_NAM_MAC_ZH_CN            ",x-mac-chinesesimp,xmacchinesesimp,mac-chinesesimp,"
#define ENC_PARSE_NAM_BIG5                 ",big5,cnbig5,csbig5,xxbig5,"
#define ENC_PARSE_NAM_MAC_ZH_TW            ",x-mac-chinesetrad,xmacchinesetrad,mac-chinesetrad,"
#define ENC_PARSE_NAM_MAC_CROATIAN         ",x-mac-croatian,xmaccroatian,mac-croatian,"
#define ENC_PARSE_NAM_DOS_866              ",CP-866,cp866,ibm866,"
#define ENC_PARSE_NAM_ISO_8859_5           ",ISO-8859-5,iso88595,csisolatin5,csisolatincyrillic,cyrillic,isoir144,"
#define ENC_PARSE_NAM_KOI8_R               ",KOI8-R,koi8r,cskoi8r,koi,koi8,"
#define ENC_PARSE_NAM_KOI8_U               ",KOI8-U,koi8u,koi8ru,"
#define ENC_PARSE_NAM_MAC_CYRILLIC         ",x-mac-cyrillic,xmaccyrillic,mac-cyrillic,"
#define ENC_PARSE_NAM_WIN_1251             ",Windows-1251,windows1251,CP-1251,cp1251,xcp1251,"
#define ENC_PARSE_NAM_ISO_8859_13          ",ISO-8859-13,iso885913,"
#define ENC_PARSE_NAM_DOS_863              ",CP-863,cp863,ibm863,"
#define ENC_PARSE_NAM_DOS_737              ",CP-737,cp737,ibm737,"
#define ENC_PARSE_NAM_ISO_8859_7           ",ISO-8859-7,iso88597,csisolatingreek,ecma118,elot928,greek,greek8,isoir126,"
#define ENC_PARSE_NAM_MAC_GREEK            ",x-mac-greek,xmacgreek,mac-greek,"
#define ENC_PARSE_NAM_WIN_1253             ",Windows-1253,windows1253,CP-1253,cp1253"
#define ENC_PARSE_NAM_DOS_869              ",CP-869,cp869,ibm869,"
#define ENC_PARSE_NAM_DOS_862              ",DOS-862,dos862,"
#define ENC_PARSE_NAM_ISO_8859_8_I         ",ISO-8859-8-I,iso88598i,logical,"
#define ENC_PARSE_NAM_ISO_8859_8           ",ISO-8859-8,iso88598,csisolatinhebrew,hebrew,isoir138,visual,"
#define ENC_PARSE_NAM_MAC_HEBREW           ",x-mac-hebrew,xmachebrew,mac-hebrew,"
#define ENC_PARSE_NAM_WIN_1255             ",Windows-1255,windows1255,CP-1255,cp1255,"
#define ENC_PARSE_NAM_DOS_861              ",CP-861,cp861,ibm861,"
#define ENC_PARSE_NAM_MAC_ICELANDIC        ",x-mac-icelandic,xmacicelandic,mac-icelandic,"
#define ENC_PARSE_NAM_MAC_JAPANESE         ",x-mac-japanese,xmacjapanese,mac-japanese,"
#define ENC_PARSE_NAM_SHIFT_JIS            ",shift-jis,shift_jis,shiftjis,shiftjs,csshiftjis,cswindows31j,mskanji,xmscp932,xsjis,"
#define ENC_PARSE_NAM_MAC_KOREAN           ",x-mac-korean,xmackorean,mac-korean,"
#define ENC_PARSE_NAM_WIN_949              ",Windows-949,windows949,UHC,uhc,EUC-KR,euckr,CP-949,cp949,ksx1001,ksc56011987,csksc5601,isoir149,korean,ksc56011989"  // ANSI/OEM Korean (Unified Hangul Code)
#define ENC_PARSE_NAM_ISO_8859_3           ",ISO-8859-3,iso88593,latin3,isoir109,l3,"
#define ENC_PARSE_NAM_ISO_8859_15          ",ISO-8859-15,iso885915,latin9,l9,"
#define ENC_PARSE_NAM_DOS_865              ",CP-865,cp865,ibm865,"
#define ENC_PARSE_NAM_DOS_437              ",CP-437,cp437,ibm437,437,codepage437,cspc8,"
#define ENC_PARSE_NAM_DOS_858              ",CP-858,cp858,ibm858,ibm00858,"
#define ENC_PARSE_NAM_DOS_860              ",CP-860,cp860,ibm860,"
#define ENC_PARSE_NAM_MAC_ROMANIAN         ",x-mac-romanian,xmacromanian,mac-romanian,"
#define ENC_PARSE_NAM_MAC_THAI             ",x-mac-thai,xmacthai,mac-thai,"
#define ENC_PARSE_NAM_WIN_874              ",Windows-874,windows874,dos874,CP-874,cp874,iso885911,TIS-620,tis620,isoir166"
#define ENC_PARSE_NAM_DOS_857              ",CP-857,cp857,ibm857,"
#define ENC_PARSE_NAM_ISO_8859_9           ",ISO-8859-9,iso88599,latin5,isoir148,l5,"
#define ENC_PARSE_NAM_MAC_TURKISH          ",x-mac-turkish,xmacturkish,mac-turkish,"
#define ENC_PARSE_NAM_WIN_1254             ",Windows-1254,windows1254,CP-1254,cp1254,"
#define ENC_PARSE_NAM_MAC_UKRAINIAN        ",x-mac-ukrainian,xmacukrainian,mac-ukrainian,"
#define ENC_PARSE_NAM_WIN_1258             ",Windows-1258,windows1258,CP-1258,cp1258,ansivietnamese"
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
#define ENC_PARSE_NAM_X_CHINESE_CNS        ",X-CHINESE-CNS,xchinesecns,"
#define ENC_PARSE_NAM_JOHAB                ",johab,"
//#define ENC_PARSE_NAM_ISO_8859_10          "ISO-8859-10,iso885910,windows-28600,windows28600,CP-28600,cp28600,"
//#define ENC_PARSE_NAM_BIG5_HKSCS           "big5hkscs,cnbig5hkscs,xxbig5hkscs,"
//=============================================================================

// Missing ICONV Strings:
// -----------------------
// "UTF-32BE / UTF-32LE / X-ISO-10646-UCS-4-34121 / X-ISO-10646-UCS-4-21431"
// "Bulgarian"
// "EUC-TW"
// "ISO-8859-16"
// "MacCentralEurope"
// "ISO-8859-10"
// "IBM855"
// "ISO-8859-11"
// "VISCII"


extern "C" NP2ENCODING g_Encodings[] = {
  /* 000 */{ NCP_ASCII_7BIT | NCP_ANSI | NCP_RECODE,              CP_ACP,   ENC_PARSE_NAM_ANSI,              IDS_ENC_ANSI,              CED_NO_MAPPING,     L"" }, // CPI_ANSI_DEFAULT       0
  /* 001 */{ NCP_ASCII_7BIT | NCP_OEM | NCP_RECODE,               CP_OEMCP, ENC_PARSE_NAM_OEM,               IDS_ENC_OEM,               CED_NO_MAPPING,     L"" }, // CPI_OEM                1
  /* 002 */{ NCP_UNICODE | NCP_UNICODE_BOM,                       CP_UTF8,  ENC_PARSE_NAM_UTF16LEBOM,        IDS_ENC_UTF16LEBOM,        CED_NO_MAPPING,     L"" }, // CPI_UNICODEBOM         2
  /* 003 */{ NCP_UNICODE | NCP_UNICODE_REVERSE | NCP_UNICODE_BOM, CP_UTF8,  ENC_PARSE_NAM_UTF16BEBOM,        IDS_ENC_UTF16BEBOM,        CED_NO_MAPPING,     L"" }, // CPI_UNICODEBEBOM       3
  /* 004 */{ NCP_UNICODE | NCP_RECODE,                            CP_UTF8,  ENC_PARSE_NAM_UTF16LE,           IDS_ENC_UTF16LE,           UTF16LE,            L"" }, // CPI_UNICODE            4
  /* 005 */{ NCP_UNICODE | NCP_UNICODE_REVERSE | NCP_RECODE,      CP_UTF8,  ENC_PARSE_NAM_UTF16BE,           IDS_ENC_UTF16BE,           UTF16BE,            L"" }, // CPI_UNICODEBE          5
  /* 006 */{ NCP_ASCII_7BIT | NCP_UTF8 | NCP_RECODE,              CP_UTF8,  ENC_PARSE_NAM_UTF8,              IDS_ENC_UTF8,              UTF8,               L"" }, // CPI_UTF8               6
  /* 007 */{ NCP_UTF8 | NCP_UTF8_SIGN,                            CP_UTF8,  ENC_PARSE_NAM_UTF8SIG,           IDS_ENC_UTF8SIG,           CED_NO_MAPPING,     L"" }, // CPI_UTF8SIGN           7
  /* 008 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     CP_UTF7,  ENC_PARSE_NAM_UTF7,              IDS_ENC_UTF7,              UTF7,               L"" }, // CPI_UTF7               8
  /* 009 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     720,      ENC_PARSE_NAM_DOS_720,           IDS_ENC_DOS_720,           CED_NO_MAPPING,     L"" },
  /* 010 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     28596,    ENC_PARSE_NAM_ISO_8859_6,        IDS_ENC_ISO_8859_6,        ISO_8859_6,         L"" },
  /* 011 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     10004,    ENC_PARSE_NAM_MAC_ARABIC,        IDS_ENC_MAC_ARABIC,        CED_NO_MAPPING,     L"" },
  /* 012 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     1256,     ENC_PARSE_NAM_WIN_1256,          IDS_ENC_WIN_1256,          MSFT_CP1256,        L"" },
  /* 013 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     775,      ENC_PARSE_NAM_DOS_775,           IDS_ENC_DOS_775,           CED_NO_MAPPING,     L"" },
  /* 014 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     28594,    ENC_PARSE_NAM_ISO_8859_4,        IDS_ENC_ISO_8859_4,        ISO_8859_4,         L"" },
  /* 015 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     1257,     ENC_PARSE_NAM_WIN_1257,          IDS_ENC_WIN_1257,          MSFT_CP1257,        L"" },
  /* 016 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     852,      ENC_PARSE_NAM_DOS_852,           IDS_ENC_DOS_852,           CZECH_CP852,        L"" },
  /* 017 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     28592,    ENC_PARSE_NAM_ISO_8859_2,        IDS_ENC_ISO_8859_2,        ISO_8859_2,         L"" },
  /* 018 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     10029,    ENC_PARSE_NAM_MAC_CENTRAL_EUROP, IDS_ENC_MAC_CENTRAL_EUROP, CED_NO_MAPPING,     L"" },
  /* 019 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     1250,     ENC_PARSE_NAM_WIN_1250,          IDS_ENC_WIN_1250,          MSFT_CP1250,        L"" },
  /* 020 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     936,      ENC_PARSE_NAM_GBK_2312,          IDS_ENC_GBK_2312,          GBK,                L"" },
  /* 021 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     10008,    ENC_PARSE_NAM_MAC_ZH_CN,         IDS_ENC_MAC_ZH_CN,         CED_NO_MAPPING,     L"" },
  /* 022 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     950,      ENC_PARSE_NAM_BIG5,              IDS_ENC_BIG5,              CHINESE_BIG5_CP950, L"" },
  /* 023 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     10002,    ENC_PARSE_NAM_MAC_ZH_TW,         IDS_ENC_MAC_ZH_TW,         CED_NO_MAPPING,     L"" },
  /* 024 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     10082,    ENC_PARSE_NAM_MAC_CROATIAN,      IDS_ENC_MAC_CROATIAN,      CED_NO_MAPPING,     L"" },
  /* 025 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     866,      ENC_PARSE_NAM_DOS_866,           IDS_ENC_DOS_866,           RUSSIAN_CP866,      L"" },
  /* 026 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     28595,    ENC_PARSE_NAM_ISO_8859_5,        IDS_ENC_ISO_8859_5,        ISO_8859_5,         L"" },
  /* 027 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     20866,    ENC_PARSE_NAM_KOI8_R,            IDS_ENC_KOI8_R,            RUSSIAN_KOI8_R,     L"" },
  /* 028 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     21866,    ENC_PARSE_NAM_KOI8_U,            IDS_ENC_KOI8_U,            RUSSIAN_KOI8_RU,    L"" },
  /* 029 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     10007,    ENC_PARSE_NAM_MAC_CYRILLIC,      IDS_ENC_MAC_CYRILLIC,      CED_NO_MAPPING,     L"" },
  /* 030 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     1251,     ENC_PARSE_NAM_WIN_1251,          IDS_ENC_WIN_1251,          RUSSIAN_CP1251,     L"" },
  /* 031 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     28603,    ENC_PARSE_NAM_ISO_8859_13,       IDS_ENC_ISO_8859_13,       ISO_8859_13,        L"" },
  /* 032 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     863,      ENC_PARSE_NAM_DOS_863,           IDS_ENC_DOS_863,           CED_NO_MAPPING,     L"" },
  /* 033 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     737,      ENC_PARSE_NAM_DOS_737,           IDS_ENC_DOS_737,           CED_NO_MAPPING,     L"" },
  /* 034 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     28597,    ENC_PARSE_NAM_ISO_8859_7,        IDS_ENC_ISO_8859_7,        ISO_8859_7,         L"" },
  /* 035 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     10006,    ENC_PARSE_NAM_MAC_GREEK,         IDS_ENC_MAC_GREEK,         CED_NO_MAPPING,     L"" },
  /* 036 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     1253,     ENC_PARSE_NAM_WIN_1253,          IDS_ENC_WIN_1253,          MSFT_CP1253,        L"" },
  /* 037 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     869,      ENC_PARSE_NAM_DOS_869,           IDS_ENC_DOS_869,           CED_NO_MAPPING,     L"" },
  /* 038 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     862,      ENC_PARSE_NAM_DOS_862,           IDS_ENC_DOS_862,           CED_NO_MAPPING,     L"" },
  /* 039 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     38598,    ENC_PARSE_NAM_ISO_8859_8_I,      IDS_ENC_ISO_8859_8_I,      ISO_8859_8_I,       L"" },
  /* 040 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     28598,    ENC_PARSE_NAM_ISO_8859_8,        IDS_ENC_ISO_8859_8,        ISO_8859_8,         L"" },
  /* 041 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     10005,    ENC_PARSE_NAM_MAC_HEBREW,        IDS_ENC_MAC_HEBREW,        CED_NO_MAPPING,     L"" },
  /* 042 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     1255,     ENC_PARSE_NAM_WIN_1255,          IDS_ENC_WIN_1255,          MSFT_CP1255,        L"" },
  /* 043 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     861,      ENC_PARSE_NAM_DOS_861,           IDS_ENC_DOS_861,           CED_NO_MAPPING,     L"" },
  /* 044 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     10079,    ENC_PARSE_NAM_MAC_ICELANDIC,     IDS_ENC_MAC_ICELANDIC,     CED_NO_MAPPING,     L"" },
  /* 045 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     10001,    ENC_PARSE_NAM_MAC_JAPANESE,      IDS_ENC_MAC_JAPANESE,      CED_NO_MAPPING,     L"" },
  /* 046 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     932,      ENC_PARSE_NAM_SHIFT_JIS,         IDS_ENC_SHIFT_JIS,         JAPANESE_SHIFT_JIS, L"" },
  /* 047 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     10003,    ENC_PARSE_NAM_MAC_KOREAN,        IDS_ENC_MAC_KOREAN,        CED_NO_MAPPING,     L"" },
  /* 048 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     949,      ENC_PARSE_NAM_WIN_949,           IDS_ENC_WIN_949,           KOREAN_EUC_KR,      L"" },
  /* 049 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     28593,    ENC_PARSE_NAM_ISO_8859_3,        IDS_ENC_ISO_8859_3,        ISO_8859_3,         L"" },
  /* 050 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     28605,    ENC_PARSE_NAM_ISO_8859_15,       IDS_ENC_ISO_8859_15,       ISO_8859_15,        L"" },
  /* 051 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     865,      ENC_PARSE_NAM_DOS_865,           IDS_ENC_DOS_865,           CED_NO_MAPPING,     L"" },
  /* 052 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     437,      ENC_PARSE_NAM_DOS_437,           IDS_ENC_DOS_437,           CED_NO_MAPPING,     L"" },
  /* 053 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     858,      ENC_PARSE_NAM_DOS_858,           IDS_ENC_DOS_858,           CED_NO_MAPPING,     L"" },
  /* 054 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     860,      ENC_PARSE_NAM_DOS_860,           IDS_ENC_DOS_860,           CED_NO_MAPPING,     L"" },
  /* 055 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     10000,    ENC_PARSE_NAM_MAC_WESTERN_EUROP, IDS_ENC_MAC_WESTERN_EUROP, MACINTOSH_ROMAN,    L"" },
  /* 056 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     10021,    ENC_PARSE_NAM_MAC_THAI,          IDS_ENC_MAC_THAI,          CED_NO_MAPPING,     L"" },
  /* 057 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     874,      ENC_PARSE_NAM_WIN_874,           IDS_ENC_WIN_874,           MSFT_CP874,         L"" },
  /* 058 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     857,      ENC_PARSE_NAM_DOS_857,           IDS_ENC_DOS_857,           CED_NO_MAPPING,     L"" },
  /* 059 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     28599,    ENC_PARSE_NAM_ISO_8859_9,        IDS_ENC_ISO_8859_9,        ISO_8859_9,         L"" },
  /* 060 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     10081,    ENC_PARSE_NAM_MAC_TURKISH,       IDS_ENC_MAC_TURKISH,       CED_NO_MAPPING,     L"" },
  /* 061 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     1254,     ENC_PARSE_NAM_WIN_1254,          IDS_ENC_WIN_1254,          MSFT_CP1254,        L"" },
  /* 062 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     10017,    ENC_PARSE_NAM_MAC_UKRAINIAN,     IDS_ENC_MAC_UKRAINIAN,     CED_NO_MAPPING,     L"" },
  /* 063 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     1258,     ENC_PARSE_NAM_WIN_1258,          IDS_ENC_WIN_1258,          CED_NO_MAPPING,     L"" },
  /* 064 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     850,      ENC_PARSE_NAM_DOS_850,           IDS_ENC_DOS_850,           CED_NO_MAPPING,     L"" },
  /* 065 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     28591,    ENC_PARSE_NAM_ISO_8859_1,        IDS_ENC_ISO_8859_1,        ISO_8859_1,         L"" },
  /* 066 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     10010,    ENC_PARSE_NAM_MAC_ROMANIAN,      IDS_ENC_MAC_ROMANIAN,      MACINTOSH_ROMAN,    L"" },
  /* 067 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     1252,     ENC_PARSE_NAM_WIN_1252,          IDS_ENC_WIN_1252,          MSFT_CP1252,        L"" },
  /* 068 */{ NCP_EXTERNAL_8BIT | NCP_RECODE,                      37,       ENC_PARSE_NAM_IBM_EBCDIC_US,     IDS_ENC_IBM_EBCDIC_US,     CED_NO_MAPPING,     L"" },
  /* 069 */{ NCP_EXTERNAL_8BIT | NCP_RECODE,                      500,      ENC_PARSE_NAM_IBM_EBCDIC_INT,    IDS_ENC_IBM_EBCDIC_INT,    CED_NO_MAPPING,     L"" },
  /* 070 */{ NCP_EXTERNAL_8BIT | NCP_RECODE,                      875,      ENC_PARSE_NAM_IBM_EBCDIC_GR,     IDS_ENC_IBM_EBCDIC_GR,     CED_NO_MAPPING,     L"" },
  /* 071 */{ NCP_EXTERNAL_8BIT | NCP_RECODE,                      1026,     ENC_PARSE_NAM_IBM_EBCDIC_LAT_5,  IDS_ENC_IBM_EBCDIC_LAT_5,  CED_NO_MAPPING,     L"" },
  /* 072 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     54936,    ENC_PARSE_NAM_GB18030,           IDS_ENC_GB18030,           GB18030,            L"" }, // Chinese Simplified (GB18030)
  /* 073 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     20932,    ENC_PARSE_NAM_EUC_JAPANESE,      IDS_ENC_EUC_JAPANESE,      JAPANESE_EUC_JP,    L"" }, // Japanese (EUC)
  /* 074 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     51949,    ENC_PARSE_NAM_EUC_KOREAN,        IDS_ENC_EUC_KOREAN,        KOREAN_EUC_KR,      L"" }, // Korean (EUC)
  /* 075 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     50229,    ENC_PARSE_NAM_ISO_2022_CN,       IDS_ENC_ISO_2022_CN,       ISO_2022_CN,        L"" }, // Chinese Traditional (ISO-2022-CN)
  /* 076 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     52936,    ENC_PARSE_NAM_HZ_GB2312,         IDS_ENC_HZ_GB2312,         HZ_GB_2312,         L"" }, // Chinese Simplified (HZ-GB2312)
  /* 077 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     50220,    ENC_PARSE_NAM_ISO_2022_JP,       IDS_ENC_ISO_2022_JP,       KDDI_ISO_2022_JP,   L"" }, // Japanese (JIS)
  /* 078 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     50225,    ENC_PARSE_NAM_ISO_2022_KR,       IDS_ENC_ISO_2022_KR,       ISO_2022_KR,        L"" }, // Korean (ISO-2022-KR)
  /* 079 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     20000,    ENC_PARSE_NAM_X_CHINESE_CNS,     IDS_ENC_X_CHINESE_CNS,     CHINESE_CNS,        L"" }, // Chinese Traditional (CNS)
  /* 080 */{ NCP_ASCII_7BIT | NCP_EXTERNAL_8BIT | NCP_RECODE,     1361,     ENC_PARSE_NAM_JOHAB,             IDS_ENC_JOHAB,             CED_NO_MAPPING,     L"" }  // Korean (Johab)
  ///* 079 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 28600, ENC_PARSE_NAM_ISO_8859_10,       IDS_ENC_ISO_8859_10,       ISO_8859_10,        L"" }, // Nordic (ISO 8859-10)
  ///* 080 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 951,   ENC_PARSE_NAM_BIG5_HKSCS,        IDS_ENC_BIG5_HKSCS,        BIG5_HKSCS,         L"" }  // Chinese (Hong Kong Supplementary Character Set)

  
#if 0
  NP2ENCODING mEncoding[] = {
  { NCP_8BIT | NCP_RECODE, 38596, "ISO-8859-6-I,ISO88596I,", 61011, L"" },// Arabic (ISO 8859-6-I Logical)
  { NCP_8BIT | NCP_RECODE, 28604, "ISO-8859-14,ISO885914,Windows-28604,windows28604,", 61055, L"" },// Celtic (ISO 8859-14)
  { NCP_8BIT | NCP_RECODE, 28606, "ISO-8859-16,ISO885916,Windows-28606,windows28606", 61054, L"" },// Latin-10 (ISO 8859-16)
};
#endif

  /* 073 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 870,   "CP870,cp870,ebcdiccproece,ebcdiccpyu,csibm870,ibm870,",                          00000, L"" }, // IBM EBCDIC (Multilingual Latin-2)
  /* 074 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 1047,  "IBM01047,ibm01047,",                                                             00000, L"" }, // IBM EBCDIC (Open System Latin-1)
  /* 075 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 1140,  "x-ebcdic-cp-us-euro,xebcdiccpuseuro,",                                           00000, L"" }, // IBM EBCDIC (US-Canada-Euro)
  /* 076 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 1141,  "x-ebcdic-germany-euro,xebcdicgermanyeuro,",                                      00000, L"" }, // IBM EBCDIC (Germany-Euro)
  /* 077 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 1142,  "x-ebcdic-denmarknorway-euro,xebcdicdenmarknorwayeuro,",                          00000, L"" }, // IBM EBCDIC (Denmark-Norway-Euro)
  /* 078 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 1143,  "x-ebcdic-finlandsweden-euro,xebcdicfinlandswedeneuro,",                          00000, L"" }, // IBM EBCDIC (Finland-Sweden-Euro)
  /* 079 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 1144,  "x-ebcdic-italy-euro,xebcdicitalyeuro,",                                          00000, L"" }, // IBM EBCDIC (Italy-Euro)
  /* 080 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 1145,  "x-ebcdic-spain-euro,xebcdicspaineuro,",                                          00000, L"" }, // IBM EBCDIC (Spain-Latin America-Euro)
  /* 081 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 1146,  "x-ebcdic-uk-euro,xebcdicukeuro,",                                                00000, L"" }, // IBM EBCDIC (UK-Euro)
  /* 082 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 1147,  "x-ebcdic-france-euro,xebcdicfranceeuro,",                                        00000, L"" }, // IBM EBCDIC (France-Euro)
  /* 083 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 1148,  "x-ebcdic-international-euro,xebcdicinternationaleuro,",                          00000, L"" }, // IBM EBCDIC (International-Euro)
  /* 084 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 1149,  "x-ebcdic-icelandic-euro,xebcdicicelandiceuro,",                                  00000, L"" }, // IBM EBCDIC (Icelandic-Euro)
  /* 086 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20273, "x-EBCDIC-Germany,xebcdicgermany,",                                               00000, L"" }, // IBM EBCDIC (Germany)
  /* 087 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20277, "x-EBCDIC-DenmarkNorway,xebcdicdenmarknorway,ebcdiccpdk,ebcdiccpno,",             00000, L"" }, // IBM EBCDIC (Denmark-Norway)
  /* 088 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20278, "x-EBCDIC-FinlandSweden,xebcdicfinlandsweden,ebcdicpfi,ebcdiccpse,",              00000, L"" }, // IBM EBCDIC (Finland-Sweden)
  /* 089 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20280, "x-EBCDIC-Italy,xebcdicitaly,",                                                   00000, L"" }, // IBM EBCDIC (Italy)
  /* 090 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20284, "x-EBCDIC-Spain,xebcdicspain,ebcdiccpes,",                                        00000, L"" }, // IBM EBCDIC (Spain-Latin America)
  /* 091 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20285, "x-EBCDIC-UK,xebcdicuk,ebcdiccpgb,",                                              00000, L"" }, // IBM EBCDIC (UK)
  /* 092 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20290, "x-EBCDIC-JapaneseKatakana,xebcdicjapanesekatakana,",                             00000, L"" }, // IBM EBCDIC (Japanese Katakana)
  /* 093 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20297, "x-EBCDIC-France,xebcdicfrance,ebcdiccpfr,",                                      00000, L"" }, // IBM EBCDIC (France)
  /* 094 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20420, "x-EBCDIC-Arabic,xebcdicarabic,ebcdiccpar1,",                                     00000, L"" }, // IBM EBCDIC (Arabic)
  /* 095 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20423, "x-EBCDIC-Greek,xebcdicgreek,ebcdiccpgr,",                                        00000, L"" }, // IBM EBCDIC (Greek)
  /* 096 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20424, "x-EBCDIC-Hebrew,xebcdichebrew,ebcdiccphe,",                                      00000, L"" }, // IBM EBCDIC (Hebrew)
  /* 097 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20833, "x-EBCDIC-KoreanExtended,xebcdickoreanextended,",                                 00000, L"" }, // IBM EBCDIC (Korean Extended)
  /* 098 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20838, "x-EBCDIC-Thai,xebcdicthai,ibmthai,csibmthai,",                                   00000, L"" }, // IBM EBCDIC (Thai)
  /* 099 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20871, "x-EBCDIC-Icelandic,xebcdicicelandic,ebcdiccpis,",                                00000, L"" }, // IBM EBCDIC (Icelandic)
  /* 100 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20880, "x-EBCDIC-CyrillicRussian,xebcdiccyrillicrussian,ebcdiccyrillic,",                00000, L"" }, // IBM EBCDIC (Cyrillic Russian)
  /* 101 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20905, "x-EBCDIC-Turkish,xebcdicturkish,ebcdiccptr,",                                    00000, L"" }, // IBM EBCDIC (Turkish)
  /* 102 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20924, "IBM00924,ibm00924,ebcdiclatin9euro,",                                            00000, L"" }, // IBM EBCDIC (Open System-Euro Latin-1)
  /* 103 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 21025, "x-EBCDIC-CyrillicSerbianBulgarian,xebcdiccyrillicserbianbulgarian,",             00000, L"" }, // IBM EBCDIC (Cyrillic Serbian-Bulgarian)
  /* 104 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 50930, "x-EBCDIC-JapaneseAndKana,xebcdicjapaneseandkana,",                               00000, L"" }, // IBM EBCDIC (Japanese and Japanese Katakana)
  /* 105 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 50931, "x-EBCDIC-JapaneseAndUSCanada,xebcdicjapaneseanduscanada,",                       00000, L"" }, // IBM EBCDIC (Japanese and US-Canada)
  /* 106 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 50933, "x-EBCDIC-KoreanAndKoreanExtended,xebcdickoreanandkoreanextended,",               00000, L"" }, // IBM EBCDIC (Korean and Korean Extended)
  /* 107 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 50935, "x-EBCDIC-SimplifiedChinese,xebcdicsimplifiedchinese,",                           00000, L"" }, // IBM EBCDIC (Chinese Simplified)
  /* 108 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 50937, "x-EBCDIC-TraditionalChinese,xebcdictraditionalchinese,",                         00000, L"" }, // IBM EBCDIC (Chinese Traditional)
  /* 109 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 50939, "x-EBCDIC-JapaneseAndJapaneseLatin,xebcdicjapaneseandjapaneselatin,",             00000, L"" }, // IBM EBCDIC (Japanese and Japanese-Latin)
  /* 110 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20105, "x-IA5,xia5,",                                                                    00000, L"" }, // Western European (IA5)
  /* 111 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20106, "x-IA5-German,xia5german,",                                                       00000, L"" }, // German (IA5)
  /* 112 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20107, "x-IA5-Swedish,xia5swedish,",                                                     00000, L"" }, // Swedish (IA5)
  /* 113 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20108, "x-IA5-Norwegian,xia5norwegian,",                                                 00000, L"" }, // Norwegian (IA5)
  /* 114 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20936, "x-cp20936,xcp20936,",                                                            00000, L"" }, // Chinese Simplified (GB2312)
  /* 115 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20932, "euc-jp,,",                                                                       00000, L"" }, // Japanese (JIS X 0208-1990 & 0212-1990)
  /* 117 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 50221, "csISO2022JP,csiso2022jp,",                                                       00000, L"" }, // Japanese (JIS-Allow 1 byte Kana)
  /* 118 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 50222, "_ISO-2022-jp$SIO,iso2022jpSIO,",                                                 00000, L"" }, // Japanese (JIS-Allow 1 byte Kana - SO/SI)
  /* 120 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 50227, "x-cp50227,xcp50227,",                                                            00000, L"" }, // Chinese Simplified (ISO-2022)
  /* 123 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20002, "x-Chinese-Eten,xchineseeten,",                                                   00000, L"" }, // Chinese Traditional (Eten)
  /* 125 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 51936, "euc-cn,euccn,xeuccn,",                                                           00000, L"" }, // Chinese Simplified (EUC)
  /* 128 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 57002, "x-iscii-de,xisciide,",                                                           00000, L"" }, // ISCII Devanagari
  /* 129 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 57003, "x-iscii-be,xisciibe,",                                                           00000, L"" }, // ISCII Bengali
  /* 130 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 57004, "x-iscii-ta,xisciita,",                                                           00000, L"" }, // ISCII Tamil
  /* 131 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 57005, "x-iscii-te,xisciite,",                                                           00000, L"" }, // ISCII Telugu
  /* 132 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 57006, "x-iscii-as,xisciias,",                                                           00000, L"" }, // ISCII Assamese
  /* 133 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 57007, "x-iscii-or,xisciior,",                                                           00000, L"" }, // ISCII Oriya
  /* 134 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 57008, "x-iscii-ka,xisciika,",                                                           00000, L"" }, // ISCII Kannada
  /* 135 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 57009, "x-iscii-ma,xisciima,",                                                           00000, L"" }, // ISCII Malayalam
  /* 136 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 57010, "x-iscii-gu,xisciigu,",                                                           00000, L"" }, // ISCII Gujarathi
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            /* 137 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 57011, "x-iscii-pa,xisciipa,",                                                           00000, L"" }, // ISCII Panjabi
};

constexpr int _CountOfEncodings() { return ARRAYSIZE(g_Encodings); }

extern "C" int Encoding_CountOf()
{
  return _CountOfEncodings();
}

//=============================================================================


constexpr int _MapCPI2Encoding(const int iNP3Encoding)
{
  if ((iNP3Encoding < 0) || (iNP3Encoding >= _CountOfEncodings())) {
    return UNKNOWN_ENCODING; // CPI_NONE, CPI_GET
  }

  int const iCED = g_Encodings[iNP3Encoding].iCEDEncoding;

  return ((iCED != CED_NO_MAPPING) ? iCED : UNKNOWN_ENCODING);
}
// ============================================================================

extern "C" void ChangeEncodingCodePage(int cpi, UINT newCP)
{
  int iCED = _MapCPI2Encoding(cpi);
  g_Encodings[cpi].uCodePage = newCP;
  g_Encodings[cpi].iCEDEncoding = iCED;
}
// ============================================================================


// ============================================================================
// ============================================================================

constexpr int _FindCodePage(const Encoding& encoding)
{
  int iCodePage = -1;

  // TODO: find more default ANSI mappings...

  switch (encoding) {
  case UTF8UTF8:
    iCodePage = CP_UTF8;
    break;
  case ISO_8859_1:  //CP-28591
  case ISO_8859_15:
    iCodePage = 1252;
    break;
  case ISO_8859_2:
    iCodePage = 1250;
    break;
  case ISO_8859_4:
  case ISO_8859_10:
    iCodePage = 1257;
    break;
  case ISO_8859_5:
  case RUSSIAN_CP1251:
    iCodePage = 1251;
    break;
  case ISO_8859_6:
    iCodePage = 1256;
    break;
  case ISO_8859_7:
    iCodePage = 1253;
    break;
  case ISO_8859_8:
    iCodePage = 1255;
    break;
  case ISO_8859_9:
    iCodePage = 1254;
    break;
  case ISO_8859_11:
    iCodePage = 874;
    break;
  case JAPANESE_JIS:
  case JAPANESE_CP932:
    iCodePage = 932;
    break;
  case CHINESE_BIG5:
    iCodePage = 950;
    break;
  case CHINESE_GB:
    iCodePage = 54936;
    break;
  case BIG5_HKSCS:
    iCodePage = 951;
    break;
  case HEBREW_VISUAL:
    iCodePage = 28598;
    break;

  default:
    for (int i = 0; i < _CountOfEncodings(); ++i) {
      if (encoding == g_Encodings[i].iCEDEncoding) {
        iCodePage = static_cast<int>(g_Encodings[i].uCodePage);
        break;
      }
    }
  }
  return iCodePage;

}
// ============================================================================



static int  _MapCEDEncoding2CPI(const char* const text, const size_t len, const Encoding& encoding, bool* pIsReliable)
{
  int cpiEncoding = CPI_NONE;

  // preprocessing: special cases
  if (encoding == ASCII_7BIT) {
    cpiEncoding = CPI_ASCII_7BIT;
  }
  else { // check for default ANSI
    if (_FindCodePage(encoding) == static_cast<int>(g_Encodings[CPI_ANSI_DEFAULT].uCodePage)) {
      cpiEncoding = CPI_ANSI_DEFAULT;
    }
  }

  if (cpiEncoding == CPI_NONE)
  {
    for (int cpiIdx = 0; cpiIdx < _CountOfEncodings(); ++cpiIdx) {
      if (encoding == g_Encodings[cpiIdx].iCEDEncoding) {
        cpiEncoding = cpiIdx;
        break;
      }
    }
  }

  // ===  special Unicode analysis  ===

  switch (encoding) 
  {
  case UNICODE:
    cpiEncoding = CPI_UNICODE;
  case UTF16LE:
  case UTF16BE:
    {
      bool bBOM;
      bool bReverse;
      if (IsValidUnicode(text, len, &bBOM, &bReverse)) {
        cpiEncoding = bBOM ? (bReverse ? CPI_UNICODEBE : CPI_UNICODE) : (bReverse ? CPI_UNICODEBE : CPI_UNICODE);
      }
    }
    break;

  case UTF8UTF8:
    cpiEncoding = CPI_UTF8;
    break;
  case UTF32BE:
    cpiEncoding = CPI_UTF32BE;
    break;
  case UTF32LE:
    cpiEncoding = CPI_UTF32;
    break;

  default:
    break;
  }

  // ===  postrocessing:  not found, guess a mapping:  ===

  if (cpiEncoding == CPI_NONE)
  {
    switch (encoding) 
    {
    case ISO_8859_10:
      cpiEncoding = CPI_NONE;
      break;
    case ISO_8859_11:
      cpiEncoding = CPI_NONE; // latin-thai
      break;

    case BIG5_HKSCS:
    case CHINESE_BIG5:
      cpiEncoding = 22;
      break;
    case CHINESE_EUC_CN:
    case CHINESE_EUC_DEC:
      *pIsReliable = false;
    case CHINESE_GB:
      cpiEncoding = 20;
      break;

    case JAPANESE_SHIFT_JIS:
      cpiEncoding = 46;
      break;
    case KDDI_SHIFT_JIS:
      cpiEncoding = 46;
      break;
    case DOCOMO_SHIFT_JIS:
      cpiEncoding = 46;
      break;
    case SOFTBANK_SHIFT_JIS:
      cpiEncoding = 46;
      break;

    case JAPANESE_JIS:
      cpiEncoding = 77;
      break;
    case SOFTBANK_ISO_2022_JP:
      cpiEncoding = 77;
      break;


    case CZECH_CSN_369103:
    case TSCII:
    case TAMIL_MONO:
    case TAMIL_BI:
    case JAGRAN:
    case BHASKAR:
    case HTCHANAKYA:
    case BINARYENC:
    case TAM_ELANGO:
    case TAM_LTTMBARANI:
    case TAM_SHREE:
    case TAM_TBOOMIS:
    case TAM_TMNEWS:
    case TAM_WEBTAMIL:

    case UNKNOWN_ENCODING:
    default:
      cpiEncoding = CPI_NONE;
      *pIsReliable = false;
      break;
    }
  }
  return cpiEncoding;
}
// ============================================================================


extern "C" int Encoding_Analyze_CED(const char* const text, const size_t len, const int encodingHint, bool* pIsReliable)
{
  int bytes_consumed;

  Encoding encoding = CompactEncDet::DetectEncoding(
    text, static_cast<int>(len),
    nullptr, nullptr, nullptr,
    _MapCPI2Encoding(encodingHint),
    UNKNOWN_LANGUAGE, CompactEncDet::QUERY_CORPUS, true,
    &bytes_consumed,
    pIsReliable);

  return _MapCEDEncoding2CPI(text, len, encoding, pIsReliable);
}
// ============================================================================



// ============================================================================
//   UCHARDET
// ============================================================================

static int _MapUCARDETEncoding2CPI(const char* const text, const size_t len, 
  const int encodingHint, const char* charset, float* pConfidence)
{
  (void)text; // UNUSED
  (void)len;  // UNUSED

  if (!charset || (charset[0] == '\0')) {
    *pConfidence = 0.0f;
    return encodingHint;
  }

  int cpiEncoding = CPI_NONE;

  // preprocessing: special cases
  if (_stricmp(charset, "ascii") == 0) {
    cpiEncoding = CPI_ASCII_7BIT;
  }
  else {
    cpiEncoding = Encoding_MatchA(charset);
  }

  // check for default ANSI
  if (cpiEncoding > CPI_ANSI_DEFAULT)
  {
    if (g_Encodings[cpiEncoding].uCodePage == g_Encodings[CPI_ANSI_DEFAULT].uCodePage) 
    {
      cpiEncoding = CPI_ANSI_DEFAULT;
    }
  }

  if (cpiEncoding == CPI_NONE)
  {
    *pConfidence = 0.0f;
    cpiEncoding = encodingHint;
  }

  return cpiEncoding;
}
// ============================================================================


extern "C" int Encoding_Analyze_UCHARDET(const char* const text, const size_t len, 
  const int encodingHint, float* pConfidence, char* origUCHARDET, int cch)
{
  int encoding = CPI_NONE;
  float confidence = 0.0f;

  uchardet_t hUcharDet = uchardet_new();

  int const result = uchardet_handle_data(hUcharDet, text, len);
  
  uchardet_data_end(hUcharDet); // transfer report

  switch (result) 
  {
  case HANDLE_DATA_RESULT_NEED_MORE_DATA:  // need more data is a result too 
  case HANDLE_DATA_RESULT_DETECTED:
  {
    const char* charset = uchardet_get_charset(hUcharDet);
    StringCchCopyA(origUCHARDET, cch, charset);  // UCHARDET

    confidence = uchardet_get_confidence(hUcharDet);
    encoding = _MapUCARDETEncoding2CPI(text, len, encodingHint, charset, &confidence);
  }
  break;

  case HANDLE_DATA_RESULT_ERROR:
  default:
    encoding = CPI_NONE;
    confidence = 0.0f;
    break;
  }

  uchardet_delete(hUcharDet);

  *pConfidence = confidence;
  return encoding;
}


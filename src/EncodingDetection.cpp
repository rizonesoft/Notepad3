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
*                                                  (c) Rizonesoft 2008-2020   *
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
#include <string_view>

//~#include <future>                 // async detection

extern "C" {
#include "Helpers.h"
#include "Encoding.h"
#include "SciCall.h"
}

// CED - Compact Encoding Detection (by Google)
#include "compact_enc_det/compact_enc_det.h"

// UCHARDET - Universal Character Detection (by Mozilla)
#include "uchardet/src/uchardet.h"

#include "resource.h"

//=============================================================================

extern "C" void Style_SetMultiEdgeLine(const int colVec[], const size_t count);

//=============================================================================

static WCHAR wchEncodingInfo[MAX_PATH] = { L'\0' };

static void _SetEncodingTitleInfo(const ENC_DET_T* pEncDetInfo);

extern "C" const WCHAR* Encoding_GetTitleInfo() { return wchEncodingInfo; }

extern "C" const char* Encoding_GetTitleInfoA() {
  static char chEncodingInfo[MAX_PATH] = { '\0' };
  ::WideCharToMultiByte(CP_ACP, 0, wchEncodingInfo, -1, chEncodingInfo, (int)COUNTOF(chEncodingInfo), NULL, NULL);
  return chEncodingInfo;
}


//=============================================================================

///////////////////////////////////////////////////////////////////////////////////////////////
/////////////                                                                     /////////////
/////////////    CED  encoding names                                              /////////////
/////////////    [ EncodingName() , MimeEncodingName(), Encoding(_CED_intern) ]   /////////////
/////////////    TODO:  find mapping below and set string above, if missing       /////////////
/////////////                                                                     /////////////
///////////////////////////////////////////////////////////////////////////////////////////////

//static const EncodingInfo kEncodingInfoTable[] = {
//  { "ASCII", "ISO-8859-1", ISO_8859_1},
//  { "Latin2", "ISO-8859-2", ISO_8859_2},
//  { "Latin3", "ISO-8859-3", UTF8},
//// MSIE 6 does not support ISO-8859-3 (XSS issue)
//  { "Latin4", "ISO-8859-4", ISO_8859_4},
//  { "ISO-8859-5", "ISO-8859-5", ISO_8859_5},
//  { "Arabic", "ISO-8859-6", ISO_8859_6},
//  { "Greek", "ISO-8859-7", ISO_8859_7},
//  { "Hebrew", "ISO-8859-8", MSFT_CP1255},
//// we do not endorse the visual order
//  { "Latin5", "ISO-8859-9", ISO_8859_9},
//  { "Latin6", "ISO-8859-10", UTF8},
//// MSIE does not support ISO-8859-10 (XSS issue)
//  { "EUC-JP",  "EUC-JP", JAPANESE_EUC_JP},
//  { "SJS", "Shift_JIS", JAPANESE_SHIFT_JIS},
//  { "JIS", "ISO-2022-JP", JAPANESE_SHIFT_JIS},
//  // due to potential confusion with HTML syntax chars
//  { "BIG5", "Big5", CHINESE_BIG5},
//  { "GB",  "GB2312", CHINESE_GB},
//  { "EUC-CN",
//      "EUC-CN",
//      // Misnamed. Should be EUC-TW.
//      CHINESE_BIG5},
//      // MSIE treats "EUC-CN" like GB2312, which is not EUC-TW,
//      // and EUC-TW is rare, so we prefer Big5 for output.
//  { "KSC", "EUC-KR", KOREAN_EUC_KR},
//  { "Unicode",
//    "UTF-16LE",
//    // Internet Explorer doesn't recognize "ISO-10646-UCS-2"
//    UTF8
//    // due to potential confusion with HTML syntax chars
//    },
//  { "EUC",
//        "EUC",  // Misnamed. Should be EUC-TW.
//        CHINESE_BIG5
//    // MSIE does not recognize "EUC" (XSS issue),
//    // and EUC-TW is rare, so we prefer Big5 for output.
//    },
//  { "CNS",
//        "CNS",  // Misnamed. Should be EUC-TW.
//        CHINESE_BIG5},
//    // MSIE does not recognize "CNS" (XSS issue),
//    // and EUC-TW is rare, so we prefer Big5 for output.
//  { "BIG5-CP950",
//        "BIG5-CP950",  // Not an IANA name
//        CHINESE_BIG5
//    // MSIE does not recognize "BIG5-CP950" (XSS issue)
//    },
//  { "CP932", "CP932",  // Not an IANA name
//        JAPANESE_SHIFT_JIS},  // MSIE does not recognize "CP932" (XSS issue)
//  { "UTF8", "UTF-8", UTF8},
//  { "Unknown", "x-unknown",  // Not an IANA name
//        UTF8},  // UTF-8 is our default output encoding
//  { "ASCII-7-bit", "US-ASCII", ASCII_7BIT},
//  { "KOI8R", "KOI8-R", RUSSIAN_KOI8_R},
//  { "CP1251", "windows-1251", RUSSIAN_CP1251},
//  { "CP1252", "windows-1252", MSFT_CP1252},
//  { "KOI8U", "KOI8-U", ISO_8859_5},  // because koi8-u is not as common
//  { "CP1250", "windows-1250", MSFT_CP1250},
//  { "ISO-8859-15", "ISO-8859-15", ISO_8859_15},
//  { "CP1254", "windows-1254", MSFT_CP1254},
//  { "CP1257", "windows-1257", MSFT_CP1257},
//  { "ISO-8859-11", "ISO-8859-11", ISO_8859_11},
//  { "CP874", "windows-874", MSFT_CP874},
//  { "CP1256", "windows-1256", MSFT_CP1256},
//  { "CP1255", "windows-1255", MSFT_CP1255},
//  { "ISO-8859-8-I", "ISO-8859-8-I", MSFT_CP1255},
//  // Java does not support iso-8859-8-i
//  { "VISUAL", "ISO-8859-8", MSFT_CP1255},
//  // we do not endorse the visual order
//  { "CP852", "cp852", MSFT_CP1250},
//  // because cp852 is not as common
//  { "CSN_369103", "csn_369103", MSFT_CP1250},
//  // MSIE does not recognize "csn_369103" (XSS issue)
//  { "CP1253", "windows-1253", MSFT_CP1253},
//  { "CP866", "IBM866", RUSSIAN_CP1251},
//  // because cp866 is not as common
//  { "ISO-8859-13", "ISO-8859-13", UTF8},
//  // because iso-8859-13 is not widely supported
//  { "ISO-2022-KR", "ISO-2022-KR", KOREAN_EUC_KR},
//  // due to potential confusion with HTML syntax chars
//  { "GBK", "GBK", GBK},
//  { "GB18030", "GB18030", GBK},
//  // because gb18030 is not widely supported
//  { "BIG5_HKSCS", "BIG5-HKSCS", CHINESE_BIG5},
//  // because Big5-HKSCS is not widely supported
//  { "ISO_2022_CN", "ISO-2022-CN", CHINESE_GB},
//  // due to potential confusion with HTML syntax chars
//  { "TSCII", "tscii", UTF8},
//  // we do not have an output converter for this font encoding
//  { "TAM", "tam", UTF8},
//  // we do not have an output converter for this font encoding
//  { "TAB", "tab", UTF8},
//  // we do not have an output converter for this font encoding
//  { "JAGRAN", "jagran", UTF8},
//  // we do not have an output converter for this font encoding
//  { "MACINTOSH", "MACINTOSH", ISO_8859_1},
//  // because macintosh is relatively uncommon
//  { "UTF7", "UTF-7",
//        UTF8},  // UTF-7 has been the subject of XSS attacks and is deprecated
//  { "BHASKAR", "bhaskar",
//        UTF8},  // we do not have an output converter for this font encoding
//  { "HTCHANAKYA", "htchanakya",  // not an IANA charset name.
//        UTF8},  // we do not have an output converter for this font encoding
//  { "UTF-16BE", "UTF-16BE",
//        UTF8},  // due to potential confusion with HTML syntax chars
//  { "UTF-16LE", "UTF-16LE",
//        UTF8},  // due to potential confusion with HTML syntax chars
//  { "UTF-32BE", "UTF-32BE",
//        UTF8},  // unlikely to cause XSS bugs, but very uncommon on Web
//  { "UTF-32LE", "UTF-32LE",
//        UTF8},  // unlikely to cause XSS bugs, but very uncommon on Web
//  { "X-BINARYENC", "x-binaryenc",  // Not an IANA name
//        UTF8},  // because this one is not intended for output (just input)
//  { "HZ-GB-2312", "HZ-GB-2312",
//        CHINESE_GB},  // due to potential confusion with HTML syntax chars
//  { "X-UTF8UTF8", "x-utf8utf8",  // Not an IANA name
//        UTF8},  // because this one is not intended for output (just input)
//  { "X-TAM-ELANGO", "x-tam-elango",
//        UTF8},  // we do not have an output converter for this font encoding
//  { "X-TAM-LTTMBARANI", "x-tam-lttmbarani",
//        UTF8},  // we do not have an output converter for this font encoding
//  { "X-TAM-SHREE", "x-tam-shree",
//        UTF8},  // we do not have an output converter for this font encoding
//  { "X-TAM-TBOOMIS", "x-tam-tboomis",
//        UTF8},  // we do not have an output converter for this font encoding
//  { "X-TAM-TMNEWS", "x-tam-tmnews",
//        UTF8},  // we do not have an output converter for this font encoding
//  { "X-TAM-WEBTAMIL", "x-tam-webtamil",
//        UTF8},  // we do not have an output converter for this font encoding
//  
//  { "X-KDDI-Shift_JIS", "Shift_JIS", JAPANESE_SHIFT_JIS},
//  // KDDI version of Shift_JIS with Google Emoji PUA mappings.
//  // Note that MimeEncodingName() returns "Shift_JIS", since KDDI uses
//  // "Shift_JIS" in HTTP headers and email messages.
//  
//  { "X-DoCoMo-Shift_JIS", "Shift_JIS", JAPANESE_SHIFT_JIS},
//  // DoCoMo version of Shift_JIS with Google Emoji PUA mappings.
//  // See the comment at KDDI_SHIFT_JIS for other issues.
//  
//  { "X-SoftBank-Shift_JIS", "Shift_JIS", JAPANESE_SHIFT_JIS},
//  // SoftBank version of Shift_JIS with Google Emoji PUA mappings.
//  // See the comment at KDDI_SHIFT_JIS for other issues.
//  
//  { "X-KDDI-ISO-2022-JP", "ISO-2022-JP", JAPANESE_SHIFT_JIS},
//  // KDDI version of ISO-2022-JP with Google Emoji PUA mappings.
//  // See the comment at KDDI_SHIFT_JIS for other issues.
//  // The preferred Web encoding is due to potential confusion with
//  // HTML syntax chars.
//  
//  { "X-SoftBank-ISO-2022-JP", "ISO-2022-JP", JAPANESE_SHIFT_JIS},
//  // SoftBank version of ISO-2022-JP with Google Emoji PUA mappings.
//  // See the comment at KDDI_SHIFT_JIS for other issues.
//  // The preferred Web encoding is due to potential confusion with
//  // HTML syntax chars.
//  
//  // Please refer to NOTE: section in the comments in the definition
//  // of "struct I18NInfoByEncoding", before adding new encodings.
//  
//  };
//  


#define ENC_PARSE_NAM_ASCII                ",ASCII,ascii,"
#define ENC_PARSE_NAM_ANSI                 ",ANSI,ansi,SYSTEM,system" ENC_PARSE_NAM_ASCII
#define ENC_PARSE_NAM_OEM                  ",OEM,oem,"
#define ENC_PARSE_NAM_UTF16LEBOM           ",UTF-16LE-BOM,"
#define ENC_PARSE_NAM_UTF16BEBOM           ",UTF-16BE-BOM,"
#define ENC_PARSE_NAM_UTF16LE              ",UTF-16LE,UTF-16,utf16,utf16le,unicode,"
#define ENC_PARSE_NAM_UTF16BE              ",UTF-16BE,utf16be,unicodebe,"
#define ENC_PARSE_NAM_UTF8                 ",UTF-8,utf8,"
#define ENC_PARSE_NAM_UTF8SIG              ",UTF-8-SIG,utf8sig,"
#define ENC_PARSE_NAM_UTF7                 ",UTF-7,utf7,"
#define ENC_PARSE_NAM_DOS_720              ",DOS-720,dos720,"
#define ENC_PARSE_NAM_ISO_8859_6           ",ISO-8859-6,iso88596,arabic,csisolatinarabic,ecma114,isoir127,"
#define ENC_PARSE_NAM_MAC_ARABIC           ",x-mac-arabic,xmacarabic,mac-arabic,macarabic,"
#define ENC_PARSE_NAM_WIN_1256             ",Windows-1256,windows1256,CP-1256,cp1256,ansiarabic"
#define ENC_PARSE_NAM_DOS_775              ",CP-500,cp500,ibm775,"
#define ENC_PARSE_NAM_ISO_8859_4           ",ISO-8859-4,iso88594,csisolatin4,isoir110,l4,latin4,"
#define ENC_PARSE_NAM_WIN_1257             ",Windows-1257,windows1257,CP-1257,cp1257,ansibaltic,"
#define ENC_PARSE_NAM_DOS_852              ",CP-852,cp852,ibm852,"
#define ENC_PARSE_NAM_ISO_8859_2           ",ISO-8859-2,iso88592,csisolatin2,isoir101,latin2,l2,"
#define ENC_PARSE_NAM_MAC_CENTRAL_EUROP    ",x-mac-ce,xmacce,mac-ce,xmaccentraleurope,maccentraleurope,"
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
#define ENC_PARSE_NAM_WIN_874              ",Windows-874,windows874,dos874,CP-874,cp874,iso885911,TIS-620,tis620,isoir166,"
#define ENC_PARSE_NAM_DOS_857              ",CP-857,cp857,ibm857,"
#define ENC_PARSE_NAM_ISO_8859_9           ",ISO-8859-9,iso88599,latin5,isoir148,l5,"
#define ENC_PARSE_NAM_MAC_TURKISH          ",x-mac-turkish,xmacturkish,mac-turkish,macturkish,"
#define ENC_PARSE_NAM_WIN_1254             ",Windows-1254,windows1254,CP-1254,cp1254,"
#define ENC_PARSE_NAM_MAC_UKRAINIAN        ",x-mac-ukrainian,xmacukrainian,mac-ukrainian,macukrainian,"
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
#define ENC_PARSE_NAM_BIG5_HKSCS           ",big5hkscs,cnbig5hkscs,xxbig5hkscs,"
//#define ENC_PARSE_NAM_ISO_8859_10          "ISO-8859-10,iso885910,windows-28600,windows28600,CP-28600,cp28600,"
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
  /* 000 */{ NCP_ASCII_7BIT | NCP_ANSI | NCP_RECODE,              CP_ACP,   ENC_PARSE_NAM_ANSI,              IDS_ENC_ANSI,              L"" }, // CPI_ANSI_DEFAULT       0
  /* 001 */{ NCP_ASCII_7BIT | NCP_OEM | NCP_RECODE,               CP_OEMCP, ENC_PARSE_NAM_OEM,               IDS_ENC_OEM,               L"" }, // CPI_OEM                1
  /* 002 */{ NCP_UNICODE | NCP_UNICODE_BOM,                       CP_UTF8,  ENC_PARSE_NAM_UTF16LEBOM,        IDS_ENC_UTF16LEBOM,        L"" }, // CPI_UNICODEBOM         2
  /* 003 */{ NCP_UNICODE | NCP_UNICODE_REVERSE | NCP_UNICODE_BOM, CP_UTF8,  ENC_PARSE_NAM_UTF16BEBOM,        IDS_ENC_UTF16BEBOM,        L"" }, // CPI_UNICODEBEBOM       3
  /* 004 */{ NCP_UNICODE | NCP_RECODE,                            CP_UTF8,  ENC_PARSE_NAM_UTF16LE,           IDS_ENC_UTF16LE,           L"" }, // CPI_UNICODE            4
  /* 005 */{ NCP_UNICODE | NCP_UNICODE_REVERSE | NCP_RECODE,      CP_UTF8,  ENC_PARSE_NAM_UTF16BE,           IDS_ENC_UTF16BE,           L"" }, // CPI_UNICODEBE          5
  /* 006 */{ NCP_ASCII_7BIT | NCP_UTF8 | NCP_RECODE,              CP_UTF8,  ENC_PARSE_NAM_UTF8,              IDS_ENC_UTF8,              L"" }, // CPI_UTF8               6
  /* 007 */{ NCP_UTF8 | NCP_UTF8_SIGN,                            CP_UTF8,  ENC_PARSE_NAM_UTF8SIG,           IDS_ENC_UTF8SIG,           L"" }, // CPI_UTF8SIGN           7
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
  /* 082 */{ NCP_EXTERNAL_8BIT | NCP_RECODE,                      951,      ENC_PARSE_NAM_BIG5_HKSCS,        IDS_ENC_BIG5_HKSCS,        L"" }  // Chinese (Hong Kong Supplementary Character Set)
  
  ///* 079 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 28600, ENC_PARSE_NAM_ISO_8859_10,       IDS_ENC_ISO_8859_10,       ISO_8859_10,        L"" }, // Nordic (ISO 8859-10)

  
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

constexpr cpi_enc_t _CountOfEncodings() { return static_cast<cpi_enc_t>(ARRAYSIZE(g_Encodings)); }

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

constexpr float clampf(float x, float lower, float upper) { return (x < lower) ? lower : ((x > upper) ? upper : x); }

//=============================================================================

cpi_enc_t GetUnicodeEncoding(const char* pBuffer, const size_t len, bool* lpbBOM, bool* lpbReverse)
{
  cpi_enc_t iEncoding = CPI_NONE;

  size_t const enoughData = 2048LL;
  size_t const cb = (len < enoughData) ? len : enoughData;

  if (!pBuffer || cb < 2) { return iEncoding; }

  // IS_TEXT_UNICODE_UNICODE_MASK -> IS_TEXT_UNICODE_ASCII16, IS_TEXT_UNICODE_STATISTICS, IS_TEXT_UNICODE_CONTROLS, IS_TEXT_UNICODE_SIGNATURE.
  // IS_TEXT_UNICODE_REVERSE_MASK -> IS_TEXT_UNICODE_REVERSE_ASCII16, IS_TEXT_UNICODE_REVERSE_STATISTICS, IS_TEXT_UNICODE_REVERSE_CONTROLS, IS_TEXT_UNICODE_REVERSE_SIGNATURE.
  // IS_TEXT_UNICODE_NOT_UNICODE_MASK -> IS_TEXT_UNICODE_ILLEGAL_CHARS, IS_TEXT_UNICODE_ODD_LENGTH, and two currently unused bit flags.
  // IS_TEXT_UNICODE_NOT_ASCII_MASK -> IS_TEXT_UNICODE_NULL_BYTES and three currently unused bit flags.
  //
  int const iAllTests = IS_TEXT_UNICODE_UNICODE_MASK | IS_TEXT_UNICODE_REVERSE_MASK | IS_TEXT_UNICODE_NOT_UNICODE_MASK | IS_TEXT_UNICODE_NOT_ASCII_MASK;

  int iTest = iAllTests;
  /*bool const ok =*/ (void)IsTextUnicode(pBuffer, (int)cb, &iTest); // don't rely on result ok

  if (iTest == iAllTests) {
    iTest = 0; // iTest doesn't seem to have been modified ...
  }

  bool const bHasBOM = (iTest & IS_TEXT_UNICODE_SIGNATURE);
  bool const bHasRBOM = (iTest & IS_TEXT_UNICODE_REVERSE_SIGNATURE);

  bool const bIsUnicode = (iTest & IS_TEXT_UNICODE_UNICODE_MASK);
  bool const bIsReverse = (iTest & IS_TEXT_UNICODE_REVERSE_MASK);
  bool const bIsIllegal = (iTest & IS_TEXT_UNICODE_NOT_UNICODE_MASK);

  //bool const bHasNullBytes = (iTest & IS_TEXT_UNICODE_NULL_BYTES);

  if (bHasBOM || bHasRBOM || ((bIsUnicode || bIsReverse) && !bIsIllegal && !(bIsUnicode && bIsReverse)))
  {
    if (lpbBOM) {
      *lpbBOM = (bHasBOM || bHasRBOM);
    }
    if (lpbReverse) {
      *lpbReverse = (bHasRBOM || bIsReverse);
    }
    if (bHasBOM || bHasRBOM) {
      iEncoding = bHasBOM ? CPI_UNICODEBOM : CPI_UNICODEBEBOM;
    }
    else if (bIsUnicode || bIsReverse) {
      iEncoding = bIsUnicode ? CPI_UNICODE : CPI_UNICODEBE;
    }
  }
  return iEncoding;
}
// ============================================================================

constexpr Encoding _MapCPI2CEDEncoding(const cpi_enc_t cpiEncoding)
{
  if ((cpiEncoding < 0) || (cpiEncoding >= _CountOfEncodings())) { return UNKNOWN_ENCODING; }
  
  char parseNames[256] = { '\0' };
  StringCchCopyA(parseNames, 256, g_Encodings[cpiEncoding].pszParseNames);
  if (parseNames[0] == '\0') { return UNKNOWN_ENCODING; }

  char* p = &(parseNames[1]); // skip 1st null
  while (*p != '\0') {
    if (*p == ',') { *p = '\0'; }
    ++p;
  }
  *(++p) = '\0'; // ensure double '\0' at the end

  Encoding encoding = UNKNOWN_ENCODING;

  p = &(parseNames[1]); // skip 1st null
  while (*p != '\0') {
    if (EncodingFromName(p, &encoding)) { break; }
    for (; *p != '\0'; ++p) {} // next
    ++p; // double null at end
  }
  return encoding;
}
// ============================================================================


constexpr cpi_enc_t _MapStdEncodingString2CPI(const char* encStrg, float* pConfidence,
                                              const char* const text, const size_t len)
{
  float const confidence = *pConfidence;
  
  cpi_enc_t cpiEncoding = CPI_NONE;

  if (encStrg && (encStrg[0] != '\0')) {
    // preprocessing: special cases
    if (_stricmp(encStrg, "ascii") == 0) {
      cpiEncoding = CPI_ASCII_7BIT;
    }
    else {
      cpiEncoding = Encoding_MatchA(encStrg);
    }

    if (Encoding_IsUNICODE(cpiEncoding))
    {
      bool bBOM = false;
      bool bReverse = false;
      cpi_enc_t const cpi = GetUnicodeEncoding(text, len, &bBOM, &bReverse);
      if (!Encoding_IsNONE(cpiEncoding)) 
      {
        cpiEncoding = cpi;
      }
      else {
        cpiEncoding = bBOM ? (bReverse ? CPI_UNICODEBE : CPI_UNICODE) : (bReverse ? CPI_UNICODEBE : CPI_UNICODE);
      }
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


#if FALSE
// ============================================================================
//   CED (Compact Encoding Detector)
// ============================================================================

cpi_enc_t AnalyzeText_CED
(
  const char* const text, const size_t len, 
  const cpi_enc_t encodingHint,
  float* pConfidence, char* encodingStrg, int cch)
{
  float const ReliableCEDConfThresh = Settings2.ReliableCEDConfidenceMapping;
  float const UnReliableCEDConfThresh = Settings2.UnReliableCEDConfidenceMapping;

  cpi_enc_t cpiEncoding = CPI_NONE;
  float confidence = 0.0f;

  int bytes_consumed = 0;
  bool isReliable = false;

  Encoding const encoding = CompactEncDet::DetectEncoding(
    text, static_cast<int>(len),
    nullptr, nullptr, nullptr,
    _MapCPI2CEDEncoding(encodingHint),
    UNKNOWN_LANGUAGE, CompactEncDet::QUERY_CORPUS, true,
    &bytes_consumed,
    &isReliable);

  //~const char* charset = EncodingName(encoding);
  const char* charset = MimeEncodingName(encoding);

  StringCchCopyA(encodingStrg, cch, charset);  // CED

  confidence = isReliable ? ReliableCEDConfThresh : UnReliableCEDConfThresh;

  cpiEncoding = _MapStdEncodingString2CPI(charset, &confidence, text, len);

#if 1
  Encoding const check_enc = _MapCPI2CEDEncoding(cpiEncoding);
  if (encoding != check_enc) {
    *pConfidence = 0.0;
  }
#endif

  *pConfidence = confidence;
  return cpiEncoding; 
}
// ============================================================================
#endif


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

  int const result = uchardet_handle_data(hUcharDet, text, len);
  
  uchardet_data_end(hUcharDet); // transfer report

  switch (result) 
  {
  case HANDLE_DATA_RESULT_NEED_MORE_DATA:  // need more data is a result too 
  case HANDLE_DATA_RESULT_DETECTED:
  {
    const char* charset = uchardet_get_charset(hUcharDet);
    StringCchCopyA(encodingStrg, cch, charset);  // UCHARDET

    confidence = uchardet_get_confidence(hUcharDet);
    cpiEncoding = _MapStdEncodingString2CPI(charset, &confidence, text, len);
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
// ============================================================================

void Encoding_AnalyzeText(const char* const text, const size_t len,
                               ENC_DET_T* pEncDetInfo, const cpi_enc_t encodingHint)
{
  if (len == 0)
  {
    pEncDetInfo->analyzedEncoding = CPI_NONE;
    pEncDetInfo->confidence = 0.0f;
    return;
  }

  float confidence_UCD = 0.0f;
  cpi_enc_t cpiEncoding_UCD = CPI_NONE;

#if FALSE
  size_t const largeFile = static_cast<size_t>(Settings2.FileLoadWarningMB) * 1024LL * 1024LL;

  if (len < largeFile)
  {
    // small file: do SERIAL encoding detection
    cpiEncoding_UCD = AnalyzeText_UCHARDET(text, len, &ucd_cnf, encodingStrg_UCD, MAX_ENC_STRG_LEN);
    cpiEncoding_CED = AnalyzeText_CED(text, len, encodingHint, &ced_cnf, encodingStrg_CED, MAX_ENC_STRG_LEN);
  }
  else {  // large file:  start ASYNC PARALLEL encoding detection

    std::future<int> cpiUCD = std::async(std::launch::async, AnalyzeText_UCHARDET,
      text, len, encodingHint, &ucd_cnf, encodingStrg_UCD, MAX_ENC_STRG_LEN);

    std::future<int> cpiCED = std::async(std::launch::async, AnalyzeText_CED,
      text, len, encodingHint, &ced_cnf, encodingStrg_CED, MAX_ENC_STRG_LEN);

    cpiEncoding_UCD = cpiUCD.get();
    cpiEncoding_CED = cpiCED.get();
  }

#else

  // no need to run analyzers asynchrony, cause they analyze only the first KB of large files ...
  //~cpiEncoding_CED = AnalyzeText_CED(text, len, encodingHint, &ced_cnf, encodingStrg_CED, MAX_ENC_STRG_LEN);
  //~if (ced_cnf < 1.0f) 
  //~{
  cpiEncoding_UCD = AnalyzeText_UCHARDET(text, len, &confidence_UCD, pEncDetInfo->encodingStrg, COUNTOF(pEncDetInfo->encodingStrg));

  //~}
  //~else {
  //~  cpiEncoding_UCD = CPI_NONE;
  //~  ucd_cnf = 1.0f;
  //~}

#endif

  // ---  re-mapping UCD ----

  switch (Encoding_GetCodePage(cpiEncoding_UCD))
  {
  case 28591:  // ISO 8859 - 1  mapped to  Windows - 1252  (HTML5 Standard advice)
    cpiEncoding_UCD = Encoding_GetByCodePage(1252); // auto detect default ANSI (!)
    break;

  /*
  case 54936:
    if ((codePage_CED == 936) || (codePage_CED == 20936))
    {
      cpiEncoding_UCD = cpiEncoding_CED; // trust CED's choice
    }
    break;
  */

  default:
    break;
  }

  // UCARDET does not rely on encodingHint, so make a bias here
  confidence_UCD += (cpiEncoding_UCD == encodingHint) ? (1.0f - confidence_UCD) / 2.0f : 0.0f;
  // Default ANSI CodePage detection ? -> bonus
  confidence_UCD += (cpiEncoding_UCD == CPI_ANSI_DEFAULT) ? ((1.0f - confidence_UCD) * Settings2.LocaleAnsiCodePageAnalysisBonus) : 0.0f;


  pEncDetInfo->confidence = confidence_UCD;
  pEncDetInfo->analyzedEncoding = cpiEncoding_UCD;

  
  /* ~~~ //////////////////////////////////////////////////////////////////////

  // ---  re-mapping CED ----

  switch (codePage_CED)
  {
  case 20936:  // Map old GB2312 -> GBK
    cpiEncoding_CED = Encoding_GetByCodePage(936);
    break;

  case 28591:  // ISO 8859-1  mapped to  Windows-1252  (HTML5 Standard advice)
    cpiEncoding_CED = Encoding_GetByCodePage(1252);
    break;

  default:
    break;
  }

  // --------------------------------------------------------------------------
  // vote for encoding prognosis based on confidence levels or reliability
  // --------------------------------------------------------------------------

  float confidence = ucd_confidence;

  if ((cpiEncoding_UCD == cpiEncoding_CED) && !Encoding_IsNONE(cpiEncoding_UCD))
  {
    iAnalyzedEncoding = cpiEncoding_UCD;
    confidence = max_f(ucd_confidence, ced_confidence);
  }
  else { // ---  ambiguous results  ---

    if (Encoding_IsNONE(cpiEncoding_UCD))
    {
      // _NO_ UCHARDET rely on CED
      iAnalyzedEncoding = cpiEncoding_CED;
      confidence = ced_confidence;
    }
    else { // _OK_ UCHARDET result

      if ((ced_confidence < ucd_confidence) || Encoding_IsNONE(cpiEncoding_CED))
      {
        // unreliable CED use UCHARDET
        iAnalyzedEncoding = cpiEncoding_UCD;
        confidence = ucd_confidence;
      }
      else  // --- more reliable CED result  ---
      {
        iAnalyzedEncoding = cpiEncoding_CED;  // prefer CED
        confidence = (ucd_confidence + ced_confidence) / 2.0f;  // adjust confidence
      }
    }
  }
  *confidence_io = confidence;
  return iAnalyzedEncoding;

  ~~~ */ //////////////////////////////////////////////////////////////////////

}
// ============================================================================


//=============================================================================
//
//  _SetEncodingTitleInfo()
//
static void _SetEncodingTitleInfo(const ENC_DET_T* pEncDetInfo)
  //const char* encodingUCD, cpi_enc_t encUCD, float ucd_confidence)
{
  WCHAR encodingUCD[80] = { L'\0' };
  ::MultiByteToWideChar(CP_ACP, 0, pEncDetInfo->encodingStrg, -1, encodingUCD, COUNTOF(encodingUCD));
  cpi_enc_t const encUCD = pEncDetInfo->analyzedEncoding;
  float const ucd_confidence = pEncDetInfo->confidence;

  StringCchCopy(wchEncodingInfo, COUNTOF(wchEncodingInfo), L"UCD='");
  if (encUCD >= 0)
  {
    StringCchCat(wchEncodingInfo, COUNTOF(wchEncodingInfo), encodingUCD);
  }
  else {
    const WCHAR* const ukn = (encodingUCD[0] == L'\0') ? L"<unknown>" : encodingUCD;
    StringCchCat(wchEncodingInfo, COUNTOF(wchEncodingInfo), (encUCD == CPI_ASCII_7BIT) ? L"ASCII" : ukn);
  }
  WCHAR tmpBuf[80] = { '\0' };
  int const ucd_conf_perc = float2int(ucd_confidence * 100.0f);
  StringCchPrintf(tmpBuf, COUNTOF(tmpBuf), L"' Conf=%i%%", ucd_conf_perc);
  StringCchCat(wchEncodingInfo, COUNTOF(wchEncodingInfo), tmpBuf);

  //~StringCchCatA(chEncodingInfo, ARRAYSIZE(chEncodingInfo), " || CED='");
  //~if (encCED >= 0)
  //~{
  //~  //::WideCharToMultiByte(CP_UTF7, 0, Encoding_GetLabel(encCED), -1, chEncodingLabel, ARRAYSIZE(chEncodingLabel), 0, 0);
  //~  StringCchCatA(chEncodingInfo, ARRAYSIZE(chEncodingInfo), encodingCED);
  //~}
  //~else {
  //~  StringCchCatA(chEncodingInfo, ARRAYSIZE(chEncodingInfo), (encCED == CPI_ASCII_7BIT) ? "ASCII" : "<unknown>");
  //~}
  //~if ((encCED >= 0) || (encCED == CPI_ASCII_7BIT)) {
  //~  bool const ced_reliable = (ced_confidence >= Settings2.ReliableCEDConfidenceMapping);
  //~  bool const ced_not_reliable = (ced_confidence <= Settings2.UnReliableCEDConfidenceMapping);
  //~  StringCchPrintfA(tmpBuf, ARRAYSIZE(tmpBuf), "' Conf=%.0f%% [%s])", ced_confidence * 100.0f,
  //~    ced_reliable ? "reliable" : (ced_not_reliable ? "NOT reliable" : "???"));
  //~  StringCchCatA(chEncodingInfo, ARRAYSIZE(chEncodingInfo), tmpBuf);
  //~}
  //~else {
  //~  StringCchCatA(chEncodingInfo, ARRAYSIZE(chEncodingInfo), "'");
  //~}
  
  int const relThreshold = float2int(Settings2.AnalyzeReliableConfidenceLevel * 100.0f);
  const WCHAR* rel_fmt = (ucd_conf_perc >= relThreshold) ? L" (reliable (%i%%))" : L" (NOT reliable (%i%%))";
  StringCchPrintf(tmpBuf, COUNTOF(tmpBuf), rel_fmt, relThreshold);
  StringCchCat(wchEncodingInfo, COUNTOF(wchEncodingInfo), tmpBuf);

  const WCHAR* const validUTF8 = (pEncDetInfo->bValidUTF8) ? L" [Valid UTF-8]" : L" [Invalid UTF-8]";
  StringCchCat(wchEncodingInfo, COUNTOF(wchEncodingInfo), validUTF8);

}



//=============================================================================
//
//  _SetFileVars()
//
static void _SetFileVars(char* buffer, size_t cch, LPFILEVARS lpfv)
{
  bool bDisableFileVar = false;

  if (!Flags.NoFileVariables)
  {
    int i;
    if (FileVars_ParseInt(buffer, "enable-local-variables", &i) && (!i)) {
      bDisableFileVar = true;
    }
    if (!bDisableFileVar) {

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
    }

    char columns[SMALL_BUFFER];
    if (FileVars_ParseStr(buffer, "fill-column", columns, COUNTOF(columns))) {
      NormalizeColumnVector(columns, lpfv->wchMultiEdgeLines, COUNTOF(lpfv->wchMultiEdgeLines));
      lpfv->mask |= FV_LONGLINESLIMIT;
    }
  }

  // Unicode Sig
  bool const bHasSignature = IsUTF8Signature(buffer) || Has_UTF16_LE_BOM(buffer, cch) || Has_UTF16_BE_BOM(buffer, cch);

  if (!bHasSignature && !Settings.NoEncodingTags && !bDisableFileVar) {

    if (FileVars_ParseStr(buffer, "coding", lpfv->chEncoding, COUNTOF(lpfv->chEncoding))) {
      lpfv->mask |= FV_ENCODING;
    } else if (FileVars_ParseStr(buffer, "encoding", lpfv->chEncoding, COUNTOF(lpfv->chEncoding))) {
      lpfv->mask |= FV_ENCODING;
    } else if (FileVars_ParseStr(buffer, "charset", lpfv->chEncoding, COUNTOF(lpfv->chEncoding))) {
      lpfv->mask |= FV_ENCODING;
    }
  }
  if (lpfv->mask & FV_ENCODING) {
    lpfv->iEncoding = Encoding_MatchA(lpfv->chEncoding);
  }

  if (!Flags.NoFileVariables && !bDisableFileVar) {
    if (FileVars_ParseStr(buffer, "mode", lpfv->chMode, COUNTOF(lpfv->chMode))) {
      lpfv->mask |= FV_MODE;
    }
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

  if ((Flags.NoFileVariables && Settings.NoEncodingTags) || !lpData || !cbData) { return true; }

  char tmpbuf[LARGE_BUFFER];
  size_t const cch = min_s(cbData + 1, COUNTOF(tmpbuf));

  StringCchCopyNA(tmpbuf, COUNTOF(tmpbuf), lpData, cch);
  _SetFileVars(tmpbuf, cch, lpfv);

  // if no file vars found, look at EOF
  if ((lpfv->mask == 0) && (cbData > COUNTOF(tmpbuf))) {
    StringCchCopyNA(tmpbuf, COUNTOF(tmpbuf), lpData + cbData - COUNTOF(tmpbuf) + 1, COUNTOF(tmpbuf));
    _SetFileVars(tmpbuf, cch, lpfv);
  }

  return true;
}


//=============================================================================
//
//  FileVars_Apply()
//
extern "C" bool FileVars_Apply(LPFILEVARS lpfv) {

  int const _iTabWidth = (lpfv->mask & FV_TABWIDTH) ? lpfv->iTabWidth : Settings.TabWidth;
  SciCall_SetTabWidth(_iTabWidth);

  int const _iIndentWidth = (lpfv->mask & FV_INDENTWIDTH) ? lpfv->iIndentWidth : ((lpfv->mask & FV_TABWIDTH) ? 0 : Settings.IndentWidth);
  SciCall_SetIndent(_iIndentWidth);

  bool const _bTabsAsSpaces = (lpfv->mask & FV_TABSASSPACES) ? lpfv->bTabsAsSpaces : Settings.TabsAsSpaces;
  SciCall_SetUseTabs(!_bTabsAsSpaces);

  bool const _bTabIndents = (lpfv->mask & FV_TABINDENTS) ? lpfv->bTabIndents : Settings.TabIndents;
  SciCall_SetTabIndents(_bTabIndents);
  SciCall_SetBackSpaceUnIndents(Settings.BackspaceUnindents);

  bool const _bWordWrap = (lpfv->mask & FV_WORDWRAP) ? lpfv->bWordWrap : Settings.WordWrap;
  int const  _iWrapMode = _bWordWrap ? ((Settings.WordWrapMode == 0) ? SC_WRAP_WHITESPACE : SC_WRAP_CHAR) : SC_WRAP_NONE;
  SciCall_SetWrapMode(_iWrapMode);

  int edgeColumns[SMALL_BUFFER];
  size_t const cnt = ReadVectorFromString(lpfv->wchMultiEdgeLines, edgeColumns, COUNTOF(edgeColumns), 0, LONG_LINES_MARKER_LIMIT, 0, true);
  Style_SetMultiEdgeLine(edgeColumns, cnt);

  return true;
}


//=============================================================================
//
//  FileVars_ParseInt()
//
extern "C" bool FileVars_ParseInt(char* pszData, char* pszName, int* piValue) {

  char* pvStart = StrStrIA(pszData, pszName);
  while (pvStart) {
    char chPrev = (pvStart > pszData) ? *(pvStart - 1) : 0;
    if (!IsCharAlphaNumericA(chPrev) && chPrev != '-' && chPrev != '_') {
      pvStart += StringCchLenA(pszName, 0);
      while (*pvStart == ' ') {
        pvStart++;
      }
      if (*pvStart == ':' || *pvStart == '=') { break; }
    }
    else {
      pvStart += StringCchLenA(pszName, 0);
    }
    pvStart = StrStrIA(pvStart, pszName); // next
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
    StrTrimA(tch, " \t:=\"'");

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
extern "C" bool FileVars_ParseStr(char* pszData, char* pszName, char* pszValue, int cchValue) {

  const char* pvStart = StrStrIA(pszData, pszName);
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
    }
    else {
      pvStart += StringCchLenA(pszName, 0);
    }
    pvStart = StrStrIA(pvStart, pszName);  // next
  }

  if (pvStart)
  {
    bool bQuoted = false;
    while (*pvStart && StrChrIA(":=\"' \t", *pvStart)) {
      if (*pvStart == '\'' || *pvStart == '"')
        bQuoted = true;
      pvStart++;
    }

    char tch[32] = { L'\0' };
    StringCchCopyNA(tch, COUNTOF(tch), pvStart, COUNTOF(tch));

    char* pvEnd = tch;
    while (*pvEnd && (IsCharAlphaNumericA(*pvEnd) || StrChrIA("+-/_", *pvEnd) || (bQuoted && *pvEnd == ' '))) {
      pvEnd++;
    }
    *pvEnd = 0;

    StrTrimA(tch, " \t:=\"'");

    StringCchCopyNA(pszValue, cchValue, tch, COUNTOF(tch));

    return true;
  }
  return false;
}


//=============================================================================
//
//  FileVars_IsUTF8()
//
extern "C" bool FileVars_IsUTF8(LPFILEVARS lpfv) {
  if (lpfv->mask & FV_ENCODING) {
    if (StringCchCompareNIA(lpfv->chEncoding, COUNTOF(lpfv->chEncoding), "utf-8", CSTRLEN("utf-8")) == 0 ||
      StringCchCompareNIA(lpfv->chEncoding, COUNTOF(lpfv->chEncoding), "utf8", CSTRLEN("utf8")) == 0)
      return true;
  }
  return false;
}


//=============================================================================
//
//  FileVars_IsValidEncoding()
//
extern "C" bool FileVars_IsValidEncoding(LPFILEVARS lpfv) {
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

//=============================================================================
//
//  GetFileEncoding()
//
extern "C" ENC_DET_T Encoding_DetectEncoding(LPWSTR pszFile, const char* lpData, const size_t cbData, 
                                             cpi_enc_t iAnalyzeHint,
                                             bool bSkipUTFDetection, bool bSkipANSICPDetection, bool bForceEncDetection)
{
  ENC_DET_T encDetRes = INIT_ENC_DET_T;

  FileVars_GetFromData(lpData, cbData, &Globals.fvCurFile);

  bool const bBOM_LE = Has_UTF16_LE_BOM(lpData, cbData);
  bool const bBOM_BE = Has_UTF16_BE_BOM(lpData, cbData);

  #define IS_ENC_ENFORCED() (!Encoding_IsNONE(encDetRes.forcedEncoding))

  // --- 1st check for force encodings ---

  LPCWSTR lpszExt = PathFindExtension(pszFile);
  bool const bNfoDizDetected = (lpszExt && !(StringCchCompareXI(lpszExt, L".nfo") && StringCchCompareXI(lpszExt, L".diz")));

  encDetRes.forcedEncoding = (Settings.LoadNFOasOEM && bNfoDizDetected) ? Globals.DOSEncoding : Encoding_Forced(CPI_GET);

  encDetRes.bHasBOM = (bBOM_LE || bBOM_BE);
  encDetRes.bIsReverse = bBOM_BE;
  encDetRes.bIs7BitASCII = IsValidUTF7(lpData, cbData);
  if (encDetRes.bIs7BitASCII) {
    bSkipUTFDetection = true;
    bSkipANSICPDetection = true;
    encDetRes.bValidUTF8 = true;
  }
  else {
    encDetRes.bIsUTF8Sig = ((cbData >= 3) ? IsUTF8Signature(lpData) : false);
    encDetRes.bValidUTF8 = IsValidUTF8(lpData, cbData);
  }

  if (!IS_ENC_ENFORCED()) 
  {
    // force file vars ?
    encDetRes.fileVarEncoding = (FileVars_IsValidEncoding(&Globals.fvCurFile)) ? FileVars_GetEncoding(&Globals.fvCurFile) : CPI_NONE;
    if (Encoding_IsValid(encDetRes.fileVarEncoding) && (Globals.fvCurFile.mask & FV_ENCODING)) {
      encDetRes.forcedEncoding = encDetRes.fileVarEncoding;
    }
  }
  if (!IS_ENC_ENFORCED() && encDetRes.bIs7BitASCII)
  {
    encDetRes.forcedEncoding = (Settings.LoadASCIIasUTF8) ? CPI_UTF8 : CPI_ANSI_DEFAULT;
  }


  // --- 2nd Use Encoding Analysis if applicable

  size_t const cbNbytes4Analysis = min_s(cbData, 200000LL);

  encDetRes.confidence = 0.0f;

  // analysis hint correction
  if (Encoding_IsUTF8(iAnalyzeHint) && !encDetRes.bValidUTF8) { iAnalyzeHint = CPI_ANSI_DEFAULT; }

  if (!IS_ENC_ENFORCED() || bForceEncDetection)
  {
    if (!bSkipANSICPDetection) 
    {
      // ---------------------------------------------------------------------------
      Encoding_AnalyzeText(lpData, cbNbytes4Analysis, &encDetRes, iAnalyzeHint);
      // ---------------------------------------------------------------------------
    }

    if (encDetRes.analyzedEncoding == CPI_NONE)
    {
      encDetRes.analyzedEncoding = iAnalyzeHint;
      encDetRes.confidence = (1.0f - Settings2.AnalyzeReliableConfidenceLevel);
    }
    else if (encDetRes.analyzedEncoding == CPI_ASCII_7BIT) 
    {
      encDetRes.analyzedEncoding = (Settings.LoadASCIIasUTF8) ? CPI_UTF8 : CPI_ANSI_DEFAULT;
    }

    if (!bSkipUTFDetection)
    {
      encDetRes.unicodeAnalysis = GetUnicodeEncoding(lpData, cbData, &(encDetRes.bHasBOM), &(encDetRes.bIsReverse));

      if (Encoding_IsNONE(encDetRes.unicodeAnalysis) && Encoding_IsUNICODE(encDetRes.analyzedEncoding))
      {
        encDetRes.unicodeAnalysis = encDetRes.analyzedEncoding;
      }

      //// check for UTF-32, can't handle
      //if (encDetRes.bHasBOM && !bBOM_LE && !bBOM_BE) {
      //  encDetRes.unicodeAnalysis = CPI_NONE;
      //}
      //else if (encDetRes.bHasBOM && encDetRes.bIsReverse && !bBOM_BE) {
      //  encDetRes.unicodeAnalysis = CPI_NONE;
      //}
      //else if (encDetRes.bHasBOM && !encDetRes.bIsReverse && !bBOM_LE) {
      //  // must be UTF-32, can't handle
      //  encDetRes.unicodeAnalysis = CPI_NONE;
      //}
    }

    if (bForceEncDetection) 
    {
      if (Encoding_IsValid(encDetRes.analyzedEncoding)) {
        // no bIsReliable check (forced unreliable detection)
        encDetRes.forcedEncoding = encDetRes.analyzedEncoding;
      }
      else if (Encoding_IsValid(encDetRes.unicodeAnalysis)) {
        encDetRes.forcedEncoding = encDetRes.unicodeAnalysis;
      }
    }
  }

  if (Flags.bDevDebugMode)
  {
    _SetEncodingTitleInfo(&encDetRes);
  }

  int const iConfidence = float2int(encDetRes.confidence * 100.0f);
  int const iReliableThreshold = float2int(Settings2.AnalyzeReliableConfidenceLevel * 100.0f);
  encDetRes.bIsAnalysisReliable = (iConfidence >= iReliableThreshold);

  // --------------------------------------------------------------------------
  // ---  choose best encoding guess  ----
  // --------------------------------------------------------------------------

  // init Preferred Encoding
  encDetRes.Encoding = CPI_PREFERRED_ENCODING;

  if (IS_ENC_ENFORCED()) 
  {
    encDetRes.Encoding = encDetRes.forcedEncoding;
  }
  else if (encDetRes.bIsUTF8Sig)
  {
    encDetRes.Encoding = CPI_UTF8SIGN;
  }
  else if (bBOM_LE || bBOM_BE) {
    encDetRes.Encoding = bBOM_LE ? CPI_UNICODEBOM : CPI_UNICODEBEBOM;
    encDetRes.bIsReverse = bBOM_BE;
  }
  else if (Encoding_IsValid(encDetRes.analyzedEncoding) && (encDetRes.bIsAnalysisReliable || !Settings.UseReliableCEDonly))
  {
    encDetRes.Encoding = encDetRes.analyzedEncoding;
  }
  else if (Encoding_IsValid(Encoding_SrcWeak(CPI_GET))) {
    encDetRes.Encoding = Encoding_SrcWeak(CPI_GET);
  }
  else if (Encoding_IsValid(iAnalyzeHint)) {
    encDetRes.Encoding = iAnalyzeHint;
  }

  if (!Encoding_IsValid(encDetRes.Encoding)) { encDetRes.Encoding = CPI_PREFERRED_ENCODING; }

  return encDetRes;
}


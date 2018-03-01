/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* Encoding.c                                                                   *
*   General helper functions                                                  *
*   Based on code from Notepad2, (c) Florian Balmer 1996-2011                 *
*	Parts taken from SciTE, (c) Neil Hodgson                                    *
*	MinimizeToTray, (c) 2000 Matthew Ellis                                      *
*                                                                             *
*                                                  (c) Rizonesoft 2015-2018   *
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

#include <windows.h>
#include <commctrl.h>
#include <uxtheme.h>

//#include "../uthash/utarray.h"

#include "scintilla.h"
#include "helpers.h"
#include "encoding.h"


extern HINSTANCE g_hInstance;


//=============================================================================
//
//  Encoding Helper Functions
//

int g_DOSEncoding;

// Supported Encodings
WCHAR wchANSI[16] = { L'\0' };
WCHAR wchOEM[16] = { L'\0' };

static NP2ENCODING g_Encodings[] = {
{ NCP_ANSI | NCP_RECODE,                               CP_ACP, "ansi,system,ascii,",                                       61000, L"" },
{ NCP_OEM | NCP_RECODE,                                CP_OEMCP, "oem,oem,",                                               61001, L"" },
{ NCP_UNICODE | NCP_UNICODE_BOM,                       CP_UTF8, "",                                                        61002, L"" },
{ NCP_UNICODE | NCP_UNICODE_REVERSE | NCP_UNICODE_BOM, CP_UTF8, "",                                                        61003, L"" },
{ NCP_UNICODE | NCP_RECODE,                            CP_UTF8, "utf-16,utf16,unicode,",                                   61004, L"" },
{ NCP_UNICODE | NCP_UNICODE_REVERSE | NCP_RECODE,      CP_UTF8, "utf-16be,utf16be,unicodebe,",                             61005, L"" },
{ NCP_UTF8 | NCP_RECODE,                               CP_UTF8, "utf-8,utf8,",                                             61006, L"" },
{ NCP_UTF8 | NCP_UTF8_SIGN,                            CP_UTF8, "utf-8,utf8,",                                             61007, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE,                      CP_UTF7, "utf-7,utf7,",                                             61008, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 720,   "DOS-720,dos720,",                                                                61009, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 28596, "iso-8859-6,iso88596,arabic,csisolatinarabic,ecma114,isoir127,",                  61010, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 10004, "x-mac-arabic,xmacarabic,",                                                       61011, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 1256,  "windows-1256,windows1256,cp1256",                                                61012, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 775,   "ibm775,ibm775,cp500,",                                                           61013, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 28594, "iso-8859-4,iso88594,csisolatin4,isoir110,l4,latin4,",                            61014, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 1257,  "windows-1257,windows1257,",                                                      61015, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 852,   "ibm852,ibm852,cp852,",                                                           61016, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 28592, "iso-8859-2,iso88592,csisolatin2,isoir101,latin2,l2,",                            61017, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 10029, "x-mac-ce,xmacce,",                                                               61018, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 1250,  "windows-1250,windows1250,xcp1250,",                                              61019, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 936,   "gb2312,gb2312,chinese,cngb,csgb2312,csgb231280,gb231280,gbk,",                   61020, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 10008, "x-mac-chinesesimp,xmacchinesesimp,",                                             61021, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 950,   "big5,big5,cnbig5,csbig5,xxbig5,",                                                61022, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 10002, "x-mac-chinesetrad,xmacchinesetrad,",                                             61023, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 10082, "x-mac-croatian,xmaccroatian,",                                                   61024, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 866,   "cp866,cp866,ibm866,",                                                            61025, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 28595, "iso-8859-5,iso88595,csisolatin5,csisolatincyrillic,cyrillic,isoir144,",          61026, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 20866, "koi8-r,koi8r,cskoi8r,koi,koi8,",                                                 61027, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 21866, "koi8-u,koi8u,koi8ru,",                                                           61028, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 10007, "x-mac-cyrillic,xmaccyrillic,",                                                   61029, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 1251,  "windows-1251,windows1251,xcp1251,",                                              61030, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 28603, "iso-8859-13,iso885913,",                                                         61031, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 863,   "ibm863,ibm863,",                                                                 61032, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 737,   "ibm737,ibm737,",                                                                 61033, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 28597, "iso-8859-7,iso88597,csisolatingreek,ecma118,elot928,greek,greek8,isoir126,",     61034, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 10006, "x-mac-greek,xmacgreek,",                                                         61035, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 1253,  "windows-1253,windows1253,",                                                      61036, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 869,   "ibm869,ibm869,",                                                                 61037, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 862,   "DOS-862,dos862,",                                                                61038, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 38598, "iso-8859-8-i,iso88598i,logical,",                                                61039, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 28598, "iso-8859-8,iso88598,csisolatinhebrew,hebrew,isoir138,visual,",                   61040, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 10005, "x-mac-hebrew,xmachebrew,",                                                       61041, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 1255,  "windows-1255,windows1255,",                                                      61042, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 861,   "ibm861,ibm861,",                                                                 61043, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 10079, "x-mac-icelandic,xmacicelandic,",                                                 61044, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 10001, "x-mac-japanese,xmacjapanese,",                                                   61045, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 932,   "shift_jis,shiftjis,shiftjs,csshiftjis,cswindows31j,mskanji,xmscp932,xsjis,",     61046, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 10003, "x-mac-korean,xmackorean,",                                                       61047, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 949,   "windows-949,windows949,ksc56011987,csksc5601,euckr,isoir149,korean,ksc56011989", 61048, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 28593, "iso-8859-3,iso88593,latin3,isoir109,l3,",                                        61049, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 28605, "iso-8859-15,iso885915,latin9,l9,",                                               61050, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 865,   "ibm865,ibm865,",                                                                 61051, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 437,   "ibm437,ibm437,437,cp437,cspc8,codepage437,",                                     61052, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 858,   "ibm858,ibm858,ibm00858,",                                                        61053, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 860,   "ibm860,ibm860,",                                                                 61054, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 10010, "x-mac-romanian,xmacromanian,",                                                   61055, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 10021, "x-mac-thai,xmacthai,",                                                           61056, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 874,   "windows-874,windows874,dos874,iso885911,tis620,",                                61057, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 857,   "ibm857,ibm857,",                                                                 61058, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 28599, "iso-8859-9,iso88599,latin5,isoir148,l5,",                                        61059, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 10081, "x-mac-turkish,xmacturkish,",                                                     61060, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 1254,  "windows-1254,windows1254,",                                                      61061, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 10017, "x-mac-ukrainian,xmacukrainian,",                                                 61062, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 1258,  "windows-1258,windows-258,",                                                      61063, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 850,   "ibm850,ibm850,",                                                                 61064, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 28591, "iso-8859-1,iso88591,cp819,latin1,ibm819,isoir100,latin1,l1,",                    61065, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 10000, "macintosh,macintosh,",                                                           61066, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 1252,  "windows-1252,windows1252,cp367,cp819,ibm367,us,xansi,",                          61067, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 37,    "ebcdic-cp-us,ebcdiccpus,ebcdiccpca,ebcdiccpwt,ebcdiccpnl,ibm037,cp037,",         61068, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 500,   "x-ebcdic-international,xebcdicinternational,",                                   61069, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 875,   "x-EBCDIC-GreekModern,xebcdicgreekmodern,",                                       61070, L"" },
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 1026,  "CP1026,cp1026,csibm1026,ibm1026,",                                               61071, L"" },
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 870,   "CP870,cp870,ebcdiccproece,ebcdiccpyu,csibm870,ibm870,",                          00000, L"" }, // IBM EBCDIC (Multilingual Latin-2)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 1047,  "IBM01047,ibm01047,",                                                             00000, L"" }, // IBM EBCDIC (Open System Latin-1)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 1140,  "x-ebcdic-cp-us-euro,xebcdiccpuseuro,",                                           00000, L"" }, // IBM EBCDIC (US-Canada-Euro)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 1141,  "x-ebcdic-germany-euro,xebcdicgermanyeuro,",                                      00000, L"" }, // IBM EBCDIC (Germany-Euro)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 1142,  "x-ebcdic-denmarknorway-euro,xebcdicdenmarknorwayeuro,",                          00000, L"" }, // IBM EBCDIC (Denmark-Norway-Euro)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 1143,  "x-ebcdic-finlandsweden-euro,xebcdicfinlandswedeneuro,",                          00000, L"" }, // IBM EBCDIC (Finland-Sweden-Euro)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 1144,  "x-ebcdic-italy-euro,xebcdicitalyeuro,",                                          00000, L"" }, // IBM EBCDIC (Italy-Euro)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 1145,  "x-ebcdic-spain-euro,xebcdicspaineuro,",                                          00000, L"" }, // IBM EBCDIC (Spain-Latin America-Euro)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 1146,  "x-ebcdic-uk-euro,xebcdicukeuro,",                                                00000, L"" }, // IBM EBCDIC (UK-Euro)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 1147,  "x-ebcdic-france-euro,xebcdicfranceeuro,",                                        00000, L"" }, // IBM EBCDIC (France-Euro)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 1148,  "x-ebcdic-international-euro,xebcdicinternationaleuro,",                          00000, L"" }, // IBM EBCDIC (International-Euro)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 1149,  "x-ebcdic-icelandic-euro,xebcdicicelandiceuro,",                                  00000, L"" }, // IBM EBCDIC (Icelandic-Euro)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 1361,  "johab,johab,",                                                                   00000, L"" }, // Korean (Johab)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20273, "x-EBCDIC-Germany,xebcdicgermany,",                                               00000, L"" }, // IBM EBCDIC (Germany)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20277, "x-EBCDIC-DenmarkNorway,xebcdicdenmarknorway,ebcdiccpdk,ebcdiccpno,",             00000, L"" }, // IBM EBCDIC (Denmark-Norway)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20278, "x-EBCDIC-FinlandSweden,xebcdicfinlandsweden,ebcdicpfi,ebcdiccpse,",              00000, L"" }, // IBM EBCDIC (Finland-Sweden)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20280, "x-EBCDIC-Italy,xebcdicitaly,",                                                   00000, L"" }, // IBM EBCDIC (Italy)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20284, "x-EBCDIC-Spain,xebcdicspain,ebcdiccpes,",                                        00000, L"" }, // IBM EBCDIC (Spain-Latin America)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20285, "x-EBCDIC-UK,xebcdicuk,ebcdiccpgb,",                                              00000, L"" }, // IBM EBCDIC (UK)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20290, "x-EBCDIC-JapaneseKatakana,xebcdicjapanesekatakana,",                             00000, L"" }, // IBM EBCDIC (Japanese Katakana)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20297, "x-EBCDIC-France,xebcdicfrance,ebcdiccpfr,",                                      00000, L"" }, // IBM EBCDIC (France)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20420, "x-EBCDIC-Arabic,xebcdicarabic,ebcdiccpar1,",                                     00000, L"" }, // IBM EBCDIC (Arabic)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20423, "x-EBCDIC-Greek,xebcdicgreek,ebcdiccpgr,",                                        00000, L"" }, // IBM EBCDIC (Greek)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20424, "x-EBCDIC-Hebrew,xebcdichebrew,ebcdiccphe,",                                      00000, L"" }, // IBM EBCDIC (Hebrew)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20833, "x-EBCDIC-KoreanExtended,xebcdickoreanextended,",                                 00000, L"" }, // IBM EBCDIC (Korean Extended)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20838, "x-EBCDIC-Thai,xebcdicthai,ibmthai,csibmthai,",                                   00000, L"" }, // IBM EBCDIC (Thai)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20871, "x-EBCDIC-Icelandic,xebcdicicelandic,ebcdiccpis,",                                00000, L"" }, // IBM EBCDIC (Icelandic)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20880, "x-EBCDIC-CyrillicRussian,xebcdiccyrillicrussian,ebcdiccyrillic,",                00000, L"" }, // IBM EBCDIC (Cyrillic Russian)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20905, "x-EBCDIC-Turkish,xebcdicturkish,ebcdiccptr,",                                    00000, L"" }, // IBM EBCDIC (Turkish)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20924, "IBM00924,ibm00924,ebcdiclatin9euro,",                                            00000, L"" }, // IBM EBCDIC (Open System-Euro Latin-1)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 21025, "x-EBCDIC-CyrillicSerbianBulgarian,xebcdiccyrillicserbianbulgarian,",             00000, L"" }, // IBM EBCDIC (Cyrillic Serbian-Bulgarian)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 50930, "x-EBCDIC-JapaneseAndKana,xebcdicjapaneseandkana,",                               00000, L"" }, // IBM EBCDIC (Japanese and Japanese Katakana)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 50931, "x-EBCDIC-JapaneseAndUSCanada,xebcdicjapaneseanduscanada,",                       00000, L"" }, // IBM EBCDIC (Japanese and US-Canada)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 50933, "x-EBCDIC-KoreanAndKoreanExtended,xebcdickoreanandkoreanextended,",               00000, L"" }, // IBM EBCDIC (Korean and Korean Extended)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 50935, "x-EBCDIC-SimplifiedChinese,xebcdicsimplifiedchinese,",                           00000, L"" }, // IBM EBCDIC (Chinese Simplified)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 50937, "x-EBCDIC-TraditionalChinese,xebcdictraditionalchinese,",                         00000, L"" }, // IBM EBCDIC (Chinese Traditional)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 50939, "x-EBCDIC-JapaneseAndJapaneseLatin,xebcdicjapaneseandjapaneselatin,",             00000, L"" }, // IBM EBCDIC (Japanese and Japanese-Latin)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20105, "x-IA5,xia5,",                                                                    00000, L"" }, // Western European (IA5)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20106, "x-IA5-German,xia5german,",                                                       00000, L"" }, // German (IA5)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20107, "x-IA5-Swedish,xia5swedish,",                                                     00000, L"" }, // Swedish (IA5)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20108, "x-IA5-Norwegian,xia5norwegian,",                                                 00000, L"" }, // Norwegian (IA5)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20936, "x-cp20936,xcp20936,",                                                            00000, L"" }, // Chinese Simplified (GB2312)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20932, "euc-jp,,",                                                                       00000, L"" }, // Japanese (JIS X 0208-1990 & 0212-1990)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 50220, "iso-2022-jp,iso2022jp,",                                                         00000, L"" }, // Japanese (JIS)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 50221, "csISO2022JP,csiso2022jp,",                                                       00000, L"" }, // Japanese (JIS-Allow 1 byte Kana)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 50222, "_iso-2022-jp$SIO,iso2022jpSIO,",                                                 00000, L"" }, // Japanese (JIS-Allow 1 byte Kana - SO/SI)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 50225, "iso-2022-kr,iso2022kr,csiso2022kr,",                                             00000, L"" }, // Korean (ISO-2022-KR)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 50227, "x-cp50227,xcp50227,",                                                            00000, L"" }, // Chinese Simplified (ISO-2022)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 50229, "iso-2022-cn,iso2022cn,",                                                         00000, L"" }, // Chinese Traditional (ISO-2022)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20000, "x-Chinese-CNS,xchinesecns,",                                                     00000, L"" }, // Chinese Traditional (CNS)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 20002, "x-Chinese-Eten,xchineseeten,",                                                   00000, L"" }, // Chinese Traditional (Eten)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 51932, "euc-jp,eucjp,xeuc,xeucjp,",                                                      00000, L"" }, // Japanese (EUC)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 51936, "euc-cn,euccn,xeuccn,",                                                           00000, L"" }, // Chinese Simplified (EUC)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 51949, "euc-kr,euckr,cseuckr,",                                                          00000, L"" }, // Korean (EUC)
//{ NCP_EXTERNAL_8BIT|NCP_RECODE, 52936, "hz-gb-2312,hzgb2312,hz,",                                                        00000, L"" }, // Chinese Simplified (HZ-GB2312)
{ NCP_EXTERNAL_8BIT | NCP_RECODE, 54936, "gb18030,gb18030,",                                                               61072, L"" }  // Chinese Simplified (GB18030)
                                                                                                                                         //{ NCP_EXTERNAL_8BIT|NCP_RECODE, 57002, "x-iscii-de,xisciide,",                                                           00000, L"" }, // ISCII Devanagari
                                                                                                                                         //{ NCP_EXTERNAL_8BIT|NCP_RECODE, 57003, "x-iscii-be,xisciibe,",                                                           00000, L"" }, // ISCII Bengali
                                                                                                                                         //{ NCP_EXTERNAL_8BIT|NCP_RECODE, 57004, "x-iscii-ta,xisciita,",                                                           00000, L"" }, // ISCII Tamil
                                                                                                                                         //{ NCP_EXTERNAL_8BIT|NCP_RECODE, 57005, "x-iscii-te,xisciite,",                                                           00000, L"" }, // ISCII Telugu
                                                                                                                                         //{ NCP_EXTERNAL_8BIT|NCP_RECODE, 57006, "x-iscii-as,xisciias,",                                                           00000, L"" }, // ISCII Assamese
                                                                                                                                         //{ NCP_EXTERNAL_8BIT|NCP_RECODE, 57007, "x-iscii-or,xisciior,",                                                           00000, L"" }, // ISCII Oriya
                                                                                                                                         //{ NCP_EXTERNAL_8BIT|NCP_RECODE, 57008, "x-iscii-ka,xisciika,",                                                           00000, L"" }, // ISCII Kannada
                                                                                                                                         //{ NCP_EXTERNAL_8BIT|NCP_RECODE, 57009, "x-iscii-ma,xisciima,",                                                           00000, L"" }, // ISCII Malayalam
                                                                                                                                         //{ NCP_EXTERNAL_8BIT|NCP_RECODE, 57010, "x-iscii-gu,xisciigu,",                                                           00000, L"" }, // ISCII Gujarathi
                                                                                                                                         //{ NCP_EXTERNAL_8BIT|NCP_RECODE, 57011, "x-iscii-pa,xisciipa,",                                                           00000, L"" }, // ISCII Panjabi
};

int Encoding_CountOf() {
  return COUNTOF(g_Encodings);
}

int Encoding_Current(int iEncoding) {
  static int CurrentEncoding = CPI_NONE;

  if (iEncoding >= 0) {
    if (Encoding_IsValid(iEncoding))
      CurrentEncoding = iEncoding;
    else
      CurrentEncoding = CPI_UTF8;
  }
  return CurrentEncoding;
}


int Encoding_SrcCmdLn(int iSrcEncoding) {
  static int SourceEncoding = CPI_NONE;

  if (iSrcEncoding >= 0) {
    if (Encoding_IsValid(iSrcEncoding))
      SourceEncoding = iSrcEncoding;
    else
      SourceEncoding = CPI_UTF8;
  }
  else if (iSrcEncoding == CPI_NONE) {
    SourceEncoding = CPI_NONE;
  }
  return SourceEncoding;
}


int  Encoding_SrcWeak(int iSrcWeakEnc) {
  static int SourceWeakEncoding = CPI_NONE;

  if (iSrcWeakEnc >= 0) {
    if (Encoding_IsValid(iSrcWeakEnc))
      SourceWeakEncoding = iSrcWeakEnc;
    else
      SourceWeakEncoding = CPI_ANSI_DEFAULT;
  }
  else if (iSrcWeakEnc == CPI_NONE) {
    SourceWeakEncoding = CPI_NONE;
  }
  return SourceWeakEncoding;
}


BOOL Encoding_HasChanged(int iOriginalEncoding) {
  static int OriginalEncoding = CPI_NONE;

  if (iOriginalEncoding >= CPI_NONE) {
    OriginalEncoding = iOriginalEncoding;
  }
  return (BOOL)(OriginalEncoding != Encoding_Current(CPI_GET));
}
// ============================================================================


void Encoding_InitDefaults() 

{
  const UINT uCodePageMBCS[20] = {
    42, // (Symbol)
    50220,50221,50222,50225,50227,50229, // (Chinese, Japanese, Korean) 
    54936, // (GB18030)
    57002,57003,57004,57005,57006,57007,57008,57009,57010,57011, // (ISCII)
    65000, // (UTF-7)
    65001  // (UTF-8)
  };

  g_Encodings[CPI_ANSI_DEFAULT].uCodePage = GetACP(); // set ANSI system CP
  StringCchPrintf(wchANSI, COUNTOF(wchANSI), L" (CP-%u)", g_Encodings[CPI_ANSI_DEFAULT].uCodePage);

  for (int i = CPI_UTF7 + 1; i < COUNTOF(g_Encodings); ++i) {
    if (Encoding_IsValid(i) && (g_Encodings[i].uCodePage == g_Encodings[CPI_ANSI_DEFAULT].uCodePage)) {
      g_Encodings[i].uFlags |= NCP_ANSI;
      if (g_Encodings[i].uFlags & NCP_EXTERNAL_8BIT)
        g_Encodings[CPI_ANSI_DEFAULT].uFlags |= NCP_EXTERNAL_8BIT;
      break;
    }
  }

  g_Encodings[CPI_OEM].uCodePage = GetOEMCP();
  StringCchPrintf(wchOEM, COUNTOF(wchOEM), L" (CP-%u)", g_Encodings[CPI_OEM].uCodePage);

  for (int i = CPI_UTF7 + 1; i < COUNTOF(g_Encodings); ++i) {
    if (Encoding_IsValid(i) && (g_Encodings[i].uCodePage == g_Encodings[CPI_OEM].uCodePage)) {
      g_Encodings[i].uFlags |= NCP_OEM;
      if (g_Encodings[i].uFlags & NCP_EXTERNAL_8BIT)
        g_Encodings[CPI_OEM].uFlags |= NCP_EXTERNAL_8BIT;
      break;
    }
  }

  // multi byte character sets
  for (int i = 0; i < COUNTOF(g_Encodings); ++i) {
    for (int k = 0; k < COUNTOF(uCodePageMBCS); k++) {
      if (g_Encodings[i].uCodePage == uCodePageMBCS[k]) {
        g_Encodings[i].uFlags |= NCP_MBCS;
      }
    }
  }

  g_DOSEncoding = CPI_OEM;
  // Try to set the DOS encoding to DOS-437 if the default OEMCP is not DOS-437
  if (g_Encodings[g_DOSEncoding].uCodePage != 437) {
    for (int i = CPI_UTF7 + 1; i < COUNTOF(g_Encodings); ++i) {
      if (Encoding_IsValid(i) && (g_Encodings[i].uCodePage == 437)) {
        g_DOSEncoding = i;
        break;
      }
    }
  }
}
// ============================================================================


int Encoding_MapIniSetting(BOOL bLoad, int iSetting) {
  if (bLoad) {
    switch (iSetting) {
    case -1: return CPI_NONE;
    case  0: return CPI_ANSI_DEFAULT;
    case  1: return CPI_UNICODEBOM;
    case  2: return CPI_UNICODEBEBOM;
    case  3: return CPI_UTF8;
    case  4: return CPI_UTF8SIGN;
    case  5: return CPI_OEM;
    case  6: return CPI_UNICODE;
    case  7: return CPI_UNICODEBE;
    case  8: return CPI_UTF7;
    default: {
      for (int i = CPI_UTF7 + 1; i < COUNTOF(g_Encodings); i++) {
        if ((g_Encodings[i].uCodePage == (UINT)iSetting) && Encoding_IsValid(i))
          return(i);
      }
      return CPI_ANSI_DEFAULT;
    }
    }
  }
  else {
    switch (iSetting) {
    case CPI_NONE:         return -1;
    case CPI_ANSI_DEFAULT: return  0;
    case CPI_UNICODEBOM:   return  1;
    case CPI_UNICODEBEBOM: return  2;
    case CPI_UTF8:         return  3;
    case CPI_UTF8SIGN:     return  4;
    case CPI_OEM:          return  5;
    case CPI_UNICODE:      return  6;
    case CPI_UNICODEBE:    return  7;
    case CPI_UTF7:         return  8;
    default: {
      if (Encoding_IsValid(iSetting))
        return(g_Encodings[iSetting].uCodePage);
      else
        return CPI_ANSI_DEFAULT;
    }
    }
  }
}
// ============================================================================


int Encoding_MapUnicode(int iUni) {

  if (iUni == CPI_UNICODEBOM)
    return CPI_UNICODE;
  else if (iUni == CPI_UNICODEBEBOM)
    return CPI_UNICODEBE;
  else if (iUni == CPI_UTF8SIGN)
    return CPI_UTF8;
  else
    return iUni;
}
// ============================================================================


void Encoding_SetLabel(int iEncoding) {
  if (g_Encodings[iEncoding].wchLabel[0] == L'\0') {
    WCHAR wch1[128] = { L'\0' };
    WCHAR wch2[128] = { L'\0' };
    GetString(g_Encodings[iEncoding].idsName, wch1, COUNTOF(wch1));
    WCHAR *pwsz = StrChr(wch1, L';');
    if (pwsz) {
      pwsz = StrChr(CharNext(pwsz), L';');
      if (pwsz) {
        pwsz = CharNext(pwsz);
      }
    }
    if (!pwsz)
      pwsz = wch1;

    StringCchCopyN(wch2, COUNTOF(wch2), pwsz, COUNTOF(wch1));

    if (Encoding_IsANSI(iEncoding))
      StringCchCatN(wch2, COUNTOF(wch2), wchANSI, COUNTOF(wchANSI));
    else if (Encoding_IsOEM(iEncoding))
      StringCchCatN(wch2, COUNTOF(wch2), wchOEM, COUNTOF(wchOEM));

    StringCchCopyN(g_Encodings[iEncoding].wchLabel, COUNTOF(g_Encodings[iEncoding].wchLabel),
      wch2, COUNTOF(g_Encodings[iEncoding].wchLabel));
  }
}
// ============================================================================


int Encoding_MatchW(LPCWSTR pwszTest) {
  char tchTest[256] = { '\0' };
  WideCharToMultiByteStrg(CP_ACP, pwszTest, tchTest);
  return(Encoding_MatchA(tchTest));
}
// ============================================================================


int Encoding_MatchA(char *pchTest) {
  char  chTest[256] = { '\0' };
  char *pchSrc = pchTest;
  char *pchDst = chTest;
  *pchDst++ = ',';
  while (*pchSrc) {
    if (IsCharAlphaNumericA(*pchSrc))
      *pchDst++ = *CharLowerA(pchSrc);
    pchSrc++;
  }
  *pchDst++ = ',';
  *pchDst = 0;
  for (int i = 0; i < COUNTOF(g_Encodings); i++) {
    if (StrStrIA(g_Encodings[i].pszParseNames, chTest)) {
      CPINFO cpi;
      if ((g_Encodings[i].uFlags & NCP_INTERNAL) ||
        IsValidCodePage(g_Encodings[i].uCodePage) &&
        GetCPInfo(g_Encodings[i].uCodePage, &cpi))
        return(i);
      else
        return(-1);
    }
  }
  return(-1);
}
// ============================================================================


int Encoding_GetByCodePage(UINT cp) {
  for (int i = 0; i < COUNTOF(g_Encodings); i++) {
    if (cp == g_Encodings[i].uCodePage) {
      return i;
    }
  }
  return CPI_ANSI_DEFAULT;
}
// ============================================================================


BOOL Encoding_IsValid(int iTestEncoding) {
  CPINFO cpi;
  if ((iTestEncoding >= 0) && (iTestEncoding < COUNTOF(g_Encodings))) {
    if ((g_Encodings[iTestEncoding].uFlags & NCP_INTERNAL) ||
      IsValidCodePage(g_Encodings[iTestEncoding].uCodePage) &&
      GetCPInfo(g_Encodings[iTestEncoding].uCodePage, &cpi)) {
      return(TRUE);
    }
  }
  return(FALSE);
}
// ============================================================================


typedef struct _ee {
  int    id;
  WCHAR  wch[256];
} ENCODINGENTRY, *PENCODINGENTRY;

int CmpEncoding(const void *s1, const void *s2) {
  return StrCmp(((PENCODINGENTRY)s1)->wch, ((PENCODINGENTRY)s2)->wch);
}
// ============================================================================


void Encoding_AddToListView(HWND hwnd, int idSel, BOOL bRecodeOnly) {
  int i;
  int iSelItem = -1;
  LVITEM lvi;
  WCHAR wchBuf[256] = { L'\0' };

  PENCODINGENTRY pEE = LocalAlloc(LPTR, COUNTOF(g_Encodings) * sizeof(ENCODINGENTRY));
  for (i = 0; i < COUNTOF(g_Encodings); i++) {
    pEE[i].id = i;
    GetString(g_Encodings[i].idsName, pEE[i].wch, COUNTOF(pEE[i].wch));
  }
  qsort(pEE, COUNTOF(g_Encodings), sizeof(ENCODINGENTRY), CmpEncoding);

  ZeroMemory(&lvi, sizeof(LVITEM));
  lvi.mask = LVIF_PARAM | LVIF_TEXT | LVIF_IMAGE;
  lvi.pszText = wchBuf;

  for (i = 0; i < COUNTOF(g_Encodings); i++) {

    int id = pEE[i].id;
    if (!bRecodeOnly || (g_Encodings[id].uFlags & NCP_RECODE)) {

      lvi.iItem = ListView_GetItemCount(hwnd);

      WCHAR *pwsz = StrChr(pEE[i].wch, L';');
      if (pwsz) {
        StringCchCopyN(wchBuf, COUNTOF(wchBuf), CharNext(pwsz), COUNTOF(wchBuf));
        pwsz = StrChr(wchBuf, L';');
        if (pwsz)
          *pwsz = 0;
      }
      else
        StringCchCopyN(wchBuf, COUNTOF(wchBuf), pEE[i].wch, COUNTOF(wchBuf));

      if (Encoding_IsANSI(id))
        StringCchCatN(wchBuf, COUNTOF(wchBuf), wchANSI, COUNTOF(wchANSI));
      else if (Encoding_IsOEM(id))
        StringCchCatN(wchBuf, COUNTOF(wchBuf), wchOEM, COUNTOF(wchOEM));

      if (Encoding_IsValid(id))
        lvi.iImage = 0;
      else
        lvi.iImage = 1;

      lvi.lParam = (LPARAM)id;
      ListView_InsertItem(hwnd, &lvi);

      if (idSel == id)
        iSelItem = lvi.iItem;
    }
  }

  LocalFree(pEE);

  if (iSelItem != -1) {
    ListView_SetItemState(hwnd, iSelItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    ListView_EnsureVisible(hwnd, iSelItem, FALSE);
  }
  else {
    ListView_SetItemState(hwnd, 0, LVIS_FOCUSED, LVIS_FOCUSED);
    ListView_EnsureVisible(hwnd, 0, FALSE);
  }
}
// ============================================================================


BOOL Encoding_GetFromListView(HWND hwnd, int *pidEncoding) {
  LVITEM lvi;

  lvi.iItem = ListView_GetNextItem(hwnd, -1, LVNI_ALL | LVNI_SELECTED);
  lvi.iSubItem = 0;
  lvi.mask = LVIF_PARAM;

  if (ListView_GetItem(hwnd, &lvi)) {
    if (Encoding_IsValid((int)lvi.lParam))
      *pidEncoding = (int)lvi.lParam;
    else
      *pidEncoding = -1;

    return (TRUE);
  }
  return(FALSE);
}
// ============================================================================


void Encoding_AddToComboboxEx(HWND hwnd, int idSel, BOOL bRecodeOnly) {
  int i;
  int iSelItem = -1;
  COMBOBOXEXITEM cbei;
  WCHAR wchBuf[256] = { L'\0' };

  PENCODINGENTRY pEE = LocalAlloc(LPTR, COUNTOF(g_Encodings) * sizeof(ENCODINGENTRY));
  for (i = 0; i < COUNTOF(g_Encodings); i++) {
    pEE[i].id = i;
    GetString(g_Encodings[i].idsName, pEE[i].wch, COUNTOF(pEE[i].wch));
  }
  qsort(pEE, COUNTOF(g_Encodings), sizeof(ENCODINGENTRY), CmpEncoding);

  ZeroMemory(&cbei, sizeof(COMBOBOXEXITEM));
  cbei.mask = CBEIF_TEXT | CBEIF_IMAGE | CBEIF_SELECTEDIMAGE | CBEIF_LPARAM;
  cbei.pszText = wchBuf;
  cbei.cchTextMax = COUNTOF(wchBuf);
  cbei.iImage = 0;
  cbei.iSelectedImage = 0;

  for (i = 0; i < COUNTOF(g_Encodings); i++) {

    int id = pEE[i].id;
    if (!bRecodeOnly || (g_Encodings[id].uFlags & NCP_RECODE)) {

      cbei.iItem = SendMessage(hwnd, CB_GETCOUNT, 0, 0);

      WCHAR *pwsz = StrChr(pEE[i].wch, L';');
      if (pwsz) {
        StringCchCopyN(wchBuf, COUNTOF(wchBuf), CharNext(pwsz), COUNTOF(wchBuf));
        pwsz = StrChr(wchBuf, L';');
        if (pwsz)
          *pwsz = 0;
      }
      else
        StringCchCopyN(wchBuf, COUNTOF(wchBuf), pEE[i].wch, COUNTOF(wchBuf));

      if (Encoding_IsANSI(id))
        StringCchCatN(wchBuf, COUNTOF(wchBuf), wchANSI, COUNTOF(wchANSI));
      else if (id == CPI_OEM)
        StringCchCatN(wchBuf, COUNTOF(wchBuf), wchOEM, COUNTOF(wchOEM));

      cbei.iImage = (Encoding_IsValid(id) ? 0 : 1);

      cbei.lParam = (LPARAM)id;
      SendMessage(hwnd, CBEM_INSERTITEM, 0, (LPARAM)&cbei);

      if (idSel == id)
        iSelItem = (int)cbei.iItem;
    }
  }

  LocalFree(pEE);

  if (iSelItem != -1)
    SendMessage(hwnd, CB_SETCURSEL, (WPARAM)iSelItem, 0);
}
// ============================================================================


BOOL Encoding_GetFromComboboxEx(HWND hwnd, int *pidEncoding) {
  COMBOBOXEXITEM cbei;

  cbei.iItem = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
  cbei.mask = CBEIF_LPARAM;

  if (SendMessage(hwnd, CBEM_GETITEM, 0, (LPARAM)&cbei)) {
    if (Encoding_IsValid((int)cbei.lParam))
      *pidEncoding = (int)cbei.lParam;
    else
      *pidEncoding = -1;

    return (TRUE);
  }
  return(FALSE);
}
// ============================================================================


UINT Encoding_GetCodePage(int iEncoding) {
  return g_Encodings[iEncoding].uCodePage;
}
// ============================================================================

BOOL Encoding_IsDefault(int iEncoding) {
  return (g_Encodings[iEncoding].uFlags & NCP_DEFAULT);
}
// ============================================================================

BOOL Encoding_IsANSI(int iEncoding) {
  return (g_Encodings[iEncoding].uFlags & NCP_ANSI);
}
// ============================================================================

BOOL Encoding_IsOEM(int iEncoding) {
  return (g_Encodings[iEncoding].uFlags & NCP_OEM);
}
// ============================================================================

BOOL Encoding_IsUTF8(int iEncoding) {
  return (g_Encodings[iEncoding].uFlags & NCP_UTF8);
}
// ============================================================================

BOOL Encoding_IsUTF8_SIGN(int iEncoding) {
  return (g_Encodings[iEncoding].uFlags & NCP_UTF8_SIGN);
}
// ============================================================================

BOOL Encoding_IsMBCS(int iEncoding) {
  return (g_Encodings[iEncoding].uFlags & NCP_MBCS);
}
// ============================================================================

BOOL Encoding_IsUNICODE(int iEncoding) {
  return (g_Encodings[iEncoding].uFlags & NCP_UNICODE);
}
// ============================================================================

BOOL Encoding_IsUNICODE_BOM(int iEncoding) {
  return (g_Encodings[iEncoding].uFlags & NCP_UNICODE_BOM);
}
// ============================================================================

BOOL Encoding_IsUNICODE_REVERSE(int iEncoding) {
  return (g_Encodings[iEncoding].uFlags & NCP_UNICODE_REVERSE);
}
// ============================================================================


BOOL Encoding_IsINTERNAL(int iEncoding) {
  return (g_Encodings[iEncoding].uFlags & NCP_INTERNAL);
}
// ============================================================================

BOOL Encoding_IsEXTERNAL_8BIT(int iEncoding) {
  return (g_Encodings[iEncoding].uFlags & NCP_EXTERNAL_8BIT);
}
// ============================================================================

BOOL Encoding_IsRECODE(int iEncoding) {
  return (g_Encodings[iEncoding].uFlags & NCP_RECODE);
}
// ============================================================================


void Encoding_SetDefaultFlag(int iEncoding) {
  g_Encodings[iEncoding].uFlags |= NCP_DEFAULT;
}
// ============================================================================


const WCHAR* Encoding_GetLabel(int iEncoding) {
  return g_Encodings[iEncoding].wchLabel;
}
// ============================================================================

const char* Encoding_GetParseNames(int iEncoding) {
  return g_Encodings[iEncoding].pszParseNames;
}
// ============================================================================




UINT Encoding_SciGetCodePage(HWND hwnd) {
  UNUSED(hwnd);
  return CP_UTF8;
  // remove internal support for Chinese, Japan, Korean DBCS  use UTF-8 instead
  /*
  int cp = (UINT)SendMessage(hwnd,SCI_GETCODEPAGE,0,0);
  if (cp == 932 || cp == 936 || cp == 949 || cp == 950) {
  return cp;
  }
  return (cp == 0) ? CP_ACP : CP_UTF8;
  */
}
// ============================================================================


int Encoding_SciMappedCodePage(int iEncoding) {
  UNUSED(iEncoding);
  return SC_CP_UTF8;
  // remove internal support for Chinese, Japan, Korean DBCS  use UTF-8 instead
  /*
  if (Encoding_IsValid(iEncoding)) {
  // check for Chinese, Japan, Korean DBCS code pages and switch accordingly
  int cp = (int)g_Encodings[iEncoding].uCodePage;
  if (cp == 932 || cp == 936 || cp == 949 || cp == 950) {
  return cp;
  }
  }
  */
}
// ============================================================================


void Encoding_SciSetCodePage(HWND hwnd, int iEncoding) {
  int cp = Encoding_SciMappedCodePage(iEncoding);
  SendMessage(hwnd, SCI_SETCODEPAGE, (WPARAM)cp, 0);
  // charsets can be changed via styles schema
  /*
  int charset = SC_CHARSET_ANSI;
  switch (cp) {
  case 932:
  charset = SC_CHARSET_SHIFTJIS;
  break;
  case 936:
  charset = SC_CHARSET_GB2312;
  break;
  case 949:
  charset = SC_CHARSET_HANGUL;
  break;
  case 950:
  charset = SC_CHARSET_CHINESEBIG5;
  break;
  default:
  charset = g_iDefaultCharSet;
  break;
  }
  SendMessage(hwnd,SCI_STYLESETCHARACTERSET,(WPARAM)STYLE_DEFAULT,(LPARAM)charset);
  */
}
// ============================================================================


extern BOOL bSkipUnicodeDetection;

BOOL IsUnicode(const char* pBuffer, int cb, LPBOOL lpbBOM, LPBOOL lpbReverse) {
  int i = 0xFFFF;

  BOOL bIsTextUnicode;

  BOOL bHasBOM;
  BOOL bHasRBOM;

  if (!pBuffer || cb < 2)
    return FALSE;

  if (!bSkipUnicodeDetection)
    bIsTextUnicode = IsTextUnicode(pBuffer, cb, &i);
  else
    bIsTextUnicode = FALSE;

  bHasBOM = (*((UNALIGNED PWCHAR)pBuffer) == 0xFEFF);
  bHasRBOM = (*((UNALIGNED PWCHAR)pBuffer) == 0xFFFE);

  if (i == 0xFFFF) // i doesn't seem to have been modified ...
    i = 0;

  if (bIsTextUnicode || bHasBOM || bHasRBOM ||
    ((i & (IS_TEXT_UNICODE_UNICODE_MASK | IS_TEXT_UNICODE_REVERSE_MASK)) &&
      !((i & IS_TEXT_UNICODE_UNICODE_MASK) && (i & IS_TEXT_UNICODE_REVERSE_MASK)) &&
      !(i & IS_TEXT_UNICODE_ODD_LENGTH) &&
      !(i & IS_TEXT_UNICODE_ILLEGAL_CHARS && !(i & IS_TEXT_UNICODE_REVERSE_SIGNATURE)) &&
      !((i & IS_TEXT_UNICODE_REVERSE_MASK) == IS_TEXT_UNICODE_REVERSE_STATISTICS))) {

    if (lpbBOM)
      *lpbBOM = (bHasBOM || bHasRBOM ||
      (i & (IS_TEXT_UNICODE_SIGNATURE | IS_TEXT_UNICODE_REVERSE_SIGNATURE)))
      ? TRUE : FALSE;

    if (lpbReverse)
      *lpbReverse = (bHasRBOM || (i & IS_TEXT_UNICODE_REVERSE_MASK)) ? TRUE : FALSE;

    return TRUE;
  }

  else

    return FALSE;
}
// ============================================================================


BOOL IsUTF8(const char* pTest, int nLength)
{
  static int byte_class_table[256] = {
    /*       00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  */
    /* 00 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 10 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 20 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 30 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 40 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 50 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 60 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 70 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 80 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    /* 90 */ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    /* A0 */ 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    /* B0 */ 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    /* C0 */ 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    /* D0 */ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    /* E0 */ 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 7, 7,
    /* F0 */ 9,10,10,10,11, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4
    /*       00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  */ };

  /* state table */
  typedef enum {
    kSTART = 0, kA, kB, kC, kD, kE, kF, kG, kERROR, kNumOfStates
  } utf8_state;

  static utf8_state state_table[] = {
    /*                            kSTART, kA,     kB,     kC,     kD,     kE,     kF,     kG,     kERROR */
    /* 0x00-0x7F: 0            */ kSTART, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR,
    /* 0x80-0x8F: 1            */ kERROR, kSTART, kA,     kERROR, kA,     kB,     kERROR, kB,     kERROR,
    /* 0x90-0x9f: 2            */ kERROR, kSTART, kA,     kERROR, kA,     kB,     kB,     kERROR, kERROR,
    /* 0xa0-0xbf: 3            */ kERROR, kSTART, kA,     kA,     kERROR, kB,     kB,     kERROR, kERROR,
    /* 0xc0-0xc1, 0xf5-0xff: 4 */ kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR,
    /* 0xc2-0xdf: 5            */ kA,     kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR,
    /* 0xe0: 6                 */ kC,     kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR,
    /* 0xe1-0xec, 0xee-0xef: 7 */ kB,     kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR,
    /* 0xed: 8                 */ kD,     kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR,
    /* 0xf0: 9                 */ kF,     kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR,
    /* 0xf1-0xf3: 10           */ kE,     kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR,
    /* 0xf4: 11                */ kG,     kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR, kERROR };

#define BYTE_CLASS(b) (byte_class_table[(unsigned char)b])
#define NEXT_STATE(b,cur) (state_table[(BYTE_CLASS(b) * kNumOfStates) + (cur)])

  utf8_state current = kSTART;
  int i;

  const char* pt = pTest;
  int len = nLength;

  for (i = 0; i < len; i++, pt++) {

    current = NEXT_STATE(*pt, current);
    if (kERROR == current)
      break;
  }

  return (current == kSTART) ? TRUE : FALSE;
}
// ============================================================================


BOOL IsUTF7(const char* pTest, int nLength) {
  int i;
  const char *pt = pTest;

  for (i = 0; i < nLength; i++) {
    if (*pt & 0x80 || !*pt)
      return FALSE;
    pt++;
  }

  return TRUE;
}
// ============================================================================


/* byte length of UTF-8 sequence based on value of first byte.
for UTF-16 (21-bit space), max. code length is 4, so we only need to look
at 4 upper bits.
*/
static const INT utf8_lengths[16] =
{
  1,1,1,1,1,1,1,1,        /* 0000 to 0111 : 1 byte (plain ASCII) */
  0,0,0,0,                /* 1000 to 1011 : not valid */
  2,2,                    /* 1100, 1101 : 2 bytes */
  3,                      /* 1110 : 3 bytes */
  4                       /* 1111 :4 bytes */
};
// ============================================================================


/*++
Function :
UTF8_mbslen_bytes [INTERNAL]

Calculates the byte size of a NULL-terminated UTF-8 string.

Parameters :
char *utf8_string : string to examine

Return value :
size (in bytes) of a NULL-terminated UTF-8 string.
-1 if invalid NULL-terminated UTF-8 string
--*/
INT UTF8_mbslen_bytes(LPCSTR utf8_string)
{
  INT length = 0;
  INT code_size;
  BYTE byte;

  while (*utf8_string) {
    byte = (BYTE)*utf8_string;

    if ((byte <= 0xF7) && (0 != (code_size = utf8_lengths[byte >> 4]))) {
      length += code_size;
      utf8_string += code_size;
    }
    else {
      /* we got an invalid byte value but need to count it,
      it will be later ignored during the string conversion */
      //WARN("invalid first byte value 0x%02X in UTF-8 sequence!\n",byte);
      length++;
      utf8_string++;
    }
  }
  length++; /* include NULL terminator */
  return length;
}
// ============================================================================


/*++
Function :
UTF8_mbslen [INTERNAL]

Calculates the character size of a NULL-terminated UTF-8 string.

Parameters :
char *utf8_string : string to examine
int byte_length : byte size of string

Return value :
size (in characters) of a UTF-8 string.
-1 if invalid UTF-8 string
--*/
INT UTF8_mbslen(LPCSTR source, INT byte_length)
{
  INT wchar_length = 0;
  INT code_size;
  BYTE byte;

  while (byte_length > 0) {
    byte = (BYTE)*source;

    /* UTF-16 can't encode 5-byte and 6-byte sequences, so maximum value
    for first byte is 11110111. Use lookup table to determine sequence
    length based on upper 4 bits of first byte */
    if ((byte <= 0xF7) && (0 != (code_size = utf8_lengths[byte >> 4]))) {
      /* 1 sequence == 1 character */
      wchar_length++;

      if (code_size == 4)
        wchar_length++;

      source += code_size;        /* increment pointer */
      byte_length -= code_size;   /* decrement counter*/
    }
    else {
      /*
      unlike UTF8_mbslen_bytes, we ignore the invalid characters.
      we only report the number of valid characters we have encountered
      to match the Windows behavior.
      */
      //WARN("invalid byte 0x%02X in UTF-8 sequence, skipping it!\n",
      //     byte);
      source++;
      byte_length--;
    }
  }
  return wchar_length;
}
// ============================================================================




/*
* Copyright (C) 2006-2016 Wu Yongwei <wuyongwei@gmail.com>
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any
* damages arising from the use of this software.
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute
* it freely, subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must
*    not claim that you wrote the original software.  If you use this
*    software in a product, an acknowledgement in the product
*    documentation would be appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must
*    not be misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source
*    distribution.
*
*
* The latest version of this software should be available at:
*      <URL:https://github.com/adah1972/tellenc>
*
*/

/**
* @file    TellEnc.c
*
* Program to detect the encoding of text.  It currently supports ASCII,
* UTF-8, UTF-16/32 (little-endian or big-endian), Latin1, Windows-1252,
* CP437, GB2312, GBK, Big5, and SJIS, among others.
*
* @version 1.22, 2016/07/26
* @author  Wu Yongwei
*/


typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;

typedef struct _char_count_t {
  uint16_t first;
  uint32_t second;
} char_count_t;

//typedef pair<uint16_t, uint32_t>  char_count_t;
//typedef map<uint16_t, uint32_t>   char_count_map_t;
//typedef vector<char_count_t>      char_count_vec_t;


typedef struct _freq_analysis_data_t {
    uint16_t    dbyte;
    const char* enc;
} freq_analysis_data_t;



typedef enum {
    UTF8_INVALID,
    UTF8_1,
    UTF8_2,
    UTF8_3,
    UTF8_4,
    UTF8_TAIL
} UTF8_State;


#define MAX_CHAR 256

static const unsigned char NON_TEXT_CHARS[] = { 0, 26, 127, 255 };
static const char NUL = '\0';
static const char DOS_EOF = '\x1A';
static const int EVEN = 0;
static const int ODD  = 1;

static UTF8_State utf8_char_table[MAX_CHAR];

// ============================================================================

static freq_analysis_data_t freq_analysis_data[] = {
    { 0x9a74, "windows-1250" },         // "št" (Czech)
    { 0xe865, "windows-1250" },         // "če" (Czech)
    { 0xf865, "windows-1250" },         // "ře" (Czech)
    { 0xe167, "windows-1250" },         // "ág" (Hungarian)
    { 0xe96c, "windows-1250" },         // "él" (Hungarian)
    { 0xb36f, "windows-1250" },         // "ło" (Polish)
    { 0xea7a, "windows-1250" },         // "ęz" (Polish)
    { 0xf377, "windows-1250" },         // "ów" (Polish)
    { 0x9d20, "windows-1250" },         // "ť " (Slovak)
    { 0xfa9d, "windows-1250" },         // "úť" (Slovak)
    { 0x9e69, "windows-1250" },         // "ži" (Slovenian)
    { 0xe869, "windows-1250" },         // "či" (Slovenian)
    { 0xe020, "windows-1252" },         // "à " (French)
    { 0xe920, "windows-1252" },         // "é " (French)
    { 0xe963, "windows-1252" },         // "éc" (French)
    { 0xe965, "windows-1252" },         // "ée" (French)
    { 0xe972, "windows-1252" },         // "ér" (French)
    { 0xe4e4, "windows-1252" },         // "ää" (Finnish)
    { 0xe474, "windows-1252" },         // "ät" (German)
    { 0xfc72, "windows-1252" },         // "ür" (German)
    { 0xed6e, "windows-1252" },         // "ín" (Spanish)
    { 0xf36e, "windows-1252" },         // "ón" (Spanish)
    { 0x8220, "cp437" },                // "é " (French)
    { 0x8263, "cp437" },                // "éc" (French)
    { 0x8265, "cp437" },                // "ée" (French)
    { 0x8272, "cp437" },                // "ér" (French)
    { 0x8520, "cp437" },                // "à " (French)
    { 0x8172, "cp437" },                // "ür" (German)
    { 0x8474, "cp437" },                // "ät" (German)
    { 0xc4c4, "cp437" },                // "──"
    { 0xcdcd, "cp437" },                // "══"
    { 0xdbdb, "cp437" },                // "██"
    { 0xa1a1, "gbk" },                  // "　"
    { 0xa1a2, "gbk" },                  // "、"
    { 0xa1a3, "gbk" },                  // "。"
    { 0xa1a4, "gbk" },                  // "·"
    { 0xa1b6, "gbk" },                  // "《"
    { 0xa1b7, "gbk" },                  // "》"
    { 0xa3ac, "gbk" },                  // "，"
    { 0xa3ba, "gbk" },                  // "："
    { 0xb5c4, "gbk" },                  // "的"
    { 0xc1cb, "gbk" },                  // "了"
    { 0xd2bb, "gbk" },                  // "一"
    { 0xcac7, "gbk" },                  // "是"
    { 0xb2bb, "gbk" },                  // "不"
    { 0xb8f6, "gbk" },                  // "个"
    { 0xc8cb, "gbk" },                  // "人"
    { 0xd5e2, "gbk" },                  // "这"
    { 0xd3d0, "gbk" },                  // "有"
    { 0xced2, "gbk" },                  // "我"
    { 0xc4e3, "gbk" },                  // "你"
    { 0xcbfb, "gbk" },                  // "他"
    { 0xcbfd, "gbk" },                  // "她"
    { 0xc9cf, "gbk" },                  // "上"
    { 0xbfb4, "gbk" },                  // "看"
    { 0xd6ae, "gbk" },                  // "之"
    { 0xbbb9, "gbk" },                  // "还"
    { 0xbfc9, "gbk" },                  // "可"
    { 0xbaf3, "gbk" },                  // "后"
    { 0xd6d0, "gbk" },                  // "中"
    { 0xd0d0, "gbk" },                  // "行"
    { 0xb1d2, "gbk" },                  // "币"
    { 0xb3f6, "gbk" },                  // "出"
    { 0xb7d1, "gbk" },                  // "费"
    { 0xb8d0, "gbk" },                  // "感"
    { 0xbef5, "gbk" },                  // "觉"
    { 0xc4ea, "gbk" },                  // "年"
    { 0xd4c2, "gbk" },                  // "月"
    { 0xc8d5, "gbk" },                  // "日"
    { 0xa140, "big5" },                 // "　"
    { 0xa141, "big5" },                 // "，"
    { 0xa143, "big5" },                 // "。"
    { 0xa147, "big5" },                 // "："
    { 0xaaba, "big5" },                 // "的"
    { 0xa446, "big5" },                 // "了"
    { 0xa440, "big5" },                 // "一"
    { 0xac4f, "big5" },                 // "是"
    { 0xa4a3, "big5" },                 // "不"
    { 0xa448, "big5" },                 // "人"
    { 0xa7da, "big5" },                 // "我"
    { 0xa741, "big5" },                 // "你"
    { 0xa54c, "big5" },                 // "他"
    { 0xa66f, "big5" },                 // "她"
    { 0xadd3, "big5" },                 // "個"
    { 0xa457, "big5" },                 // "上"
    { 0xa662, "big5" },                 // "在"
    { 0xbba1, "big5" },                 // "說"
    { 0xa65e, "big5" },                 // "回"
    { 0x8140, "sjis" },                 // "　"
    { 0x8141, "sjis" },                 // "、"
    { 0x8142, "sjis" },                 // "。"
    { 0x8145, "sjis" },                 // "・"
    { 0x8146, "sjis" },                 // "："
    { 0x815b, "sjis" },                 // "ー"
    { 0x82b5, "sjis" },                 // "し"
    { 0x82bd, "sjis" },                 // "た"
    { 0x82c8, "sjis" },                 // "な"
    { 0x82c9, "sjis" },                 // "に"
    { 0x82cc, "sjis" },                 // "の"
    { 0x82dc, "sjis" },                 // "ま"
    { 0x82f0, "sjis" },                 // "を"
    { 0x8367, "sjis" },                 // "ト"
    { 0x8393, "sjis" },                 // "ン"
    { 0x89ef, "sjis" },                 // "会"
    { 0x906c, "sjis" },                 // "人"
    { 0x9094, "sjis" },                 // "数"
    { 0x93fa, "sjis" },                 // "日"
    { 0x95f1, "sjis" },                 // "報"
    { 0xa1bc, "euc-jp" },               // "ー"
    { 0xa4bf, "euc-jp" },               // "た"
    { 0xa4ca, "euc-jp" },               // "な"
    { 0xa4cb, "euc-jp" },               // "に"
    { 0xa4ce, "euc-jp" },               // "の"
    { 0xa4de, "euc-jp" },               // "ま"
    { 0xa4f2, "euc-jp" },               // "を"
    { 0xa5c8, "euc-jp" },               // "ト"
    { 0xa5f3, "euc-jp" },               // "ン"
    { 0xb2f1, "euc-jp" },               // "会"
    { 0xbfcd, "euc-jp" },               // "人"
    { 0xbff4, "euc-jp" },               // "数"
    { 0xc6fc, "euc-jp" },               // "日"
    { 0xcaf3, "euc-jp" },               // "報"
    { 0xc0cc, "euc-kr" },               // "이"
    { 0xb0fa, "euc-kr" },               // "과"
    { 0xb1e2, "euc-kr" },               // "기"
    { 0xb4c2, "euc-kr" },               // "는"
    { 0xb7ce, "euc-kr" },               // "로"
    { 0xb1db, "euc-kr" },               // "글"
    { 0xc5e4, "euc-kr" },               // "토"
    { 0xc1a4, "euc-kr" },               // "정"
    { 0xc920, "koi8-r" },               // "и "
    { 0xc7cf, "koi8-r" },               // "го"
    { 0xcbcf, "koi8-r" },               // "ко"
    { 0xd3cb, "koi8-r" },               // "ск"
    { 0xd3d4, "koi8-r" },               // "ст"
    { 0xa6a7, "koi8-u" },               // "ії"
    { 0xa6ce, "koi8-u" },               // "ін"
    { 0xa6d7, "koi8-u" },               // "ів"
    { 0xa7ce, "koi8-u" },               // "їн"
    { 0xd0cf, "koi8-u" },               // "по"
    { 0xd4c9, "koi8-u" },               // "ти"
};
// ============================================================================


static size_t nul_count_byte[2];
static size_t nul_count_word[2];

static bool is_binary = false;
static bool is_valid_utf8 = true;
static bool is_valid_latin1 = true;
static uint32_t dbyte_cnt = 0;
static uint32_t dbyte_hihi_cnt = 0;


// ============================================================================
// ============================================================================


static inline bool is_non_text(char ch)
{
  for (size_t i = 0; i < sizeof(NON_TEXT_CHARS); ++i) {
    if (ch == NON_TEXT_CHARS[i]) {
      return true;
    }
  }
  return false;
}
// ============================================================================



void init_utf8_char_table()
{
  int ch = 0;
  utf8_char_table[ch] = UTF8_INVALID;
  ++ch;
  for (; ch <= 0x7f; ++ch) {
    utf8_char_table[ch] = UTF8_1;
  }
  for (; ch <= 0xbf; ++ch) {
    utf8_char_table[ch] = UTF8_TAIL;
  }
  for (; ch <= 0xc1; ++ch) {
    utf8_char_table[ch] = UTF8_INVALID;
  }
  for (; ch <= 0xdf; ++ch) {
    utf8_char_table[ch] = UTF8_2;
  }
  for (; ch <= 0xef; ++ch) {
    utf8_char_table[ch] = UTF8_3;
  }
  for (; ch <= 0xf4; ++ch) {
    utf8_char_table[ch] = UTF8_4;
  }
  for (; ch <= 0xff; ++ch) {
    utf8_char_table[ch] = UTF8_INVALID;
  }
}
// ============================================================================



static void init_sbyte_char_count(char_count_t sbyte_char_cnt[])
{
  for (size_t i = 0; i < MAX_CHAR; ++i) {
    sbyte_char_cnt[i].first = (uint16_t)i;
    sbyte_char_cnt[i].second = 0;
  }
}
// ============================================================================






#if FALSE


typedef struct _pattern_t {
  const char* name;
  const char* pattern;
  size_t pattern_len;
} pattern_t;

static const char* check_ucs_bom(const unsigned char* const buffer, const size_t len)
{
  const pattern_t patterns[] = {
      { "ucs-4",     "\x00\x00\xFE\xFF",  4 },
      { "ucs-4le",   "\xFF\xFE\x00\x00",  4 },
      { "utf-8",     "\xEF\xBB\xBF",      3 },
      { "utf-16",    "\xFE\xFF",          2 },
      { "utf-16le",  "\xFF\xFE",          2 },
      { NULL,        NULL,                0 }
  };
  for (size_t i = 0; patterns[i].name; ++i) {
    const pattern_t* item = &(patterns[i]);
    if (len >= item->pattern_len &&  memcmp(buffer, item->pattern, item->pattern_len) == 0) {
      return item->name;
    }
  }
  return NULL;
}
// ============================================================================



static const char* check_freq_dbyte(uint16_t dbyte)
{
  for (size_t i = 0;
    i < sizeof freq_analysis_data / sizeof(freq_analysis_data_t);
    ++i) {
    if (dbyte == freq_analysis_data[i].dbyte) {
      return freq_analysis_data[i].enc;
    }
  }
  return NULL;
}
// ============================================================================



static const char* search_freq_dbytes(const char_count_vec_t* dbyte_char_cnt)
{
  size_t max_comp_idx = 10;
  if (max_comp_idx > dbyte_char_cnt->size()) {
    max_comp_idx = dbyte_char_cnt->size();
  }
  for (size_t i = 0; i < max_comp_idx; ++i) {
    const char* enc = check_freq_dbyte(dbyte_char_cnt[i].first);
    if (enc) {
      return enc;
    }
  }
  return NULL;
}
// ============================================================================



const char* tellenc(const unsigned char* const buffer, const size_t len)
{
    if (len == 0) {
        return "unknown";
    }

    const char* result = check_ucs_bom(buffer, len);
    if (result) {
        return result;
    }

    char_count_t sbyte_char_cnt[MAX_CHAR];
    char_count_map_t dbyte_char_cnt_map;
    init_sbyte_char_count(sbyte_char_cnt);

    unsigned char ch;
    int last_ch = EOF;
    int utf8_state = UTF8_1;
    for (size_t i = 0; i < len; ++i) {
        ch = buffer[i];
        sbyte_char_cnt[ch].second++;

        // Check for binary data (including UTF-16/32)
        if (is_non_text(ch)) {
            if (!is_binary && !(ch == DOS_EOF && i == len - 1)) {
                is_binary = true;
            }
            if (ch == NUL) {
                // Count for NULs in even- and odd-number bytes
                nul_count_byte[i & 1]++;
                if (i & 1) {
                    if (buffer[i - 1] == NUL) {
                        // Count for NULs in even- and odd-number words
                        nul_count_word[(i / 2) & 1]++;
                    }
                }
            }
        }

        // Check for UTF-8 validity
        if (is_valid_utf8) {
            switch (utf8_char_table[ch]) {
            case UTF8_INVALID:
                is_valid_utf8 = false;
                break;
            case UTF8_1:
                if (utf8_state != UTF8_1) {
                    is_valid_utf8 = false;
                }
                break;
            case UTF8_2:
                if (utf8_state != UTF8_1) {
                    is_valid_utf8 = false;
                } else {
                    utf8_state = UTF8_2;
                }
                break;
            case UTF8_3:
                if (utf8_state != UTF8_1) {
                    is_valid_utf8 = false;
                } else {
                    utf8_state = UTF8_3;
                }
                break;
            case UTF8_4:
                if (utf8_state != UTF8_1) {
                    is_valid_utf8 = false;
                } else {
                    utf8_state = UTF8_4;
                }
                break;
            case UTF8_TAIL:
                if (utf8_state > UTF8_1) {
                    utf8_state--;
                } else {
                    is_valid_utf8 = false;
                }
                break;
            }
        }

        // Check whether non-Latin1 characters appear
        if (is_valid_latin1) {
            if (ch >= 0x80 && ch < 0xa0) {
                is_valid_latin1 = false;
            }
        }

        // Construct double-bytes and count
        if (last_ch != EOF) {
            uint16_t dbyte_char = (last_ch << 8) + ch;
            dbyte_char_cnt_map[dbyte_char]++;
            dbyte_cnt++;
            if (last_ch > 0xa0 && ch > 0xa0) {
                dbyte_hihi_cnt++;
            }
            last_ch = EOF;
        } else if (ch >= 0x80) {
            last_ch = ch;
        }
    }

    // Get the character counts in descending order
    sort(sbyte_char_cnt, sbyte_char_cnt + MAX_CHAR, greater_char_count());

    // Get the double-byte counts in descending order
    char_count_vec_t dbyte_char_cnt;
    for (char_count_map_t::iterator it = dbyte_char_cnt_map.begin();
            it != dbyte_char_cnt_map.end(); ++it) {
        dbyte_char_cnt.push_back(*it);
    }
    sort(dbyte_char_cnt.begin(),
         dbyte_char_cnt.end(),
         greater_char_count());

    if (!is_valid_utf8 && is_binary) {
        // Heuristics for UTF-16/32
        if        (nul_count_byte[EVEN] > 4 &&
                   (nul_count_byte[ODD] == 0 ||
                    nul_count_byte[EVEN] / nul_count_byte[ODD] > 20)) {
            return "utf-16";
        } else if (nul_count_byte[ODD] > 4 &&
                   (nul_count_byte[EVEN] == 0 ||
                    nul_count_byte[ODD] / nul_count_byte[EVEN] > 20)) {
            return "utf-16le";
        } else if (nul_count_word[EVEN] > 4 &&
                   (nul_count_word[ODD] == 0 ||
                    nul_count_word[EVEN] / nul_count_word[ODD] > 20)) {
            return "ucs-4";   // utf-32 is not a built-in encoding for Vim
        } else if (nul_count_word[ODD] > 4 &&
                   (nul_count_word[EVEN] == 0 ||
                    nul_count_word[ODD] / nul_count_word[EVEN] > 20)) {
            return "ucs-4le"; // utf-32le is not a built-in encoding for Vim
        } else {
            return "binary";
        }
    } else if (dbyte_cnt == 0) {
        // No characters outside the scope of ASCII
        return "ascii";
    } else if (is_valid_utf8) {
        // Only valid UTF-8 sequences
        return "utf-8";
    } else if (const char* enc = search_freq_dbytes(dbyte_char_cnt)) {
        return enc;
    } else if (dbyte_hihi_cnt * 100 / dbyte_cnt < 5) {
        // Mostly a low-byte follows a high-byte
        return "windows-1252";
    }
    return NULL;
}
// ============================================================================


#endif
const char* tellenc(const unsigned char* const buffer, const size_t len) { UNUSED(buffer); UNUSED(len);  return NULL; }


const char* tellenc_simplify(const char* const buffer, const size_t len)
{
  const char* enc = tellenc((const unsigned char*)buffer, len);
    if (enc) {
        if (strcmp(enc, "windows-1252") == 0 && is_valid_latin1) {
            // Latin1 is subset of Windows-1252
            return "latin1";
        } else if (strcmp(enc, "gbk") == 0 && dbyte_hihi_cnt == dbyte_cnt) {
            // Special case for GB2312: no high-byte followed by a low-byte
            return "gb2312";
        }
    }
    return enc;
}
// ============================================================================



static bool bInitDone = false;

int GetBufferEncoding(const char* const buffer, const size_t len)
{
  if (!bInitDone) {
    init_utf8_char_table();
    bInitDone = true;
  }

  const char* enc = tellenc_simplify(buffer, len);

  if (enc)
    return 1;

  return 0; // unknown
}
// ============================================================================

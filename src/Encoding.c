/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* Encoding.c                                                                  *
*   Handling and Helpers for File Encoding                                    *
*   Based on code from Notepad2, (c) Florian Balmer 1996-2011                 *
*	                                                                            *
*	                                                                            *
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
#include <stdlib.h>

#include "../uthash/utarray.h"

#include "scintilla.h"
#include "helpers.h"
#include "encoding.h"


extern HINSTANCE g_hInstance;
extern BOOL bLoadASCIIasUTF8;

//=============================================================================



static NP2ENCODING g_Encodings[] = {
  /* 000 */{ NCP_ANSI | NCP_RECODE,                               CP_ACP, "ansi,system,ascii,",                                       61000, L"" },
  /* 001 */{ NCP_OEM | NCP_RECODE,                                CP_OEMCP, "oem,oem,",                                               61001, L"" },
  /* 002 */{ NCP_UNICODE | NCP_UNICODE_BOM,                       CP_UTF8, "",                                                        61002, L"" },
  /* 003 */{ NCP_UNICODE | NCP_UNICODE_REVERSE | NCP_UNICODE_BOM, CP_UTF8, "",                                                        61003, L"" },
  /* 004 */{ NCP_UNICODE | NCP_RECODE,                            CP_UTF8, "utf-16,utf16,unicode,",                                   61004, L"" },
  /* 005 */{ NCP_UNICODE | NCP_UNICODE_REVERSE | NCP_RECODE,      CP_UTF8, "utf-16be,utf16be,unicodebe,",                             61005, L"" },
  /* 006 */{ NCP_UTF8 | NCP_RECODE,                               CP_UTF8, "utf-8,utf8,",                                             61006, L"" },
  /* 007 */{ NCP_UTF8 | NCP_UTF8_SIGN,                            CP_UTF8, "utf-8,utf8,",                                             61007, L"" },
  /* 008 */{ NCP_EXTERNAL_8BIT | NCP_RECODE,                      CP_UTF7, "utf-7,utf7,",                                             61008, L"" },
  /* 009 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 720,   "DOS-720,dos720,",                                                                61009, L"" },
  /* 010 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 28596, "iso-8859-6,iso88596,arabic,csisolatinarabic,ecma114,isoir127,",                  61010, L"" },
  /* 011 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 10004, "x-mac-arabic,xmacarabic,",                                                       61011, L"" },
  /* 012 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 1256,  "windows-1256,windows1256,cp1256",                                                61012, L"" },
  /* 013 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 775,   "ibm775,ibm775,cp500,",                                                           61013, L"" },
  /* 014 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 28594, "iso-8859-4,iso88594,csisolatin4,isoir110,l4,latin4,",                            61014, L"" },
  /* 015 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 1257,  "windows-1257,windows1257,",                                                      61015, L"" },
  /* 016 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 852,   "ibm852,ibm852,cp852,",                                                           61016, L"" },
  /* 017 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 28592, "iso-8859-2,iso88592,csisolatin2,isoir101,latin2,l2,",                            61017, L"" },
  /* 018 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 10029, "x-mac-ce,xmacce,",                                                               61018, L"" },
  /* 019 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 1250,  "windows-1250,windows1250,xcp1250,",                                              61019, L"" },
  /* 020 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 936,   "gb2312,gb2312,chinese,cngb,csgb2312,csgb231280,gb231280,gbk,",                   61020, L"" },
  /* 021 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 10008, "x-mac-chinesesimp,xmacchinesesimp,",                                             61021, L"" },
  /* 022 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 950,   "big5,big5,cnbig5,csbig5,xxbig5,",                                                61022, L"" },
  /* 023 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 10002, "x-mac-chinesetrad,xmacchinesetrad,",                                             61023, L"" },
  /* 024 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 10082, "x-mac-croatian,xmaccroatian,",                                                   61024, L"" },
  /* 025 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 866,   "cp866,cp866,ibm866,",                                                            61025, L"" },
  /* 026 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 28595, "iso-8859-5,iso88595,csisolatin5,csisolatincyrillic,cyrillic,isoir144,",          61026, L"" },
  /* 027 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 20866, "koi8-r,koi8r,cskoi8r,koi,koi8,",                                                 61027, L"" },
  /* 028 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 21866, "koi8-u,koi8u,koi8ru,",                                                           61028, L"" },
  /* 029 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 10007, "x-mac-cyrillic,xmaccyrillic,",                                                   61029, L"" },
  /* 030 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 1251,  "windows-1251,windows1251,xcp1251,",                                              61030, L"" },
  /* 031 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 28603, "iso-8859-13,iso885913,",                                                         61031, L"" },
  /* 032 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 863,   "ibm863,ibm863,",                                                                 61032, L"" },
  /* 033 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 737,   "ibm737,ibm737,",                                                                 61033, L"" },
  /* 034 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 28597, "iso-8859-7,iso88597,csisolatingreek,ecma118,elot928,greek,greek8,isoir126,",     61034, L"" },
  /* 035 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 10006, "x-mac-greek,xmacgreek,",                                                         61035, L"" },
  /* 036 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 1253,  "windows-1253,windows1253,",                                                      61036, L"" },
  /* 037 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 869,   "ibm869,ibm869,",                                                                 61037, L"" },
  /* 038 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 862,   "DOS-862,dos862,",                                                                61038, L"" },
  /* 039 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 38598, "iso-8859-8-i,iso88598i,logical,",                                                61039, L"" },
  /* 040 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 28598, "iso-8859-8,iso88598,csisolatinhebrew,hebrew,isoir138,visual,",                   61040, L"" },
  /* 041 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 10005, "x-mac-hebrew,xmachebrew,",                                                       61041, L"" },
  /* 042 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 1255,  "windows-1255,windows1255,",                                                      61042, L"" },
  /* 043 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 861,   "ibm861,ibm861,",                                                                 61043, L"" },
  /* 044 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 10079, "x-mac-icelandic,xmacicelandic,",                                                 61044, L"" },
  /* 045 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 10001, "x-mac-japanese,xmacjapanese,",                                                   61045, L"" },
  /* 046 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 932,   "shift_jis,shiftjis,shiftjs,csshiftjis,cswindows31j,mskanji,xmscp932,xsjis,",     61046, L"" },
  /* 047 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 10003, "x-mac-korean,xmackorean,",                                                       61047, L"" },
  /* 048 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 949,   "windows-949,windows949,ksc56011987,csksc5601,euckr,isoir149,korean,ksc56011989", 61048, L"" },
  /* 049 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 28593, "iso-8859-3,iso88593,latin3,isoir109,l3,",                                        61049, L"" },
  /* 050 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 28605, "iso-8859-15,iso885915,latin9,l9,",                                               61050, L"" },
  /* 051 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 865,   "ibm865,ibm865,",                                                                 61051, L"" },
  /* 052 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 437,   "ibm437,ibm437,437,cp437,cspc8,codepage437,",                                     61052, L"" },
  /* 053 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 858,   "ibm858,ibm858,ibm00858,",                                                        61053, L"" },
  /* 054 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 860,   "ibm860,ibm860,",                                                                 61054, L"" },
  /* 055 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 10010, "x-mac-romanian,xmacromanian,",                                                   61055, L"" },
  /* 056 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 10021, "x-mac-thai,xmacthai,",                                                           61056, L"" },
  /* 057 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 874,   "windows-874,windows874,dos874,iso885911,tis620,",                                61057, L"" },
  /* 058 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 857,   "ibm857,ibm857,",                                                                 61058, L"" },
  /* 059 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 28599, "iso-8859-9,iso88599,latin5,isoir148,l5,",                                        61059, L"" },
  /* 060 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 10081, "x-mac-turkish,xmacturkish,",                                                     61060, L"" },
  /* 061 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 1254,  "windows-1254,windows1254,",                                                      61061, L"" },
  /* 062 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 10017, "x-mac-ukrainian,xmacukrainian,",                                                 61062, L"" },
  /* 063 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 1258,  "windows-1258,windows-258,",                                                      61063, L"" },
  /* 064 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 850,   "ibm850,ibm850,",                                                                 61064, L"" },
  /* 065 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 28591, "iso-8859-1,iso88591,cp819,latin1,ibm819,isoir100,latin1,l1,",                    61065, L"" },
  /* 066 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 10000, "macintosh,macintosh,",                                                           61066, L"" },
  /* 067 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 1252,  "windows-1252,windows1252,cp367,cp819,ibm367,us,xansi,",                          61067, L"" },
  /* 068 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 37,    "ebcdic-cp-us,ebcdiccpus,ebcdiccpca,ebcdiccpwt,ebcdiccpnl,ibm037,cp037,",         61068, L"" },
  /* 069 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 500,   "x-ebcdic-international,xebcdicinternational,",                                   61069, L"" },
  /* 070 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 875,   "x-EBCDIC-GreekModern,xebcdicgreekmodern,",                                       61070, L"" },
  /* 071 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 1026,  "CP1026,cp1026,csibm1026,ibm1026,",                                               61071, L"" },
  /* 072 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 54936, "gb18030,gb18030,",                                                               61072, L"" }, // Chinese Simplified (GB18030)
  /* 073 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 51932, "euc-jp,eucjp,xeuc,xeucjp,",                                                      61073, L"" }, // Japanese (EUC)
  /* 074 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 51949, "euc-kr,euckr,cseuckr,",                                                          61074, L"" },  // Korean (EUC)
  /* 075 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 50229, "iso-2022-cn,iso2022cn,",                                                         61075, L"" }, // Chinese Traditional (ISO-2022-CN)
  /* 076 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 52936, "hz-gb-2312,hzgb2312,hz,",                                                        61076, L"" }, // Chinese Simplified (HZ-GB2312)
  /* 077 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 50220, "iso-2022-jp,iso2022jp,",                                                         61077, L"" }, // Japanese (JIS)
  /* 078 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 50225, "iso-2022-kr,iso2022kr,csiso2022kr,",                                             61078, L"" }, // Korean (ISO-2022-KR)
  /* 079 */{ NCP_EXTERNAL_8BIT | NCP_RECODE, 20000, "x-Chinese-CNS,xchinesecns,",                                                     61079, L"" } // Chinese Traditional (CNS)

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
  /* 085 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 1361,  "johab,johab,",                                                                   00000, L"" }, // Korean (Johab)
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
  /* 118 *///{ NCP_EXTERNAL_8BIT|NCP_RECODE, 50222, "_iso-2022-jp$SIO,iso2022jpSIO,",                                                 00000, L"" }, // Japanese (JIS-Allow 1 byte Kana - SO/SI)
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

int Encoding_CountOf()
{
  return COUNTOF(g_Encodings);
}
//=============================================================================


//=============================================================================
//
//  Encoding Helper Functions
//

int g_DOSEncoding;

// Supported Encodings
WCHAR wchANSI[16] = { L'\0' };
WCHAR wchOEM[16] = { L'\0' };

// ============================================================================


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
// ============================================================================


int Encoding_SrcCmdLn(int iSrcEncoding) {
  static int SourceEncoding = CPI_NONE;

  if (iSrcEncoding >= 0) {
    if (Encoding_IsValid(iSrcEncoding))
      SourceEncoding = iSrcEncoding;
    else
      SourceEncoding = CPI_ANSI_DEFAULT;
  }
  else if (iSrcEncoding == CPI_NONE) {
    SourceEncoding = CPI_NONE;
  }
  return SourceEncoding;
}
// ============================================================================


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
// ============================================================================


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
  return (iEncoding >= 0) ? g_Encodings[iEncoding].uCodePage : CP_ACP;
}
// ============================================================================

BOOL Encoding_IsDefault(int iEncoding) {
  return  (iEncoding >= 0) ? (g_Encodings[iEncoding].uFlags & NCP_DEFAULT) : FALSE;
}
// ============================================================================

BOOL Encoding_IsANSI(int iEncoding) {
  return  (iEncoding >= 0) ? (g_Encodings[iEncoding].uFlags & NCP_ANSI) : FALSE;
}
// ============================================================================

BOOL Encoding_IsOEM(int iEncoding) {
  return  (iEncoding >= 0) ? (g_Encodings[iEncoding].uFlags & NCP_OEM) : FALSE;
}
// ============================================================================

BOOL Encoding_IsUTF8(int iEncoding) {
  return  (iEncoding >= 0) ? (g_Encodings[iEncoding].uFlags & NCP_UTF8) : FALSE;
}
// ============================================================================

BOOL Encoding_IsUTF8_SIGN(int iEncoding) {
  return  (iEncoding >= 0) ? (g_Encodings[iEncoding].uFlags & NCP_UTF8_SIGN) : FALSE;
}
// ============================================================================

BOOL Encoding_IsMBCS(int iEncoding) {
  return  (iEncoding >= 0) ? (g_Encodings[iEncoding].uFlags & NCP_MBCS) : FALSE;
}
// ============================================================================

BOOL Encoding_IsUNICODE(int iEncoding) {
  return  (iEncoding >= 0) ? (g_Encodings[iEncoding].uFlags & NCP_UNICODE) : FALSE;
}
// ============================================================================

BOOL Encoding_IsUNICODE_BOM(int iEncoding) {
  return  (iEncoding >= 0) ? (g_Encodings[iEncoding].uFlags & NCP_UNICODE_BOM) : FALSE;
}
// ============================================================================

BOOL Encoding_IsUNICODE_REVERSE(int iEncoding) {
  return  (iEncoding >= 0) ? (g_Encodings[iEncoding].uFlags & NCP_UNICODE_REVERSE) : FALSE;
}
// ============================================================================


BOOL Encoding_IsINTERNAL(int iEncoding) {
  return  (iEncoding >= 0) ? (g_Encodings[iEncoding].uFlags & NCP_INTERNAL) : FALSE;
}
// ============================================================================

BOOL Encoding_IsEXTERNAL_8BIT(int iEncoding) {
  return  (iEncoding >= 0) ? (g_Encodings[iEncoding].uFlags & NCP_EXTERNAL_8BIT) : FALSE;
}
// ============================================================================

BOOL Encoding_IsRECODE(int iEncoding) {
  return  (iEncoding >= 0) ? (g_Encodings[iEncoding].uFlags & NCP_RECODE) : FALSE;
}
// ============================================================================


void Encoding_SetDefaultFlag(int iEncoding) {
  if (iEncoding >= 0)
    g_Encodings[iEncoding].uFlags |= NCP_DEFAULT;
}
// ============================================================================


const WCHAR* Encoding_GetLabel(int iEncoding) {
  return (iEncoding >= 0) ? g_Encodings[iEncoding].wchLabel : NULL;
}
// ============================================================================

const char* Encoding_GetParseNames(int iEncoding) {
  return (iEncoding >= 0) ? g_Encodings[iEncoding].pszParseNames : NULL;
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


BOOL IsUnicode(const char* pBuffer, int cb, LPBOOL lpbBOM, LPBOOL lpbReverse) {
  int i = 0xFFFF;

  BOOL bIsTextUnicode;

  BOOL bHasBOM;
  BOOL bHasRBOM;

  if (!pBuffer || cb < 2)
    return FALSE;

  bIsTextUnicode = IsTextUnicode(pBuffer, cb, &i);

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

  return (current == kSTART) ? true : false;
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
static const size_t utf8_lengths[16] =
{
  1,1,1,1,1,1,1,1,        /* 0000 to 0111 : 1 byte (plain ASCII) */
  0,0,0,0,                /* 1000 to 1011 : not valid */
  2,2,                    /* 1100, 1101 : 2 bytes */
  3,                      /* 1110 : 3 bytes */
  4                       /* 1111 : 4 bytes */
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
size_t UTF8_mbslen_bytes(LPCSTR utf8_string)
{
  size_t length = 0;
  size_t code_size;
  BYTE byte;

  while (*utf8_string) 
  {
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
size_t UTF8_mbslen(LPCSTR utf8_string, size_t byte_length)
{
  size_t wchar_length = 0;
  size_t code_size;
  BYTE byte;

  while (byte_length > 0) {
    byte = (BYTE)*utf8_string;

    /* UTF-16 can't encode 5-byte and 6-byte sequences, so maximum value
    for first byte is 11110111. Use lookup table to determine sequence
    length based on upper 4 bits of first byte */
    if ((byte <= 0xF7) && (0 != (code_size = utf8_lengths[byte >> 4]))) {
      /* 1 sequence == 1 character */
      wchar_length++;

      if (code_size == 4)
        wchar_length++;

      utf8_string += code_size;        /* increment pointer */
      byte_length -= code_size;   /* decrement counter*/
    }
    else {
      /*
      unlike UTF8_mbslen_bytes, we ignore the invalid characters.
      we only report the number of valid characters we have encountered
      to match the Windows behavior.
      */
      //WARN("invalid byte 0x%02X in UTF-8 sequence, skipping it!\n", byte);
      utf8_string++;
      byte_length--;
    }
  }
  return wchar_length;
}
// ============================================================================



bool UTF8_ContainsInvalidChars(LPCSTR utf8_string, size_t byte_length)
{
  return ((UTF8_mbslen_bytes(UTF8StringStart(utf8_string)) - 1) != 
           UTF8_mbslen(UTF8StringStart(utf8_string), IsUTF8Signature(utf8_string) ? (byte_length - 3) : byte_length));
}
// ============================================================================


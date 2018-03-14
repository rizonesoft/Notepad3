/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* EncTables.hpp                                                               *
*   Encoding detector tables                                     *
*   Based on code from Notepad2, (c) Florian Balmer 1996-2011                 *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2017   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/

#pragma once
#ifndef _NP3_ENCTABLES_H_
#define _NP3_ENCTABLES_H_

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


/******************************************************************************
*                                                                             *
*                                                                             *
* tellenc                                                                     *
*                                                                             *
*                                                                             *
* Copyright (C) 2006-2016 Wu Yongwei <wuyongwei@gmail.com>                    *
*                                                                             *
* This software is provided 'as-is', without any express or implied           *
* warranty.  In no event will the authors be held liable for any              *
* damages arising from the use of this software.                              *
*                                                                             *
* Permission is granted to anyone to use this software for any purpose,       *
* including commercial applications, and to alter it and redistribute         *
* it freely, subject to the following restrictions:                           *
*                                                                             *
* 1. The origin of this software must not be misrepresented; you must         *
*    not claim that you wrote the original software.  If you use this         *
*    software in a product, an acknowledgement in the product                 *
*    documentation would be appreciated but is not required.                  *
* 2. Altered source versions must be plainly marked as such, and must         *
*    not be misrepresented as being the original software.                    *
* 3. This notice may not be removed or altered from any source                *
*    distribution.                                                            *
*                                                                             *
*                                                                             *
* The latest version of this software should be available at:                 *
*      <URL:https://github.com/adah1972/tellenc>                              *
*                                                                             *
*                                                                             *
*******************************************************************************/

typedef struct {
  int         encID;     // index of encoding related to table "g_Encodings" (above)
  uint16_t    dbyte;     // encoding typically double byte sequence 
  const char* encoding;  // for human readability only
} freq_analysis_data_t;


static freq_analysis_data_t freq_analysis_data[] = 
{
{ 19, 0x9a74, "windows-1250" },         // "št" (Czech)
{ 19, 0xe865, "windows-1250" },         // "če" (Czech)
{ 19, 0xf865, "windows-1250" },         // "ře" (Czech)
{ 19, 0xe167, "windows-1250" },         // "ág" (Hungarian)
{ 19, 0xe96c, "windows-1250" },         // "él" (Hungarian)
{ 19, 0xb36f, "windows-1250" },         // "ło" (Polish)
{ 19, 0xea7a, "windows-1250" },         // "ęz" (Polish)
{ 19, 0xf377, "windows-1250" },         // "ów" (Polish)
{ 19, 0x9d20, "windows-1250" },         // "ť " (Slovak)
{ 19, 0xfa9d, "windows-1250" },         // "úť" (Slovak)
{ 19, 0x9e69, "windows-1250" },         // "ži" (Slovenian)
{ 19, 0xe869, "windows-1250" },         // "či" (Slovenian)
  
{ 30, 0xe820, "windows-1251" },         // "и " (Cyrillic)
{ 30, 0xe3ee, "windows-1251" },         // "го" (Cyrillic)
{ 30, 0xeaee, "windows-1251" },         // "ко" (Cyrillic)
{ 30, 0xf1ea, "windows-1251" },         // "ск" (Cyrillic)
{ 30, 0xf1f2, "windows-1251" },         // "ст" (Cyrillic)

{ 67, 0xe020, "windows-1252" },         // "à " (French)
{ 67, 0xe920, "windows-1252" },         // "é " (French)
{ 67, 0xe963, "windows-1252" },         // "éc" (French)
{ 67, 0xe965, "windows-1252" },         // "ée" (French)
{ 67, 0xe972, "windows-1252" },         // "ér" (French)
{ 67, 0xe4e4, "windows-1252" },         // "ää" (Finnish)
{ 67, 0xe474, "windows-1252" },         // "ät" (German)
{ 67, 0xfc72, "windows-1252" },         // "ür" (German)
{ 67, 0xed6e, "windows-1252" },         // "ín" (Spanish)
{ 67, 0xf36e, "windows-1252" },         // "ón" (Spanish)
{ 52, 0x8220, "cp437" },                // "é " (French)
{ 52, 0x8263, "cp437" },                // "éc" (French)
{ 52, 0x8265, "cp437" },                // "ée" (French)
{ 52, 0x8272, "cp437" },                // "ér" (French)
{ 52, 0x8520, "cp437" },                // "à " (French)
{ 52, 0x8172, "cp437" },                // "ür" (German)
{ 52, 0x8474, "cp437" },                // "ät" (German)
{ 52, 0xc4c4, "cp437" },                // "──"
{ 52, 0xcdcd, "cp437" },                // "══"
{ 52, 0xdbdb, "cp437" },                // "██"
{ 20, 0xa1a1, "gbk" },                  // "　"
{ 20, 0xa1a2, "gbk" },                  // "、"
{ 20, 0xa1a3, "gbk" },                  // "。"
{ 20, 0xa1a4, "gbk" },                  // "·"
{ 20, 0xa1b6, "gbk" },                  // "《"
{ 20, 0xa1b7, "gbk" },                  // "》"
{ 20, 0xa3ac, "gbk" },                  // "，"
{ 20, 0xa3ba, "gbk" },                  // "："
{ 20, 0xb5c4, "gbk" },                  // "的"
{ 20, 0xc1cb, "gbk" },                  // "了"
{ 20, 0xd2bb, "gbk" },                  // "一"
{ 20, 0xcac7, "gbk" },                  // "是"
{ 20, 0xb2bb, "gbk" },                  // "不"
{ 20, 0xb8f6, "gbk" },                  // "个"
{ 20, 0xc8cb, "gbk" },                  // "人"
{ 20, 0xd5e2, "gbk" },                  // "这"
{ 20, 0xd3d0, "gbk" },                  // "有"
{ 20, 0xced2, "gbk" },                  // "我"
{ 20, 0xc4e3, "gbk" },                  // "你"
{ 20, 0xcbfb, "gbk" },                  // "他"
{ 20, 0xcbfd, "gbk" },                  // "她"
{ 20, 0xc9cf, "gbk" },                  // "上"
{ 20, 0xbfb4, "gbk" },                  // "看"
{ 20, 0xd6ae, "gbk" },                  // "之"
{ 20, 0xbbb9, "gbk" },                  // "还"
{ 20, 0xbfc9, "gbk" },                  // "可"
{ 20, 0xbaf3, "gbk" },                  // "后"
{ 20, 0xd6d0, "gbk" },                  // "中"
{ 20, 0xd0d0, "gbk" },                  // "行"
{ 20, 0xb1d2, "gbk" },                  // "币"
{ 20, 0xb3f6, "gbk" },                  // "出"
{ 20, 0xb7d1, "gbk" },                  // "费"
{ 20, 0xb8d0, "gbk" },                  // "感"
{ 20, 0xbef5, "gbk" },                  // "觉"
{ 20, 0xc4ea, "gbk" },                  // "年"
{ 20, 0xd4c2, "gbk" },                  // "月"
{ 20, 0xc8d5, "gbk" },                  // "日"
{ 22, 0xa140, "big5" },                 // "　"
{ 22, 0xa141, "big5" },                 // "，"
{ 22, 0xa143, "big5" },                 // "。"
{ 22, 0xa147, "big5" },                 // "："
{ 22, 0xaaba, "big5" },                 // "的"
{ 22, 0xa446, "big5" },                 // "了"
{ 22, 0xa440, "big5" },                 // "一"
{ 22, 0xac4f, "big5" },                 // "是"
{ 22, 0xa4a3, "big5" },                 // "不"
{ 22, 0xa448, "big5" },                 // "人"
{ 22, 0xa7da, "big5" },                 // "我"
{ 22, 0xa741, "big5" },                 // "你"
{ 22, 0xa54c, "big5" },                 // "他"
{ 22, 0xa66f, "big5" },                 // "她"
{ 22, 0xadd3, "big5" },                 // "個"
{ 22, 0xa457, "big5" },                 // "上"
{ 22, 0xa662, "big5" },                 // "在"
{ 22, 0xbba1, "big5" },                 // "說"
{ 22, 0xa65e, "big5" },                 // "回"
{ 46, 0x8140, "sjis" },                 // "　"
{ 46, 0x8141, "sjis" },                 // "、"
{ 46, 0x8142, "sjis" },                 // "。"
{ 46, 0x8145, "sjis" },                 // "・"
{ 46, 0x8146, "sjis" },                 // "："
{ 46, 0x815b, "sjis" },                 // "ー"
{ 46, 0x82b5, "sjis" },                 // "し"
{ 46, 0x82bd, "sjis" },                 // "た"
{ 46, 0x82c8, "sjis" },                 // "な"
{ 46, 0x82c9, "sjis" },                 // "に"
{ 46, 0x82cc, "sjis" },                 // "の"
{ 46, 0x82dc, "sjis" },                 // "ま"
{ 46, 0x82f0, "sjis" },                 // "を"
{ 46, 0x8367, "sjis" },                 // "ト"
{ 46, 0x8393, "sjis" },                 // "ン"
{ 46, 0x89ef, "sjis" },                 // "会"
{ 46, 0x906c, "sjis" },                 // "人"
{ 46, 0x9094, "sjis" },                 // "数"
{ 46, 0x93fa, "sjis" },                 // "日"
{ 46, 0x95f1, "sjis" },                 // "報"
{ 73, 0xa1bc, "euc-jp" },               // "ー"
{ 73, 0xa4bf, "euc-jp" },               // "た"
{ 73, 0xa4ca, "euc-jp" },               // "な"
{ 73, 0xa4cb, "euc-jp" },               // "に"
{ 73, 0xa4ce, "euc-jp" },               // "の"
{ 73, 0xa4de, "euc-jp" },               // "ま"
{ 73, 0xa4f2, "euc-jp" },               // "を"
{ 73, 0xa5c8, "euc-jp" },               // "ト"
{ 73, 0xa5f3, "euc-jp" },               // "ン"
{ 73, 0xb2f1, "euc-jp" },               // "会"
{ 73, 0xbfcd, "euc-jp" },               // "人"
{ 73, 0xbff4, "euc-jp" },               // "数"
{ 73, 0xc6fc, "euc-jp" },               // "日"
{ 73, 0xcaf3, "euc-jp" },               // "報"
{ 74, 0xc0cc, "euc-kr" },               // "이"
{ 74, 0xb0fa, "euc-kr" },               // "과"
{ 74, 0xb1e2, "euc-kr" },               // "기"
{ 74, 0xb4c2, "euc-kr" },               // "는"
{ 74, 0xb7ce, "euc-kr" },               // "로"
{ 74, 0xb1db, "euc-kr" },               // "글"
{ 74, 0xc5e4, "euc-kr" },               // "토"
{ 74, 0xc1a4, "euc-kr" },               // "정"
{ 27, 0xc920, "koi8-r" },               // "и "
{ 27, 0xc7cf, "koi8-r" },               // "го"
{ 27, 0xcbcf, "koi8-r" },               // "ко"
{ 27, 0xd3cb, "koi8-r" },               // "ск"
{ 27, 0xd3d4, "koi8-r" },               // "ст"
{ 28, 0xa6a7, "koi8-u" },               // "ії"
{ 28, 0xa6ce, "koi8-u" },               // "ін"
{ 28, 0xa6d7, "koi8-u" },               // "ів"
{ 28, 0xa7ce, "koi8-u" },               // "їн"
{ 28, 0xd0cf, "koi8-u" },               // "по"
{ 28, 0xd4c9, "koi8-u" },               // "ти"
};
// ============================================================================

#endif //_NP3_ENCTABLES_H_

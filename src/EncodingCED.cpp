/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* EncodingCED.cpp                                                             *
*   Interface to Google's Compact Encoding Detector                           *
*                                                                             *
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


extern "C" {
#include "Encoding.h"
}

#include "compact_enc_det/compact_enc_det.h"

// Global settings...
//extern "C" g_Encodings;

// ============================================================================

int __fastcall MapEncoding2CPI(const Encoding& encoding)
{
  int iNP3Encoding = CPI_NONE;

  // map corresponding ID of global 'g_Encodings'

  switch (encoding) {
  case ISO_8859_1:
    iNP3Encoding = 65;
    break;
  case ISO_8859_2:
    iNP3Encoding = 17;
    break;
  case ISO_8859_3:
    iNP3Encoding = 49;
    break;
  case ISO_8859_4:
    iNP3Encoding = 14;
    break;
  case ISO_8859_5:
    iNP3Encoding = 26;
    break;
  case ISO_8859_6:
    iNP3Encoding = 10;
    break;
  case ISO_8859_7:
    iNP3Encoding = 34;
    break;
  case ISO_8859_8:
    iNP3Encoding = 40;
    break;
  case ISO_8859_9:
    iNP3Encoding = 59;
    break;
  case ISO_8859_10:
    iNP3Encoding = CPI_NONE; // ???
    break;
  case JAPANESE_EUC_JP:
    iNP3Encoding = 73;
    break;
  case JAPANESE_SHIFT_JIS:
    iNP3Encoding = 46;
    break;
  case JAPANESE_JIS:
    iNP3Encoding = 77; // ???
    break;
  case CHINESE_BIG5:
    iNP3Encoding = 22;
    break;
  case CHINESE_GB:
    iNP3Encoding = 20;
    break;
  case CHINESE_EUC_CN:
    iNP3Encoding = CPI_NONE;
    break;
  case KOREAN_EUC_KR:
    iNP3Encoding = 74;
    break;
  case UNICODE:
    iNP3Encoding = 4;
    break;
  case CHINESE_EUC_DEC:
    iNP3Encoding = CPI_NONE;
    break;
  case CHINESE_CNS:
    iNP3Encoding = 79;
    break;
  case CHINESE_BIG5_CP950:
    iNP3Encoding = 22;
    break;
  case JAPANESE_CP932:
    iNP3Encoding = 46;
    break;
  case UTF8:
    iNP3Encoding = CPI_UTF8;
    break;
  case ASCII_7BIT:
    iNP3Encoding = CPI_ANSI_DEFAULT;
    break;
  case RUSSIAN_KOI8_R:
    iNP3Encoding = 27;
    break;
  case RUSSIAN_CP1251:
    iNP3Encoding = 30;
    break;
  case MSFT_CP1252:
    iNP3Encoding = 67;
    break;
  case RUSSIAN_KOI8_RU:
    iNP3Encoding = 28;
    break;
  case MSFT_CP1250:
    iNP3Encoding = 19;
    break;
  case ISO_8859_15:
    iNP3Encoding = 50;
    break;
  case MSFT_CP1254:
    iNP3Encoding = 61;
    break;
  case MSFT_CP1257:
    iNP3Encoding = 15;
    break;
  case ISO_8859_11:
    iNP3Encoding = CPI_NONE;
    break;
  case MSFT_CP874:
    iNP3Encoding = 57;
    break;
  case MSFT_CP1256:
    iNP3Encoding = 12;
    break;
  case MSFT_CP1255:
    iNP3Encoding = 42;
    break;
  case ISO_8859_8_I:
    iNP3Encoding = 39;
    break;
  case HEBREW_VISUAL:
    iNP3Encoding = 40;
    break;
  case CZECH_CP852:
    iNP3Encoding = 16;
    break;
  case CZECH_CSN_369103:
    iNP3Encoding = CPI_NONE; // ???
    break;
  case MSFT_CP1253:
    iNP3Encoding = 36;
    break;
  case RUSSIAN_CP866:
    iNP3Encoding = 25;
    break;
  case ISO_8859_13:
    iNP3Encoding = 31;
    break;
  case ISO_2022_KR:
    iNP3Encoding = 78;
    break;
  case GBK:
    iNP3Encoding = 20;
    break;
  case GB18030:
    iNP3Encoding = 72;
    break;
  case BIG5_HKSCS:
    iNP3Encoding = 22;
    break;
  case ISO_2022_CN:
    iNP3Encoding = 75;
    break;
  case TSCII:
    iNP3Encoding = CPI_NONE; // ???
    break;
  case TAMIL_MONO:
    iNP3Encoding = CPI_NONE; // ???
    break;
  case TAMIL_BI:
    iNP3Encoding = CPI_NONE; // ???
    break;
  case JAGRAN:
    iNP3Encoding = CPI_NONE; // ???
    break;
  case MACINTOSH_ROMAN:
    iNP3Encoding = 55;
    break;
  case UTF7:
    iNP3Encoding = CPI_UTF7;
    break;
  case BHASKAR:
    iNP3Encoding = CPI_NONE; // ???
    break;
  case HTCHANAKYA:
    iNP3Encoding = CPI_NONE; // ???
    break;
  case UTF16BE:
    iNP3Encoding = CPI_UNICODEBE;
    break;
  case UTF16LE:
    iNP3Encoding = CPI_UNICODE;
    break;
  case UTF32BE:
    iNP3Encoding = CPI_UTF32BE;
    break;
  case UTF32LE:
    iNP3Encoding = CPI_UTF32;
    break;
  case BINARYENC:
    iNP3Encoding = CPI_NONE; // ???
    break;
  case HZ_GB_2312:
    iNP3Encoding = 76;
    break;
  case UTF8UTF8:
    iNP3Encoding = CPI_UTF8;
    break;
  case TAM_ELANGO:
    iNP3Encoding = CPI_NONE; // ???
    break;
  case TAM_LTTMBARANI:
    iNP3Encoding = CPI_NONE; // ???
    break;
  case TAM_SHREE:
    iNP3Encoding = CPI_NONE; // ???
    break;
  case TAM_TBOOMIS:
    iNP3Encoding = CPI_NONE; // ???
    break;
  case TAM_TMNEWS:
    iNP3Encoding = CPI_NONE; // ???
    break;
  case TAM_WEBTAMIL:
    iNP3Encoding = CPI_NONE; // ???
    break;
  case KDDI_SHIFT_JIS:
    iNP3Encoding = 46; // ???
    break;
  case DOCOMO_SHIFT_JIS:
    iNP3Encoding = 46; // ???
    break;
  case SOFTBANK_SHIFT_JIS:
    iNP3Encoding = 46; // ???
    break;
  case KDDI_ISO_2022_JP:
    iNP3Encoding = 77;
    break;
  case SOFTBANK_ISO_2022_JP:
    iNP3Encoding = 77; // ???
    break;

  case UNKNOWN_ENCODING:
  default:
    iNP3Encoding = CPI_NONE;
    break;
  }
  return iNP3Encoding;
}
// ============================================================================


extern "C" int Encoding_Analyze(const char* const text, const size_t len, bool* pIsReliable)
{
  int bytes_consumed;

  Encoding encoding = CompactEncDet::DetectEncoding(
    text, static_cast<int>(len),
    nullptr, nullptr, nullptr,
    UNKNOWN_ENCODING,
    UNKNOWN_LANGUAGE,
    CompactEncDet::WEB_CORPUS,
    false,
    &bytes_consumed,
    pIsReliable);

  return MapEncoding2CPI(encoding);
}
// ============================================================================

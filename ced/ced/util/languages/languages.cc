// Copyright 2016 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
////////////////////////////////////////////////////////////////////////////////

#include "util/languages/languages.h"

#include "util/basictypes.h"
#include "util/string_util.h"


Language default_language() {return ENGLISH;}


// Language names and codes

struct LanguageInfo {
  const char * language_name_;
  const char * language_code_639_1_;   // the ISO-639-1 code for the language
  const char * language_code_639_2_;   // the ISO-639-2 code for the language
  const char * language_code_other_;   // some nonstandard code for the language
};

static const LanguageInfo kLanguageInfoTable[] = {
  { "ENGLISH",             "en", "eng", nullptr},
  { "DANISH",              "da", "dan", nullptr},
  { "DUTCH",               "nl", "dut", nullptr},
  { "FINNISH",             "fi", "fin", nullptr},
  { "FRENCH",              "fr", "fre", nullptr},
  { "GERMAN",              "de", "ger", nullptr},
  { "HEBREW",              "he", "heb", nullptr},
  { "ITALIAN",             "it", "ita", nullptr},
  { "Japanese",            "ja", "jpn", nullptr},
  { "Korean",              "ko", "kor", nullptr},
  { "NORWEGIAN",           "nb", "nor", nullptr},
  { "POLISH",              "pl", "pol", nullptr},
  { "PORTUGUESE",          "pt", "por", nullptr},
  { "RUSSIAN",             "ru", "rus", nullptr},
  { "SPANISH",             "es", "spa", nullptr},
  { "SWEDISH",             "sv", "swe", nullptr},
  { "Chinese",             "zh", "chi", "zh-CN"},
  { "CZECH",               "cs", "cze", nullptr},
  { "GREEK",               "el", "gre", nullptr},
  { "ICELANDIC",           "is", "ice", nullptr},
  { "LATVIAN",             "lv", "lav", nullptr},
  { "LITHUANIAN",          "lt", "lit", nullptr},
  { "ROMANIAN",            "ro", "rum", nullptr},
  { "HUNGARIAN",           "hu", "hun", nullptr},
  { "ESTONIAN",            "et", "est", nullptr},
  // TODO: Although Teragram has two output names "TG_UNKNOWN_LANGUAGE"
  // and "Unknown", they are essentially the same. Need to unify them.
  // "un" and "ut" are invented by us, not from ISO-639.
  //
  { "TG_UNKNOWN_LANGUAGE", nullptr, nullptr, "ut"},
  { "Unknown",             nullptr, nullptr, "un"},
  { "BULGARIAN",           "bg", "bul", nullptr},
  { "CROATIAN",            "hr", "scr", nullptr},
  { "SERBIAN",             "sr", "scc", nullptr},
  { "IRISH",               "ga", "gle", nullptr},
  { "GALICIAN",            "gl", "glg", nullptr},
  // Impossible to tell Tagalog from Filipino at the moment.
  // Use ISO 639-2 code for Filipino here.
  { "TAGALOG",             nullptr, "fil", nullptr},
  { "TURKISH",             "tr", "tur", nullptr},
  { "UKRAINIAN",           "uk", "ukr", nullptr},
  { "HINDI",               "hi", "hin", nullptr},
  { "MACEDONIAN",          "mk", "mac", nullptr},
  { "BENGALI",             "bn", "ben", nullptr},
  { "INDONESIAN",          "id", "ind", nullptr},
  { "LATIN",               "la", "lat", nullptr},
  { "MALAY",               "ms", "may", nullptr},
  { "MALAYALAM",           "ml", "mal", nullptr},
  { "WELSH",               "cy", "wel", nullptr},
  { "NEPALI",              "ne", "nep", nullptr},
  { "TELUGU",              "te", "tel", nullptr},
  { "ALBANIAN",            "sq", "alb", nullptr},
  { "TAMIL",               "ta", "tam", nullptr},
  { "BELARUSIAN",          "be", "bel", nullptr},
  { "JAVANESE",            "jw", "jav", nullptr},
  { "OCCITAN",             "oc", "oci", nullptr},
  { "URDU",                "ur", "urd", nullptr},
  { "BIHARI",              "bh", "bih", nullptr},
  { "GUJARATI",            "gu", "guj", nullptr},
  { "THAI",                "th", "tha", nullptr},
  { "ARABIC",              "ar", "ara", nullptr},
  { "CATALAN",             "ca", "cat", nullptr},
  { "ESPERANTO",           "eo", "epo", nullptr},
  { "BASQUE",              "eu", "baq", nullptr},
  { "INTERLINGUA",         "ia", "ina", nullptr},
  { "KANNADA",             "kn", "kan", nullptr},
  { "PUNJABI",             "pa", "pan", nullptr},
  { "SCOTS_GAELIC",        "gd", "gla", nullptr},
  { "SWAHILI",             "sw", "swa", nullptr},
  { "SLOVENIAN",           "sl", "slv", nullptr},
  { "MARATHI",             "mr", "mar", nullptr},
  { "MALTESE",             "mt", "mlt", nullptr},
  { "VIETNAMESE",          "vi", "vie", nullptr},
  { "FRISIAN",             "fy", "fry", nullptr},
  { "SLOVAK",              "sk", "slo", nullptr},
  { "ChineseT",
    nullptr,  nullptr,  // We intentionally set these 2 fields to NULL to avoid
                  // confusion between CHINESE_T and CHINESE.
    "zh-TW"},
  { "FAROESE",             "fo", "fao", nullptr},
  { "SUNDANESE",           "su", "sun", nullptr},
  { "UZBEK",               "uz", "uzb", nullptr},
  { "AMHARIC",             "am", "amh", nullptr},
  { "AZERBAIJANI",         "az", "aze", nullptr},
  { "GEORGIAN",            "ka", "geo", nullptr},
  { "TIGRINYA",            "ti", "tir", nullptr},
  { "PERSIAN",             "fa", "per", nullptr},
  { "BOSNIAN",             "bs", "bos", nullptr},
  { "SINHALESE",           "si", "sin", nullptr},
  { "NORWEGIAN_N",         "nn", "nno", nullptr},
  { "PORTUGUESE_P",        nullptr, nullptr, "pt-PT"},
  { "PORTUGUESE_B",        nullptr, nullptr, "pt-BR"},
  { "XHOSA",               "xh", "xho", nullptr},
  { "ZULU",                "zu", "zul", nullptr},
  { "GUARANI",             "gn", "grn", nullptr},
  { "SESOTHO",             "st", "sot", nullptr},
  { "TURKMEN",             "tk", "tuk", nullptr},
  { "KYRGYZ",              "ky", "kir", nullptr},
  { "BRETON",              "br", "bre", nullptr},
  { "TWI",                 "tw", "twi", nullptr},
  { "YIDDISH",             "yi", "yid", nullptr},
  { "SERBO_CROATIAN",      "sh", nullptr, nullptr},
  { "SOMALI",              "so", "som", nullptr},
  { "UIGHUR",              "ug", "uig", nullptr},
  { "KURDISH",             "ku", "kur", nullptr},
  { "MONGOLIAN",           "mn", "mon", nullptr},
  { "ARMENIAN",            "hy", "arm", nullptr},
  { "LAOTHIAN",            "lo", "lao", nullptr},
  { "SINDHI",              "sd", "snd", nullptr},
  { "RHAETO_ROMANCE",      "rm", "roh", nullptr},
  { "AFRIKAANS",           "af", "afr", nullptr},
  { "LUXEMBOURGISH",       "lb", "ltz", nullptr},
  { "BURMESE",             "my", "bur", nullptr},
  // KHMER is known as Cambodian for Google user interfaces.
  { "KHMER",               "km", "khm", nullptr},
  { "TIBETAN",             "bo", "tib", nullptr},
  { "DHIVEHI",             "dv", "div", nullptr},
  { "CHEROKEE",            nullptr, "chr", nullptr},
  { "SYRIAC",              nullptr, "syr", nullptr},
  { "LIMBU",               nullptr, nullptr, "sit-NP"},
  { "ORIYA",               "or", "ori", nullptr},
  { "ASSAMESE",            "as", "asm", nullptr},
  { "CORSICAN",            "co", "cos", nullptr},
  { "INTERLINGUE",         "ie", "ine", nullptr},
  { "KAZAKH",              "kk", "kaz", nullptr},
  { "LINGALA",             "ln", "lin", nullptr},
  { "MOLDAVIAN",           "mo", "mol", nullptr},
  { "PASHTO",              "ps", "pus", nullptr},
  { "QUECHUA",             "qu", "que", nullptr},
  { "SHONA",               "sn", "sna", nullptr},
  { "TAJIK",               "tg", "tgk", nullptr},
  { "TATAR",               "tt", "tat", nullptr},
  { "TONGA",               "to", "tog", nullptr},
  { "YORUBA",              "yo", "yor", nullptr},
  { "CREOLES_AND_PIDGINS_ENGLISH_BASED", nullptr, "cpe", nullptr},
  { "CREOLES_AND_PIDGINS_FRENCH_BASED",  nullptr, "cpf", nullptr},
  { "CREOLES_AND_PIDGINS_PORTUGUESE_BASED", nullptr, "cpp", nullptr},
  { "CREOLES_AND_PIDGINS_OTHER", nullptr, "crp", nullptr},
  { "MAORI",               "mi", "mao", nullptr},
  { "WOLOF",               "wo", "wol", nullptr},
  { "ABKHAZIAN",           "ab", "abk", nullptr},
  { "AFAR",                "aa", "aar", nullptr},
  { "AYMARA",              "ay", "aym", nullptr},
  { "BASHKIR",             "ba", "bak", nullptr},
  { "BISLAMA",             "bi", "bis", nullptr},
  { "DZONGKHA",            "dz", "dzo", nullptr},
  { "FIJIAN",              "fj", "fij", nullptr},
  { "GREENLANDIC",         "kl", "kal", nullptr},
  { "HAUSA",               "ha", "hau", nullptr},
  { "HAITIAN_CREOLE",       "ht", nullptr, nullptr},
  { "INUPIAK",             "ik", "ipk", nullptr},
  { "INUKTITUT",           "iu", "iku", nullptr},
  { "KASHMIRI",            "ks", "kas", nullptr},
  { "KINYARWANDA",         "rw", "kin", nullptr},
  { "MALAGASY",            "mg", "mlg", nullptr},
  { "NAURU",               "na", "nau", nullptr},
  { "OROMO",               "om", "orm", nullptr},
  { "RUNDI",               "rn", "run", nullptr},
  { "SAMOAN",              "sm", "smo", nullptr},
  { "SANGO",               "sg", "sag", nullptr},
  { "SANSKRIT",            "sa", "san", nullptr},
  { "SISWANT",             "ss", "ssw", nullptr},
  { "TSONGA",              "ts", "tso", nullptr},
  { "TSWANA",              "tn", "tsn", nullptr},
  { "VOLAPUK",             "vo", "vol", nullptr},
  { "ZHUANG",              "za", "zha", nullptr},
  { "KHASI",               nullptr, "kha", nullptr},
  { "SCOTS",               nullptr, "sco", nullptr},
  { "GANDA",               "lg", "lug", nullptr},
  { "MANX",                "gv", "glv", nullptr},
  { "MONTENEGRIN",         nullptr, nullptr, "sr-ME"},
  { "XX",                  nullptr, nullptr, "XX"},
};

COMPILE_ASSERT(arraysize(kLanguageInfoTable) == NUM_LANGUAGES + 1,
               kLanguageInfoTable_has_incorrect_length);


// LANGUAGE NAMES

const char* default_language_name() {
  return kLanguageInfoTable[ENGLISH].language_name_;
}

static const char* const kInvalidLanguageName = "invalid_language";

const char *invalid_language_name() {
  return kInvalidLanguageName;
}

const char* LanguageName(Language lang) {
  return IsValidLanguage(lang)
      ? kLanguageInfoTable[lang].language_name_
      : kInvalidLanguageName;
}



// LANGUAGE CODES


// The space before invalid_language_code is intentional. It is used
// to prevent it matching any two letter language code.
//
static const char* const kInvalidLanguageCode = " invalid_language_code";

const char *invalid_language_code() {
  return kInvalidLanguageCode;
}

const char * LanguageCode(Language lang) {
  if (! IsValidLanguage(lang))
    return kInvalidLanguageCode;
  const LanguageInfo& info = kLanguageInfoTable[lang];
  if (info.language_code_639_1_) {
    return info.language_code_639_1_;
  } else if (info.language_code_639_2_) {
    return info.language_code_639_2_;
  } else if (info.language_code_other_) {
    return info.language_code_other_;
  } else {
    return kInvalidLanguageCode;
  }
}

const char* default_language_code() {
  return kLanguageInfoTable[ENGLISH].language_code_639_1_;
}

const char* LanguageCodeISO639_1(Language lang) {
  if (! IsValidLanguage(lang))
    return kInvalidLanguageCode;
  if (const char* code = kLanguageInfoTable[lang].language_code_639_1_)
    return code;
  return kInvalidLanguageCode;
}

const char* LanguageCodeISO639_2(Language lang) {
  if (! IsValidLanguage(lang))
    return kInvalidLanguageCode;
  if (const char* code = kLanguageInfoTable[lang].language_code_639_2_)
    return code;
  return kInvalidLanguageCode;
}

const char* LanguageCodeWithDialects(Language lang) {
  if (lang == CHINESE)
    return "zh-CN";
  return LanguageCode(lang);
}



bool LanguageFromCode(const char* lang_code, Language *language) {
  *language = UNKNOWN_LANGUAGE;
  if ( lang_code == nullptr ) return false;

  for ( int i = 0 ; i < kNumLanguages ; i++ ) {
    const LanguageInfo& info = kLanguageInfoTable[i];
    if ((info.language_code_639_1_ &&
         !base::strcasecmp(lang_code, info.language_code_639_1_)) ||
        (info.language_code_639_2_ &&
         !base::strcasecmp(lang_code, info.language_code_639_2_)) ||
        (info.language_code_other_ &&
         !base::strcasecmp(lang_code, info.language_code_other_))) {
      *language = static_cast<Language>(i);
      return true;
    }
  }

  // For convenience, this function can also parse the non-standard
  // five-letter language codes "zh-cn" and "zh-tw" which are used by
  // front-ends such as GWS to distinguish Simplified from Traditional
  // Chinese.
  if (!base::strcasecmp(lang_code, "zh-cn") ||
      !base::strcasecmp(lang_code, "zh_cn")) {
    *language = CHINESE;
    return true;
  }
  if (!base::strcasecmp(lang_code, "zh-tw") ||
      !base::strcasecmp(lang_code, "zh_tw")) {
    *language = CHINESE_T;
    return true;
  }
  if (!base::strcasecmp(lang_code, "sr-me") ||
      !base::strcasecmp(lang_code, "sr_me")) {
    *language = MONTENEGRIN;
    return true;
  }

  // Process language-code synonyms.
  if (!base::strcasecmp(lang_code, "he")) {
    *language = HEBREW;  // Use "iw".
    return true;
  }
  if (!base::strcasecmp(lang_code, "in")) {
    *language = INDONESIAN;  // Use "id".
    return true;
  }
  if (!base::strcasecmp(lang_code, "ji")) {
    *language = YIDDISH;  // Use "yi".
    return true;
  }

  // Process language-detection synonyms.
  // These distinct languages cannot be differentiated by our current
  // language-detection algorithms.
  if (!base::strcasecmp(lang_code, "fil")) {
    *language = TAGALOG;
    return true;
  }

  return false;
}

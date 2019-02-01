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

// ---------------------------------------------------------------------
// ---  Disable/Enable some CodeAnalysis Warnings  ---
#pragma warning ( disable: 26451)
//#pragma warning ( enable : 6001 )
// ---------------------------------------------------------------------

#include "compact_enc_det/compact_enc_det.h"

#include <math.h>                       // for sqrt
#include <stddef.h>                     // for size_t
#include <stdio.h>                      // for printf, fprintf, NULL, etc
#include <stdlib.h>                     // for qsort
#include <string.h>                     // for memset, memcpy, memcmp, etc
#include <memory>
#include <string>                       // for string, operator==, etc

#include "compact_enc_det/compact_enc_det_hint_code.h"
#include "util/string_util.h"
#include "util/basictypes.h"
#include "util/commandlineflags.h"
#include "util/logging.h"

using std::string;

// TODO as of 2007.10.09:
//
// Consider font=TT-BHxxx as user-defined => binary
// Demote GB18030 if no 8x3x pair
// Map byte2 ascii punct to 0x60, digits to 0x7e, gets them into hires
// Consider removing/ignoring bytes 01-1F to avoid crap pollution
// Possibly boost declared encoding in robust scan
// googlebot tiny files
// look for ranges of encodings
// consider tags just as > < within aligned block of 32
// flag too few characters in postproc (Latin 6 problem)
// Remove slow scan beyond 16KB
// Consider removing kMostLikelyEncoding or cut it in half


// A note on mixed encodings
//
// The most common encoding error on the web is a page containing a mixture of
// CP-1252 and UTF-8. A less common encoding error is a third-party feed that
// has been converted from CP-1252 to UTF-8 and then those bytes converted a
// second time to UTF-8. CED originally attempted to detect these error cases
// by using two  synthetic encodings, UTF8CP1252 and UTF8UTF8. The intended
// implementation was to start these just below CP1252 and UTF8 respectively in
// overall  liklihood, and allow 1252 and UTF8 to fall behind if mixtures are
// found.
//
// The UTF8UTF8 encoding is a possible outcome from CED, but unfortunately the
// UTF8CP1252 internal encoding was added late and not put into encodings.proto,
// so at the final step it is mapped to UTF8UTF8 also. This was a bad idea and
// is removed in this November 2011 CL.
//
// Mixed encoding detection never worked out as well as envisioned, so the
// ced_allow_utf8utf8 flag normally disables all this.
//
// The effect is that CP-1252 and UTF-8 mixtures will usually be detected as
// UTF8, and the inputconverter code for UTF8 normally will convert bare
// CP-1252 bytes to UTF-8, instead of the less-helpful FFFD substitution. UTF-8
// and double-UTF-8 mixtures will be detected as UTF-8, and the double
// conversion will stand.
//
// However, it is occasionally useful to use CED to detect double-converted
// UTF-8 coming from third-party data feeds, so they can be fixed at the source.
// For this purpose, the  UTF8UTF8 encoding remains available under the
// ced_allow_utf8utf8 flag.
//
// When UTF8UTF8 is detected, the inputconverter code will undo the double
// conversion, giving good text.

// Norbert Runge has noted these words in CP1252 that are mistakenly identified
// as UTF-8 because of the last pair of characters:
//  NESTLÉ®               0xC9 0xAE U+00C9 U+00AE   C9AE = U+026E;SMALL LEZH
//  drauß\u2019           0xDF 0x92 U+00DF U+2019   DF92 = U+07D2;NKO LETTER N
//  Mutterschoß\u201c     0xDF 0x93 U+00DF U+201C   DF93 = U+07D3;NKO LETTER BA
//  Schoß\u201c           0xDF 0x93 U+00DF U+201C
//  weiß\u201c            0xDF 0x93 U+00DF U+00AB
//  Schnellfuß\u201c      0xDF 0x93 U+00DF U+201C
//  süß«                  0xDF 0xAB U+00DF U+00AB   DFAB = U+07EB;NKO HIGH TONE
// These four byte combinations now explicitly boost Latin1/CP1252.

// And for reference, here are a couple of Portuguese spellings
// that may be mistaken as double-byte encodings.
//   informações          0xE7 0xF5
//   traição              0xE7 0xE3


static const char* kVersion = "2.2";

DEFINE_bool(ced_allow_utf8utf8, false, "Allow the UTF8UTF8 encoding, "
                                       "to handle mixtures of CP1252 "
                                       "converted to UTF-8 zero, one, "
                                       "or two times");
DEFINE_int32(enc_detect_slow_max_kb, 16,
             "Maximum number of Kbytes to examine for "
             "7-bit-only (2022, Hz, UTF7) encoding detect. "
             "You are unlikely to want to change this.");
DEFINE_int32(enc_detect_fast_max_kb, 256,
             "Maximum number of Kbytes to examine for encoding detect. "
             "You are unlikely to want to change this.");

DEFINE_int32(ced_reliable_difference, 300, "30 * Bits of minimum probablility "
             "difference 1st - 2nd to be considered reliable \n"
             "  2 corresponds to min 4x difference\n"
             "  4 corresponds to min 16x difference\n"
             "  8 corresponds to min 256x difference\n"
             "  10 corresponds to min 1024x difference\n"
             "  20 corresponds to min 1Mx difference.");

// Text debug output options
DEFINE_bool(enc_detect_summary, false,
            "Print first 16 interesting pairs at exit.");
DEFINE_bool(counts, false, "Count major-section usage");

// PostScript debug output options
DEFINE_bool(enc_detect_detail, false,
             "Print PostScript of every update, to stderr.");
DEFINE_bool(enc_detect_detail2, false,
             "More PostScript detail of every update, to stderr.");
DEFINE_bool(enc_detect_source, false, "Include source text in detail");
// Encoding name must exactly match FIRST column of kI18NInfoByEncoding in
// lang_enc.cc

// Following flags are not in use. Replace them with constants to
// avoid static initialization.

//DEFINE_string(enc_detect_watch1, "", "Do detail2 about this encoding name.");
//DEFINE_string(enc_detect_watch2, "", "Do detail2 about this encoding name.");

static const char* const FLAGS_enc_detect_watch1 = "";
static const char* const FLAGS_enc_detect_watch2 = "";

// Only for experiments. Delete soon.
DEFINE_bool(force127, false, "Force Latin1, Latin2, Latin7 based on trigrams");

// Demo-mode/debugging experiment
DEFINE_bool(demo_nodefault, false,
             "Default to all equal; no boost for declared encoding.");
DEFINE_bool(dirtsimple, false, "Just scan and count for all encodings");
DEFINE_bool(ced_echo_input, false, "Echo ced input to stderr");


static const int XDECILOG2 = 3;             // Multiplier for log base 2 ** n/10
static const int XLOG2 = 30;                // Multiplier for log base 2 ** n

static const int kFinalPruneDifference = 10 * XLOG2;
                                            // Final bits of minimum
                                            // probability difference 1st-nth
                                            // to be pruned

static const int kInititalPruneDifference = kFinalPruneDifference * 4;
                                            // Initial bits of minimum
                                            // probability difference 1st-nth
                                            // to be pruned
                                            //
static const int kPruneDiffDecrement = kFinalPruneDifference;
                                            // Decrements bits of minimum
                                            // probability difference 1st-nth
                                            // to be pruned

static const int kSmallInitDiff = 2 * XLOG2;       // bits of minimum
                                            // probability difference, base to
                                            // superset encodings

static const int kBoostInitial = 20 * XLOG2;    // bits of boost for
                                            // initial byte patterns (BOM, 00)

static const int kBadPairWhack = 20 * XLOG2;    // bits of whack for
                                            // one bad pair

static const int kBoostOnePair = 20 * XLOG2;    // bits of boost for
                                            // one good pair in Hz, etc.

static const int kGentleOnePair = 4 * XLOG2;    // bits of boost for
                                            // one good sequence
                                            //
static const int kGentlePairWhack = 2 * XLOG2;       // bits of whack
                                            // for ill-formed sequence

static const int kGentlePairBoost = 2 * XLOG2;       // bits of boost
                                            // for well-formed sequence

static const int kDeclaredEncBoost = 5 * XDECILOG2;  // bits/10 of boost for
                                            // best declared encoding per bigram

static const int kBestEncBoost = 5 * XDECILOG2;     // bits/10 of boost for
                                            // best encoding per bigram

static const int kTrigramBoost = 2 * XLOG2; // bits of boost for Latin127 tri

static const int kMaxPairs = 48;            // Max interesting pairs to look at
                                            // If you change this,
                                            // adjust *PruneDiff*

static const int kPruneMask = 0x07;         // Prune every 8 interesting pairs


static const int kBestPairsCount = 16;      // For first N pairs, do extra boost
                                            // based on most likely encoding
                                            // of pair over entire web

static const int kDerateHintsBelow = 12;    // If we have fewer than N bigrams,
                                            // weaken the hints enough that
                                            // unhinted encodings have a hope of
                                            // rising to the top

static const int kMinRescanLength = 800;    // Don't bother rescanning for
                                            // unreliable encoding if fewer
                                            // than this many bytes unscanned.
                                            // We will rescan at most last half
                                            // of this.

static const int kStrongBinary = 12;  // Make F_BINARY the only encoding
static const int kWeakerBinary = 4;   // Make F_BINARY likely encoding

// These are byte counts from front of file
static const int kBinaryHardAsciiLimit = 6 * 1024;  // Not binary if all ASCII
static const int kBinarySoftAsciiLimit = 8 * 1024;  //   "   if mostly ASCII

// We try here to avoid having title text dominate the encoding detection,
// for the not-infrequent error case of title in encoding1, body in encoding2:
// we want to bias toward encoding2 winning.
//
// kMaxBigramsTagTitleText should be a multiple of 2, 3, and 4, so that we
// rarely cut off mid-character in the original (not-yet-detected) encoding.
// This matters most for UTF-8 two- and three-byte codes and for
// Shift-JIS three-byte codes.
static const int kMaxBigramsTagTitleText = 12;      // Keep only some tag text
static const int kWeightshiftForTagTitleText = 4;   // Give text in tags, etc.
                                                    // 1/16 normal weight

static const int kStrongPairs = 6;          // Let reliable enc with this many
                                            // pairs overcome missing hint

enum CEDInternalFlags {
  kCEDNone = 0,           // The empty flag
  kCEDRescanning = 1,     // Do not further recurse
  kCEDSlowscore = 2,      // Do extra scoring
  kCEDForceTags = 4,      // Always examine text inside tags
};

// Forward declaration
Encoding InternalDetectEncoding(
    CEDInternalFlags flags, const char* text, int text_length,
    const char* url_hint, const char* http_charset_hint,
    const char* meta_charset_hint, const int encoding_hint,
    const Language language_hint,  // User interface lang
    const CompactEncDet::TextCorpusType corpus_type,
    bool ignore_7bit_mail_encodings, int* bytes_consumed, bool* is_reliable,
    Encoding* second_best_enc);

typedef struct {
  const uint8* hires[4];  // Pointers to possible high-resolution bigram deltas
  uint8 x_bar;          // Average byte2 value
  uint8 y_bar;          // Average byte1 value
  uint8 x_stddev;       // Standard deviation of byte2 value
  uint8 y_stddev;       // Standard deviation of byte1 value
  int so;               // Scaling offset -- add to probabilities below
  uint8 b1[256];        // Unigram probability for first byte of aligned bigram
  uint8 b2[256];        // Unigram probability for second byte of aligned bigram
  uint8 b12[256];       // Unigram probability for cross bytes of aligned bigram
} UnigramEntry;

//typedef struct {
//  uint8 b12[256*256]; // Bigram probability for aligned bigram
//} FullBigramEntry;


// Include all the postproc-generated tables here:
// RankedEncoding
// kMapToEncoding
// unigram_table
// kMostLIkelyEncoding
// kTLDHintProbs
// kCharsetHintProbs
// HintEntry, kMaxTldKey kMaxTldVector, etc.
// =============================================================================

#include "compact_enc_det/compact_enc_det_generated_tables.h"


#define F_ASCII F_Latin1    // "ASCII" is a misnomer, so this code uses "Latin1"

#define F_BINARY F_X_BINARYENC        // We are mid-update for name change
#define F_UTF8UTF8 F_X_UTF8UTF8       // We are mid-update for name change
#define F_BIG5_CP950 F_BIG5           // We are mid-update for name change
#define F_Unicode F_UTF_16LE          // We are mid-update for name change
// =============================================================================

// 7-bit encodings have at least one "interesting" byte value < 0x80
//   (00 0E 1B + ~)
// JIS 2022-cn 2022-kr hz utf7
// Unicode UTF-16 UTF-32
// 8-bit encodings have no interesting byte values < 0x80
static const uint32 kSevenBitActive = 0x00000001;   // needs <80 to detect
static const uint32 kUTF7Active     = 0x00000002;   // <80 and +
static const uint32 kHzActive       = 0x00000004;   // <80 and ~
static const uint32 kIso2022Active  = 0x00000008;   // <80 and 1B 0E 0F
static const uint32 kUTF8Active     = 0x00000010;
static const uint32 kUTF8UTF8Active = 0x00000020;
static const uint32 kUTF1632Active  = 0x00000040;   // <80 and 00
static const uint32 kBinaryActive   = 0x00000080;   // <80 and 00
static const uint32 kTwobyteCode    = 0x00000100;   // Needs 8xxx
static const uint32 kIsIndicCode    = 0x00000200;   //
static const uint32 kHighAlphaCode  = 0x00000400;   // full alphabet in 8x-Fx
static const uint32 kHighAccentCode = 0x00000800;   // accents in 8x-Fx
static const uint32 kEUCJPActive    = 0x00001000;   // Have to mess with phase


// Debug only. not thread safe
static int encdet_used = 0;
static int rescore_used = 0;
static int rescan_used = 0;
static int robust_used = 0;
static int looking_used = 0;
static int doing_used = 0;


// For debugging only -- about 256B/entry times about 500 = 128KB
// TODO: only allocate this if being used
typedef struct {
  int offset = 0;
  int best_enc = -1;  // Best ranked encoding for this bigram, or
                      // -1 for overhead entries
  string label = "";
  int detail_enc_prob[NUM_RANKEDENCODING] = {};
} DetailEntry;

static int watch1_rankedenc = -1;     // Debug. not threadsafe
static int watch2_rankedenc = -1;     // Debug. not threadsafe
////static int next_detail_entry = 0;     // Debug. not threadsafe
////static DetailEntry details[kMaxPairs * 10];  // Allow 10 details per bigram
// End For debugging only

// Must match kTestPrintableAsciiTildePlus exit codes, minus one
enum PairSet {AsciiPair = 0, OtherPair = 1, NUM_PAIR_SETS = 2};

// The reasons for pruning
enum PruneReason {PRUNE_NORMAL, PRUNE_SLOWEND, PRUNE_FINAL};

static const char* kWhatSetName[] = {"Ascii", "Other"};


// State for encodings that do shift-out/shift-in between one- and two-byte
// regions (ISO-2022-xx, HZ)
enum StateSoSi {SOSI_NONE, SOSI_ERROR, SOSI_ONEBYTE, SOSI_TWOBYTE};

#define UTF8_ARR_CNT 6
#define BYTE32_ARR_CNT 8

typedef struct {
  const uint8* initial_src;       // For calculating byte offsets
  const uint8* limit_src;         // Range of input source
  const uint8* prior_src;         // Source consumed by prior call to BoostPrune
  const uint8* last_pair;         // Last pair inserted into interesting_pairs

  DetailEntry* debug_data;        // Normally NULL. Ptr to debug data for
                                  // FLAGS_enc_detect_detail PostScript data
  int next_detail_entry;          // Debug

  bool done;
  bool reliable;
  bool hints_derated;
  int declared_enc_1;             // From http/meta hint
  int declared_enc_2;             // from http/meta hint
  int prune_count;                // Number of times we have pruned

  int trigram_highwater_mark;       // Byte offset of last trigram processing
  bool looking_for_latin_trigrams;  // True if we should test for doing
                                    //  Latin1/2/7 trigram processing
  bool do_latin_trigrams;           // True if we actually are scoring trigrams

  // Miscellaneous state variables for difficult encodings
  int binary_quadrants_count;           // Number of four bigram quadrants seen:
                                        //  0xxxxxxx0xxxxxxx 0xxxxxxx1xxxxxx
                                        //  1xxxxxxx0xxxxxxx 1xxxxxxx1xxxxxx
  int binary_8x4_count;                 // Number of 8x4 buckets seen:
  uint32 binary_quadrants_seen;         // Bit[i] set if bigram i.......i....... seen
  uint32 binary_8x4_seen;               // Bit[i] set if bigram iii.....ii...... seen
  int utf7_starts;                      // Count of possible UTF-7 beginnings seen
  int prior_utf7_offset;                // Source consumed by prior UTF-7 string
  int next_utf8_ministate;              // Mini state for UTF-8 sequences
  int utf8_minicount[UTF8_ARR_CNT];     // Number of correct 2- 3- 4-byte seq, errors
  int next_utf8utf8_ministate;          // Mini state for UTF8UTF8 sequences
  int utf8utf8_odd_byte;                // UTF8UTF8 seq has odd number of bytes
  int utf8utf8_minicount[UTF8_ARR_CNT]; // Number of correct 2- 3- 4-byte seq, errors
  StateSoSi next_2022_state;            // Mini state for 2022 sequences
  StateSoSi next_hz_state;              // Mini state for HZ sequences
  bool next_eucjp_oddphase;             // Mini state for EUC-JP sequences
  int byte32_count[BYTE32_ARR_CNT];     // Count of top 3 bits of byte1 of bigram
                                        // 0x1x 2x3x 4x5x 6x7x 8x9x AxBx CxDx ExFx
  uint32 active_special;                // Bits showing which special cases are active
                                        
  Encoding tld_hint;                    // Top TLD encoding or UNKNOWN
  Encoding http_hint;                   // What the document says about itself or
  Encoding meta_hint;                   // UNKNOWN_ENCODING. BOM is initial byte
  Encoding bom_hint;                    // order mark for UTF-xx

  // small cache of previous interesting bigrams
  int next_prior_bigram;
  int prior_bigram[4];
  int prior_binary[1];

  int top_rankedencoding;         // Top two probabilities and families
  int second_top_rankedencoding;
  int top_prob;
  int second_top_prob;
  int prune_difference;           // Prune things this much below the top prob
  int rankedencoding_list_len;                // Number of active encodings
  int rankedencoding_list[NUM_RANKEDENCODING];  // List of active encodings
                                                //
  int enc_prob[NUM_RANKEDENCODING];           // Cumulative probability per enc
                                              // This is where all the action is
  int hint_prob[NUM_RANKEDENCODING];          // Initial hint probabilities
  int hint_weight[NUM_RANKEDENCODING];        // Number of hints for this enc

  // Two sets -- one for printable ASCII, one for the rest
  int prior_interesting_pair[NUM_PAIR_SETS];  // Pairs consumed by prior call
  int next_interesting_pair[NUM_PAIR_SETS];   // Next pair to write
  char interesting_pairs[NUM_PAIR_SETS][kMaxPairs * 2];   // Two bytes per pair
  int interesting_offsets[NUM_PAIR_SETS][kMaxPairs];      // Src offset of pair
  int interesting_weightshift[NUM_PAIR_SETS][kMaxPairs];  // weightshift of pair
} DetectEncodingState;


// Record a debug event that changes probabilities
void SetDetailsEncProb(DetectEncodingState* destatep,
                       int offset, int best_enc, const char* label) {
  int next = destatep->next_detail_entry;
  destatep->debug_data[next].offset = offset;
  destatep->debug_data[next].best_enc = best_enc;
  destatep->debug_data[next].label = label;
  memcpy(&destatep->debug_data[next].detail_enc_prob,
         &destatep->enc_prob,
         sizeof(destatep->enc_prob));
  ++destatep->next_detail_entry;
}

// Record a debug event that changes probabilities, copy offset
void SetDetailsEncProbCopyOffset(DetectEncodingState* destatep,
                                 int best_enc, const char* label) {
  int next = destatep->next_detail_entry;
  destatep->debug_data[next].offset = destatep->debug_data[next - 1].offset;
  destatep->debug_data[next].best_enc = best_enc;
  destatep->debug_data[next].label = label;
  memcpy(&destatep->debug_data[next].detail_enc_prob,
         &destatep->enc_prob,
         sizeof(destatep->enc_prob));
  ++destatep->next_detail_entry;
}

// Record a debug event that changes probs and has simple text label
void SetDetailsEncLabel(DetectEncodingState* destatep, const char* label) {
  int next = destatep->next_detail_entry;
  destatep->debug_data[next].offset = destatep->debug_data[next - 1].offset;
  destatep->debug_data[next].best_enc = -1;
  destatep->debug_data[next].label = label;
  memcpy(&destatep->debug_data[next].detail_enc_prob,
         &destatep->enc_prob,
         sizeof(destatep->enc_prob));
  ++destatep->next_detail_entry;
}

// Record a debug event that is just a text label, no change in probs
void SetDetailsLabel(DetectEncodingState* destatep, const char* label) {
  int next = destatep->next_detail_entry;
  destatep->debug_data[next].offset = destatep->debug_data[next - 1].offset;
  destatep->debug_data[next].best_enc = -1;
  destatep->debug_data[next].label = label;
  memcpy(&destatep->debug_data[next].detail_enc_prob,
         &destatep->debug_data[next - 1].detail_enc_prob,
         sizeof(destatep->enc_prob));
  ++destatep->next_detail_entry;
}


// Maps superset encodings to base, to see if 2 encodings are compatible
// (Non-identity mappings are marked "-->" below.)
static const Encoding kMapEncToBaseEncoding[] = {
  ISO_8859_1,       // 0: Teragram ASCII
  ISO_8859_2,       // 1: Teragram Latin2
  ISO_8859_3,       // 2: in BasisTech but not in Teragram
  ISO_8859_4,       // 3: Teragram Latin4
  ISO_8859_5,       // 4: Teragram ISO-8859-5
  ISO_8859_6,       // 5: Teragram Arabic
  ISO_8859_7,       // 6: Teragram Greek
  MSFT_CP1255,      // 7: Teragram Hebrew --> 36
  ISO_8859_9,       // 8: in BasisTech but not in Teragram
  ISO_8859_10,      // 9: in BasisTech but not in Teragram
  JAPANESE_EUC_JP,  // 10: Teragram EUC_JP
  JAPANESE_SHIFT_JIS,  // 11: Teragram SJS
  JAPANESE_JIS,     // 12: Teragram JIS
  CHINESE_BIG5,     // 13: Teragram BIG5
  CHINESE_GB,       // 14: Teragram GB
  CHINESE_EUC_CN,   // 15: Teragram EUC-CN
  KOREAN_EUC_KR,    // 16: Teragram KSC
  UNICODE,          // 17: Teragram Unicode
  CHINESE_EUC_CN,   // 18: Teragram EUC --> 15
  CHINESE_EUC_CN,   // 19: Teragram CNS --> 15
  CHINESE_BIG5,     // 20: Teragram BIG5_CP950 --> 13
  JAPANESE_SHIFT_JIS,   // 21: Teragram CP932 --> 11
  UTF8,             // 22
  UNKNOWN_ENCODING, // 23
  ISO_8859_1,       // 24: ISO_8859_1 with all characters <= 127 --> 0
  RUSSIAN_KOI8_R,   // 25: Teragram KOI8R
  RUSSIAN_CP1251,   // 26: Teragram CP1251
  ISO_8859_1,       // 27: CP1252 aka MSFT euro ascii --> 0
  RUSSIAN_KOI8_RU,  // 28: CP21866 aka KOI8_RU, used for Ukrainian
  MSFT_CP1250,      // 29: CP1250 aka MSFT eastern european
  ISO_8859_1,       // 30: aka ISO_8859_0 aka ISO_8859_1 euroized --> 0
  ISO_8859_9,       // 31: used for Turkish
  ISO_8859_13,      // 32: used in Baltic countries --> 43
  ISO_8859_11,      // 33: aka TIS-620, used for Thai
  ISO_8859_11,      // 34: used for Thai --> 33
  MSFT_CP1256,      // 35: used for Arabic
  MSFT_CP1255,      // 36: Logical Hebrew Microsoft
  MSFT_CP1255,      // 37: Iso Hebrew Logical --> 36
  MSFT_CP1255,      // 38: Iso Hebrew Visual --> 36
  CZECH_CP852,      // 39
  ISO_8859_2,       // 40: aka ISO_IR_139 aka KOI8_CS --> 1
  MSFT_CP1253,      // 41: used for Greek, but NOT a superset of 8859-7
  RUSSIAN_CP866,    // 42
  ISO_8859_13,      // 43
  ISO_2022_KR,      // 44
  CHINESE_GB,       // 45 GBK --> 14
  CHINESE_GB,       // 46 GB18030 --> 14
  CHINESE_BIG5,     // 47 BIG5_HKSCS --> 13
  ISO_2022_KR,      // 48 ISO_2022_CN --> 44
  TSCII,            // 49 Indic encoding
  TAMIL_MONO,       // 50 Indic encoding - Tamil
  TAMIL_BI,         // 51 Indic encoding - Tamil
  JAGRAN,           // 52 Indic encoding - Devanagari
  MACINTOSH_ROMAN,  // 53
  UTF7,             // 54
  BHASKAR,          // 55 Indic encoding - Devanagari
  HTCHANAKYA,       // 56 Indic encoding - Devanagari
  UTF16BE,          // 57
  UTF16LE,          // 58
  UTF32BE,          // 59
  UTF32LE,          // 60
  BINARYENC,        // 61
  HZ_GB_2312,       // 62
  UTF8UTF8,         // 63
  TAM_ELANGO,       // 64 Elango - Tamil
  TAM_LTTMBARANI,   // 65 Barani - Tamil
  TAM_SHREE,        // 66 Shree - Tamil
  TAM_TBOOMIS,      // 67 TBoomis - Tamil
  TAM_TMNEWS,       // 68 TMNews - Tamil
  TAM_WEBTAMIL,     // 69 Webtamil - Tamil
  KDDI_SHIFT_JIS,         // 70 KDDI Shift_JIS
  DOCOMO_SHIFT_JIS,       // 71 DoCoMo Shift_JIS
  SOFTBANK_SHIFT_JIS,     // 72 SoftBank Shift_JIS
  KDDI_ISO_2022_JP,       // 73 KDDI ISO-2022-JP
  SOFTBANK_ISO_2022_JP,   // 74 SOFTBANK ISO-2022-JP
};

COMPILE_ASSERT(arraysize(kMapEncToBaseEncoding) == NUM_ENCODINGS,
               kMapEncToBaseEncoding_has_incorrect_size);

// Maps base encodings to 0, supersets to 1+, undesired to -1
// (Non-identity mappings are marked "-->" below.)
static const int kMapEncToSuperLevel[] = {
  0,       // 0: Teragram ASCII
  0,       // 1: Teragram Latin2
  0,       // 2: in BasisTech but not in Teragram
  0,       // 3: Teragram Latin4
  0,       // 4: Teragram ISO-8859-5
  0,       // 5: Teragram Arabic
  0,       // 6: Teragram Greek
  0,       // 7: Teragram Hebrew
  0,       // 8: in BasisTech but not in Teragram
  0,      // 9: in BasisTech but not in Teragram
  0,      // 10: Teragram EUC_JP
  0,      // 11: Teragram SJS
  0,      // 12: Teragram JIS
  0,      // 13: Teragram BIG5
  0,       // 14: Teragram GB
  0,      // 15: Teragram EUC-CN
  0,      // 16: Teragram KSC
  0,          // 17: Teragram Unicode
  -1,     // 18: Teragram EUC --> 15
  -1,     // 19: Teragram CNS --> 15
  1,      // 20: Teragram BIG5_CP950 --> 13
  1,      // 21: Teragram CP932 --> 11
  0,             // 22
  -1,     // 23
  -1,       // 24: ISO_8859_1 with all characters <= 127 --> 0
  0,      // 25: Teragram KOI8R
  0,      // 26: Teragram CP1251
  1,       // 27: CP1252 aka MSFT euro ascii --> 0
  0,      // 28: CP21866 aka KOI8_RU, used for Ukrainian
  0,      // 29: CP1250 aka MSFT eastern european
  1,       // 30: aka ISO_8859_0 aka ISO_8859_1 euroized --> 0
  0,       // 31: used for Turkish
  1,      // 32: used in Baltic countries --> 43
  0,      // 33: aka TIS-620, used for Thai
  1,      // 34: used for Thai --> 33
  0,      // 35: used for Arabic
  0,      // 36: Logical Hebrew Microsoft
  -1,      // 37: Iso Hebrew Logical --> 36
  -1,       // 38: Iso Hebrew Visual --> 7
  0,      // 39
  1,       // 40: aka ISO_IR_139 aka KOI8_CS --> 1
  0,       // 41: used for Greek, NOT superset of 8859-7
  0,      // 42
  0,      // 43
  0,      // 44
  1,       // 45 GBK --> 14
  1,       // 46 GB18030 --> 14
  1,      // 47 BIG5_HKSCS --> 13
  1,      // 48 ISO_2022_CN --> 44
  0,      // 49 Indic encoding
  0,       // 50 Indic encoding - Tamil
  0,         // 51 Indic encoding - Tamil
  0,           // 52 Indic encoding - Devanagari
  0,      // 53
  0,      // 54
  0,      // 55 Indic encoding - Devanagari
  0,      // 56 Indic encoding - Devanagari
  0,          // 57
  0,          // 58
  0,          // 59
  0,          // 60
  0,        // 61
  0,       // 62
  2,         // 63
  0, 0, 0, 0, 0, 0,         // add six more Tamil
  0, 0, 0, 0, 0,            // add five encodings with emoji
};

COMPILE_ASSERT(arraysize(kMapEncToSuperLevel) == NUM_ENCODINGS,
               kMapEncToSuperLevel_has_incorrect_size);



// Subscripted by Encoding enum value
static const uint32 kSpecialMask[] = {
  kHighAccentCode,                    // 0
  kHighAccentCode,
  kHighAccentCode,
  kHighAccentCode,
  kHighAlphaCode,                     // 4
  kHighAlphaCode,
  kHighAlphaCode,
  kHighAlphaCode,
  kHighAccentCode,
  kHighAccentCode,

  kTwobyteCode + kEUCJPActive,        // 10 euc-jp
  kTwobyteCode,
  kSevenBitActive + kIso2022Active,   // jis
  kTwobyteCode,
  kTwobyteCode,
  kTwobyteCode,
  kTwobyteCode,
  kSevenBitActive + kUTF1632Active,   // Unicode
  kTwobyteCode,
  kTwobyteCode,

  kTwobyteCode,                       // 20
  kTwobyteCode,
  kUTF8Active,                        // UTF-8
  0,
  0,
  kHighAlphaCode,                     // 25
  kHighAlphaCode,
  kHighAccentCode,
  kHighAlphaCode,
  kHighAccentCode,

  kHighAccentCode,                   // 30
  kHighAccentCode,
  kHighAccentCode,
  kHighAlphaCode,
  kHighAlphaCode,
  kHighAlphaCode,                    // 35
  kHighAlphaCode,
  kHighAlphaCode,
  kHighAlphaCode,
  0,

  0,                                  // 40
  kHighAlphaCode,
  kHighAlphaCode,
  kHighAccentCode,
  kSevenBitActive + kIso2022Active,   // 2022-kr
  kTwobyteCode,
  kTwobyteCode,
  kTwobyteCode,
  kSevenBitActive + kIso2022Active,   // 2022-cn
  kHighAlphaCode + kIsIndicCode,       // 49 TSCII

  kHighAlphaCode + kIsIndicCode,       // 50 TAMIL_MONO
  kHighAlphaCode + kIsIndicCode,       // 51 TAMIL_BI
  kHighAlphaCode + kIsIndicCode,       // 52 JAGRAN
  kHighAccentCode,                     // 53 MACINTOSH_ROMAN
  kSevenBitActive + kUTF7Active,      // 54 UTF-7
  kHighAlphaCode + kIsIndicCode,       // 55 BHASKAR Indic encoding - Devanagari
  kHighAlphaCode + kIsIndicCode,       // 56 HTCHANAKYA Indic encoding - Devanagari
  kSevenBitActive + kUTF1632Active,   // 57 UTF16BE
  kSevenBitActive + kUTF1632Active,   // 58 UTF16LE
  kSevenBitActive + kUTF1632Active,   // 59 UTF32BE
  kSevenBitActive + kUTF1632Active,   // 60 UTF32LE

  kSevenBitActive + kBinaryActive,    // 61 BINARYENC
  kSevenBitActive + kHzActive,        // 62 HZ_GB_2312
  kHighAccentCode + kUTF8Active + kUTF8UTF8Active,      // 63 UTF8UTF8
  kHighAlphaCode + kIsIndicCode,       // 64 Elango - Tamil
  kHighAlphaCode + kIsIndicCode,       // 65 Barani - Tamil
  kHighAlphaCode + kIsIndicCode,       // 66 Shree - Tamil
  kHighAlphaCode + kIsIndicCode,       // 67 TBoomis - Tamil
  kHighAlphaCode + kIsIndicCode,       // 68 TMNews - Tamil
  kHighAlphaCode + kIsIndicCode,       // 69 Webtamil - Tamil
  kTwobyteCode,                       // 70 KDDI Shift_JIS
  kTwobyteCode,                       // 71 DoCoMo Shift_JIS
  kTwobyteCode,                       // 72 SoftBank Shift_JIS
  kSevenBitActive + kIso2022Active,   // 73 KDDI-ISO-2022-JP
  kSevenBitActive + kIso2022Active,   // 74 SOFTBANK-ISO-2022-JP
};

COMPILE_ASSERT(arraysize(kSpecialMask) == NUM_ENCODINGS,
               kSpecialMask_has_incorrect_size);


/***
  kHighAlphaCode -- full alphabet in 8x-Fx range, not just accents

  ISO_8859_5,       // 4: Teragram ISO-8859-5 Cyrl      UL bd
  RUSSIAN_CP1251,   // 26: Teragram CP1251              UL cdef
  RUSSIAN_KOI8_R,   // 25: Teragram KOI8R               LU cdef
  RUSSIAN_KOI8_RU,  // 28: CP21866 aka KOI8_RU,         LU cdef
  RUSSIAN_CP866,     // 42                              89ae

  ISO_8859_6,       // 5: Teragram Arabic               nocase cde
  MSFT_CP1256,      // 35: used for Arabic              nocase cde

  ISO_8859_7,       // 6: Teragram Greek                UL cdef
  MSFT_CP1253,       // 41: used for Greek              UL cdef

  ISO_8859_8,       // 7: Teragram Hebrew               nocase ef
  MSFT_CP1255,      // 36: Logical Hebrew Microsoft     nocase ef
  ISO_8859_8_I,     // 37: Iso Hebrew Logical           nocase ef
  HEBREW_VISUAL,    // 38: Iso Hebrew Visual            nocase ef

  ISO_8859_11,      // 33: aka TIS-620, used for Thai   nocase abcde
  MSFT_CP874,       // 34: used for Thai                nocase abcde

  TSCII,             // 49                              8-f
  TAMIL_MONO,        // 50
  TAMIL_BI,          // 51
  JAGRAN,            // 52
  BHASKAR,           // 55 Indic encoding - Devanagari
  HTCHANAKYA,        // 56 Indic encoding - Devanagari
***/

// We can scan bytes using this at about 500 MB/sec 2.8GHz P4
// Slow scan uses this, stopping on NUL ESC SO SI bad C0 and + ~
// We allow FF, 0x0C, here because it gives a better result for old
// Ascii text formatted for a TTY
// non-zero exits scan loop -- 1 for printable ASCII, 2 otherwise
static const char kTestPrintableAsciiTildePlus[256] = {
  2,2,2,2,2,2,2,2, 2,0,0,2,0,0,2,2, 2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2,
  0,0,0,0,0,0,0,0, 0,0,0,1,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,1,2,

  2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2,
  2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2,
  2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2,
  2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2,
};

// We can scan bytes using this at about 550 MB/sec 2.8GHz P4
// Slow scan uses this, stopping on NUL ESC SO SI and bad C0
// after Hz and UTF7 are pruned away
// We allow Form Feed, 0x0C, here
static const char kTestPrintableAscii[256] = {
  2,2,2,2,2,2,2,2, 2,0,0,2,0,0,2,2, 2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2,
  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,2,

  2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2,
  2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2,
  2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2,
  2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2,
};

// Used in first-four-byte testing
static const char kIsPrintableAscii[256] = {
  0,0,0,0,0,0,0,0, 0,1,1,0,0,1,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
  1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,0,

  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
};


static const signed char kBase64Value[256] = {
  -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,62,-1,-1,-1,63,
  52,53,54,55,56,57,58,59, 60,61,-1,-1,-1,-1,-1,-1,

  -1, 0, 1, 2, 3, 4, 5, 6,  7, 8, 9,10,11,12,13,14,
  15,16,17,18,19,20,21,22, 23,24,25,-1,-1,-1,-1,-1,
  -1,26,27,28,29,30,31,32, 33,34,35,36,37,38,39,40,
  41,42,43,44,45,46,47,48, 49,50,51,-1,-1,-1,-1,-1,

  -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,

  -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
};


// Subscripted by <state, byte/16>
// Accepts Cx->8x Dx->8x Ex->8x->8x Fx->8x->8x->8x
//
// Fixed Problem: GB has sequences like B2DB B8D6 BDE1 B9B9
// which we can mis-parse as an error byte followed by good UTF-8:
//                                      B2 DBB8 D6BD E1B9B9
// To counteract this, we now require an ASCII7 byte to resync out
// of the error state
// Next problem: good UTF-8 with bad byte
// efbc a012 eea4 bee7 b280 c2b7
// efbca0 12 eea4be e7b280 c2b7
//        ^^ bad byte
// fix: change state0 byte 1x to be don't-care
//
// Short UTF-8 ending in ASCII7 byte should resync immediately:
// E0 20 E0 A6 AA should give one error and resync at 2nd E0
//
static const char kMiniUTF8State[8][16] = {
  {0,0,0,0,0,0,0,0, 7,7,7,7,1,1,2,4,},      // [0] start char (allow cr/lf/ht)
  {0,7,0,0,0,0,0,0, 0,0,0,0,7,7,7,7,},      // [1] continue 1 of 2
  {0,7,0,0,0,0,0,0, 3,3,3,3,7,7,7,7,},      // [2] continue 1 of 3
  {0,7,0,0,0,0,0,0, 0,0,0,0,7,7,7,7,},      // [3] continue 2 of 3
  {0,7,0,0,0,0,0,0, 5,5,5,5,7,7,7,7,},      // [4] continue 1 of 4
  {0,7,0,0,0,0,0,0, 6,6,6,6,7,7,7,7,},      // [5] continue 2 of 4
  {0,7,0,0,0,0,0,0, 0,0,0,0,7,7,7,7,},      // [6] continue 3 of 4
  {0,7,0,0,0,0,0,0, 7,7,7,7,7,7,7,7,},      // [7] error, soak up continues,
                                            // ONLY resync after Ascii char
                                            //     then restart
};
// Counter to increment: 0-don'tcare 1-error 2-good_2B 3-good_3B 4-good_4B
static const char kMiniUTF8Count[8][16] = {
  {0,0,0,0,0,0,0,0, 1,1,1,1,0,0,0,0,},      // [0] start char (allow cr/lf/ht)
  {1,1,1,1,1,1,1,1, 2,2,2,2,1,1,1,1,},      // [1] continue 1 of 2
  {1,1,1,1,1,1,1,1, 0,0,0,0,1,1,1,1,},      // [2] continue 1 of 3
  {1,1,1,1,1,1,1,1, 3,3,3,3,1,1,1,1,},      // [3] continue 2 of 3
  {1,1,1,1,1,1,1,1, 0,0,0,0,1,1,1,1,},      // [4] continue 1 of 4
  {1,1,1,1,1,1,1,1, 0,0,0,0,1,1,1,1,},      // [5] continue 2 of 4
  {1,1,1,1,1,1,1,1, 4,4,4,4,1,1,1,1,},      // [6] continue 3 of 4
  {0,1,0,0,0,0,0,0, 1,1,1,1,1,1,1,1,},      // [7] error, soak up continues,
                                            //     then restart
};

// Subscripted by <state, f(byte1) + g(byte2)>
// where f(x)= E2->4, Cx->8 and C3->12 and 0 otherwise
// and g(x) = (x >> 4) & 3        8x->0 9x->1 Ax->2 Bx->3 Cx->0, etc.
//                                (no checking for illegal bytes)
// Here are example patterns of CP1252 converted to UTF-8 0/1/2 times. We want
// to detect two, so we can back-convert to one.
// zero one    two                 pattern
// ---- ------ ----------------    -----------------
// 81   C281   C382C281            C3->8x->C2->xx
// 98   CB9C   C38BC593            C3->8x->C5->xx
// C3   C383   C383C692            C3->8x->C6->xx
// C8   C388   C383CB86            C3->8x->CB->xx
// 83   C692   C386E28099          C3->8x->E2->xx->8x
// 80   E282AC C3A2E2809AC2AC      C3->A2->E2->xx->xx->Cx->xx
// 92   E28099 C3A2E282ACE284A2    C3->A2->E2->xx->xx->E2->xx->xx
//
// We also want to detect bare-byte extra UTF-8 conversions:
// zero one    two                 pattern
// ---- ------ ----------------    -----------------
// C3   C3     C383                C3->8x->C2->xx
// D3   D3     C393                C3->9x->C2->xx->C2->xx
// E3   E3     C3A3                C3->Ax->C2->xx->C2->xx->C2->xx
// F3   F3     C3B2                C3->Bx->C2->xx->C2->xx->C2->xx->C2->xx
//

/**
CP1252 => UTF8 => UTF8UTF8
80 => E282AC => C3A2E2809AC2AC
81 => C281 => C382C281
82 => E2809A => C3A2E282ACC5A1
83 => C692 => C386E28099
84 => E2809E => C3A2E282ACC5BE
85 => E280A6 => C3A2E282ACC2A6
86 => E280A0 => C3A2E282ACC2A0
87 => E280A1 => C3A2E282ACC2A1
88 => CB86 => C38BE280A0
89 => E280B0 => C3A2E282ACC2B0
8A => C5A0 => C385C2A0
8B => E280B9 => C3A2E282ACC2B9
8C => C592 => C385E28099
8D => C28D => C382C28D
8E => C5BD => C385C2BD
8F => C28F => C382C28F
90 => C290 => C382C290
91 => E28098 => C3A2E282ACCB9C
92 => E28099 => C3A2E282ACE284A2
93 => E2809C => C3A2E282ACC593
94 => E2809D => C3A2E282ACC29D
95 => E280A2 => C3A2E282ACC2A2
96 => E28093 => C3A2E282ACE2809C
97 => E28094 => C3A2E282ACE2809D
98 => CB9C => C38BC593
99 => E284A2 => C3A2E2809EC2A2
9A => C5A1 => C385C2A1
9B => E280BA => C3A2E282ACC2BA
9C => C593 => C385E2809C
9D => C29D => C382C29D
9E => C5BE => C385C2BE
9F => C5B8 => C385C2B8
A0 => C2A0 => C382C2A0
A1 => C2A1 => C382C2A1
A2 => C2A2 => C382C2A2
A3 => C2A3 => C382C2A3
A4 => C2A4 => C382C2A4
A5 => C2A5 => C382C2A5
A6 => C2A6 => C382C2A6
A7 => C2A7 => C382C2A7
A8 => C2A8 => C382C2A8
A9 => C2A9 => C382C2A9
AA => C2AA => C382C2AA
AB => C2AB => C382C2AB
AC => C2AC => C382C2AC
AD => C2AD => C382C2AD
AE => C2AE => C382C2AE
AF => C2AF => C382C2AF
B0 => C2B0 => C382C2B0
B1 => C2B1 => C382C2B1
B2 => C2B2 => C382C2B2
B3 => C2B3 => C382C2B3
B4 => C2B4 => C382C2B4
B5 => C2B5 => C382C2B5
B6 => C2B6 => C382C2B6
B7 => C2B7 => C382C2B7
B8 => C2B8 => C382C2B8
B9 => C2B9 => C382C2B9
BA => C2BA => C382C2BA
BB => C2BB => C382C2BB
BC => C2BC => C382C2BC
BD => C2BD => C382C2BD
BE => C2BE => C382C2BE
BF => C2BF => C382C2BF
C0 => C380 => C383E282AC
C1 => C381 => C383C281
C2 => C382 => C383E2809A
C3 => C383 => C383C692
C4 => C384 => C383E2809E
C5 => C385 => C383E280A6
C6 => C386 => C383E280A0
C7 => C387 => C383E280A1
C8 => C388 => C383CB86
C9 => C389 => C383E280B0
CA => C38A => C383C5A0
CB => C38B => C383E280B9
CC => C38C => C383C592
CD => C38D => C383C28D
CE => C38E => C383C5BD
CF => C38F => C383C28F
D0 => C390 => C383C290
D1 => C391 => C383E28098
D2 => C392 => C383E28099
D3 => C393 => C383E2809C
D4 => C394 => C383E2809D
D5 => C395 => C383E280A2
D6 => C396 => C383E28093
D7 => C397 => C383E28094
D8 => C398 => C383CB9C
D9 => C399 => C383E284A2
DA => C39A => C383C5A1
DB => C39B => C383E280BA
DC => C39C => C383C593
DD => C39D => C383C29D
DE => C39E => C383C5BE
DF => C39F => C383C5B8
E0 => C3A0 => C383C2A0
E1 => C3A1 => C383C2A1
E2 => C3A2 => C383C2A2
E3 => C3A3 => C383C2A3
E4 => C3A4 => C383C2A4
E5 => C3A5 => C383C2A5
E6 => C3A6 => C383C2A6
E7 => C3A7 => C383C2A7
E8 => C3A8 => C383C2A8
E9 => C3A9 => C383C2A9
EA => C3AA => C383C2AA
EB => C3AB => C383C2AB
EC => C3AC => C383C2AC
ED => C3AD => C383C2AD
EE => C3AE => C383C2AE
EF => C3AF => C383C2AF
F0 => C3B0 => C383C2B0
F1 => C3B1 => C383C2B1
F2 => C3B2 => C383C2B2
F3 => C3B3 => C383C2B3
F4 => C3B4 => C383C2B4
F5 => C3B5 => C383C2B5
F6 => C3B6 => C383C2B6
F7 => C3B7 => C383C2B7
F8 => C3B8 => C383C2B8
F9 => C3B9 => C383C2B9
FA => C3BA => C383C2BA
FB => C3BB => C383C2BB
FC => C3BC => C383C2BC
FD => C3BD => C383C2BD
FE => C3BE => C383C2BE
FF => C3BF => C383C2BF
**/

// Subscripted by <state, f(byte1) + g(byte2)>
// where f(x)= E2->4, C2/5/6/B->8 and C3->12 and 0 otherwise
// and g(x) = (x >> 4) & 3        8x->0 9x->1 Ax->2 Bx->3 Cx->0, etc.

// 81   C281   C382C281            C3->8x->C2->xx
// 98   CB9C   C38BC593            C3->8x->C5->xx
// C3   C383   C383C692            C3->8x->C6->xx
// C8   C388   C383CB86            C3->8x->CB->xx
//                                 [0]     [2]   [0]
// 83   C692   C386E28099          C3->8x->E2->xx->xx
//   odd_byte=0                    [0]     [2]       [0+]  odd_byte flipped
//   odd_byte=1                    [0+]    [2] [0]   [0]   odd_byte unflipped
// 80   E282AC C3A2E2809AC2AC      C3->A2->E2->xx->xx->Cx->xx
//   odd_byte=0                    [0]     [3]         [4]   [0+]
//   odd_byte=1                    [0+]    [3] [4]     [4]   [0]
// 92   E28099 C3A2E282ACE284A2    C3->A2->E2->xx->xx->E2->xx->xx
//   odd_byte=0                    [0]     [3]         [4] [0]   [0]
//   odd_byte=1                    [0+]    [3] [4]     [4]       [0+]
//
// When an E2xxxx sequence is encountered, we absorb the two bytes E2xx and flip
// the odd_byte state. If that goes from 0 to 1, the next pair is offset up
// by one byte, picking up the two bytes just after E2xxxx. If odd_byte goes
// from 1 to 0, the next two bytes picked up are the two bytes xxxx of E2xxxx.
// These are absorbed with no error in state 0 or state 4
//
// C3   C3     C383                C3->8x->C2->xx
// D3   D3     C393                C3->9x->C2->xx->C2->xx
// E3   E3     C3A3                C3->Ax->C2->xx->C2->xx->C2->xx
// F3   F3     C3B2                C3->Bx->C2->xx->C2->xx->C2->xx->C2->xx
// Counter3 for Fx Ex sequences is incremented at last C2

static const char kMiniUTF8UTF8State[8][16] = {
  // xxxx  E2xx     CXxx    C3xx
  //       8 9 a b  8 9 a b 8 9 a b
  {0,0,0,0,1,1,1,1, 1,1,1,1,2,2,3,5,},      // [0] looking for C38x/C3Ax/2020/8x8x, or err
  {0,0,0,0,1,1,1,1, 1,1,1,1,2,2,3,5,},      // [1] error, back to looking
  {1,1,1,1,0,0,0,0, 0,0,0,0,1,1,1,1,},      // [2] C38x looking for CXxx/E2xxxx
  //       + + + +                          //      E2xxxx flips odd_byte
  {1,1,1,1,4,4,4,4, 7,7,7,7,1,1,1,1,},      // [3] C3Ax looking for E2xx or C2xxC2xx
  //       + + + +                          //      E2xxxx flips odd_byte
  {4,4,4,4,0,0,0,0, 0,0,0,0,1,1,1,1,},      // [4] C3AxE2xx-- looking for C2xx/E2xxxx
  //       + + + +                          //      E2xxxx flips odd_byte
  {1,1,1,1,1,1,1,1, 6,6,6,6,1,1,1,1,},      // [5] C3Bx -- looking for C2xxC2xxC2xx
  {1,1,1,1,1,1,1,1, 7,7,7,7,1,1,1,1,},      // [6] C3Bx -- looking for C2xxC2xx
  {1,1,1,1,1,1,1,1, 0,0,0,0,1,1,1,1,},      // [7] C3Bx -- looking for C2xx
};
// Counter to increment: 0-don'tcare 1-error 2-good_2B 3-good_3B 4-good_4B
static const char kMiniUTF8UTF8Count[8][16] = {
  // xxxx  E2xx     C2Xx    C3xx
  //       8 9 a b  8 9 a b 8 9 a b
  {0,0,0,0,1,1,1,1, 1,1,1,1,0,0,0,0,},      // [0] looking for C38x/C3Ax/2020/8x8x, or err
  {0,0,0,0,1,1,1,1, 1,1,1,1,0,0,0,0,},      // [1] error, back to looking
  {1,1,1,1,3,3,3,3, 2,2,2,2,1,1,1,1,},      // [2] C38x looking for CXxx/E2xxxx
  //       + + + +                          //      E2xxxx flips odd_byte
  {1,1,1,1,0,0,0,0, 0,0,0,0,1,1,1,1,},      // [3] C3Ax looking for E2xx
  //       + + + +                          //      E2xxxx flips odd_byte
  {1,1,1,1,4,4,4,4, 4,4,4,4,1,1,1,1,},      // [4] C3AxE2xx-- looking for C2xx/E2xxxx
  //       + + + +                          //      E2xxxx flips odd_byte
  {1,1,1,1,1,1,1,1, 0,0,0,0,1,1,1,1,},      // [5] C3Bx -- looking for C2xxC2xxC2xx
  {1,1,1,1,1,1,1,1, 0,0,0,0,1,1,1,1,},      // [6] C3Bx -- looking for C2xxC2xx
  {1,1,1,1,1,1,1,1, 3,3,3,3,1,1,1,1,},      // [7] C3Bx -- looking for C2xx
};

static const char kMiniUTF8UTF8Odd[8][16] = {
  // xxxx  E2xx     C2Xx    C3xx
  //       8 9 a b  8 9 a b 8 9 a b
  {0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,},      // [0] looking for C38x/C3Ax/2020/8x8x, or err
  {0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,},      // [1] error, back to looking
  {0,0,0,0,1,1,1,1, 0,0,0,0,0,0,0,0,},      // [2] C38x looking for CXxx/E2xxxx
  //       + + + +                          //      E2xxxx flips odd_byte
  {0,0,0,0,1,1,1,1, 0,0,0,0,0,0,0,0,},      // [3] C3Ax looking for E2xx
  //       + + + +                          //      E2xxxx flips odd_byte
  {0,0,0,0,1,1,1,1, 0,0,0,0,0,0,0,0,},      // [4] C3AxE2xx-- looking for C2xx/E2xxxx
  //       + + + +                          //      E2xxxx flips odd_byte
  {0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,},      // [5] C3Bx -- looking for C2xxC2xxC2xx
  {0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,},      // [6] C3Bx -- looking for C2xxC2xx
  {0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,},      // [7] C3Bx -- looking for C2xx
};

// Turn a pair of bytes into the subscript for UTF8UTF8 tables above
int UTF88Sub(char s0, char s1) {
  int sub = (s1 >> 4) & 0x03;
  auto u0 = static_cast<uint8>(s0);
  if (u0 == 0xc3) {
    sub += 12;
  } else if ((u0 & 0xf0) == 0xc0) {
    if ((u0 == 0xc2) || (u0 == 0xc5) || (u0 == 0xc6) || (u0 == 0xcb)) {
      sub += 8;
    }
  } else if (u0 == 0xe2) {
    sub += 4;
  }
  return sub;
}





// Default probability for an encoding rankedencoding
// Based on a scan of 55M web pages
// These values are 255 - log base 2**1/10 (occurrences / total)
// Large values are most likely. This the reverse of some Google code
// 255 = 1.0, 245 = 1/2, 235 = 1/4, 15 = 1/2**24, 0 = 0 (< 1/50M)
//
// TODO change this to be per encoding, not permuted
//


// Support function for unit test program
// Return ranked encoding corresponding to enc
// (also exported to compact_enc_det_text.cc)
int CompactEncDet::BackmapEncodingToRankedEncoding(Encoding enc) {
  for (int i = 0; i < NUM_RANKEDENCODING; ++i) {
    if (kMapToEncoding[i] == enc) {
      return i;
    }
  }
  return -1;
}


string DecodeActive(uint32 active) {
  string temp("");
  if (active & kBinaryActive) {
    temp.append("Binary ");
  }
  if (active & kUTF1632Active) {
    temp.append("UTF1632 ");
  }
  if (active & kUTF8UTF8Active) {
    temp.append("UTF8UTF8 ");
  }
  if (active & kUTF8Active) {
    temp.append("UTF8 ");
  }
  if (active & kIso2022Active) {
    temp.append("Iso2022 ");
  }
  if (active & kHzActive) {
    temp.append("Hz ");
  }
  if (active & kUTF7Active) {
    temp.append("UTF7A ");
  }
  if (active & kSevenBitActive) {
    temp.append("SevenBit ");
  }
  if (active & kIsIndicCode) {
    temp.append("Indic ");
  }
  if (active & kHighAlphaCode) {
    temp.append("HighAlpha ");
  }
  if (active & kHighAccentCode) {
    temp.append("HighAccent ");
  }
  if (active & kEUCJPActive) {
    temp.append("EUCJP ");
  }
  return temp;
}

static inline bool SevenBitEncoding(int enc) {
  return ((kSpecialMask[enc] & kSevenBitActive) != 0);
}
static inline bool TwoByteEncoding(int enc) {
  return ((kSpecialMask[enc] & kTwobyteCode) != 0);
}
static inline bool IndicEncoding(int enc) {
  return ((kSpecialMask[enc] & kIsIndicCode) != 0);
}
static inline bool HighAlphaEncoding(int enc) {
  return ((kSpecialMask[enc] & kHighAlphaCode) != 0);
}
static inline bool HighAccentEncoding(int enc) {
  return ((kSpecialMask[enc] & kHighAccentCode) != 0);
}


static inline bool AnyActive(DetectEncodingState* destatep) {
  return (destatep->active_special != 0);
}
static inline bool SevenBitActive(DetectEncodingState* destatep) {
  return (destatep->active_special & kSevenBitActive) != 0;
}
static inline bool HzActive(DetectEncodingState* destatep) {
  return (destatep->active_special & kHzActive) != 0;
}
static inline bool Iso2022Active(DetectEncodingState* destatep) {
  return (destatep->active_special & kIso2022Active) != 0;
}
static inline bool UTF8Active(DetectEncodingState* destatep) {
  return (destatep->active_special & kUTF8Active) != 0;
}
static inline bool UTF8UTF8Active(DetectEncodingState* destatep) {
  return (destatep->active_special & kUTF8UTF8Active) != 0;
}
static inline bool UTF1632Active(DetectEncodingState* destatep) {
  return (destatep->active_special & kUTF1632Active) != 0;
}
static inline bool BinaryActive(DetectEncodingState* destatep) {
  return (destatep->active_special & kBinaryActive) != 0;
}
static inline bool UTF7OrHzActive(DetectEncodingState* destatep) {
  return (destatep->active_special & (kHzActive + kUTF7Active)) != 0;
}
static inline bool EUCJPActive(DetectEncodingState* destatep) {
  return ((destatep->active_special & kEUCJPActive) != 0);
}
static inline bool OtherActive(DetectEncodingState* destatep) {
  return (destatep->active_special & (kIso2022Active + kBinaryActive +
                                      kUTF8Active + kUTF8UTF8Active +
                                      kUTF1632Active + kEUCJPActive)) != 0;
}


static inline bool CEDFlagRescanning(CEDInternalFlags flags) {
  return (flags & kCEDRescanning) != 0;
}

static inline bool CEDFlagForceTags(CEDInternalFlags flags) {
  return (flags & kCEDForceTags) != 0;
}


static inline int maxint(int a, int b) {return (a > b) ? a : b;}
static inline int minint(int a, int b) {return (a < b) ? a : b;}

static inline const char* MyRankedEncName(int r_enc) {
  return MyEncodingName(kMapToEncoding[r_enc]);
}


// Only for debugging. not thread safe
static const int kPsSourceWidth = 32;
static int pssourcenext = 0;    // debug only. not threadsafe. dump only >= this
static int pssourcewidth = 0;   // debug only.
static char* pssource_mark_buffer = nullptr;
int next_do_src_line;
int do_src_offset[16];


void PsSourceInit(int len) {
   pssourcenext = 0;
   pssourcewidth = len;
   delete[] pssource_mark_buffer;
   // Allocate 2 Ascii characters per input byte
   pssource_mark_buffer = new char[(pssourcewidth * 2) + 8];  // 8 = overscan
   memset(pssource_mark_buffer, ' ', pssourcewidth * 2);
   memset(pssource_mark_buffer + (pssourcewidth * 2), '\0', 8);

   next_do_src_line = 0;
   memset(do_src_offset, 0, sizeof(do_src_offset));
}

void PsSourceFinish() {
  // Print preceding mark buffer
  int j = (pssourcewidth * 2) - 1;
  while ((0 <= j) && (pssource_mark_buffer[j] == ' ')) {--j;}   // trim
  pssource_mark_buffer[j + 1] = '\0';
  fprintf(stderr, "(      %s) do-src\n", pssource_mark_buffer);
  memset(pssource_mark_buffer, ' ', pssourcewidth * 2);
  memset(pssource_mark_buffer + (pssourcewidth * 2), '\0', 8);

  delete[] pssource_mark_buffer;
  pssource_mark_buffer = nullptr;
}

// Dump aligned len bytes src... if not already dumped
void PsSource(const uint8* src, const uint8* isrc, const uint8* srclimit) {
  auto offset = static_cast<int>(src - isrc);
  offset -= (offset % pssourcewidth);     // round down to multiple of len bytes
  if (offset < pssourcenext) {
    return;
  }
  pssourcenext = offset + pssourcewidth;  // Min offset for next dump

  // Print preceding mark buffer
  int j = (pssourcewidth * 2) - 1;
  while ((0 <= j) && (pssource_mark_buffer[j] == ' ')) {--j;}   // trim
  pssource_mark_buffer[j + 1] = '\0';
  fprintf(stderr, "(      %s) do-src\n", pssource_mark_buffer);
  memset(pssource_mark_buffer, ' ', pssourcewidth * 2);
  memset(pssource_mark_buffer + (pssourcewidth * 2), '\0', 8);

  // Print source bytes
  const uint8* src_aligned = isrc + offset;
  auto length = static_cast<int>(srclimit - src_aligned);
  length = minint(pssourcewidth, length);

  fprintf(stderr, "(%05x ", offset);
  for (int i = 0; i < length; ++i) {
    char c = src_aligned[i];
    if (c == '\n') {c = ' ';}
    if (c == '\r') {c = ' ';}
    if (c == '\t') {c = ' ';}
    if (c == '(') {
      fprintf(stderr, "%s", "\\( ");
    } else if (c == ')') {
      fprintf(stderr, "%s", "\\) ");
    } else if (c == '\\') {
      fprintf(stderr, "%s", "\\\\ ");
    } else if ((0x20 <= c) && (c <= 0x7e)) {
      fprintf(stderr, "%c ", c);
    } else {
      fprintf(stderr, "%02x", c);
    }
  }
  fprintf(stderr, ") do-src\n");
  // Remember which source offsets are where, mod 16
  do_src_offset[next_do_src_line & 0x0f] = offset;
  ++next_do_src_line;
}

// Mark bytes in just-previous source bytes
void PsMark(const uint8* src, int len, const uint8* isrc, int weightshift) {
  auto offset = static_cast<int>(src - isrc);
  offset = (offset % pssourcewidth);     // mod len bytes
  char mark = (weightshift == 0) ? '-' : 'x';

  pssource_mark_buffer[(offset * 2)] = '=';
  pssource_mark_buffer[(offset * 2) + 1] = '=';
  for (int i = 1; i < len; ++i) {
    pssource_mark_buffer[(offset + i) * 2] = mark;
    pssource_mark_buffer[((offset + i) * 2) + 1] = mark;
  }
}


// Highlight trigram bytes in just-previous source bytes
// Unfortunately, we have to skip back N lines since source was printed for
// up to 8 bigrams before we get here. Match on src+1 to handle 0/31 better
void PsHighlight(const uint8* src, const uint8* isrc, int trigram_val, int n) {
  auto offset = src ? static_cast<int>((src + 1) - isrc) :
                      static_cast<int>((const uint8*)1 - isrc);
  int offset32 = (offset % pssourcewidth);    // mod len bytes
  offset -= offset32;                         // round down to multiple of len bytes

  for (int i = 1; i <= 16; ++i) {
    if (do_src_offset[(next_do_src_line - i) & 0x0f] == offset) {
      fprintf(stderr, "%d %d %d do-highlight%d\n",
              i, offset32 - 1, trigram_val, n);
      break;
    }
  }
}


void InitDetectEncodingState(DetectEncodingState* destatep) {
  destatep->initial_src = nullptr;       // Filled in by caller
  destatep->limit_src = nullptr;
  destatep->prior_src = nullptr;
  destatep->last_pair = nullptr;

  destatep->debug_data = nullptr;
  destatep->next_detail_entry = 0;

  destatep->done = false;
  destatep->reliable = false;
  destatep->hints_derated = false;
  //destatep->declared_enc_1 init in ApplyHints
  //destatep->declared_enc_2 init in ApplyHints
  destatep->prune_count = 0;

  destatep->trigram_highwater_mark = 0;
  destatep->looking_for_latin_trigrams = false;
  destatep->do_latin_trigrams = false;

  // Miscellaneous state variables for difficult encodings
  destatep->binary_quadrants_count = 0;
  destatep->binary_8x4_count = 0;
  destatep->binary_quadrants_seen = 0;
  destatep->binary_8x4_seen = 0;
  destatep->utf7_starts = 0;
  destatep->prior_utf7_offset = 0;
  destatep->next_utf8_ministate = 0;
  //for (int & i : destatep->utf8_minicount) {i = 0;}
  std::fill(destatep->utf8_minicount, destatep->utf8_minicount + UTF8_ARR_CNT, 0);
  destatep->next_utf8utf8_ministate = 0;
  destatep->utf8utf8_odd_byte = 0;
  //for (int & i : destatep->utf8utf8_minicount) {i = 0;}
  std::fill(destatep->utf8utf8_minicount, destatep->utf8utf8_minicount + UTF8_ARR_CNT, 0);
  destatep->next_2022_state = SOSI_NONE;
  destatep->next_hz_state = SOSI_NONE;
  destatep->next_eucjp_oddphase = false;
  //for (int & i : destatep->byte32_count) {i = 0;}
  std::fill(destatep->byte32_count, destatep->byte32_count + BYTE32_ARR_CNT, 0);
  destatep->active_special = 0xffffffff;
  destatep->tld_hint = UNKNOWN_ENCODING;
  destatep->http_hint = UNKNOWN_ENCODING;
  destatep->meta_hint = UNKNOWN_ENCODING;
  destatep->bom_hint = UNKNOWN_ENCODING;
  destatep->top_rankedencoding = 0;         // ASCII [seven-bit] is the default
  destatep->second_top_rankedencoding = 0;  // ASCII [seven-bit] is the default
  destatep->top_prob = -1;
  destatep->second_top_prob = -1;
  // This is wide for first pruning, shrinks for 2nd and later
  destatep->prune_difference = kInititalPruneDifference;

  destatep->next_prior_bigram = 0;
  destatep->prior_bigram[0] = -1;
  destatep->prior_bigram[1] = -1;
  destatep->prior_bigram[2] = -1;
  destatep->prior_bigram[3] = -1;

  destatep->prior_binary[0] = -1;

  // Initialize with all but Indic encodings, which we never detect
  int k = 0;
  for (int rankedencoding = 0;
        rankedencoding < NUM_RANKEDENCODING;
        rankedencoding++) {
    Encoding enc = kMapToEncoding[rankedencoding];
    if (!IndicEncoding(enc)) {
      destatep->rankedencoding_list[k++] = rankedencoding;
    }
  }
  destatep->rankedencoding_list_len = k;

  // This is where all the action is
  memset(destatep->enc_prob, 0, sizeof(destatep->enc_prob));

  memset(destatep->hint_prob, 0, sizeof(destatep->hint_prob));
  memset(destatep->hint_weight, 0, sizeof(destatep->hint_weight));

  destatep->prior_interesting_pair[AsciiPair] = 0;
  destatep->prior_interesting_pair[OtherPair] = 0;
  destatep->next_interesting_pair[AsciiPair] = 0;
  destatep->next_interesting_pair[OtherPair] = 0;
  // interesting_pairs/offsets/weightshifts not initialized; no need
}

// Probability strings are uint8, with zeros removed via simple run-length:
//  (<skip-take byte> <data bytes>)*
// skip-take:
//  00  end
//  x0  skip 16 x locations, take 0 data values
//  xy  skip x locations, take y data values
// Multiply all the incoming values by 3 to account for 3x unigram sums
//
// {{0x77,0x69,0x6e,0x64,0x31,0x32,0x35,0x35,
//   0x01,0xc2,0x10,0x41,0xfe,0x71,0xba,0x00,}}, // "wind1255"
//
// Weight is 0..100 percent
//
// Returns subscript of largest (most probable) value
//


//  {{0x6e,0x6c,0x5f,0x5f, 0x05,0xb2,0xae,0xa0,0x32,0xa1,0x36,0x31,0x42,0x39,0x3b,0x33,0x45,0x11,0x6f,0x00,}}, // "nl__"
//        // ASCII-7-bit=178  Latin1=174  UTF8=160  GB=50  CP1252=161  BIG5=49  Latin2=66  CP1251=57  CP1256=59  CP1250=51  Latin5=69  ISO-8859-15=111  [top ASCII-7-bit]
int ApplyCompressedProb(const unsigned char* iprob, int len,
                         int weight, DetectEncodingState* destatep) {
  int* dst = &destatep->enc_prob[0];
  int* dst2 = &destatep->hint_weight[0];
  const auto* prob = reinterpret_cast<const uint8*>(iprob);
  const uint8* problimit = prob + len;

  int largest = -1;
  int subscript_of_largest = 0;

  // Continue with first byte and subsequent ones
  while (prob < problimit) {
    int skiptake = *prob++;
    ptrdiff_t skip = (skiptake & 0xf0) >> 4;
    int take = skiptake & 0x0f;
    if (skiptake == 00) {
      break;
    } else if (take == 0) {
      dst += (skip << 4);
      dst2 += (skip << 4);
    } else {
      dst += skip;    // Normal case
      dst2 += skip;  // Normal case
      for (int i = 0; i < take; i++) {
        int enc = static_cast<int>(dst - &destatep->enc_prob[0]) + i;
        if (largest < prob[i]) {
          largest = prob[i];
          subscript_of_largest = enc;
        }

        int increment = prob[i] * 3;    // The actual increment

        // Do maximum of previous hints plus this new one
        if (weight > 0) {
          increment = (increment * weight)  / 100;
          dst[i] = maxint(dst[i], increment);
          dst2[i] = 1;              // New total weight
        }
      }
      prob += take;
      dst += take;
      dst2 += take;
    }
  }
  return subscript_of_largest;
}


// Returns subscript of largest (most probable) value [for unit test]
int TopCompressedProb(const unsigned char* iprob, int len) {
  const auto* prob = reinterpret_cast<const uint8*>(iprob);
  const uint8* problimit = prob + len;
  int next_prob_sub = 0;
  int topprob = 0;
  int toprankenc = 0;

  while (prob < problimit) {
    int skiptake = *prob++;
    int skip = (skiptake & 0xf0) >> 4;
    int take = skiptake & 0x0f;
    if (skiptake == 0) {
      break;
    } else if (take == 0) {
      next_prob_sub += (skip << 4);
    } else {
      next_prob_sub += skip;    // Normal case
      for (int i = 0; i < take; i++) {
        if (topprob < prob[i]) {
          topprob = prob[i];
          toprankenc = next_prob_sub + i;
        }
      }
      prob += take;
      next_prob_sub += take;
    }
  }
  return toprankenc;
}


// Find subscript of matching key in first 8 bytes of sorted hint array, or -1
int HintBinaryLookup8(const HintEntry* hintprobs, int hintprobssize,
                     const char* norm_key) {
  // Key is always in range [lo..hi)
  int lo = 0;
  int hi = hintprobssize;
  while (lo < hi) {
    int mid = (lo + hi) >> 1;
    int comp = memcmp(&hintprobs[mid].key_prob[0], norm_key, 8);
    if (comp < 0) {
      lo = mid + 1;
    } else if (comp > 0) {
      hi = mid;
    } else {
      return mid;
    }
  }
  return -1;
}

// Find subscript of matching key in first 4 bytes of sorted hint array, or -1
int HintBinaryLookup4(const HintEntry* hintprobs, int hintprobssize,
                     const char* norm_key) {
  // Key is always in range [lo..hi)
  int lo = 0;
  int hi = hintprobssize;
  while (lo < hi) {
    int mid = (lo + hi) >> 1;
    int comp = memcmp(&hintprobs[mid].key_prob[0], norm_key, 4);
    if (comp < 0) {
      lo = mid + 1;
    } else if (comp > 0) {
      hi = mid;
    } else {
      return mid;
    }
  }
  return -1;
}

static inline void Boost(DetectEncodingState* destatep, int r_enc, int boost) {
  destatep->enc_prob[r_enc] += boost;
}

static inline void Whack(DetectEncodingState* destatep, int r_enc, int whack) {
  destatep->enc_prob[r_enc] -= whack;
}

// Apply initial probability hint based on top level domain name
// Weight is 0..100 percent
// Return 1 if name match found
int ApplyTldHint(const char* url_tld_hint, int weight,
                  DetectEncodingState* destatep) {
  if (url_tld_hint[0] == '~') {
    return 0;
  }
  string normalized_tld = MakeChar4(string(url_tld_hint));
  int n = HintBinaryLookup4(kTLDHintProbs, kTLDHintProbsSize,
                           normalized_tld.c_str());
  if (n >= 0) {
    // TLD is four bytes, probability table is ~12 bytes
    int best_sub = ApplyCompressedProb(&kTLDHintProbs[n].key_prob[kMaxTldKey],
                                       kMaxTldVector, weight, destatep);
    // Never boost ASCII7; do CP1252 instead
    if (best_sub == F_ASCII_7_bit) {best_sub = F_CP1252;}
    destatep->declared_enc_1 = best_sub;
    if (destatep->debug_data != nullptr) {
      // Show TLD hint
      SetDetailsEncProb(destatep, 0, best_sub, url_tld_hint);
    }
    return 1;
  }
  return 0;
}

// Apply initial probability hint based on charset= name
// Weight is 0..100 percent
// Return 1 if name match found
int ApplyCharsetHint(const char* charset_hint, int weight,
                      DetectEncodingState* destatep) {
  if (charset_hint[0] == '~') {
    return 0;
  }
  string normalized_charset = MakeChar44(string(charset_hint));
  int n = HintBinaryLookup8(kCharsetHintProbs, kCharsetHintProbsSize,
                           normalized_charset.c_str());
  if (n >= 0) {
    // Charset is eight bytes, probability table is ~eight bytes
    int best_sub = ApplyCompressedProb(&kCharsetHintProbs[n].key_prob[kMaxCharsetKey],
                                       kMaxCharsetVector, weight, destatep);
    // Never boost ASCII7; do CP1252 instead
    if (best_sub == F_ASCII_7_bit) {best_sub = F_CP1252;}
    destatep->declared_enc_1 = best_sub;

    // If first explicitly declared charset is confusable with Latin1/1252, put
    // both declared forms in declared_enc_*, displacing Latin1/1252.
    // This avoids a bit of Latin1 creep.
    // Also boost the declared encoding and its pair
    // TODO: This should all be folded into postproc-enc-detect.cc
    if ((destatep->http_hint == UNKNOWN_ENCODING) &&
        (destatep->meta_hint == UNKNOWN_ENCODING)) {
      // This is the first charset=hint
      switch (best_sub) {
      case F_Latin2:            // 8859-2 Latin2, east euro
        destatep->declared_enc_2 = F_CP1250;
        Boost(destatep, F_Latin2, kGentleOnePair);
        Boost(destatep, F_CP1250, kGentleOnePair);
        break;
      case F_CP1250:
        destatep->declared_enc_2 = F_Latin2;
        Boost(destatep, F_Latin2, kGentleOnePair);
        Boost(destatep, F_CP1250, kGentleOnePair);
        break;

      case F_Latin3:            // 8859-3 Latin3, south euro, Esperanto
        destatep->declared_enc_2 = F_ASCII_7_bit;
        Boost(destatep, F_Latin3, kGentleOnePair);
        break;

      case F_Latin4:            // 8859-4 Latin4, north euro
        destatep->declared_enc_2 = F_ASCII_7_bit;
        Boost(destatep, F_Latin4, kGentleOnePair);
        break;

      case F_ISO_8859_5:        // 8859-5 Cyrillic
        destatep->declared_enc_2 = F_ASCII_7_bit;       // Don't boost 1251
        Boost(destatep, F_ISO_8859_5, kGentleOnePair);  // (too different)
        break;
      case F_CP1251:
        destatep->declared_enc_2 = F_ASCII_7_bit;       // Don't boost -5
        Boost(destatep, F_CP1251, kGentleOnePair);      // (too different)
        break;

      case F_Arabic:            // 8859-6 Arabic
        destatep->declared_enc_2 = F_CP1256;
        Boost(destatep, F_Arabic, kGentleOnePair);
        Boost(destatep, F_CP1256, kGentleOnePair);
        break;
      case F_CP1256:
        destatep->declared_enc_2 = F_Arabic;
        Boost(destatep, F_Arabic, kGentleOnePair);
        Boost(destatep, F_CP1256, kGentleOnePair);
        break;

      case F_Greek:             // 8859-7 Greek
        destatep->declared_enc_2 = F_CP1253;
        Boost(destatep, F_Greek, kGentleOnePair);
        Boost(destatep, F_CP1253, kGentleOnePair);
        break;
      case F_CP1253:
        destatep->declared_enc_2 = F_Greek;
        Boost(destatep, F_Greek, kGentleOnePair);
        Boost(destatep, F_CP1253, kGentleOnePair);
        break;

      case F_Hebrew:            // 8859-8 Hebrew
        destatep->declared_enc_2 = F_CP1255;
        Boost(destatep, F_Hebrew, kGentleOnePair);
        Boost(destatep, F_CP1255, kGentleOnePair);
        break;
      case F_CP1255:
        destatep->declared_enc_2 = F_Hebrew;
        Boost(destatep, F_Hebrew, kGentleOnePair);
        Boost(destatep, F_CP1255, kGentleOnePair);
        break;

      case F_Latin5:            // 8859-9 Latin5, Turkish
        destatep->declared_enc_2 = F_ASCII_7_bit;       // Don't boost 1254
        Boost(destatep, F_Latin5, kGentleOnePair);      // (too different)
        break;
      case F_CP1254:
        destatep->declared_enc_2 = F_ASCII_7_bit;       // Don't boost Latin5
        Boost(destatep, F_CP1254, kGentleOnePair);      // (too different)
        break;

      case F_Latin6:            // 8859-10 Latin6, Nordic
        destatep->declared_enc_2 = F_ASCII_7_bit;
        Boost(destatep, F_Latin6, kGentleOnePair);
        break;

      case F_ISO_8859_11:       // 8859-11 Thai,
        destatep->declared_enc_2 = F_CP874;
        Boost(destatep, F_ISO_8859_11, kGentleOnePair);
        Boost(destatep, F_CP874, kGentleOnePair);
        break;
      case F_CP874:
        destatep->declared_enc_2 = F_ISO_8859_11;
        Boost(destatep, F_ISO_8859_11, kGentleOnePair);
        Boost(destatep, F_CP874, kGentleOnePair);
        break;

      case F_ISO_8859_13:       // 8859-13 Latin7, Baltic
        destatep->declared_enc_2 = F_CP1257;
        Boost(destatep, F_ISO_8859_13, kGentleOnePair);
        Boost(destatep, F_CP1257, kGentleOnePair);
        break;
      case F_CP1257:
        destatep->declared_enc_2 = F_ISO_8859_13;
        Boost(destatep, F_ISO_8859_13, kGentleOnePair);
        Boost(destatep, F_CP1257, kGentleOnePair);
        break;

      case F_ISO_8859_15:       // 8859-15 Latin9, Latin0, Euro-ized Latin1
        destatep->declared_enc_2 = F_ASCII_7_bit;
        Boost(destatep, F_ISO_8859_15, kGentleOnePair);
        break;


        // Greek all-caps is confusable with KOI8x all-lower and Hebrew.
        // This turns some Greek documents into Cyrillic, etc. by mistake.
        // Greek and Hebrew are boosted explicitly above; do KOI8x here.
        // Boosting the declared encodingmakes it harder for the wrong one to
        // creep up.
      case F_KOI8R:
        Boost(destatep, F_KOI8R, kGentleOnePair);
        break;
      case F_KOI8U:
        Boost(destatep, F_KOI8U, kGentleOnePair);
        break;

      default:
        break;
      }
    }

    if (destatep->debug_data != nullptr) {
      // Show charset hint
      SetDetailsEncProb(destatep, 0, best_sub, charset_hint);
    }

    //
    // Some fix-ups for the declared encodings
    //

    // If non-UTF8, non-Latin1/1252 encoding declared, disable UTF8 combos
    // TODO: This should all be folded into postproc-enc-detect.cc
    if ((best_sub != F_UTF8) &&
        (best_sub != F_Latin1) &&
        (best_sub != F_CP1252)) {
      Whack(destatep, F_UTF8UTF8, kBadPairWhack * 4);         // demote
    }

    // Latin2 and CP1250 differ in the overlap part, such as B1 or B9
    // The initial probabilites for charset=Latin2 explicitly put CP1250
    // down twice as far as normal, and vice versa. This is done in
    // postproc-enc-detect.cc

    // If charset=user-defined, treat as Binary --
    // we can safely only do low ASCII, might be Indic
    if (normalized_charset.substr(0,4) == "user") {
      Boost(destatep, F_BINARY, kBoostInitial * kStrongBinary);
    }

    return 1;
  }
  return 0;
}

// Apply initial probability hint based on caller-supplied encoding
// Negative hint whacks ~encoding, non-negative boosts encoding
//
// Negative hints are an experiment to see if they might be useful.
// Not operator used instead of unary minus to allow specifying not-zero
int ApplyEncodingHint(const int encoding_hint, int weight,
                       DetectEncodingState* destatep) {
  auto enc_hint = static_cast<Encoding>((encoding_hint < 0) ?
                                            ~encoding_hint : encoding_hint);
  // Map to the right internal subscript
  int rankedenc_hint = CompactEncDet::BackmapEncodingToRankedEncoding(enc_hint);

  // I'm not sure how strong this hint should be. Weight 100% = 1 bigram
  int increment = (kBoostOnePair * weight) / 100;

  if (encoding_hint < 0) {
    destatep->enc_prob[rankedenc_hint] -= increment;
  } else {
    destatep->enc_prob[rankedenc_hint] += increment;
  }

  if (destatep->debug_data != nullptr) {
    // Show encoding hint
    SetDetailsEncProb(destatep, 0, -1, MyEncodingName(enc_hint));
  }
  return 1;
}

// Apply initial probability hint based on user interface language
// Weight is 0..100 percent
// Return 1 if name match found
int ApplyUILanguageHint(const Language language_hint,
                        int weight, DetectEncodingState* destatep) {
  if (language_hint == UNKNOWN_LANGUAGE) {
    return 0;
  }
  string normalized_lang = MakeChar8(LanguageName(language_hint));
  int n = HintBinaryLookup8(kLangHintProbs, kLangHintProbsSize,
                           normalized_lang.c_str());
  if (n >= 0) {
    // Language is eight bytes, probability table is ~eight bytes
    int best_sub = ApplyCompressedProb(&kLangHintProbs[n].key_prob[kMaxLangKey],
                                       kMaxLangVector, weight, destatep);
    // Never boost ASCII7; do CP1252 instead
    if (best_sub == F_ASCII_7_bit) {best_sub = F_CP1252;}
    destatep->declared_enc_1 = best_sub;
    if (destatep->debug_data != nullptr) {
      // Show language hint
      SetDetailsEncProb(destatep, 0, best_sub, normalized_lang.c_str());
    }
    return 1;
  }
  return 0;
}

// Apply initial probability hint based on corpus type (web, email, etc)
// Return 1 if name match found
int ApplyDefaultHint(const CompactEncDet::TextCorpusType corpus_type,
                      DetectEncodingState* destatep) {

  for (int i = 0; i < NUM_RANKEDENCODING; i++) {
    // Set the default probability
    destatep->enc_prob[i] = kDefaultProb[i] * 3;
    // Deliberately set 2022 seven-bit encodings to zero,
    // so we can look for actual use
    // TODO: This should all be folded into postproc-enc-detect.cc
    if (SevenBitEncoding(kMapToEncoding[i])) {
      destatep->enc_prob[i] = 0;
    }
  }

  //  A little corpus distinction
  switch (corpus_type) {
  case CompactEncDet::WEB_CORPUS:
  case CompactEncDet::XML_CORPUS:
    // Allow double-converted UTF-8 to start nearly equal to normal UTF-8
    destatep->enc_prob[F_UTF8UTF8] =
      destatep->enc_prob[F_UTF8] - kSmallInitDiff;
  break;
  case CompactEncDet::QUERY_CORPUS:
  case CompactEncDet::EMAIL_CORPUS:
  default:
    break;
  }

  if (FLAGS_demo_nodefault) {
    // Demo, make initial probs all zero
    std::fill(destatep->enc_prob, destatep->enc_prob + NUM_RANKEDENCODING, 0);
  }

  if (destatep->debug_data != nullptr) {
    // Show default hint
    SetDetailsEncProb(destatep, 0, -1, "Default");
  }
  return 1;
}



// Do reverse search for c in [str..str+len)
// Note: initial pointer is to FRONT of string, not back
const char* MyMemrchr(const char* str, char c, size_t len) {
  const char* ret = str + len;
  while (str <= --ret) {
    if (*ret == c) {return ret;}
  }
  return nullptr;
}


// Minimum real URL is 11 bytes: "http://a.bc" -- shorter is assumed to be TLD
// Now that we are no longer trying to do Indic font-based encodigns, we
// don't need the full URL and can go back to simple TLD. This test remains for
// backwards compatility with any caller using full URL.
static const int kMinURLLength = 11;

// Extract TLD from a full URL or just a TLD
// Return hostname and length if a full URL
void ExtractTLD(const char* url_hint, char* tld_hint, int tld_hint_len,
                const char** ret_host_start, int* ret_host_len) {
  // url_hint can either be a full URL (preferred) or just top-level domain name
  // Extract the TLD from a full URL and use it for
  // a normal TLD hint

  strncpy_s(tld_hint, tld_hint_len, "~", tld_hint_len);
  tld_hint[tld_hint_len - 1] = '\0';
  *ret_host_start = nullptr;
  *ret_host_len = 0;

  int url_len = (url_hint != nullptr) ? static_cast<int>(strlen(url_hint)) : 0;
  if (url_len == 0) {
    // Empty TLD
    return;
  }

  // Minimum real URL is 11 bytes: "http://a.bc" -- shorter is assumed to be TLD
  if (kMinURLLength <= url_len) {
    // See if it really is a URL
    const char* first_slash = strchr(url_hint, '/');
    if ((first_slash != nullptr) && (first_slash != url_hint) &&
        (first_slash[-1] == ':') && (first_slash[1] == '/') &&
        (memrchr(url_hint, '.', first_slash - url_hint) == nullptr)) {
      // We found :// and no dot in front of it, so declare a real URL

      const char* hostname_start = first_slash + 2;
      const char* hostname_end = strchr(hostname_start, '/');
      if (hostname_end == nullptr) {
        // No slash; end is first byte off end of the URL string
        hostname_end = url_hint + url_len;
      }
      size_t hostname_len = hostname_end - hostname_start;
      const auto* port_start =
        (const char*)memchr(hostname_start, ':', hostname_len);
      if (port_start != nullptr) {
        // Port; shorten hostname
        hostname_end = port_start;
        hostname_len = hostname_end - hostname_start;
      }

      const char* tld_start = MyMemrchr(hostname_start, '.', hostname_len);
      if (tld_start != nullptr) {
        // Remember the TLD we just found
        auto tld_len = static_cast<int>(hostname_start + hostname_len - tld_start - 1);
        if (tld_len > (tld_hint_len - 1)) {
          tld_len = tld_hint_len - 1;
        }
        memcpy(tld_hint, tld_start + 1, tld_len);
        tld_hint[tld_len] = '\0';
      }
      *ret_host_start = hostname_start;
      *ret_host_len = (int)hostname_len;
      return;
    }
  } else {
    strncpy_s(tld_hint, tld_hint_len, url_hint, tld_hint_len);
    tld_hint[tld_hint_len - 1] = '\0';
  }
}

// Apply hints, if any, to probabilities
// NOTE: Encoding probabilites are all zero at this point
void ApplyHints(const char* url_hint,
                const char* http_charset_hint,
                const char* meta_charset_hint,
                const int encoding_hint,
                const Language language_hint,
                const CompactEncDet::TextCorpusType corpus_type,
                DetectEncodingState* destatep) {
  int hint_count = 0;
  // url_hint can either be a full URL (preferred) or just top-level domain name
  // Extract the TLD from a full URL and use it for
  // a normal TLD hint

  char tld_hint[16];
  const char* hostname_start = nullptr;
  int hostname_len = 0;
  ExtractTLD(url_hint, tld_hint, sizeof(tld_hint),
             &hostname_start, &hostname_len);


  // Initial hints give slight boost to Ascii-7-bit and code page 1252
  // ApplyXxx routines copy enc_1 to enc_2 then update declared_enc_1
  // This gives a boost to 1252 if one of HTTP/META is specified,
  // but this could be the wrong thing to do if Latin2/3/4/etc. is specified
  destatep->declared_enc_1 = F_CP1252;
  destatep->declared_enc_2 = F_ASCII_7_bit;

  // Applying various hints takes max of new hint and any old hint.
  // This does better on multiple hints that a weighted average

  // Weight is 0..100 percent
  if ((http_charset_hint != nullptr) && (http_charset_hint[0] != '~')) {
    destatep->declared_enc_2 = destatep->declared_enc_1;
    hint_count += ApplyCharsetHint(http_charset_hint, 100, destatep);
    destatep->http_hint = kMapToEncoding[destatep->declared_enc_1];
    if ((destatep->declared_enc_1 == F_CP1252) ||
        (destatep->declared_enc_1 == F_Latin1)) {
      destatep->looking_for_latin_trigrams = true;
    }
  }
  if ((meta_charset_hint != nullptr) && (meta_charset_hint[0] != '~')) {
    destatep->declared_enc_2 = destatep->declared_enc_1;
    hint_count += ApplyCharsetHint(meta_charset_hint, 100, destatep);
    destatep->meta_hint = kMapToEncoding[destatep->declared_enc_1];
    if ((destatep->declared_enc_1 == F_CP1252) ||
        (destatep->declared_enc_1 == F_Latin1)) {
      destatep->looking_for_latin_trigrams = true;
    }
  }
  if (encoding_hint != UNKNOWN_ENCODING) {
    destatep->declared_enc_2 = destatep->declared_enc_1;
    hint_count += ApplyEncodingHint(encoding_hint, 50, destatep);
  }
  if (language_hint != UNKNOWN_LANGUAGE) {
    destatep->declared_enc_2 = destatep->declared_enc_1;
    hint_count += ApplyUILanguageHint(language_hint, 50, destatep);
  }
  // Use top level domain if not .com and <=1 other hint was available
  if (url_hint != nullptr) {
    destatep->tld_hint = CompactEncDet::TopEncodingOfTLDHint(tld_hint);
    if (hint_count == 0) {
      // Apply with weight 100%
      destatep->declared_enc_2 = destatep->declared_enc_1;
      hint_count += ApplyTldHint(tld_hint, 100, destatep);
      if ((destatep->declared_enc_1 == F_CP1252) ||
          (destatep->declared_enc_1 == F_Latin1)) {
        destatep->looking_for_latin_trigrams = true;
      }
      if (strcmp("hu", tld_hint) == 0) {
        // Hungarian is particularly difficult to separate Latin2 from Latin1,
        // so always look for trigram scanning if bare TLD=hu hint
        destatep->looking_for_latin_trigrams = true;
      }
    // Treat .com as no TLD hint at all
    } else if ((hint_count == 1) && (strcmp("com", tld_hint) != 0)) {
      // Either shift weighting or consider doing no TLD here -- seems to
      // distract from correct charset= hints. Or perhaps apply only if
      // charset = Latin1/1252...
      // Apply with weight 50%
      destatep->declared_enc_2 = destatep->declared_enc_1;
      hint_count += ApplyTldHint(tld_hint, 50, destatep);
      if ((destatep->declared_enc_1 == F_CP1252) ||
          (destatep->declared_enc_1 == F_Latin1)) {
        destatep->looking_for_latin_trigrams = true;  // These need trigrams
      }
    }
    // Else ignore TLD hint entirely
  }

  // Use all-web default distribution if not even a TLD hint
  if (hint_count == 0) {
    destatep->looking_for_latin_trigrams = true;    // Default needs trigrams
    destatep->declared_enc_2 = destatep->declared_enc_1;
    //~hint_count += ApplyDefaultHint(corpus_type, destatep);
    ApplyDefaultHint(corpus_type, destatep);
  }


// ISO-Microsoft Pairs
//    F_Latin1, F_CP1252,
//    F_Latin2, F_CP1250,   NOT really strict subset/superset pairs
//    F_Latin3,
//    F_Latin4,
//    F_ISO_8859_5, F_CP1251,
//    F_Arabic, F_CP1256,   NOT
//    F_Greek,  F_CP1253,   NOT really pairs
//                              (or upgrade incvt to make Greek use CP)
//    F_Hebrew, F_CP1255,   NOT really pairs
//    F_Latin5, F_CP1254,
//    F_Latin6,
//    F_ISO_8859_11,
//    F_ISO_8859_13, F_CP1257,
//    F_ISO_8859_15,
// ISO-Microsoft Pairs

  // Get important families started together
  // // This should fall out of the initializatoin vectors for charset,
  // but we need to get rid of families alltogetrher
  //
  // TODO make this more graceful

  // Add small bias for subsets

  // Subtract small bias for supersets
  destatep->enc_prob[F_CP932] = destatep->enc_prob[F_SJS] - kSmallInitDiff;

  destatep->enc_prob[F_GBK] = destatep->enc_prob[F_GB] - kSmallInitDiff;
  destatep->enc_prob[F_GB18030] = destatep->enc_prob[F_GB] - kSmallInitDiff;

  destatep->enc_prob[F_BIG5_CP950] = destatep->enc_prob[F_BIG5] -
    kSmallInitDiff;
  destatep->enc_prob[F_BIG5_HKSCS] = destatep->enc_prob[F_BIG5] -
    kSmallInitDiff;

  // Deliberate over-bias Ascii7 and underbias Binary [unneeded]
  // destatep->enc_prob[F_ASCII_7_bit] = destatep->enc_prob[F_ASCII_7_bit] + kSmallInitDiff;
  // destatep->enc_prob[F_BINARY] = destatep->enc_prob[F_BINARY] - (kBoostInitial / 2);

  if (destatep->debug_data != nullptr) {
    // Show state at end of hints
    SetDetailsEncProb(destatep, 0, -1, "Endhints");
    if(FLAGS_enc_detect_detail2) {
      // Add a line showing the watched encoding(s)
      if (watch1_rankedenc >= 0) {
        SetDetailsEncProb(destatep, 0,
                          watch1_rankedenc, FLAGS_enc_detect_watch1);
      }
      if (watch2_rankedenc >= 0) {
        SetDetailsEncProb(destatep, 0,
                          watch2_rankedenc, FLAGS_enc_detect_watch2);
      }
    }     // End detail2
  }

  // If duplicate hints, set second one to ASCII_7BIT to prevent double-boost
  if (destatep->declared_enc_1 == destatep->declared_enc_2) {
    destatep->declared_enc_2 = F_ASCII_7_bit;
  }

  if (FLAGS_force127) {
    destatep->do_latin_trigrams = true;
    if (FLAGS_enc_detect_source) {
      PsHighlight(nullptr, destatep->initial_src, 0, 2);
    }
  }


  if (FLAGS_counts && destatep->looking_for_latin_trigrams) {++looking_used;}
  if (FLAGS_counts && destatep->do_latin_trigrams) {++doing_used;}

  //
  // At this point, destatep->enc_prob[] is an initial probability vector based
  // on the given hints/default. In general, it spreads out least-likely
  // encodings to be about 2**-25 below the most-likely encoding.
  // For input text with lots of bigrams, an unlikely encoding can rise to
  // the top at a rate of about 2**6 per bigram, and more commonly 2**2 per
  // bigram. So more than 4 bigrams and commonly more than 12 are
  // needed to overcome the initial hints when the least-likely encoding
  // is in fact the correct answer. So if the entire text has very few bigrams
  // (as a two-word query might), it can be impossible for the correct
  // encoding to win.
  //
  // To compensate for this, we take the initial hint vector and effectively
  // apply it at the rate of 1/16 every bigram for the first 16 bigrams. The
  // actual mechanism is done just before the last prune.
  //

  // Remember Initial hint probabilities
  memcpy(destatep->hint_prob, destatep->enc_prob, sizeof(destatep->enc_prob));
}

// Look for specific high-value patterns in the first 4 bytes
// Byte order marks (BOM)
//  EFBBBF    UTF-8
//  FEFF      UTF-16 BE
//  FFFE      UTF-16 LE
//  FFFE0000  UTF-32 BE
//  0000FEFF  UTF-32 LE
//
// Likely UTF-x of seven-bit ASCII
//  00xx      UTF-16 BE  xx printable ASCII
//  xx00      UTF-16 LE
//  000000xx  UTF-32 BE
//  xx000000  UTF-32 LE
//
void InitialBytesBoost(const uint8* src,
                       int text_length,
                       DetectEncodingState* destatep) {
  if (text_length < 4) {return;}

  uint32 pair01 = (src[0] << 8) | src[1];
  uint32 pair23 = (src[2] << 8) | src[3];
  uint32 quad0123 = (pair01 << 16) | pair23;
  
  bool utf_16_indication = false;
  bool utf_32_indication = false;
  int best_enc = -1;

  // Byte order marks
  // UTF-8
  if ((quad0123 & 0xffffff00) == 0xEFBBBF00) {
    destatep->bom_hint = UTF8;
    Boost(destatep, F_UTF8, kBoostInitial * 2);
    Boost(destatep, F_UTF8UTF8, kBoostInitial * 2);
    best_enc = F_UTF8;
  // UTF-32 (test before UTF-16)
  } else if (quad0123 == 0x0000FEFF) {
    destatep->bom_hint = UTF32BE;
    Boost(destatep, F_UTF_32BE, kBoostInitial * 2);
    best_enc = F_UTF_32BE;
  } else if (quad0123 == 0xFFFE0000) {
    destatep->bom_hint = UTF32LE;
    Boost(destatep, F_UTF_32LE, kBoostInitial * 2);
    best_enc = F_UTF_32LE;
  // UTF-16
  } else if (pair01 == 0xFEFF) {
    destatep->bom_hint = UTF16BE;
    Boost(destatep, F_UTF_16BE, kBoostInitial * 3);
    best_enc = F_UTF_16BE;
  } else if (pair01 == 0xFFFE) {
    destatep->bom_hint = UTF16LE;
    Boost(destatep, F_UTF_16LE, kBoostInitial * 3);
    best_enc = F_UTF_16LE;

  // Possible seven-bit ASCII encoded as UTF-16/32
  // UTF-32 (test before UTF-16)
  } else if (((quad0123 & 0xffffff00) == 0) &&
             (kIsPrintableAscii[src[3]] != 0)) {
    Boost(destatep, F_UTF_32BE, kBoostInitial);
    Whack(destatep, F_UTF_32LE, kBadPairWhack);         // Illegal char
    best_enc = F_UTF_32BE;
  } else if (((quad0123 & 0x00ffffff) == 0) &&
             (kIsPrintableAscii[src[0]] != 0)) {
    Boost(destatep, F_UTF_32LE, kBoostInitial);
    Whack(destatep, F_UTF_32BE, kBadPairWhack);         // Illegal char
    best_enc = F_UTF_32LE;
  } else if ((src[0] == 0x00) && (kIsPrintableAscii[src[1]] != 0)) {
    Boost(destatep, F_UTF_16BE, kBoostInitial);
    best_enc = F_UTF_16BE;
  } else if ((src[1] == 0x00) && (kIsPrintableAscii[src[0]] != 0)) {
    Boost(destatep, F_UTF_16LE, kBoostInitial);
    best_enc = F_UTF_16LE;

  // Whack if 0000 or FFFF
  // UTF-32 (test before UTF-16)
  } else if (quad0123 == 0x00000000) {
    Whack(destatep, F_UTF_32BE, kBadPairWhack);         // Illegal char
    Whack(destatep, F_UTF_32LE, kBadPairWhack);
    Whack(destatep, F_UTF_16BE, kBadPairWhack);
    Whack(destatep, F_UTF_16LE, kBadPairWhack);
    best_enc = -1;
  } else if (quad0123 == 0xffffffff) {
    Whack(destatep, F_UTF_32BE, kBadPairWhack);         // Illegal char
    Whack(destatep, F_UTF_32LE, kBadPairWhack);
    Whack(destatep, F_UTF_16BE, kBadPairWhack);
    Whack(destatep, F_UTF_16LE, kBadPairWhack);
    best_enc = -1;
  } else if (pair01 == 0x0000) {
    Whack(destatep, F_UTF_16BE, kBadPairWhack);         // Illegal char
    Whack(destatep, F_UTF_16LE, kBadPairWhack);
    best_enc = -1;
  } else if (pair01 == 0xffff) {
    Whack(destatep, F_UTF_16BE, kBadPairWhack);         // Illegal char
    Whack(destatep, F_UTF_16LE, kBadPairWhack);
    best_enc = -1;


  // These are the first four bytes of some known binary file formats

  // Boost BINARY bigtime if JPEG FFD8FFxx
  // Boost BINARY bigtime if png  89504E47  (.PNG)
  // Boost BINARY bigtime if gif  47494638  (GIF8)
  // Boost BINARY bigtime if zip  504B0304  (PK..)
  // Boost BINARY bigtime if gzip 1F8B08xx
  // Boost BINARY bigtime if gzip 78DAxxxx
  // Boost BINARY if PDF 25504446 (%PDF)
  // Boost BINARY if SWF (FWSx or CWSx where x <= 0x1f)
  } else if ((quad0123 & 0xffffff00) == 0xFFD8FF00) {       // JPEG FFD8FFxx
    Boost(destatep, F_BINARY, kBoostInitial * kStrongBinary);
  } else if (quad0123 == 0x89504E47) {                      // Hex 89 P N G
    Boost(destatep, F_BINARY, kBoostInitial * kStrongBinary);
  } else if (quad0123 == 0x47494638) {                      // Hex GIF8
    Boost(destatep, F_BINARY, kBoostInitial * kStrongBinary);
  } else if (quad0123 == 0x504B0304) {                      // Hex P K 03 04
    Boost(destatep, F_BINARY, kBoostInitial * kStrongBinary);
  } else if ((quad0123 & 0xffffff00) == 0x1F8B0800) {       // gzip 1F8B08xx
    Boost(destatep, F_BINARY, kBoostInitial * kStrongBinary);
  } else if (pair01 == 0x78DA) {                            // gzip 78DAxxxx
    Boost(destatep, F_BINARY, kBoostInitial * kStrongBinary);
  } else if (quad0123 == 0x25504446) {                      // Hex %PDF
    Boost(destatep, F_BINARY, kBoostInitial * kStrongBinary);
  } else if ((quad0123 & 0xffffff1f) == 0x66535700) {       // Hex FWSx
    Boost(destatep, F_BINARY, kBoostInitial * kStrongBinary);
  } else if ((quad0123 & 0xffffff1f) == 0x63535700) {       // Hex CWSx
    Boost(destatep, F_BINARY, kBoostInitial * kStrongBinary);

  // More binary detect prefixes
  // 7F E L F   Executable and linking format
  // M M 00 *   TIFF (little-endian)
  // * 00 M M   TIFF (big-endian)
  // 01 f c p   Final cut pro
  } else if (quad0123 == 0x7F454C46) {                      // Hex 7F E L F
    Boost(destatep, F_BINARY, kBoostInitial * kStrongBinary);
  } else if (quad0123 == 0x4D4D002A) {                      // Hex M M 00 *
    Boost(destatep, F_BINARY, kBoostInitial * kStrongBinary);
  } else if (quad0123 == 0x2A004D4D) {                      // Hex * 00 M M
    Boost(destatep, F_BINARY, kBoostInitial * kStrongBinary);
  } else if (quad0123 == 0x01666370) {                      // Hex 01 f c p
    Boost(destatep, F_BINARY, kBoostInitial * kStrongBinary);

  // More binary detect prefixes; all-ASCII names; heavy weight to avoid ASCII
  // prefix overcoming binary
  // C C S D    USGS ISIS 3-D cube files
  // S I M P    FITS image header    "SIMPLE "
  } else if (quad0123 == 0x43435344) {                      // Hex C C S D
    Boost(destatep, F_BINARY, kBoostInitial * kStrongBinary);
  } else if (quad0123 == 0x53494D50) {                      // Hex S I M P
    Boost(destatep, F_BINARY, kBoostInitial * kStrongBinary);

  // More binary detect prefixes; all-ASCII names; lighter weight
  // H W P      Hangul word processor
  // 8 B P S    Photoshop
  // P D S _    xx "PDS_VERSION_ID "
  } else if (quad0123 == 0x48575020) {                      // Hex H W P
    if ((19 <= text_length) &&
        (memcmp(src, "HWP.Document.File.V", 19) == 0)) {
      Boost(destatep, F_BINARY, kBoostInitial * kStrongBinary);
    } else if ((19 <= text_length) &&
               (memcmp(src, "HWP Document File V", 19) == 0)) {
      Boost(destatep, F_BINARY, kBoostInitial * kStrongBinary);
    } else {
      Boost(destatep, F_BINARY, kBoostInitial * kWeakerBinary);
    }
  } else if (quad0123 == 0x38425053) {                      // Hex 8 B P S
    Boost(destatep, F_BINARY, kBoostInitial * kStrongBinary);
  } else if (quad0123 == 0x5044535F) {                      // Hex P D S _
    if ((14 <= text_length) && (memcmp(src, "PDS_VERSION_ID", 14) == 0)) {
      Boost(destatep, F_BINARY, kBoostInitial * kStrongBinary);
    } else {
      Boost(destatep, F_BINARY, kBoostInitial * kWeakerBinary);
    }
  }

  // There are several main Windows EXE file formats.
  // Not examined here (prefix too short; never see them in Google pipeline)
  // M Z        DOS .exe  Mark Zbikowski
  // N E        DOS 4.0 16-bit
  // L E        OS/2 VxD drivers
  // L X        OS/2
  // P E        Windows NT


  // More user-defined
  // http://www.freenet.am/armscii/ Armenian

  // If any hints or BOM, etc. keep UTF 16/32 around
  if ((destatep->enc_prob[F_UTF_16BE] > 0) ||
      (destatep->enc_prob[F_UTF_16LE] > 0)) {
    utf_16_indication = true;
  }
  if ((destatep->enc_prob[F_UTF_32BE] > 0) ||
      (destatep->enc_prob[F_UTF_32LE] > 0)) {
    utf_32_indication = true;
  }


  // Kill UTF16/32 right now if no positive indication of them
  // Otherwise, they tend to rise to the top in 7-bit files with an
  // occasional 0x02 byte  in some comment or javascript
  if (!utf_16_indication) {
    Whack(destatep, F_UTF_16BE, kBadPairWhack * 8);
    Whack(destatep, F_UTF_16LE, kBadPairWhack * 8);
    Whack(destatep, F_Unicode, kBadPairWhack * 8);
  }
  if (!utf_32_indication) {
    Whack(destatep, F_UTF_32BE, kBadPairWhack * 8);
    Whack(destatep, F_UTF_32LE, kBadPairWhack * 8);
  }

  // Usually kill mixed encodings
  if (!FLAGS_ced_allow_utf8utf8) {
    Whack(destatep, F_UTF8UTF8, kBadPairWhack * 8);
  }
  // 2011.11.07 never use UTF8CP1252 -- answer will be UTF8 instead
  Whack(destatep, F_UTF8CP1252, kBadPairWhack * 8);

  if (destatep->debug_data != nullptr) {
    // Show first four bytes of the input
    char buff[16];
    snprintf(buff, sizeof(buff), "%04x%04x", pair01, pair23);
    SetDetailsEncProb(destatep, 0, best_enc, buff);
  }
}



// Descending order
int IntCompare(const void* v1, const void* v2) {
  const auto* p1 = reinterpret_cast<const int*>(v1);
  const auto* p2 = reinterpret_cast<const int*>(v2);
  if (*p1 < *p2) {return 1;}
  if (*p1 > *p2) {return -1;}
  return 0;
}

bool Base64Char(uint8 c) {
  if (('A' <= c) && (c <= 'Z')) {return true;}
  if (('a' <= c) && (c <= 'z')) {return true;}
  if (('0' <= c) && (c <= '9')) {return true;}
  if ('+' == c) {return true;}
  if ('/' == c) {return true;}
  return false;
}

int Base64ScanLen(const uint8* start, const uint8* limit) {
  // We have a plausible beginning; scan entire base64 string
  const uint8* ib64str = start;
  const uint8* b64str = ib64str;
  const uint8* b64strlimit = limit;
  // if starts with + +++, assume it is drawing, so bogus
  if (((limit - start) > 3) && (start[0] == '+') &&
    (start[1] == '+') && (start[2] == '+')) {
    return 81;
  }
  // Scan over base64
  while ((b64str < b64strlimit) && (kBase64Value[*b64str++] >= 0))  {
  }
  b64str--;      // We overshot by 1
  return static_cast<int>(b64str - ib64str);
}

// Input is at least 8-character legal base64 string after +.
// But might be say + "Presse+Termine"
bool GoodUnicodeFromBase64(const uint8* start, const uint8* limit) {
  // Reject base64 string len N if density of '+' is > 1 + N/16 (expect 1/64)
  // Reject base64 string len N if density of A-Z is < 1 + N/16 (expect 26/64)
  // Reject base64 string len N if density of a-z is < 1 + N/16 (expect 26/64)
  // Reject base64 string len N if density of 0-9 is < 1 + N/32 (expect 10/64)
  // NOTE: this requires at least one lower AND one upper AND one digit to pass
  //
  int plus_count = 0;
  int lower_count = 0;
  int upper_count = 0;
  int digit_count = 0;
  auto len = static_cast<int>(limit - start);
  for (const uint8* src = start; src < limit; ++src) {
    uint8 c = *src;
    if (('a' <= c) && (c <= 'z')) {
      ++lower_count;
    } else if (('A' <= c) && (c <= 'Z')) {
      ++upper_count;
    } else if (('0' <= c) && (c <= '0')) {
      ++digit_count;
    } else if (*src == '+') {
      ++plus_count;
    }
  }

  if (plus_count > (1 + (len >> 4))) {return false;}
  if (lower_count < (1 + (len >> 4))) {return false;}
  if (upper_count < (1 + (len >> 4))) {return false;}
  if (digit_count < (1 + (len >> 5))) {return false;}

  // checking the last character to reduce false positive
  // since the last character may be padded to 0 bits at the end.
  // refer to http://en.wikipedia.org/wiki/UTF-7
  int nmod8 = len & 7;
  const uint8 last = *(start+len-1);
  // When UTF-7 string length%8=3, the last two bits must be padded as 0
  if ((nmod8 == 3) && (kBase64Value[last] & 3)) {return false;}
  // When UTF-7 string length%8=6, the last four bits must be padded as 0
  if ((nmod8 == 6) && (kBase64Value[last] & 15)) {return false;}
  return true;
}

// Prune here after N bytes
// Boost here for seven-bit sequences (at every prune)
// if (sevenbitrankedencoding)
//   + UTF7   scan and boost/demote len mod 8 = 0 3 6
//   ~ Hz     scan and boost/demote len mod 8 = 0 2 4 6
//   1B 2022  scan and boost/demote len mod 8 = 0 2 4 6
//   0E 2022  scan and boost/demote len mod 8 = 0 2 4 6
//   [0F 2022  boost/demote]
//   00 UTF16/32  scan and boost/demote offset = even/odd
//
// If still some seven-bit possibilities > pure ASCII,
// scan each possibility for clearer prob, s.t. about
// two good sequences is a clear win
// A-Z 00-19 00xx-64xx   (B = 04xx)
// a-z 1A-33 68xx-CCxx   (f = 7Cxx)
// 0-9 34-3D D0xx-F4xx   (1 = D4xx)
// +   3E    F8xx
// /   3F    FCxx
// do another chunk  with slow scan


// Boost, whack, or leave alone UTF-7 probablilty
void UTF7BoostWhack(DetectEncodingState* destatep, int next_pair, uint8 byte2) {
  int off = destatep->interesting_offsets[AsciiPair][next_pair];
  if (off >= destatep->prior_utf7_offset) {
    // Not part of a previous successful UTF-7 string
    ++destatep->utf7_starts;

    if (byte2 == '-') {
      // +- encoding for '+'  neutral
    } else if (!Base64Char(byte2)) {
      // Not base64 -- not UTF-7, whack
      Whack(destatep, F_UTF7, kBadPairWhack);                 // Illegal pair
    } else {
      // Starts with base64 byte, might be a good UTF7 sequence
      const uint8* start = destatep->initial_src + off + 1;   // over the +
      int n = Base64ScanLen(start, destatep->limit_src);
      int nmod8 = n & 7;
      if ((n == 3) || (n == 6)) {
        // short but legal -- treat as neutral
      } else if ((nmod8 == 0) || (nmod8 == 3) || (nmod8 == 6)) {
        // Good length. Check for good Unicode.
        if (GoodUnicodeFromBase64(start, start + n)) {
          // Good length and Unicode, boost
          Boost(destatep, F_UTF7, kBoostOnePair);         // Found good
          destatep->prior_utf7_offset = off + n + 1;
        } else {
          // Bad Unicode. Whack
          Whack(destatep, F_UTF7, kBadPairWhack);         // Illegal length
        }
      } else {
        // Bad length. Whack
        Whack(destatep, F_UTF7, kBadPairWhack);         // Illegal length
      }
    }
  }
}

// Boost, whack, or leave alone HZ probablilty
void HzBoostWhack(DetectEncodingState* destatep, uint8 byte2) {
  if ((byte2 == '{') || (byte2 == '}')) {
    Boost(destatep, F_HZ_GB_2312, kBoostOnePair);         // Found ~{ or ~}
  } else if ((byte2 == '~') || (byte2 == '\n')) {
    destatep->enc_prob[F_HZ_GB_2312] += 0;                // neutral
  } else {
    Whack(destatep, F_HZ_GB_2312, kBadPairWhack);         // Illegal pair
  }
}

// Boost, whack, or leave alone BINARY probablilty
void BinaryBoostWhack(DetectEncodingState* destatep, uint8 byte1, uint8 byte2) {
  int quadrant = ((byte1 & 0x80) >> 6) | ((byte2 & 0x80) >> 7);
  int bucket8x4 = ((byte1 & 0xe0) >> 3) | ((byte2 & 0xc0) >> 6);
  uint32 quad_mask = 1 << quadrant;
  uint32 bucket8x4_mask = 1 << bucket8x4;
  if ((destatep->binary_quadrants_seen & quad_mask) == 0) {
    destatep->binary_quadrants_seen |= quad_mask;
    destatep->binary_quadrants_count += 1;
    if (destatep->binary_quadrants_count == 4) {
      Boost(destatep, F_BINARY, kBoostOnePair * 2);   // Found all 4 quadrants,
                                                      // boost 2 pairs
    }
  }
  if ((destatep->binary_8x4_seen & bucket8x4_mask) == 0) {
    destatep->binary_8x4_seen |= bucket8x4_mask;
    destatep->binary_8x4_count += 1;
    if (destatep->binary_8x4_count >= 11) {
      Boost(destatep, F_BINARY, kBoostOnePair * 4);   // Found 11+/20 buckets,
                                                      // boost 4 pairs each time
    }
  }
}


// Demote UTF-16/32 on 0000 or FFFF, favoring Binary
void UTF1632BoostWhack(DetectEncodingState* destatep, int offset, uint8 byte1) {
  if (byte1 == 0) {     // We have 0000
    Whack(destatep, F_UTF_16BE, kBadPairWhack);           // Illegal pair
    Whack(destatep, F_UTF_16LE, kBadPairWhack);           // Illegal pair
    switch (offset & 3) {
    case 0:         // We get called with 0 4 8, etc. for ASCII/BMP as UTF-32BE
      Whack(destatep, F_UTF_32LE, kBadPairWhack);         // Illegal pair
      Boost(destatep, F_UTF_32BE, kSmallInitDiff);        // Good pair
      break;
    case 1:         // We get called with 1 5 9, etc. for ASCII as UTF-32LE
    case 2:         // We get called with 2 6 10, etc. for BMP as UTF-32LE
      Whack(destatep, F_UTF_32BE, kBadPairWhack);         // Illegal pair
      Boost(destatep, F_UTF_32LE, kSmallInitDiff);        // Good pair
      break;
    case 3:         // ambiguous
      break;
    }
  } else {              // We have ffff
    Whack(destatep, F_UTF_32BE, kBadPairWhack);           // Illegal pair
    Whack(destatep, F_UTF_32LE, kBadPairWhack);           // Illegal pair
    Whack(destatep, F_UTF_16BE, kBadPairWhack);           // Illegal pair
    Whack(destatep, F_UTF_16LE, kBadPairWhack);           // Illegal pair
  }
}

// Make even offset
void UTF16MakeEven(DetectEncodingState* destatep, int next_pair) {
  destatep->interesting_offsets[OtherPair][next_pair] &= ~1;
}

bool ConsecutivePair(DetectEncodingState* destatep, int i) {
  if (i <= 0) {
    return false;
  }
  return destatep->interesting_offsets[OtherPair][i] ==
         (destatep->interesting_offsets[OtherPair][i - 1] + 2);
}

// boost, whack, or leave alone UTF-8 probablilty
// Any whacks are also applied to UTF8UTF8; CheckUTF8UTF8Seq assumes good UTF8
// Returns total boost
int CheckUTF8Seq(DetectEncodingState* destatep, int weightshift) {
  int startcount = destatep->prior_interesting_pair[OtherPair];
  int endcount = destatep->next_interesting_pair[OtherPair];

  int demotion_count = 0;
  for (int i = startcount; i < endcount; ++i) {
    int sub;
    char* s = &destatep->interesting_pairs[OtherPair][i * 2];
    // Demote four byte patterns that are more likely Latin1 than UTF-8
    // C9AE, DF92, DF93, DFAB. See note at top.
    // Demotion also boosts Latin1 and CP1252
    auto s0 = static_cast<uint8>(s[0]);
    auto s1 = static_cast<uint8>(s[1]);
    if ((s0 == 0xc9) && (s1 == 0xae)) {++demotion_count;}
    if ((s0 == 0xdf) && (s1 == 0x92)) {++demotion_count;}
    if ((s0 == 0xdf) && (s1 == 0x93)) {++demotion_count;}
    if ((s0 == 0xdf) && (s1 == 0xab)) {++demotion_count;}

    if (!ConsecutivePair(destatep, i)) {
      // Insert a blank into the sequence; avoid wrong splices
      sub = (' ' >> 4) & 0x0f;
      ++destatep->utf8_minicount[
          static_cast<int>(kMiniUTF8Count[static_cast<int>(destatep->next_utf8_ministate)][sub])];
      destatep->next_utf8_ministate =
        kMiniUTF8State[destatep->next_utf8_ministate][sub];
    }
    // Byte 0
    sub = (s0 >> 4) & 0x0f;
    ++destatep->utf8_minicount[
        static_cast<int>(kMiniUTF8Count[static_cast<int>(destatep->next_utf8_ministate)][sub])];
    destatep->next_utf8_ministate =
      kMiniUTF8State[destatep->next_utf8_ministate][sub];
    // Byte 1
    sub = (s1 >> 4) & 0x0f;
    ++destatep->utf8_minicount[
        static_cast<int>(kMiniUTF8Count[static_cast<int>(destatep->next_utf8_ministate)][sub])];
    destatep->next_utf8_ministate =
      kMiniUTF8State[destatep->next_utf8_ministate][sub];
    DCHECK((0 <= destatep->next_utf8_ministate) &&
           (destatep->next_utf8_ministate < 8));
  }


  // For the four specific byte combinations above, Latin1/CP1252 is more likely
  if (demotion_count > 0) {
    Boost(destatep, F_Latin1, kGentleOnePair * demotion_count);
    Boost(destatep, F_CP1252, kGentleOnePair * demotion_count);
  }

  // Boost UTF8 for completed good sequences
  int total_boost = 2 * destatep->utf8_minicount[2] +
                    3 * destatep->utf8_minicount[3] +
                    4 * destatep->utf8_minicount[4];
  // But not so much for demoted bytes
  total_boost -= (3 * demotion_count);

  total_boost *= kGentleOnePair;
  total_boost >>= weightshift;
  // Design: boost both UTF8 and UTF8UTF8 for each good sequence
  Boost(destatep, F_UTF8, total_boost);
  Boost(destatep, F_UTF8UTF8, total_boost);

  destatep->utf8_minicount[5] += destatep->utf8_minicount[2];   // total chars
  destatep->utf8_minicount[5] += destatep->utf8_minicount[3];   // total chars
  destatep->utf8_minicount[5] += destatep->utf8_minicount[4];   // total chars
  destatep->utf8_minicount[2] = 0;
  destatep->utf8_minicount[3] = 0;
  destatep->utf8_minicount[4] = 0;

  // Whack (2 bytes) for errors
  int error_whack = 2 * destatep->utf8_minicount[1];
  error_whack *= kGentlePairWhack;
  error_whack >>= weightshift;
  Whack(destatep, F_UTF8, error_whack);
  Whack(destatep, F_UTF8UTF8, error_whack);
  destatep->utf8_minicount[1] = 0;

  return total_boost - error_whack;
}


// Boost, whack, or leave alone UTF8UTF8 probablilty
//
// We are looking for
// (1) chars ONLY in set UTF8(0080)..UTF8(00FF), including for 80..9F the
//     MS CP1252 mappings, and
// (2) sequences of 2 or more such characters
//
// If so, we could be looking at some non-7-bit encoding extra-converted
// to UTF-8. The most common observed is CP1252->UTF8 twice,
//    1252=>UTF8 : 1252=>UTF8
// where the colon means "take those bytes and pretend that they are 1252".
// We have a couple of examples of BIG5 bytes converted as though
// they were 1252,
//    BIG5 : 1252=>UTF8
//
// Of course, we don't want correctly converted 1252 to be flagged here
//    1252=>UTF8
// So we want the input high bytes to be in pairs or longer, hence the
// output UTF8 in groups of four bytes or more
//
// Good chars: C2xx, C3xx,
// Good chars: C592, C593, C5A0, C5A1, C5B8, C5BD, C5BE, C692, CB86, CB9C
// Good chars: E280xx E282AC E284A2
//             C2xx 1100001x 10xxxxxx   (128/128)
//             C5xx 11000101 10xx00xx   (16/4)
//             C5xx 11000101 10111xxx   (8/3)
//             C692 11000110 10010010   (1/1)
//             CBxx 11001011 100xx1x0   (8/2)
//             E28x 11100010 10000xx0   (4/3)
//
// Returns total boost
int CheckUTF8UTF8Seq(DetectEncodingState* destatep, int weightshift) {
  int this_pair = destatep->prior_interesting_pair[OtherPair];
  int startbyteoffset = this_pair * 2;
  int endbyteoffset = destatep->next_interesting_pair[OtherPair] * 2;
  char* startbyte = &destatep->interesting_pairs[OtherPair][startbyteoffset];
  char* endbyte = &destatep->interesting_pairs[OtherPair][endbyteoffset];

  int pair_number = this_pair;
  for (char* s = startbyte; s < endbyte; s += 2) {
    int next = destatep->next_utf8utf8_ministate;
    if (!ConsecutivePair(destatep, pair_number)) {
      // Insert two blanks into the sequence to avoid wrong splices
      // go back to no odd-byte offset
      destatep->utf8utf8_odd_byte = 0;
      int sub = UTF88Sub(' ', ' ');
      ++destatep->utf8utf8_minicount[static_cast<int>(kMiniUTF8UTF8Count[next][sub])];
      next = kMiniUTF8UTF8State[next][sub];
    }

    int odd = destatep->utf8utf8_odd_byte;
    if (s + 1 + odd >= endbyte) continue;
    int sub = UTF88Sub(s[0 + odd], s[1 + odd]);
    destatep->utf8utf8_odd_byte ^= kMiniUTF8UTF8Odd[next][sub];
    ++destatep->utf8utf8_minicount[
        static_cast<int>(kMiniUTF8UTF8Count[next][sub])];
    destatep->next_utf8utf8_ministate = kMiniUTF8UTF8State[next][sub];
    ++pair_number;
  }

  // Boost for completed good sequences; each count covers two chars.
  // Design: boost UTF8UTF8 above UTF8 for each good sequence
  int total_boost = (2) * destatep->utf8utf8_minicount[2] +
                    (2) * destatep->utf8utf8_minicount[3] +
                    (2) * destatep->utf8utf8_minicount[4];
  total_boost *= kGentleOnePair;
  total_boost >>= weightshift;
  Boost(destatep, F_UTF8UTF8, total_boost);

  // Track total characters
  destatep->utf8utf8_minicount[5] += destatep->utf8utf8_minicount[2];
  destatep->utf8utf8_minicount[5] += destatep->utf8utf8_minicount[3];
  destatep->utf8utf8_minicount[5] += destatep->utf8utf8_minicount[4];
  destatep->utf8utf8_minicount[2] = 0;
  destatep->utf8utf8_minicount[3] = 0;
  destatep->utf8utf8_minicount[4] = 0;

  // Design: Do not whack UTF8UTF8 below UTF8 for each bad sequence

  destatep->utf8utf8_minicount[1] = 0;
  return total_boost;
}


// We give a gentle boost for each paired SO ... SI, whack others
void CheckIso2022ActiveSeq(DetectEncodingState* destatep) {
  int this_pair = destatep->prior_interesting_pair[OtherPair];
  int startbyteoffset = this_pair * 2;
  int endbyteoffset = destatep->next_interesting_pair[OtherPair] * 2;
  char* startbyte = &destatep->interesting_pairs[OtherPair][startbyteoffset];
  char* endbyte = &destatep->interesting_pairs[OtherPair][endbyteoffset];

  // Initial <esc> char must precede SO/SI
  // HZ_GB_2312 has no alternation constraint on 1- and 2-byte segments
  // ISO-2022-JP (JIS) has no alternation constraint on 1- and 2-byte segments
  // ISO-2022-CN has no alternation constraint on 1- and 2-byte segments
  // ISO-2022-KR requires alternation between 1- and 2-byte segments
  // JIS:
  //  <esc> ( B ISO-2022-JP     [1b 28 42]  SI to ASCII
  //  <esc> ( J ISO-2022-JP     [1b 28 4a]  SI to X0201
  //  <esc> $ @ ISO-2022-JP     [1b 24 40]  SO to X0208-78 twobyte
  //  <esc> $ B ISO-2022-JP     [1b 24 42]  SO to X0208-83 twobyte
  for (char* s = startbyte; s < endbyte; s += 2) {
    if (s[0] == 0x1b) {
      if (s[1] == 0x24) {
        // <esc> $  is SO
        destatep->next_2022_state = SOSI_TWOBYTE;       // SO to two-byte
      } else if (s[1] == 0x28) {
        if (destatep->next_2022_state == SOSI_TWOBYTE) {
          Boost(destatep, F_JIS, kGentlePairBoost);
        } else if (destatep->next_2022_state == SOSI_ONEBYTE) {
          Whack(destatep, F_JIS, kGentlePairWhack);
        }
        destatep->next_2022_state = SOSI_ONEBYTE;       // JIS SI to one-byte
      } else {
        Whack(destatep, F_JIS, kBadPairWhack);
        Whack(destatep, F_ISO_2022_CN, kBadPairWhack);
        Whack(destatep, F_ISO_2022_KR, kBadPairWhack);
        destatep->next_2022_state = SOSI_ERROR;     // not 2022
      }
    } else if (s[0] == 0x0e)  {
      // <so>
      Whack(destatep, F_JIS, kBadPairWhack);
      if (destatep->next_2022_state != SOSI_NONE) {
        destatep->next_2022_state = SOSI_TWOBYTE;       // SO to two-byte
      } else {
        // ESC required before SO/SI
        Whack(destatep, F_ISO_2022_CN, kBadPairWhack * 4);
        Whack(destatep, F_ISO_2022_KR, kBadPairWhack * 4);
        destatep->next_2022_state = SOSI_ERROR;     // SO not after SI
      }
    } else if (s[0] == 0x0f)  {
      // <si>
      Whack(destatep, F_JIS, kBadPairWhack);
      if (destatep->next_2022_state != SOSI_NONE) {
        if (destatep->next_2022_state == SOSI_TWOBYTE) {
          Boost(destatep, F_ISO_2022_CN, kGentlePairBoost);
          Boost(destatep, F_ISO_2022_KR, kGentlePairBoost);
        } else if (destatep->next_2022_state == SOSI_ONEBYTE) {
          Whack(destatep, F_ISO_2022_CN, kGentlePairWhack);
          Whack(destatep, F_ISO_2022_KR, kGentlePairWhack);
        }
        destatep->next_2022_state = SOSI_ONEBYTE;       // SI to one-byte
      } else {
        // ESC required before SO/SI
        Whack(destatep, F_ISO_2022_CN, kBadPairWhack * 4);
        Whack(destatep, F_ISO_2022_KR, kBadPairWhack * 4);
        destatep->next_2022_state = SOSI_ERROR;     // SI not after SO
      }
    } else if (s[0] <= 0x1f)  {
      // Some other control code. Allow ht lf [ff] cr
      if ((s[0] != 0x09) && (s[0] != 0x0a) &&
          (s[0] != 0x0c) && (s[0] != 0x0d)) {
        // Otherwise these can float to the top on bad bytes
        Whack(destatep, F_JIS, kBadPairWhack);
        Whack(destatep, F_ISO_2022_CN, kBadPairWhack);
        Whack(destatep, F_ISO_2022_KR, kBadPairWhack);
      }
    }
  }

  // If no start, keep the probability pinned at zero (or below)
  if (destatep->next_2022_state == SOSI_NONE) {
    destatep->enc_prob[F_ISO_2022_CN] =
      minint(0, destatep->enc_prob[F_ISO_2022_CN]);
    destatep->enc_prob[F_ISO_2022_KR] =
      minint(0, destatep->enc_prob[F_ISO_2022_KR]);
    destatep->enc_prob[F_JIS] =
      minint(0, destatep->enc_prob[F_JIS]);
  }
}

// We give a gentle boost for each paired ~{ ... ~}, whack others
void CheckHzActiveSeq(DetectEncodingState* destatep) {
  int this_pair = destatep->prior_interesting_pair[AsciiPair];
  int startbyteoffset = this_pair * 2;
  int endbyteoffset = destatep->next_interesting_pair[AsciiPair] * 2;
  char* startbyte = &destatep->interesting_pairs[AsciiPair][startbyteoffset];
  char* endbyte = &destatep->interesting_pairs[AsciiPair][endbyteoffset];

  for (char* s = startbyte; s < endbyte; s += 2) {
    // Look for initial ~{ pair
    if ((s[0] == '~') && (s[1] == '{')) {
      destatep->next_hz_state = SOSI_TWOBYTE;       // SO to two-byte
    }
    // Also look for closing ~} pair
    if ((s[0] == '~') && (s[1] == '}'))  {
      if (destatep->next_hz_state == SOSI_TWOBYTE) {
        Boost(destatep, F_HZ_GB_2312, kGentlePairBoost);
      } else if (destatep->next_hz_state == SOSI_ONEBYTE) {
        Whack(destatep, F_HZ_GB_2312, kGentlePairWhack);
      }
      destatep->next_hz_state = SOSI_ONEBYTE;       // SI to one-byte
    }
  }

  // If no start, keep the probability pinned at zero (or below)
  if (destatep->next_hz_state == SOSI_NONE) {
    destatep->enc_prob[F_HZ_GB_2312] =
      minint(0, destatep->enc_prob[F_HZ_GB_2312]);
  }
}

// We give a gentle boost after an odd number of 8Fxxxx triples, which
// put subsequent bigrams out of phase until a low byte or another 8Fxxxx
void CheckEucJpSeq(DetectEncodingState* destatep) {
  int this_pair = destatep->prior_interesting_pair[OtherPair];
  int startbyteoffset = this_pair * 2;
  int endbyteoffset = destatep->next_interesting_pair[OtherPair] * 2;
  char* startbyte = &destatep->interesting_pairs[OtherPair][startbyteoffset];
  char* endbyte = &destatep->interesting_pairs[OtherPair][endbyteoffset];

  for (char* s = startbyte; s < endbyte; s += 2) {
    // Boost if out of phase (otherwise, EUC-JP will score badly after 8Fxxxx)
    if (destatep->next_eucjp_oddphase) {
      //printf("  EucJp boost[%02x%02x]\n", s[0], s[1]);    // TEMP
      Boost(destatep, F_EUC_JP, kGentlePairBoost * 2);
    }

    auto s0 = static_cast<uint8>(s[0]);
    auto s1 = static_cast<uint8>(s[1]);
    // Look for phase flip at 8F
    if ((s0 & 0x80) == 0x00) {
      destatep->next_eucjp_oddphase = false;
    } else if (s0 == 0x8f) {
      destatep->next_eucjp_oddphase = !destatep->next_eucjp_oddphase;
    }
    if ((s1 & 0x80) == 0x00) {
      destatep->next_eucjp_oddphase = false;
    } else if (s1 == 0x8f) {
      destatep->next_eucjp_oddphase = !destatep->next_eucjp_oddphase;
    }
  }
}

// Boost, whack, or leave alone BINARY probablilty
// Also called if UTF 16/32 active
void CheckBinaryDensity(const uint8* src, DetectEncodingState* destatep,
                        int delta_otherpairs) {
  // No change if not much gathered information
  if (delta_otherpairs == 0) {
    // Only ASCII pairs this call
    return;
  }
  int next_pair = destatep->next_interesting_pair[OtherPair];

  // Look at density of interesting pairs [0..src)
  auto delta_offset =  static_cast<int>(src - destatep->initial_src);   // actual

  // Look at density of interesting pairs [0..next_interesting)
  int low_byte = destatep->interesting_offsets[OtherPair][0];
  //int high_byte = destatep->interesting_offsets[OtherPair][next_pair - 1] + 2;
  //int byte_span = high_byte - low_byte;
  int byte_span = delta_offset - low_byte;

  // If all ASCII for the first 4KB, reject
  // If mostly ASCII in the first 5KB, reject
  if ((low_byte >= kBinaryHardAsciiLimit) || (delta_offset >= kBinarySoftAsciiLimit)) {
    // Not binary early enough in text
    Whack(destatep, F_BINARY, kBadPairWhack * 4);
    Whack(destatep, F_UTF_32BE, kBadPairWhack * 4);
    Whack(destatep, F_UTF_32LE, kBadPairWhack * 4);
    Whack(destatep, F_UTF_16BE, kBadPairWhack * 4);
    Whack(destatep, F_UTF_16LE, kBadPairWhack * 4);
    return;
  }

  // Density 1.0 for N pairs takes 2*N bytes
  // Whack if < 1/16 after first non_ASCII pair
  if ((next_pair * 2 * 16) < byte_span) {
    // Not dense enough
    Whack(destatep, F_BINARY, kBadPairWhack * 4);
    Whack(destatep, F_UTF_32BE, kBadPairWhack * 4);
    Whack(destatep, F_UTF_32LE, kBadPairWhack * 4);
    Whack(destatep, F_UTF_16BE, kBadPairWhack * 4);
    Whack(destatep, F_UTF_16LE, kBadPairWhack * 4);
  }

  if (next_pair < 8) {
    // Fewer than 8 non-ASCII total; too soon to boost
    return;
  }

  // Density 1.0 for N pairs takes 2*N bytes
  // Boost if density >= 1/4, whack if < 1/16
  if ((next_pair * 2 * 4) >= byte_span) {
    // Very dense
    // Only boost if at least 2 quadrants seen
    if (destatep->binary_quadrants_count >= 2) {
      Boost(destatep, F_BINARY, kSmallInitDiff);
      Boost(destatep, F_UTF_32BE, kSmallInitDiff);
      Boost(destatep, F_UTF_32LE, kSmallInitDiff);
      Boost(destatep, F_UTF_16BE, kSmallInitDiff);
      Boost(destatep, F_UTF_16LE, kSmallInitDiff);
    }
  }
}


// Look at a number of special-case encodings whose reliable detection depends
// on sequencing or other properties
// AsciiPair probibilities (UTF7 and HZ) are all done here
void ActiveSpecialBoostWhack(const uint8* src, DetectEncodingState* destatep) {
  int delta_asciipairs = destatep->next_interesting_pair[AsciiPair] -
    destatep->prior_interesting_pair[AsciiPair];
  int delta_otherpairs = destatep->next_interesting_pair[OtherPair] -
    destatep->prior_interesting_pair[OtherPair];

  // The two pure ASCII encodings
  if (UTF7OrHzActive(destatep) && (delta_asciipairs > 0)) {
    // Adjust per pair
    for (int i = 0; i < delta_asciipairs; ++i) {
      int next_pair = destatep->prior_interesting_pair[AsciiPair] + i;
      uint8 byte1 = destatep->interesting_pairs[AsciiPair][next_pair * 2 + 0];
      uint8 byte2 = destatep->interesting_pairs[AsciiPair][next_pair * 2 + 1];
      if (byte1 == '+') {
        // Boost, whack, or leave alone UTF-7 probablilty
        UTF7BoostWhack(destatep, next_pair, byte2);
        if (destatep->debug_data != nullptr) {
          // Show UTF7 entry
          char buff[16];
          snprintf(buff, sizeof(buff), "%02x%02x+", byte1, byte2);
          SetDetailsEncProb(destatep,
                            destatep->interesting_offsets[AsciiPair][next_pair],
                            kMostLikelyEncoding[(byte1 << 8) + byte2],
                            buff);
        }
      } else if (byte1 == '~') {
        // Boost, whack, or leave alone HZ probablilty
        HzBoostWhack(destatep, byte2);
        if (destatep->debug_data != nullptr) {
          // Show Hz entry
          char buff[16];
          snprintf(buff, sizeof(buff), "%02x%02x~", byte1, byte2);
          SetDetailsEncProb(destatep,
                            destatep->interesting_offsets[AsciiPair][next_pair],
                            kMostLikelyEncoding[(byte1 << 8) + byte2],
                            buff);
        }
      }
    }

    // Kill UTF-7 now if at least 8 + pairs and not confirmed valid UTF-7
    if ((destatep->utf7_starts >= 8) && (destatep->prior_utf7_offset == 0)) {
      Whack(destatep, F_UTF7, kBadPairWhack * 8);         // flush
    }
  }



  // All the other encodings
  if (OtherActive(destatep) && (delta_otherpairs > 0)) {
    // Adjust per pair
    int biggest_weightshift = 0;
    for (int i = 0; i < delta_otherpairs; ++i) {
      int next_pair = destatep->prior_interesting_pair[OtherPair] + i;
      uint8 byte1 = destatep->interesting_pairs[OtherPair][next_pair * 2 + 0];
      uint8 byte2 = destatep->interesting_pairs[OtherPair][next_pair * 2 + 1];
      int off = destatep->interesting_offsets[OtherPair][next_pair];
      int weightshift = destatep->interesting_weightshift[OtherPair][next_pair];
      biggest_weightshift = maxint(biggest_weightshift, weightshift);

      if (byte1 == 0x00) {
        if (byte2 == 0x00) {
          UTF1632BoostWhack(destatep, off, byte1);
        } else if ((kIsPrintableAscii[byte2] != 0) && ((off & 1) != 0)) {
          // We have 00xx at an odd offset. Turn into preceding even offset
          // for possible Ascii text in UTF-16LE or UTF-32LE (vs BE)
          // This will cascade into caller's probability update
          // 00 is illegal for all other encodings, so it doesn't matter to them
          UTF16MakeEven(destatep, next_pair);
        }
        if (destatep->debug_data != nullptr) {
          // Show 0000 detail entry for this bigram
          char buff[16];
          snprintf(buff, sizeof(buff), "%02x%02xZ", byte1, byte2);
          SetDetailsEncProb(destatep,
                            destatep->interesting_offsets[OtherPair][next_pair],
                            kMostLikelyEncoding[(byte1 << 8) + byte2],
                            buff);
        }
      }
      if (byte1 == 0xff) {
        if (byte2 == 0xff) {
          UTF1632BoostWhack(destatep, off, byte1);
        }
        if (destatep->debug_data != nullptr) {
          // Show FFFF detail entry for this bigram
          char buff[16];
          snprintf(buff, sizeof(buff), "%02x%02xF", byte1, byte2);
          SetDetailsEncProb(destatep,
                            destatep->interesting_offsets[OtherPair][next_pair],
                            kMostLikelyEncoding[(byte1 << 8) + byte2],
                            buff);
        }
      }
      if (BinaryActive(destatep)) {
        BinaryBoostWhack(destatep, byte1, byte2);
      }
    }         // End for i

    // Adjust per entire-pair-span
    if (UTF8Active(destatep)) {
      CheckUTF8Seq(destatep, biggest_weightshift);
    }

    if (UTF8UTF8Active(destatep)) {
      CheckUTF8UTF8Seq(destatep, biggest_weightshift);
    }

    if (Iso2022Active(destatep)) {
      CheckIso2022ActiveSeq(destatep);
    }

    if (HzActive(destatep)) {
      CheckHzActiveSeq(destatep);
    }

    if (EUCJPActive(destatep)) {
      CheckEucJpSeq(destatep);
    }

    if (BinaryActive(destatep) || UTF1632Active(destatep)) {
      CheckBinaryDensity(src, destatep, delta_otherpairs);
    }
  }
  // ISO-2022 do OK on their own, using stright probabilities? Not on bad bytes

  if (destatep->debug_data != nullptr) {
    // Show sequencing result
    SetDetailsEncLabel(destatep, "seq");
  }
}


void PrintTopEnc(DetectEncodingState* destatep, int n) {
  // Print top n or fewer
  int temp_sort[NUM_RANKEDENCODING];
  for (int j = 0; j < destatep->rankedencoding_list_len; ++j) {
    int rankedencoding = destatep->rankedencoding_list[j];
    temp_sort[j] = destatep->enc_prob[rankedencoding];
  }

  qsort(temp_sort, destatep->rankedencoding_list_len,
        sizeof(temp_sort[0]), IntCompare);

  int top_n = minint(n, destatep->rankedencoding_list_len);
  int showme = temp_sort[top_n - 1];    // Print this value and above

  printf("rankedencodingList top %d: ", top_n);
  for (int j = 0; j < destatep->rankedencoding_list_len; ++j) {
    int rankedencoding = destatep->rankedencoding_list[j];
    if (showme <= destatep->enc_prob[rankedencoding]) {
      printf("%s=%d ",
             MyEncodingName(kMapToEncoding[rankedencoding]),
             destatep->enc_prob[rankedencoding]);
    }
  }
  printf("\n\n");
}

// If the same bigram repeats, don't boost its best encoding too much
bool RepeatedBigram(DetectEncodingState* destatep, uint8 byte1, uint8 byte2) {
  int this_bigram = (byte1 << 8) | byte2;
  // If 00xx 01xx 02xx ... 1fxx, take out bottom 4 bits of xx.
  // This ignores parts of Yahoo 0255 0254 0243 0247 0245 0243 0250 0255 ...
  // It may screw up UTF-16BE
  // It may screw up ISO-2022 (1b24 suppresses 1b28)
  if (byte1 < 0x20) {
    this_bigram &= 0xfff0;
  }
  if (this_bigram == destatep->prior_bigram[0]) {return true;}
  if (this_bigram == destatep->prior_bigram[1]) {return true;}
  if (this_bigram == destatep->prior_bigram[2]) {return true;}
  if (this_bigram == destatep->prior_bigram[3]) {return true;}
  // Round-robin replacement
  destatep->prior_bigram[destatep->next_prior_bigram] = this_bigram;
  destatep->next_prior_bigram = (destatep->next_prior_bigram + 1) & 3;
  return false;
}

// Sometimes illegal bytes are used as markers between text that Javascript
// is going to decode. Don't overboost the Binary encoding for markers 01-FF.
// Just count first pair per 8x4 bucket
bool RepeatedBinary(DetectEncodingState* destatep, uint8 byte1, uint8 byte2) {
  int bucket8x4 = ((byte1 & 0xe0) >> 3) | ((byte2 & 0xc0) >> 6);
  uint32 bucket8x4_mask = 1 << bucket8x4;
  if ((destatep->binary_8x4_seen & bucket8x4_mask) == 0) {
    destatep->binary_8x4_seen |= bucket8x4_mask;
    destatep->binary_8x4_count += 1;
    return false;
  }
  return true;
}




// Find current top two rankedencoding probabilities
void ReRank(DetectEncodingState* destatep) {
  destatep->top_prob = -1;
  destatep->second_top_prob = -1;
  // Leave unchanged
  //destatep->top_rankedencoding =
  //  destatep->rankedencoding_list[0];     // Just to make well-defined
  //destatep->second_top_rankedencoding =
  //  destatep->rankedencoding_list[1];     // Just to make well-defined
  for (int j = 0; j < destatep->rankedencoding_list_len; j++) {
    int rankedencoding = destatep->rankedencoding_list[j];
    if (destatep->top_prob < destatep->enc_prob[rankedencoding]) {
      // Make sure top 2 are in different superset groups
      if (kMapEncToBaseEncoding[kMapToEncoding[destatep->top_rankedencoding]] !=
          kMapEncToBaseEncoding[kMapToEncoding[rankedencoding]]) {
        destatep->second_top_prob =
          destatep->top_prob;             // old top to second
        destatep->second_top_rankedencoding =
          destatep->top_rankedencoding;   // old top to second
      }
      destatep->top_prob = destatep->enc_prob[rankedencoding];
      destatep->top_rankedencoding = rankedencoding;
    } else if (destatep->second_top_prob < destatep->enc_prob[rankedencoding]) {
      if (kMapEncToBaseEncoding[kMapToEncoding[destatep->top_rankedencoding]] !=
          kMapEncToBaseEncoding[kMapToEncoding[rankedencoding]]) {
        destatep->second_top_prob = destatep->enc_prob[rankedencoding];
        destatep->second_top_rankedencoding = rankedencoding;
      }
    }
  }
}

void SimplePrune(DetectEncodingState* destatep, int prune_diff) {
  // Prune the list of active encoding families
  int keep_prob = destatep->top_prob - prune_diff;

  destatep->active_special = 0;
  int k = 0;
  for (int j = 0; j < destatep->rankedencoding_list_len; j++) {
    bool keep = true;
    int rankedencoding = destatep->rankedencoding_list[j];

    // If count is too low, ditch it
    if (destatep->enc_prob[rankedencoding] < keep_prob) {keep = false;}

    // Keep it. This will always keep at least top_prob rankedencoding
    if (keep) {
      destatep->active_special |= kSpecialMask[kMapToEncoding[rankedencoding]];
      destatep->rankedencoding_list[k++] = rankedencoding;
    }
  }

  destatep->rankedencoding_list_len = k;
}

// Recalculate reliable
void CalcReliable(DetectEncodingState* destatep) {
  // Encoding result is reliable if big difference in top two, or if
  // only Ascii7 ever encountered
  // Also reliable if exactly one OtherPair and it's best encoding matches top
  destatep->reliable = false;
  if (destatep->next_interesting_pair[OtherPair] == 0) {
    // Only 7-bit ASCII
    destatep->reliable = true;
    return;
  }
  if ((destatep->top_prob - destatep->second_top_prob) >=
      FLAGS_ced_reliable_difference) {
    destatep->reliable = true;
    return;
  }
  if (destatep->next_interesting_pair[OtherPair] == 1) {
    uint8 byte1 = destatep->interesting_pairs[OtherPair][0];
    uint8 byte2 = destatep->interesting_pairs[OtherPair][1];
    int best_enc = kMostLikelyEncoding[(byte1 << 8) + byte2];
    if (best_enc == destatep->top_rankedencoding) {
      destatep->reliable = true;
      return;
    }
  }

  // If we pruned to one encoding, we are done
  if (destatep->rankedencoding_list_len == 1) {
    destatep->reliable = true;
    destatep->done = true;
    return;
  }

  // If we pruned to two or three encodings in the same *superset/subset
  // rankedencoding*  and enough pairs, we are done. Else keep going
  if (destatep->rankedencoding_list_len == 2) {
    Encoding enc0 = kMapToEncoding[destatep->rankedencoding_list[0]];
    Encoding enc1 = kMapToEncoding[destatep->rankedencoding_list[1]];
    if (kMapEncToBaseEncoding[enc0] == kMapEncToBaseEncoding[enc1]) {
      if (destatep->prune_count >= 3) {
        destatep->reliable = true;
        destatep->done = true;
        return;
      }
    }
  } else if (destatep->rankedencoding_list_len == 3) {
    Encoding enc0 = kMapToEncoding[destatep->rankedencoding_list[0]];
    Encoding enc1 = kMapToEncoding[destatep->rankedencoding_list[1]];
    Encoding enc2 = kMapToEncoding[destatep->rankedencoding_list[2]];
    Encoding base0 = kMapEncToBaseEncoding[enc0];
    Encoding base1 = kMapEncToBaseEncoding[enc1];
    Encoding base2 = kMapEncToBaseEncoding[enc2];

    if ((base0 == base1) && (base0 == base2)) {
      if (destatep->prune_count >= 3) {
        destatep->reliable = true;
        destatep->done = true;
        return;
      }
    }
  }

}


// Find current top two rankedencoding probabilities
void FindTop2(DetectEncodingState* destatep,
              int* first_renc, int* second_renc,
              int* first_prob, int* second_prob) {
  *first_prob = -1;
  *second_prob = -1;
  *first_renc = 0;
  *second_renc = 0;
  for (int j = 0; j < destatep->rankedencoding_list_len; j++) {
    int rankedencoding = destatep->rankedencoding_list[j];
    if (*first_prob < destatep->enc_prob[rankedencoding]) {
      *second_prob = *first_prob;             // old top to second
      *second_renc = *first_renc;   // old top to second
      *first_prob = destatep->enc_prob[rankedencoding];
      *first_renc = rankedencoding;
    } else if (*second_prob < destatep->enc_prob[rankedencoding]) {
      *second_prob = destatep->enc_prob[rankedencoding];
      *second_renc = rankedencoding;
    }
  }
}


void PrintRankedEncodingList(DetectEncodingState* destatep, const char* str) {
  printf("Current ranked encoding list %s\n", str);
  for (int j = 0; j < destatep->rankedencoding_list_len; j++) {
    int rankedencoding = destatep->rankedencoding_list[j];
    if ((rankedencoding < 0) || (rankedencoding > NUM_RANKEDENCODING)) {
      printf(" [%d] BOGUS rankedencoding = %d\n", j, rankedencoding);
    } else {
      printf(" [%d] rankedencoding = %d %-12.12s enc_prob = %d\n",
             j, rankedencoding, MyRankedEncName(rankedencoding),
             destatep->enc_prob[rankedencoding]);
    }
  }
  printf("End current ranked encoding list\n\n");
}




// Map unencoded bytes down to five bits, largely preserving letters
// This design struggles to put 33 values into 5 bits.
#define XX 0    // Punctuation (00-7F range)
#define HA 27   // High vowel a in Latin1/2/sometimes7
#define HE 28   // High vowel e
#define HI 29   // High vowel i
#define HO 30   // High vowel o
#define HU 30   // High vowel u on top of HO
#define Hc 31   // High consonant (80-FF range)
static const char kMapToFiveBits[256] = {
  XX,XX,XX,XX,XX,XX,XX,XX,  XX,XX,XX,XX,XX,XX,XX,XX,
  XX,XX,XX,XX,XX,XX,XX,XX,  XX,XX,XX,XX,XX,XX,XX,XX,
  XX,XX,XX,XX,XX,XX,XX,XX,  XX,XX,XX,XX,XX,XX,XX,XX,
  XX,XX,XX,XX,XX,XX,XX,XX,  XX,XX,XX,XX,XX,XX,XX,XX,

  XX, 1, 2, 3, 4, 5, 6, 7,   8, 9,10,11,12,13,14,15,
  16,17,18,19,20,21,22,23,  24,25,26,XX,XX,XX,XX,XX,
  XX, 1, 2, 3, 4, 5, 6, 7,   8, 9,10,11,12,13,14,15,
  16,17,18,19,20,21,22,23,  24,25,26,XX,XX,XX,XX,XX,

  Hc,HA,Hc,Hc,Hc,Hc,Hc,Hc,  HO,Hc,Hc,Hc,Hc,Hc,Hc,Hc,
  Hc,HA,Hc,Hc,Hc,Hc,Hc,Hc,  HO,Hc,Hc,Hc,Hc,Hc,Hc,Hc,
  Hc,HA,Hc,Hc,Hc,Hc,Hc,Hc,  HO,Hc,Hc,Hc,Hc,Hc,Hc,Hc,
  Hc,HA,Hc,Hc,Hc,Hc,Hc,Hc,  HO,Hc,Hc,Hc,Hc,Hc,Hc,Hc,

  Hc,HA,HA,HA,HA,Hc,Hc,Hc,  Hc,HE,HE,HE,HI,HI,HI,Hc,
  Hc,Hc,Hc,HO,HO,HO,HO,Hc,  Hc,HU,HU,HU,HU,Hc,Hc,Hc,
  Hc,HA,HA,HA,HA,Hc,Hc,Hc,  Hc,HE,HE,HE,HI,HI,HI,Hc,
  Hc,Hc,Hc,HO,HO,HO,HO,Hc,  Hc,HU,HU,HU,HU,Hc,Hc,Hc,

};
#undef XX
#undef HA
#undef HE
#undef HI
#undef HO
#undef HU
#undef Hc

static const int kTriLatin1Likely = 1;
static const int kTriLatin2Likely = 2;
static const int kTriLatin7Likely = 3;

// Each table entry has 32 times two bits, selected by byte[2]
// Entry subscript is selected by byte[0] and byte[1]
// Latin1/2/7 boost vector, generated 2007.09.26 by postproc-enc-detect-short.cc
static const uint64 kLatin127Trigrams[1024] = {
0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL,
0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL,
0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL,
0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL,
0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL,
0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL,
0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL,
0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL,
0x0000000000000000ULL, 0x304080c0402c3330ULL, 0x0008400004000000ULL, 0x082800000c200000ULL,
0x23a0000420800030ULL, 0x00000000000ccc00ULL, 0x0500100100100000ULL, 0x0388400000200010ULL,
0x0000000000000c00ULL, 0xd0f0300740f0cf00ULL, 0x2aa0a2a22882a2acULL, 0x081d800000000080ULL,
0x0c82000020000000ULL, 0x200a03c000a00000ULL, 0x0008400400290000ULL, 0x0400870000000000ULL,
0x00f040c00000c080ULL, 0x0008004000000410ULL, 0x0020300000000030ULL, 0x00a030002c300000ULL,
0x0c8030c020a00000ULL, 0x15410030f0f4c000ULL, 0x3000000300a00000ULL, 0xa2880980a0880a88ULL,
0x0900300000000000ULL, 0x0000040100300000ULL, 0x0888820020a00000ULL, 0xc044002242010000ULL,
0x000000121d300040ULL, 0x40100040440c0d54ULL, 0x00008423102f8144ULL, 0x0b40808400000280ULL,
0x0000000000000000ULL, 0x0680a000000c0000ULL, 0x0880008020aa0000ULL, 0x2aaa0141010a4940ULL,
0xcb80000000010000ULL, 0x2280000000000000ULL, 0x5248000001800000ULL, 0x8000401004040010ULL,
0x1540010201001010ULL, 0x0080080400000000ULL, 0x5a00044040000108ULL, 0x0288000282080008ULL,
0x4800008002200000ULL, 0x4a00000000010100ULL, 0x8a88040080000800ULL, 0x0140800000000400ULL,
0x40010050000c0000ULL, 0x0000008000000000ULL, 0x0028000020140040ULL, 0x8620401401005308ULL,
0xc082000000000400ULL, 0x05c0b004c0240600ULL, 0x0288000080000000ULL, 0x0000014000000000ULL,
0x00000000040000c0ULL, 0x8001861008004280ULL, 0x0200000000000300ULL, 0x0000240242288620ULL,
0x801000c05434c200ULL, 0x9020162040a2d2b4ULL, 0x0021840000240704ULL, 0x2a80280080084908ULL,
0x0000000000000000ULL, 0x0500004000000040ULL, 0x0080000000040000ULL, 0x0108058104440000ULL,
0x0900000000040000ULL, 0x00c0000000208008ULL, 0x2000005000000000ULL, 0x0080000000050000ULL,
0x0808000000001080ULL, 0x9880810100308000ULL, 0x2285480080081a08ULL, 0x8a80000080080000ULL,
0x1450000000600010ULL, 0x2210000100000000ULL, 0x8a88000100011000ULL, 0x1541804000000010ULL,
0xc084011140040100ULL, 0x0000000000000800ULL, 0x0400000000000030ULL, 0x2a800000a0890128ULL,
0x1140a00054000104ULL, 0x1440000101200404ULL, 0x028800400400d800ULL, 0x0000000000000000ULL,
0x0000000000002330ULL, 0x0020820228a02280ULL, 0xa2888a02aa8008a8ULL, 0xd0040a0044202500ULL,
0x8000044104a29424ULL, 0xc000100178b2c5b4ULL, 0x0000810100241504ULL, 0xd040030000380008ULL,
0x0000000000000000ULL, 0x26c08c0000200130ULL, 0x4a08000110080000ULL, 0x2aa0004001080800ULL,
0x0aac000000004000ULL, 0x2000000000200000ULL, 0x4240000100020000ULL, 0x4100000080000000ULL,
0x4900040000000000ULL, 0x0800000400300040ULL, 0x6a80000000040800ULL, 0x2a08182000588008ULL,
0x0a00000c81000008ULL, 0x0a000c0010000000ULL, 0x8a88001080280808ULL, 0x0020000200300600ULL,
0xaac00000900a0000ULL, 0x0000100004000000ULL, 0x0020081020000000ULL, 0x8220105010084110ULL,
0x4a80800000004000ULL, 0x050000c0c0200000ULL, 0x288c000084000000ULL, 0xa048082280000000ULL,
0x0000000000000000ULL, 0x8000900000032080ULL, 0xee889e81b8880820ULL, 0xc2200a8142800424ULL,
0xc020141543361010ULL, 0x10a000204a801634ULL, 0x3a808800802a00a0ULL, 0x28808b00803d0800ULL,
0x0000000000000000ULL, 0x0020000000000030ULL, 0x0808400121010040ULL, 0x0c28240100200040ULL,
0x2008200028800000ULL, 0xc10004c80f30c030ULL, 0x0400440114100000ULL, 0x2208200280a22220ULL,
0x0600000030c01000ULL, 0x1201001040c00000ULL, 0x0aa02ea22aa22aa0ULL, 0x30008000000200a0ULL,
0x20c8400400800000ULL, 0x08280b0420800000ULL, 0x0800100000210000ULL, 0x10000300c0100400ULL,
0xc8c0000420000000ULL, 0x1000000010000000ULL, 0x0420000400000000ULL, 0x0220000500204000ULL,
0x2200000420000000ULL, 0x0000540400000000ULL, 0x0000000020000000ULL, 0x00080c00a0810080ULL,
0x1540000000043000ULL, 0x0000000000100000ULL, 0x2e88a22220200a20ULL, 0xc06030e34ea503a0ULL,
0x0001100204048500ULL, 0x000000e0000c0d54ULL, 0x3000820310a31400ULL, 0x13088c0320e00280ULL,
0x0000000000000000ULL, 0x0480000000200000ULL, 0x4000200100000000ULL, 0x0000300040040000ULL,
0x4400000000000000ULL, 0x0401000002240000ULL, 0x0540000000040000ULL, 0x4004010000000000ULL,
0x4001111001100000ULL, 0x2880000000300040ULL, 0x4040004040002404ULL, 0x0200000000000000ULL,
0x0140040000100000ULL, 0x4040010040040080ULL, 0x0a00140000041004ULL, 0x0000a00400808000ULL,
0x1010200000430040ULL, 0x0010000000000000ULL, 0x0540000000104000ULL, 0x1400114005000000ULL,
0x0000204000440010ULL, 0x0500000000004400ULL, 0x4500000018000400ULL, 0x0000400000000000ULL,
0x000000300000cc00ULL, 0x0100001011300000ULL, 0x0040000000000000ULL, 0xc0e0000248a00444ULL,
0x0000040020340144ULL, 0x0000046445105454ULL, 0x32a0a80280880128ULL, 0x0880040000100100ULL,
0x0000000000000000ULL, 0x14003000030c0004ULL, 0x4a04001100000000ULL, 0x0a00108010000000ULL,
0x28a8004000200248ULL, 0x0100040000b00000ULL, 0x42000000000008c0ULL, 0x6008044010550010ULL,
0x0800401000010400ULL, 0x080080040cf80000ULL, 0x5080000001001010ULL, 0x2a80100000000000ULL,
0xcc8010010d401100ULL, 0x0200000001001000ULL, 0x0480001004001000ULL, 0x8d00800040b40210ULL,
0x6200800000300000ULL, 0x0000010000000000ULL, 0x0428004100010000ULL, 0x4320105141501100ULL,
0xe28c0000000c1000ULL, 0xd5c000c3c0e00300ULL, 0x0001000000100200ULL, 0x1004010202400008ULL,
0x0000000000003000ULL, 0x2aa038a0800aab08ULL, 0x2a88038000000000ULL, 0xc220040242f09720ULL,
0x8020200200ba0420ULL, 0x0020106105101004ULL, 0x0480800000220400ULL, 0x2280100080000008ULL,
0x0000000000000000ULL, 0x9000000000200000ULL, 0x0001000000100000ULL, 0x2aa40c0000080800ULL,
0x0040000040010000ULL, 0x0040000000c01000ULL, 0x4000000040000400ULL, 0x0000001000200000ULL,
0x0000010000000000ULL, 0x05808004000c0000ULL, 0x50400c0000000400ULL, 0x020040008f000040ULL,
0x0800000000100000ULL, 0x0000000000000000ULL, 0x0a08440000004000ULL, 0x0064000400008200ULL,
0x0010010010034170ULL, 0x0000000010000000ULL, 0x0100204021000000ULL, 0x022000d000010100ULL,
0x0840300000c00000ULL, 0x1400000040204400ULL, 0x09800c0040000000ULL, 0x0209708000000000ULL,
0x000000000000c040ULL, 0x90000c50204040a0ULL, 0x0000000000000000ULL, 0x00e1500040200004ULL,
0x8020260540204494ULL, 0x0020026150201054ULL, 0x0281800380105634ULL, 0x0884900481105000ULL,
0x0000000000000000ULL, 0x84203c00002c0200ULL, 0xc089040000000000ULL, 0xc2a8100040200004ULL,
0xe00c1c0000000000ULL, 0x0ce1330080200080ULL, 0x0000000000200000ULL, 0xc400110000404010ULL,
0x0088400000000000ULL, 0x00083cc00c00c00cULL, 0xcac01c00c000580cULL, 0xe300b0f000100000ULL,
0x0300000000000000ULL, 0xc0000f0000000000ULL, 0xc3c01c0400000000ULL, 0x81008004c0f40000ULL,
0xc3d8003000000440ULL, 0x0000000000000000ULL, 0xc430000000000000ULL, 0x0060000000001000ULL,
0x0800000000000000ULL, 0x00c03300f0fc0008ULL, 0x3000000400200010ULL, 0xa2a80892a0880a28ULL,
0x0500000040000004ULL, 0x0000000000000000ULL, 0xc80032070c200020ULL, 0x0220820060a296a0ULL,
0x802084021db486a0ULL, 0x00000d60080c0080ULL, 0xb281803313a32428ULL, 0x1808300320300000ULL,
0x0000000000000000ULL, 0x85208cc0ccac1f20ULL, 0x2081000186100808ULL, 0x22a80880000a0808ULL,
0xaaa8086880000000ULL, 0x802084800a2e9200ULL, 0xa280000000002008ULL, 0xa000000080080400ULL,
0x2080010000000008ULL, 0x802020c00c028c80ULL, 0x2080000000140810ULL, 0x2a80086080080008ULL,
0x2a800000a8000800ULL, 0xaa881800a2080800ULL, 0xaa98004080280808ULL, 0x004483d0c0300000ULL,
0xa280002080080000ULL, 0x0000000000300000ULL, 0x22a1030000000008ULL, 0xa8a0301088880880ULL,
0xaa80002080222808ULL, 0x85400c03fc030400ULL, 0x8a88000000000008ULL, 0xa008008010080008ULL,
0x0000000000010000ULL, 0x0040100000301040ULL, 0x28800000a0002008ULL, 0x122482306cbc0eacULL,
0x8020224222b8c6a0ULL, 0x802002004a82c284ULL, 0x0aa08fc440a41c80ULL, 0x888080d181385098ULL,
0x0000000000000000ULL, 0x00c0b000000c0080ULL, 0x2208001000000800ULL, 0x0a28000000200000ULL,
0x0000000300000000ULL, 0x00c1040000200000ULL, 0x0203020000000000ULL, 0x0248000000020000ULL,
0x0000840000100000ULL, 0x0a808c00c000008cULL, 0x5200040040000004ULL, 0x02000c00000080a0ULL,
0x0b0c000020000000ULL, 0x0b04000001000000ULL, 0x088c0010002000c0ULL, 0x80e08b00c0030c20ULL,
0x0280000200014040ULL, 0x0000000000000000ULL, 0x0e20a0a008000020ULL, 0x0e280fd03f00111cULL,
0x200080c020001000ULL, 0x8cc00c02c02f0400ULL, 0x480c0001000c404cULL, 0x0208014281080808ULL,
0x000000000000fcfcULL, 0x004403300cf00030ULL, 0x2200000000004400ULL, 0x02202000c08c0c20ULL,
0x02202022683a80a0ULL, 0x4020228028008c00ULL, 0x32208cc0002c0200ULL, 0x3ec00c0080304008ULL,
0x0000000000000000ULL, 0x34000c00002c0000ULL, 0x0b00000100100030ULL, 0x0823018000000000ULL,
0x0e8c001c01e00000ULL, 0x1200800600330000ULL, 0x4000110000000000ULL, 0x0080000300000000ULL,
0x0800000000000000ULL, 0x08c08c04000c0000ULL, 0x0080400000880000ULL, 0x0a08000080c00008ULL,
0x0800000304400000ULL, 0x0208000000c00000ULL, 0x2888300080400800ULL, 0x8dc0204400000000ULL,
0xc0000000c0800000ULL, 0x0000c10000000000ULL, 0x24000c4010c00000ULL, 0x272000541d811000ULL,
0x0200400000001000ULL, 0x0400000400001004ULL, 0xc08c007004001000ULL, 0x2048004000000000ULL,
0x000000000003fcfcULL, 0x2aa030000cf8c800ULL, 0xe280000000000000ULL, 0x0a21008142000340ULL,
0x0021002000b61040ULL, 0x800004064006d444ULL, 0x3aa0800300230008ULL, 0x0b00030000300000ULL,
0x0000000000000000ULL, 0x01c080000000040cULL, 0x0100000000004000ULL, 0x0aa8018010001000ULL,
0x0800000000100000ULL, 0x3000000000008c00ULL, 0x5400000013000000ULL, 0x02c0c00004004010ULL,
0x5241100010000c00ULL, 0x0e00080000000808ULL, 0x5281000000000800ULL, 0x0a08108020000800ULL,
0x0a80000000005210ULL, 0x0100000041000000ULL, 0x2a88000002080110ULL, 0x8520800000c00080ULL,
0x01000010108c0100ULL, 0x0000000000000000ULL, 0x42a0420080000000ULL, 0x0020001004010010ULL,
0xc4000000000c0000ULL, 0x01000c00c0200400ULL, 0x4600000100000000ULL, 0x0000000000000000ULL,
0x0010001000000010ULL, 0x910400900820d030ULL, 0x2280000000000000ULL, 0xc2212004400040e4ULL,
0x8001000000b61420ULL, 0xa00002a248e810b4ULL, 0x32008000002c0008ULL, 0x0c010034803c5010ULL,
0x0000000000000000ULL, 0x85008002002c0000ULL, 0x0204001000004010ULL, 0x0120008000200000ULL,
0x000010000c2000c0ULL, 0xccc0000000200000ULL, 0x0400000c00100040ULL, 0x0003300100004100ULL,
0x4000551040000004ULL, 0x0e0080000c820808ULL, 0xc000000000080800ULL, 0xc803000000000000ULL,
0x0a4000c000200000ULL, 0x0040000000c00000ULL, 0x0918145000405000ULL, 0x81400000c0300400ULL,
0x0050000000000000ULL, 0xd000045000000000ULL, 0x0400004000400000ULL, 0x0420104010000110ULL,
0x0700000000203000ULL, 0x34800300c0e00704ULL, 0x4440100044000400ULL, 0x0040000040000000ULL,
0x0030000044000000ULL, 0xeaaca0008808c880ULL, 0x0a01000000200000ULL, 0x1220a300403ccf20ULL,
0x002024c200b61044ULL, 0x802014346aa2d434ULL, 0x30008c00c0820c44ULL, 0x0a000000000c4800ULL,
0x0000000000000000ULL, 0x0000404000340c90ULL, 0x08a8a10820800280ULL, 0x8128009022201000ULL,
0x0020808228a000a0ULL, 0x0020400100410000ULL, 0x0400000110000000ULL, 0xa609000000200000ULL,
0x8008330000d00000ULL, 0x8060100040404010ULL, 0xeaa00ea0ea00808cULL, 0x200c8020a0000020ULL,
0x0408800020200000ULL, 0x0189001403200000ULL, 0xc00800000000c000ULL, 0x200430c00c300000ULL,
0x0100300100004000ULL, 0x0000040000000000ULL, 0x2420000400001000ULL, 0x89a1200400000000ULL,
0x20c8a000208c0000ULL, 0x8080000000000000ULL, 0x28a0108020210080ULL, 0xa2a84800a0880988ULL,
0x258008000400c000ULL, 0x0140000000100000ULL, 0xa028a222a0aa0228ULL, 0xc060012054044040ULL,
0x0010010400000000ULL, 0x00000050150c0114ULL, 0x0000008010c20010ULL, 0xaa088000a0200880ULL,
0x0000000000000000ULL, 0x0700b0c0000c0000ULL, 0x2200040000080030ULL, 0x2aa8808040240800ULL,
0x08b0500000000100ULL, 0x1000830400200000ULL, 0x4204000010000000ULL, 0x40c2200050040050ULL,
0x0104404001010000ULL, 0x1a808c8103c00030ULL, 0x30900010c0000b00ULL, 0x200812b283000008ULL,
0x000c000020e00000ULL, 0x2140000000400000ULL, 0x0288000080200000ULL, 0x8060a200c8a20280ULL,
0x0400114010215000ULL, 0x0000000000000000ULL, 0x082b200002000010ULL, 0x22a0030000031000ULL,
0x008100001000000cULL, 0x05400c00c0230400ULL, 0xca3000003c080100ULL, 0x0000000020000004ULL,
0x0000000100000000ULL, 0x8004320813f5c000ULL, 0xa280080200000800ULL, 0xc22000044e334c20ULL,
0x000004146e361024ULL, 0x800126806aa0d584ULL, 0xb000a0040023c41cULL, 0x0a083000803053d8ULL,
0x0000000000000000ULL, 0x0000100000020000ULL, 0x0000000010000010ULL, 0x0000000045040004ULL,
0x0000000000100000ULL, 0x0000020400000010ULL, 0x0003015000000000ULL, 0x0400000000000000ULL,
0x0000000400000000ULL, 0x0100000000000800ULL, 0x0000001000000000ULL, 0x0000000000000000ULL,
0x0000000040000000ULL, 0x0000000000000000ULL, 0x0004001000000000ULL, 0x0008001000000000ULL,
0x0010000000000004ULL, 0x0000010100001000ULL, 0x0004000000000004ULL, 0x0000014040050014ULL,
0x0014000000000040ULL, 0x5540000000041000ULL, 0x0000000000000000ULL, 0x0000040000000d00ULL,
0x0000000000000000ULL, 0x0000000000100000ULL, 0x0001000000000000ULL, 0x0000000000000000ULL,
0x0000000000000000ULL, 0x0000000000000000ULL, 0x4500000000040400ULL, 0x0000800000000400ULL,
0x0000000000000000ULL, 0x13e080000020000cULL, 0xcf00001005100000ULL, 0x04a8008000200300ULL,
0x00280100100000c0ULL, 0x1c8c000040200000ULL, 0x0600005000100000ULL, 0x050800000c104000ULL,
0x4c10101000110000ULL, 0x0c00000000300000ULL, 0x22040c00100000c0ULL, 0x0800700010100000ULL,
0x0000000000001000ULL, 0x0a08000010000040ULL, 0x0800034004210010ULL, 0x04e0000400000000ULL,
0x0800030020000000ULL, 0x0000005000000000ULL, 0x0400110101304110ULL, 0x0428000010a01000ULL,
0x060b000000800010ULL, 0x35810c00c020c000ULL, 0x00800c4321800000ULL, 0x4208088020000080ULL,
0x040000111003ff00ULL, 0x0020900020202080ULL, 0x22888180a8000888ULL, 0x0225200542005420ULL,
0x2020040400340020ULL, 0x10300424500cc444ULL, 0x3081a00400e00200ULL, 0x33001300c0300000ULL,
0x0000000000000000ULL, 0x04003c0000000000ULL, 0x0a04001000100100ULL, 0x1408000001000000ULL,
0x1800000044100000ULL, 0x3400040400000300ULL, 0x5000040801000040ULL, 0x4088401040000040ULL,
0x1010110130100000ULL, 0xca800c3000300000ULL, 0x5a01000000080100ULL, 0x020280000cd01300ULL,
0x0302000410200010ULL, 0x0000102000300000ULL, 0x0b09000000000000ULL, 0x20008004c4800004ULL,
0x28c0410010000000ULL, 0x0004015041000050ULL, 0x0a01006000200200ULL, 0x0020d00000100040ULL,
0x0010a00100900000ULL, 0x3500bf00c0030300ULL, 0x080c010000200d00ULL, 0x2248000004020010ULL,
0x0000c00000000000ULL, 0x8044b00200e08000ULL, 0xaaa82aa2aa8a2aa8ULL, 0x0220002241c08604ULL,
0x4200260440328444ULL, 0x68001226103008b4ULL, 0x3a0080c0b0000400ULL, 0x2a804804803c4008ULL,
0x0000000000000000ULL, 0x04008c0300000400ULL, 0x008000c0000c0000ULL, 0x088001000000001cULL,
0x0840000001000010ULL, 0x0400000000200c00ULL, 0x4244000101040000ULL, 0x4238007011100000ULL,
0x1000d00100000010ULL, 0x1d00800400300000ULL, 0x4204080c00000000ULL, 0x2a88080080000008ULL,
0x08001c0200001000ULL, 0x0a00000400000000ULL, 0x8a88003080080000ULL, 0x0521800400300000ULL,
0x3200051000201000ULL, 0x0000000000000000ULL, 0x0020801404000000ULL, 0x322010401c0c101cULL,
0x0c01100013000000ULL, 0x04003000c0204000ULL, 0x088c0020a0cc0000ULL, 0x2200000080000018ULL,
0x0404000044000000ULL, 0x82a0b000008820b0ULL, 0x0000040020440000ULL, 0xc2650004403f1420ULL,
0x0021340241b64464ULL, 0x8020040242c2d474ULL, 0x32018c0480288000ULL, 0x00800b0080300000ULL,
0x0000000000000000ULL, 0x05008c0000040130ULL, 0xc0d8000000800000ULL, 0x0020000020200200ULL,
0x23a2000120204000ULL, 0x5052100550104150ULL, 0x1000101100040000ULL, 0xc40001c301000000ULL,
0x8288000000c00000ULL, 0x5150040144d01404ULL, 0xea8c0ea028ae088cULL, 0xc31010c000000c80ULL,
0x0002000060000000ULL, 0xc80800f030000000ULL, 0x0000000400300000ULL, 0xc00080c00ff0c344ULL,
0x00080001200c0000ULL, 0x0000050080000000ULL, 0x0328000300300000ULL, 0x082030000cc01040ULL,
0xeb08800100004000ULL, 0x8030003300c80f00ULL, 0xfb0d0000e4ac0000ULL, 0x0020006080000008ULL,
0x0500100100040000ULL, 0x1140000000000000ULL, 0xcb883330a0e00000ULL, 0xc000010050000080ULL,
0x0010104005b54150ULL, 0x40111d5155001554ULL, 0x80000070140f0004ULL, 0x0b0830c3a0003380ULL,
0x0000000000000000ULL, 0x04c13000000f830cULL, 0x2808000000000000ULL, 0x2810000000000800ULL,
0x08c0080004400000ULL, 0x04c0240300801c20ULL, 0x4040000080000004ULL, 0x0000400100100010ULL,
0x020001008000c0c0ULL, 0x1d008c000c3c0000ULL, 0x0080003000000800ULL, 0x2288080080000008ULL,
0x0a84004020220000ULL, 0x0800080000100000ULL, 0xaa80004080400008ULL, 0x8024000400c01660ULL,
0x80841c2001000104ULL, 0x0001000000000000ULL, 0x0020028020020280ULL, 0x0860404011900100ULL,
0xec80080200000000ULL, 0x010103c100200400ULL, 0x0200004000000000ULL, 0x0000000000400400ULL,
0x000010000003fcfcULL, 0x8040083238c20000ULL, 0x08800220a0920a00ULL, 0x08210004483c0c24ULL,
0xc020240740b0a200ULL, 0x802006014a201494ULL, 0x3201233070ac0e00ULL, 0x08002806033a48a0ULL,
0x0000000000000000ULL, 0x8020820028a00680ULL, 0x2000002000000104ULL, 0x22a80801100a0808ULL,
0xa2a8002080000000ULL, 0xa000800008a08000ULL, 0x0000100000400000ULL, 0x8000002100000000ULL,
0x0000010000004404ULL, 0xa2a0088080000888ULL, 0x0000000010400800ULL, 0xa280082080080008ULL,
0x2280000080010008ULL, 0x2000000000000000ULL, 0x228800008c080808ULL, 0x8021828002a98200ULL,
0xa200002000080000ULL, 0x0000040000000000ULL, 0x22a0000080000000ULL, 0x202882c200800080ULL,
0xa000000001004000ULL, 0x000000c808a00600ULL, 0x0000000010000000ULL, 0x000001000000040cULL,
0x0000000000000000ULL, 0x802002a2a8aa82a0ULL, 0x20000024a8088228ULL, 0x8020820001000000ULL,
0x8020000000808280ULL, 0x8000000000000000ULL, 0x0020800000200280ULL, 0x2080082280a00888ULL,
0x0000000000000000ULL, 0x0000015000000040ULL, 0x0000040000040000ULL, 0x0100010010001000ULL,
0x0000003210008000ULL, 0x0000000404000000ULL, 0x0000000000000400ULL, 0x0200000000000000ULL,
0x0000000000000100ULL, 0x5180014400004050ULL, 0x1000000014000000ULL, 0x4200000000000000ULL,
0x0040200000000000ULL, 0x0201004000000000ULL, 0x0a00000000000010ULL, 0x0040200000800000ULL,
0x0040051000000500ULL, 0x0000000100800400ULL, 0x6000000000000000ULL, 0x0000000000000000ULL,
0x280000c1400040ccULL, 0x4180001000000000ULL, 0x00000000c1000104ULL, 0x0000000000000000ULL,
0x0000000000000000ULL, 0x0000000000000000ULL, 0x0080000000c00000ULL, 0x0004006066004000ULL,
0x0000005000040440ULL, 0x0000106005804044ULL, 0x0000a10511004440ULL, 0x0000000000000110ULL,
0x0000000000000000ULL, 0x0000000000080000ULL, 0xeb0808a020800080ULL, 0x29a80081002a1800ULL,
0x0b2c000202100100ULL, 0x0001000000888000ULL, 0x2280102010000000ULL, 0x020000602a004110ULL,
0x8a800160a6108100ULL, 0x0280000000000020ULL, 0x8a8000a0a8808208ULL, 0x0280882080500308ULL,
0x0b18010020804100ULL, 0xeb080000c0080080ULL, 0x2b08000000810130ULL, 0x0000000008040020ULL,
0xaa0a08e082894140ULL, 0x0000000000000000ULL, 0x202081409010001cULL, 0x8aa8805082806000ULL,
0xeb082900289c0000ULL, 0x0000000000008000ULL, 0xf80c2e20002e0000ULL, 0xa288080420880888ULL,
0x0000010000000000ULL, 0x0000000000102000ULL, 0x22880000a8a80808ULL, 0x022022a22aa880a0ULL,
0x0000222222aa0620ULL, 0x0000022002800000ULL, 0x208080004028a000ULL, 0x2b888800801c0828ULL,
0x0000000000000000ULL, 0x22e0828280a08028ULL, 0xaa88002082080308ULL, 0x0ea80080410a0040ULL,
0x2a28222000a00000ULL, 0x8aa2808028a0a2a0ULL, 0x0200001000000000ULL, 0x82080000a0000000ULL,
0x8800000082000808ULL, 0x2a008a0000300888ULL, 0x0a80080080080808ULL, 0xaa882800840b0808ULL,
0x0a80000080000040ULL, 0xea080820a0000000ULL, 0xaa88080080080808ULL, 0x8040a2800a8024a0ULL,
0xaa800020a0080808ULL, 0x0000040000000000ULL, 0x2a280a0080080880ULL, 0x2a20081080008a00ULL,
0x2a88882088aa0008ULL, 0x81800202c0a01480ULL, 0xea88082082200000ULL, 0xaa88002080080008ULL,
0x0000100000000000ULL, 0x802082a22aa0a2a0ULL, 0x2e80000000000000ULL, 0x0220a2a26aa0a2a8ULL,
0x800022a2228a22a0ULL, 0x880002212e82c0b0ULL, 0x02a0aa0002a82228ULL, 0x2d808b0080380008ULL,
0x0000000000000000ULL, 0x000407551c154244ULL, 0x2a00208088a02228ULL, 0x12a82182a2402a88ULL,
0xe32821e020826d00ULL, 0x801130100ccc1330ULL, 0x028010c000841008ULL, 0x88a08002a0a664a0ULL,
0x0048270080000100ULL, 0x00001f010cd10f30ULL, 0xe2242ce22aaea2a0ULL, 0xc2c00cc20ae22460ULL,
0xe208003128021c10ULL, 0x2a2021c010821080ULL, 0x2a88202082202020ULL, 0x4010111104941410ULL,
0xc80c02c182b00080ULL, 0x0000040000000000ULL, 0xe28030068002c300ULL, 0x2aa02024a2a22228ULL,
0xe20889328aa22080ULL, 0x0000000000210100ULL, 0xaa0028e0a9b221a0ULL, 0x2000008080400000ULL,
0x0000010041150404ULL, 0x0000105114410100ULL, 0xeaa82aa6aaaaaaa8ULL, 0x000000f44300c434ULL,
0x0000222222b00020ULL, 0x0000002000000000ULL, 0x0000004014000000ULL, 0x0039b3f73fbcd3fcULL,
0x0000000000000000ULL, 0x0000104015045040ULL, 0x20a80490a08800a0ULL, 0x40a8258410a909a0ULL,
0xe0a8a2022aa2e2a0ULL, 0xc111010014000500ULL, 0x2080044041840004ULL, 0x28a8200220a2aba0ULL,
0x008400a0a2840800ULL, 0x0101015451009464ULL, 0x20000ea0e02c2c2cULL, 0xe2a828a2aca2aaa8ULL,
0x682020a228a222a0ULL, 0xe8882ae22aa2a2a0ULL, 0xe9a80e6022a24140ULL, 0x0011055005001040ULL,
0x2aa8208229a0aaa4ULL, 0x0000040000000000ULL, 0x28a0228026a62260ULL, 0xe2a020a422a2a020ULL,
0xe808a0022aa1a220ULL, 0x0000010014000100ULL, 0x28ac22802aa2a020ULL, 0x0020000000000000ULL,
0x0100010100040000ULL, 0x0000000000000000ULL, 0x22a822a22a8aaaa0ULL, 0x0000000000000000ULL,
0x0000102410800100ULL, 0x0000000000000000ULL, 0x0000000002000000ULL, 0x00000fb2a08c0aa8ULL,
0x0000000000000000ULL, 0x4010005015440140ULL, 0x18c81c00b180001cULL, 0x2800048021820800ULL,
0x8ab820c06a802580ULL, 0x00100170f4040000ULL, 0x4000144041041404ULL, 0x0ac800d0002e440cULL,
0x20880820a2000808ULL, 0x400000f03f300c00ULL, 0xaa000ea22aa22aa0ULL, 0xa2880ac0a8942a20ULL,
0xaa880a81a1804188ULL, 0xeea022a0aaa02080ULL, 0xaaa820a2aaa66120ULL, 0x0000005115800150ULL,
0x2a880920a0840040ULL, 0x0000040000000000ULL, 0xaea82222aaa22a28ULL, 0x8a28041260055150ULL,
0xa28824008aa28880ULL, 0x0000025014019000ULL, 0xea882ae02aa200a0ULL, 0x0000000000000000ULL,
0x0000000040000400ULL, 0x0000000000000000ULL, 0xaaa82aa22aaaaaa0ULL, 0x0000000000000000ULL,
0x0000000000000000ULL, 0x002003003c80c000ULL, 0x0000020014000000ULL, 0x00200010a0980a20ULL,
0x0000000000000000ULL, 0x0020001200801240ULL, 0x0a88000089800020ULL, 0xcaa00080a1000000ULL,
0x0a200c0020a04080ULL, 0x4002034003840880ULL, 0x4690500190000050ULL, 0x2228004000601000ULL,
0x0a803f00803f400cULL, 0x400033e24dd0cf34ULL, 0xaa80a2a229a220a0ULL, 0x0a224000002c0000ULL,
0x028000202000008cULL, 0x0a08000070000030ULL, 0x00800c040020000cULL, 0x0000000002850000ULL,
0x02881cc310200000ULL, 0x0000040004000000ULL, 0xcba8000400000080ULL, 0xcaa02c0680000000ULL,
0xcc880002008c4080ULL, 0x300000f007f0cf0cULL, 0x0a80001080a00000ULL, 0x820880802a880a80ULL,
0x0000050001040004ULL, 0x0000011000000000ULL, 0x0a8020a2a0202000ULL, 0x0000022202008000ULL,
0x0000222212808000ULL, 0x0020226010000000ULL, 0x000033f33ff3c33cULL, 0x00288002a08c02a8ULL,
0x0000000000000000ULL, 0x04408e0000008200ULL, 0x0808004000900000ULL, 0x0aa8200010ca00c0ULL,
0x0ba80101005d4010ULL, 0x00018604802c8288ULL, 0x00049400101c0000ULL, 0x000c101110505010ULL,
0x0000000000100000ULL, 0x30000c00c022000cULL, 0xd0c00dd0d51d431cULL, 0x0008000010100000ULL,
0x000c1001a0280000ULL, 0x0bc80000c0000000ULL, 0x0a00000080280000ULL, 0x8000a00220308420ULL,
0x0808000010301000ULL, 0x0000040000000000ULL, 0x0d00031480100000ULL, 0x07200000108c0300ULL,
0x0bc0a0c000004000ULL, 0x8000b002c0208480ULL, 0x340c0100118c111cULL, 0x8008008020890000ULL,
0x0000000000040010ULL, 0x0020b00320c1d0b0ULL, 0x00002000000c0000ULL, 0x0020be226e2008a0ULL,
0x002010c03fb0a6a0ULL, 0x00202e222aaec284ULL, 0x00008f0000208400ULL, 0x0000000000300000ULL,
};
// Latin1 6%, Latin2 11%, Latin7 3%



// Just for debugging. not thread-safe
static char tri_string[4];
char* Latin127Str(int trisub) {
  tri_string[0] = "_abcdefghijklmnopqrstuvwxyzAEIOC"[(trisub >> 10) & 0x1f];
  tri_string[1] = "_abcdefghijklmnopqrstuvwxyzAEIOC"[(trisub >> 5) & 0x1f];
  tri_string[2] = "_abcdefghijklmnopqrstuvwxyzAEIOC"[(trisub >> 0) & 0x1f];
  tri_string[3] = '\0';
  return tri_string;
}

// Returns two bits per three-byte trigram, indicating
// dont-care, Latin1 likely, Latin2 likely, and Latin7 (ISO-8859-13) likely
int TrigramValue(const uint8* trisrc) {
  int byte0_p = kMapToFiveBits[trisrc[0]];
  int byte1_p = kMapToFiveBits[trisrc[1]];
  int byte2_p = kMapToFiveBits[trisrc[2]];
  int subscr = ((byte0_p) << 5) | byte1_p;
  auto temp = static_cast<int>((kLatin127Trigrams[subscr] >> (byte2_p * 2)));
  //printf("%s=%d ", Latin127Str((subscr << 5) | byte2_p), temp & 3);
  return temp & 3;
}


// Put out trigrams for surrounding 32 bytes for Latin encodings
// Return true if more Latin2 & 7 than Latin1
bool BoostLatin127Trigrams(int tri_block_offset,
                           DetectEncodingState* destatep) {
  //printf("BoostLatin127Trigrams[%06x]\n", tri_block_offset);
  int excess_latin27 = 0;
  auto srclen = static_cast<int>(destatep->limit_src - destatep->initial_src);
  int hi_limit = minint(tri_block_offset + 32, srclen - 2);
  const uint8* trisrc = &destatep->initial_src[tri_block_offset];
  const uint8* trisrclimit = &destatep->initial_src[hi_limit];
  while (trisrc < trisrclimit) {
    // Selectively boost Latin1, Latin2, or Latin7 and friends
    int trigram_val = TrigramValue(trisrc);
    if (trigram_val != 0) {
      if (FLAGS_enc_detect_source) {
        PsHighlight(trisrc, destatep->initial_src, trigram_val, 1);
      }
      if (trigram_val == kTriLatin1Likely) {
        Boost(destatep, F_Latin1, kTrigramBoost);
        Boost(destatep, F_CP1252, kTrigramBoost);
        // We don't want to upset the relative rank of a declared 8859-15
        Boost(destatep, F_ISO_8859_15, kTrigramBoost);
        --excess_latin27;
      } else if (trigram_val == kTriLatin2Likely) {
        Boost(destatep, F_Latin2, kTrigramBoost);
        Boost(destatep, F_CP1250, kTrigramBoost);
        ++excess_latin27;
      } else if (trigram_val == kTriLatin7Likely) {
        Boost(destatep, F_ISO_8859_13, kTrigramBoost);
        Boost(destatep, F_CP1257, kTrigramBoost);
        // We don't want to upset the relative rank of a declared 8859-4 or -6
        // for Estonian
        Boost(destatep, F_Latin4, kTrigramBoost);
        Boost(destatep, F_Latin6, kTrigramBoost);
        ++excess_latin27;
      }
    }

    ++trisrc;
  }
  //printf("\n");

  return (0 < excess_latin27);
}



// Boost any encodings that need extra detection help, then prune
// src is first unscanned byte
// slowend means extra pruning when dropping out of initial slow scan
// final means last call -- no bigram at src
void BoostPrune(const uint8* src, DetectEncodingState* destatep,
                int prunereason) {
  int delta_asciipairs = destatep->next_interesting_pair[AsciiPair] -
    destatep->prior_interesting_pair[AsciiPair];
  int delta_otherpairs = destatep->next_interesting_pair[OtherPair] -
    destatep->prior_interesting_pair[OtherPair];

  if (prunereason == PRUNE_FINAL) {
    // We are about done
    // If we get here with very little accumulated data, the initial hints
    // were too strong, so we derate them to n+1 / 12 for n bigrams
    if (!destatep->hints_derated  &&
        (destatep->next_interesting_pair[OtherPair] < kDerateHintsBelow)) {
      int n = destatep->next_interesting_pair[OtherPair];

      // Map N pairs to (N+1)/12 portions of the initial hints, etc.
      // Floor of 3/12 -- 1/12 and 2/12 are too easy to overcome
      int m = maxint(3, (n + 1));
      for (int i = 0; i < NUM_RANKEDENCODING; ++i) {
        int original_delta = destatep->hint_prob[i];
        int scaled_delta = (original_delta * m) / kDerateHintsBelow;
        destatep->enc_prob[i] -= original_delta;
        destatep->enc_prob[i] += scaled_delta;
      }
      destatep->hints_derated = true;
      if (destatep->debug_data != nullptr) {
        // Show derated-hint result
        char buff[32];
        snprintf(buff, sizeof(buff), "Hints %d/%d", m, kDerateHintsBelow);
        SetDetailsEncLabel(destatep, buff);
      }
    }
  }


  ++destatep->prune_count;

  if (prunereason != PRUNE_FINAL) {
    // Early outs
    if (destatep->rankedencoding_list_len <= 1) {            // nothing to prune
      destatep->done = true;
      return;
    }

    if ((destatep->prune_count > 0) &&
        (delta_asciipairs + delta_otherpairs) == 0) {
      // Nothing to do; must have just been called earlier
      return;
    }
  }



  // INCREMENT
  // ====================
  // Accumulate OtherPair probibilities over all active families
  // AsciiPair probibilities are all done in ActiveSpecialBoostWhack
  uint8 prior_bad_byte1 = ' ';    // won't match first bad pair
  uint8 prior_bad_byte2 = ' ';    // won't match first bad pair
  uint8 or_byte1 = 0;             // Track if any current pair has a high bit
  int counted_otherpairs = 0;
  uint8 prior_byte1x2x = 0;
  for (int i = 0; i < delta_otherpairs; ++i) {
    int watch1_incr = 0;
    int watch2_incr = 0;
    int next_pair = destatep->prior_interesting_pair[OtherPair] + i;

    uint8 byte1 = destatep->interesting_pairs[OtherPair][next_pair * 2 + 0];
    uint8 byte2 = destatep->interesting_pairs[OtherPair][next_pair * 2 + 1];
    uint8 byte1x2x = (byte1 & 0xf0) | ((byte2 >> 4) & 0x0f);
    int weightshift = destatep->interesting_weightshift[OtherPair][next_pair];

    int offset_byte12 = destatep->interesting_offsets[OtherPair][next_pair];

    // To help distinguish some Cyrillic, Arabic, Greek, Hebrew, Thai
    // Remember if this is a CDEF pair immediately following the previous pair
    // 8xxx CxCx or CxCx 8xxx
    bool next_pair_consec_hi = false;
    if (ConsecutivePair(destatep, next_pair)) {
      if ((byte1x2x & 0xcc) == 0xcc) {                // 8xxx CxCx
        next_pair_consec_hi = true;
      } else if ((prior_byte1x2x & 0xcc) == 0xcc) {   // CxCx 8xxx
        next_pair_consec_hi = true;
      }
    }
    //printf("prior/cur/consec %02x %02x %d\n",
    // prior_byte1x2x, byte1x2x, next_pair_consec_hi);
    prior_byte1x2x = byte1x2x;

    or_byte1 |= byte1;
    uint8 byte1f = byte1;
    // Flip top bit of subscript to better separate quadrant 4 (esp. for Hebrew)
    byte1f ^= (byte2 & 0x80);

    // If the same bigram occurred recently, don't increment again
    bool pair_used = false;
    if (!RepeatedBigram(destatep, byte1, byte2)) {
      ++counted_otherpairs;
      pair_used = true;
      // Boost both charset= declared encodings, so
      // Nearly-same probability nearby encoding doesn't drift to the top
      if (!FLAGS_demo_nodefault) {
        destatep->enc_prob[destatep->declared_enc_1] += kDeclaredEncBoost >> weightshift;
        destatep->enc_prob[destatep->declared_enc_2] += kDeclaredEncBoost >> weightshift;
      }
      bool was_bad_pair = false;
      for (int j = 0; j < destatep->rankedencoding_list_len; j++) {
        int incr_shift = 0;
        int rankedencoding = destatep->rankedencoding_list[j];
        Encoding enc = kMapToEncoding[rankedencoding];

        // For binary, Skip over repeated marker bytes, such as 02, FF, etc.
        if ((rankedencoding == F_BINARY) &&
            RepeatedBinary(destatep, byte1, byte2)) {
          incr_shift = 2;       // count 1/4 as much if repeated
        }

        // If byte 1x2x for this encoding is exactly zero, illegal byte pair
        // Don't increment, but instead penalize
        const UnigramEntry* ue = &unigram_table[rankedencoding];
        if (ue->b12[byte1x2x] == 0) {
          // Don't whack consecutive duplicate bad pairs -- overkill
          if ((byte1 != prior_bad_byte1) || (byte2 != prior_bad_byte2)) {
            // Extra whack for illegal pair in this encoding
            Whack(destatep, rankedencoding, kBadPairWhack >> weightshift);
            was_bad_pair = true;
          }
        } else {
          // OK to do the real increment
          int incr = ue->b1[byte1f] + ue->b2[byte2] + ue->b12[byte1x2x];
          if ((ue->b12[byte1x2x] & 0x01) != 0) {
            // Use a more-precise table
            int byte32x32 = ((byte1 & 0x1f) << 5) | (byte2 & 0x1f);
            int hiressub = (byte2 & 0x60) >> 5;   // select w/bits 5&6 of byte 2
            DCHECK(ue->hires[hiressub] != NULL);
            incr += ue->hires[hiressub][byte32x32];
          } else {
            // Default final offset
            incr += ue->so;
          }
          incr >>= incr_shift;

          incr >>= weightshift;
          destatep->enc_prob[rankedencoding] += incr;   // The actual increment

          if (FLAGS_enc_detect_detail2) {
            if (watch1_rankedenc == rankedencoding) {watch1_incr = incr;}
            if (watch2_rankedenc == rankedencoding) {watch2_incr = incr;}
          }
        }


        // If consecutive pair of high bytes, give slight boost to one-byte
        // encodings that have a full alphabet in the high bytes
        if (next_pair_consec_hi && HighAlphaEncoding(enc)) {
          Boost(destatep, rankedencoding, kDeclaredEncBoost >> weightshift);
        }
      }     // End for j < rankedencoding_list_len

      if (was_bad_pair) {
        prior_bad_byte1 = byte1;
        prior_bad_byte2 = byte2;
      }

      // Fold in per-bigram most likely encoding for first N bigrams
      if (next_pair < kBestPairsCount) {
        int best_enc = kMostLikelyEncoding[(byte1 << 8) + byte2];
        Boost(destatep, best_enc, kBestEncBoost >> weightshift);
      }

      // Possibly score 32 trigrams around a bigram to better separate
      // Latin1 from Latin2 and Latin7. Especially helpful for detecting
      // mis-labelled Hungarian latin2.
      // If looking and at bigram 0,8,16,... do full scoring, else just 1 tri
      if (destatep->do_latin_trigrams ||
          destatep->looking_for_latin_trigrams) {
        // If just looking, do full scan every 8 times
        // Just look up one trigram the other 7 and do full scan if Latin2,7
        bool scan32 = false;
        const uint8* trisrc = &destatep->initial_src[offset_byte12 - 1];
        if (!destatep->do_latin_trigrams) {
          if ((i & 7) == 0 || trisrc + 3 > destatep->limit_src) {
            scan32 = true;
          } else {
            scan32 = (kTriLatin1Likely < TrigramValue(trisrc));
          }
        }
        if (destatep->do_latin_trigrams || scan32) {
          // Just score each block of 32 bytes once
          int tri_block_offset = offset_byte12 & ~0x1f;
          if (destatep->trigram_highwater_mark <= tri_block_offset) {
            bool turnon = BoostLatin127Trigrams(tri_block_offset, destatep);
            if (FLAGS_counts && !destatep->do_latin_trigrams && turnon) {
              ++doing_used;    // First time
            }
            if (FLAGS_enc_detect_source) {
              if (!destatep->do_latin_trigrams && turnon) {
                // First time
                PsHighlight(trisrc, destatep->initial_src, 0, 2);
              }
            }
            destatep->do_latin_trigrams |= turnon;
            destatep->trigram_highwater_mark = tri_block_offset + 32;
          }
        }
      }

    }       // end if RepeatedBigram()

    // Keep track of initial byte high 3 bits
    ++destatep->byte32_count[byte1 >> 5];


    // TODO: boost subset/superset also
    // Boost(destatep, kRelatedEncoding[best_enc], kBestEncBoost);

    if (destatep->debug_data != nullptr) {
      // Show detail entry for this bigram
      char buff[16];
      snprintf(buff, sizeof(buff), "%c%02x%02x%c%c",
               pair_used ? ' ' : '[',
               byte1,
               byte2,
               pair_used ? ' ' : ']',
               (weightshift == 0) ? ' ' : '-');

      SetDetailsEncProb(destatep,
                        destatep->interesting_offsets[OtherPair][next_pair],
                        kMostLikelyEncoding[(byte1 << 8) + byte2],
                        buff);
    }
    if (FLAGS_enc_detect_detail2) {
      if ((watch1_incr != 0) || (watch2_incr != 0)) {
        // Show increment detail for this encoding
        char buff[32];
        snprintf(buff, sizeof(buff), "%c%d %c%d",
                 (watch1_incr < 0) ? '-' : '+', watch1_incr,
                 (watch2_incr < 0) ? '-' : '+', watch2_incr);
        SetDetailsEncLabel(destatep, buff);
      }
    }
  }       // End for i


  // If no high bit on, demote all the two-byte codes
  // WAS BUG. This was inside the loop above and should be outside
  if ((counted_otherpairs > 0) && ((or_byte1 & 0x80) == 0)) {
    // No high bit in this group (just 02xx, etc.). Whack 2-byte codes
    // This keeps SJS from creeping past Latin1 on illegal C0 bytes
    for (int j = 0; j < destatep->rankedencoding_list_len; j++) {
      int rankedencoding = destatep->rankedencoding_list[j];
      Encoding enc = kMapToEncoding[rankedencoding];
      if (TwoByteEncoding(enc)) {
        Whack(destatep, rankedencoding, kGentlePairWhack * counted_otherpairs);
      }
    }
  }


  // BOOST
  // ====================
  if (AnyActive(destatep)) {
    ActiveSpecialBoostWhack(src, destatep);
  }

  // Update for next time
  destatep->prior_src = src;
  destatep->prior_interesting_pair[AsciiPair] =
    destatep->next_interesting_pair[AsciiPair];
  destatep->prior_interesting_pair[OtherPair] =
    destatep->next_interesting_pair[OtherPair];


  // Do any pre-prune final adjustments
  // ====================
  if (prunereason == PRUNE_FINAL) {
    // If UTF8 not in base state, whack
    if (destatep->next_utf8_ministate != 0) {
      Whack(destatep, F_UTF8, kGentlePairWhack * 2 * 1);
    }
    // If UTF8UTF8 not in base state, whack
    if (destatep->next_utf8utf8_ministate != 0) {
      Whack(destatep, F_UTF8UTF8, kGentlePairWhack * 2 * 1);
    }

    // If no valid UTF-8 char ever seen, whack
    if (destatep->utf8_minicount[5] == 0) {
      Whack(destatep, F_UTF8, kBadPairWhack * 8);           // No sequence
      Whack(destatep, F_UTF8UTF8, kBadPairWhack * 8);       // No sequence
    }

    // If no valid UTF8UTF8 char ever seen, whack
    if (destatep->utf8utf8_minicount[5] == 0) {
      Whack(destatep, F_UTF8UTF8, kBadPairWhack * 8);       // No sequence
    }

    // If not all four binary quadrants, whack BINARY;
    // worth 2 pair if 3 quads, 4 pair if 1 or 2 quads
    if (destatep->binary_quadrants_count < 4) {
      if (destatep->binary_quadrants_count == 3) {
        Whack(destatep, F_BINARY, kBadPairWhack * 2);
      } else {
        Whack(destatep, F_BINARY, kBadPairWhack * 4);
      }
    }

    // If 1st pair is 1b24, choose between ISO-2022-xx
    //  <esc> $ ) C ISO-2022-KR   [1b 24 29 43]
    //  <esc> $ ) A ISO-2022-CN   [1b 24 29 41]
    //  <esc> $ ) G ISO-2022-CN   [1b 24 29 47]
    //  <esc> $ * H ISO-2022-CN   [1b 24 2a 48]
    //  <esc> ( B ISO-2022-JP     [1b 28 42]  to ASCII
    //  <esc> ( J ISO-2022-JP     [1b 28 4a]  to X0201
    //  <esc> $ @ ISO-2022-JP     [1b 24 40]  to X0208-78 twobyte
    //  <esc> $ B ISO-2022-JP     [1b 24 42]  to X0208-83 twobyte
    if ((destatep->next_interesting_pair[OtherPair] >= 1) &&
        Iso2022Active(destatep)) {
      if ((destatep->interesting_pairs[OtherPair][0] == 0x1b) &&
          (destatep->interesting_pairs[OtherPair][1] == 0x24)) {
        int offset = destatep->interesting_offsets[OtherPair][0];
        const uint8* esc_src = destatep->initial_src + offset;
        if ((destatep->initial_src + offset) < (destatep->limit_src - 3)) {
          if ((esc_src[2] == ')') && (esc_src[3] == 'C')) {
            Boost(destatep, F_ISO_2022_KR, kBoostOnePair);
            Whack(destatep, F_ISO_2022_CN, kBadPairWhack);
            Whack(destatep, F_JIS, kBadPairWhack);
          } else if ((esc_src[2] == ')') && ((esc_src[3] == 'A') ||
                                             (esc_src[3] == 'G'))) {
            Boost(destatep, F_ISO_2022_CN, kBoostOnePair);
            Whack(destatep, F_ISO_2022_KR, kBadPairWhack);
            Whack(destatep, F_JIS, kBadPairWhack);
          } else if ((esc_src[2] == '@') || (esc_src[2] == 'B')) {
            Boost(destatep, F_JIS, kBoostOnePair);
            Whack(destatep, F_ISO_2022_CN, kBadPairWhack);
            Whack(destatep, F_ISO_2022_KR, kBadPairWhack);
          }
        } else {
          // Incomplete escape sequence. Whack them all
          Whack(destatep, F_JIS, kBadPairWhack);
          Whack(destatep, F_ISO_2022_CN, kBadPairWhack);
          Whack(destatep, F_ISO_2022_KR, kBadPairWhack);
        }
      }
    }
    if (destatep->debug_data != nullptr) {
      SetDetailsEncLabel(destatep, "pre-final");
    }
  }

  // PRUNE
  // ====================
  // Find current top two rankedencoding probabilities
  ReRank(destatep);

  if (prunereason == PRUNE_SLOWEND) {
    if (destatep->debug_data != nullptr) {
      SetDetailsEncLabel(destatep, "slow-end");
    }
  }

  // Keep every rankedencoding with probablity >= top_prob - prune_difference
  int prune_diff = destatep->prune_difference;
  // If the top encoding is BINARY, it might be overstated, and we might
  // therefore prune away the real encoding. Make the pruning delta
  // twice as big.
  if (destatep->top_rankedencoding == F_BINARY) {
    prune_diff *= 2;
  }
  int keep_prob = destatep->top_prob - prune_diff;

  // Tighten pruning difference (we start wide) for next time
  if (destatep->prune_difference > kFinalPruneDifference) {
    int decrement = kPruneDiffDecrement;
    // If only ASCII pairs, small tighten; if some non-ASCII, full tighten
    if (counted_otherpairs == 0) {
      decrement >>= 1;
    }
    destatep->prune_difference -= decrement;
  }

  // Prune the list of active encoding families
  destatep->active_special = 0;
  int k = 0;
  for (int j = 0; j < destatep->rankedencoding_list_len; j++) {
    bool keep = true;
    int rankedencoding = destatep->rankedencoding_list[j];

    // If count is too low, ditch it
    if (destatep->enc_prob[rankedencoding] < keep_prob) {
      keep = false;
    }

    // If at end of slow section, ditch any 7-bit with zero evidence so far
    if ((prunereason == PRUNE_SLOWEND) &&
        SevenBitEncoding(kMapToEncoding[rankedencoding]) &&
        (destatep->enc_prob[rankedencoding] <= 0) &&
        (rankedencoding != destatep->top_rankedencoding)) {
      keep = false;
    }

    // Keep it. This will always keep at least top_prob rankedencoding
    if (keep) {
      destatep->active_special |= kSpecialMask[kMapToEncoding[rankedencoding]];
      destatep->rankedencoding_list[k++] = rankedencoding;
    }
  }

  if (destatep->debug_data != nullptr) {
    char buff[32];
    snprintf(buff, sizeof(buff), "%d prune", prune_diff / XLOG2);
    SetDetailsEncLabel(destatep, buff);
  }
  destatep->rankedencoding_list_len = k;



  // Force final result in some cases
  // Do any post-prune final adjustments
  if (prunereason == PRUNE_FINAL) {
    // If no high-byte pairs, result is ASCII7, BINARY, UTF7, 2022, or HZ
    if (destatep->next_interesting_pair[OtherPair] == 0) {
      if ((destatep->top_rankedencoding != F_BINARY) &&
          (destatep->top_rankedencoding != F_UTF7) &&
          (destatep->top_rankedencoding != F_ISO_2022_CN) &&
          (destatep->top_rankedencoding != F_ISO_2022_KR) &&
          (destatep->top_rankedencoding != F_JIS) &&
          (destatep->top_rankedencoding != F_HZ_GB_2312)) {
        destatep->top_rankedencoding = F_ASCII_7_bit;
        Boost(destatep, F_ASCII_7_bit, kBoostOnePair * 2);
      }
    }

    // If some 89 pairs, not ISO_8859_x  and vice versa
    if (destatep->byte32_count[4] > 0) {
      switch (destatep->top_rankedencoding) {
      case F_ASCII:         // ISO-8859-1
        destatep->top_rankedencoding = F_CP1252;
        // Better: destatep->enc_prob[F_ASCII] <==> destatep->enc_prob[F_CP1252]
        Boost(destatep, F_CP1252, kBoostOnePair * 2);
        break;
      case F_Latin2:        // ISO-8859-2
        // Don't swap back; not superset
        //destatep->top_rankedencoding = F_CP1250;
        //Boost(destatep, F_CP1250, kBoostOnePair * 2);
        break;
      case F_Arabic:         // ISO-8859-6
        destatep->top_rankedencoding = F_CP1256;
        Boost(destatep, F_CP1256, kBoostOnePair * 2);
        break;
      case F_Greek:         // ISO-8859-7
        // Don't swap -- not proper superset
        // Capital Alpha tonos at 0xB6 in ISO-8859-7, 0xA2 in CP1253
        //destatep->top_rankedencoding = F_CP1253;
        //Boost(destatep, F_CP1253, kBoostOnePair * 2);
        break;
      case F_Hebrew:        // ISO-8859-8
        // Don't swap -- visual vs. logical
        //destatep->top_rankedencoding = F_CP1255;
        //Boost(destatep, F_CP1255, kBoostOnePair * 2);
        break;
      case F_Latin5:        // ISO-8859-9
        destatep->top_rankedencoding = F_CP1254;
        Boost(destatep, F_CP1254, kBoostOnePair * 2);
        break;
      case F_ISO_8859_11:   // ISO-8859-11
        destatep->top_rankedencoding = F_CP874;
        Boost(destatep, F_CP874, kBoostOnePair * 2);
        break;
      }
    } else {
      switch (destatep->top_rankedencoding) {
      case F_CP1252:        // ISO-8859-1
        destatep->top_rankedencoding = F_ASCII;
        Boost(destatep, F_ASCII, kBoostOnePair * 2);
        break;
      case F_CP1250:        // ISO-8859-2
        // Don't swap back; not superset
        //destatep->top_rankedencoding = F_Latin2;
        //Boost(destatep, F_Latin2, kBoostOnePair * 2);
        break;
      case F_CP1256:        // ISO-8859-6
        // Don't swap back -- not proper superset
        //destatep->top_rankedencoding = F_Arabic;
        //Boost(destatep, F_Arabic, kBoostOnePair * 2);
        break;
      case F_CP1253:        // ISO-8859-7
        // Don't swap back -- not proper superset
        //destatep->top_rankedencoding = F_Greek;
        //Boost(destatep, F_Greek, kBoostOnePair * 2);
        break;
      case F_CP1255:        // ISO-8859-8
        // Don't swap back -- not proper superset
        //destatep->top_rankedencoding = F_Hebrew;
        //Boost(destatep, F_Hebrew, kBoostOnePair * 2);
        break;
      case F_CP1254:        // ISO-8859-9
        destatep->top_rankedencoding = F_Latin5;
        Boost(destatep, F_Latin5, kBoostOnePair * 2);
        break;
      case F_CP874:         // ISO-8859-11
        destatep->top_rankedencoding = F_ISO_8859_11;
        Boost(destatep, F_ISO_8859_11, kBoostOnePair * 2);
        break;
      }
    }

    if (destatep->debug_data != nullptr) {
      char buff[32];
      snprintf(buff, sizeof(buff), "final %d",
               static_cast<int>(src - destatep->initial_src));
      SetDetailsEncLabel(destatep, buff);

      // Show winning encoding and its delta log base2 from 2nd-best
      // Divide delta by XLOG2 to get log base 2
      int delta = destatep->top_prob - destatep->second_top_prob;
      if (delta < (2 * XLOG2)) {
        delta /= XDECILOG2;
        snprintf(buff, sizeof(buff), "+%d.%d %s ",
                 delta / 10, delta % 10,
                 MyEncodingName(kMapToEncoding[destatep->top_rankedencoding]));
      } else if (delta < (50 * XLOG2)) {
        delta /= XLOG2;
        snprintf(buff, sizeof(buff), "+%d %s",
                 delta,
                 MyEncodingName(kMapToEncoding[destatep->top_rankedencoding]));
      } else {
        snprintf(buff, sizeof(buff), "%s",
                 MyEncodingName(kMapToEncoding[destatep->top_rankedencoding]));
      }
      SetDetailsEncProbCopyOffset(destatep, destatep->top_rankedencoding, buff);
    }
  }


  // FINISH
  // ====================
  // Eventual encoding result is reliable if big difference in top two, or if
  // only Ascii7 ever encountered
  // Also reliable if exactly one OtherPair and it's best encoding matches top
  destatep->reliable = false;
  if (destatep->next_interesting_pair[OtherPair] == 0) {
    // Only 7-bit ASCII
    destatep->reliable = true;
  }
  if ((destatep->top_prob - destatep->second_top_prob) >=
      FLAGS_ced_reliable_difference) {
    destatep->reliable = true;
  }
  if (destatep->next_interesting_pair[OtherPair] == 1) {
    uint8 byte1 = destatep->interesting_pairs[OtherPair][0];
    uint8 byte2 = destatep->interesting_pairs[OtherPair][1];
    int best_enc = kMostLikelyEncoding[(byte1 << 8) + byte2];
    if (best_enc == destatep->top_rankedencoding) {
      destatep->reliable = true;
    }
  }

  // If we pruned to one encoding, we are done
  if (destatep->rankedencoding_list_len == 1) {
    destatep->reliable = true;
    destatep->done = true;
  }

  // If we pruned to two or three encodings in the same *superset/subset
  // rankedencoding*  and enough pairs, we are done. Else keep going
  if (destatep->rankedencoding_list_len == 2) {
    Encoding enc0 = kMapToEncoding[destatep->rankedencoding_list[0]];
    Encoding enc1 = kMapToEncoding[destatep->rankedencoding_list[1]];
    if (kMapEncToBaseEncoding[enc0] == kMapEncToBaseEncoding[enc1]) {
      if (destatep->prune_count >= 3) {
        destatep->reliable = true;
        destatep->done = true;
      }
    }
  } else if (destatep->rankedencoding_list_len == 3) {
    Encoding enc0 = kMapToEncoding[destatep->rankedencoding_list[0]];
    Encoding enc1 = kMapToEncoding[destatep->rankedencoding_list[1]];
    Encoding enc2 = kMapToEncoding[destatep->rankedencoding_list[2]];
    Encoding base0 = kMapEncToBaseEncoding[enc0];
    Encoding base1 = kMapEncToBaseEncoding[enc1];
    Encoding base2 = kMapEncToBaseEncoding[enc2];

    if ((base0 == base1) && (base0 == base2)) {
      if (destatep->prune_count >= 3) {
        destatep->reliable = true;
        destatep->done = true;
      }
    }
  }
}


// Accumulate aligned byte-pair at src
// Occasionally, calc boost for some encodings and then prune the active list
// weightshift is used to give low weight some text, such as inside tags
// Returns true if pruning occurred
bool IncrementAndBoostPrune(const uint8* src,
                            int remaining_length,
                            DetectEncodingState* destatep,
                            int weightshift,
                            int exit_reason) {
  destatep->last_pair = src;
  // Pick up byte pair, or very last byte plus 0x20
  uint8 byte1 = src[0];
  uint8 byte2 = 0x20;
  if (1 < remaining_length) {byte2 = src[1];}

  // whatset=0 for Ascii + ~, 1 for all others; see kTestPrintableAsciiTildePlus
  int whatset = exit_reason - 1;
  int next_pair = destatep->next_interesting_pair[whatset];

  if (next_pair > 16) {
    // If not clear by 16 bigrams, stop accumulating + ~ 00
    if (byte1 == '+') {return false;}
    if (byte1 == '~') {return false;}
    if (byte1 == 0x00) {return false;}
  }

  // Remember pair in appropriate list
  if (next_pair >= kMaxPairs) {
    // We have filled up our alloted space for interesting pairs with no
    // decision. If ASCII pairs full, just skip until end of slow loop; if
    // non-Ascii pairs full, force done
    if (whatset == OtherPair) {
      destatep->done = true;
    }
  } else {
    auto offset = static_cast<int>(src - destatep->initial_src);
    destatep->interesting_pairs[whatset][next_pair * 2 + 0] = byte1;
    destatep->interesting_pairs[whatset][next_pair * 2 + 1] = byte2;
    destatep->interesting_offsets[whatset][next_pair] = offset;
    destatep->interesting_weightshift[whatset][next_pair] = weightshift;
    ++destatep->next_interesting_pair[whatset];
    ++next_pair;
  }

  // Prune now and then , but always if forced to be done
  if (destatep->done || ((next_pair & kPruneMask) == 0)) {  // Prune every M
    BoostPrune(src + 2, destatep, PRUNE_NORMAL);  // src+2 first unscanned byte
                                                  // may be off end of input
    return true;
  }
  return false;
}

void DumpSummary(DetectEncodingState* destatep, int whatset, int n) {
  printf("  %sSummary[%2d]: ", kWhatSetName[whatset],
         destatep->next_interesting_pair[whatset]);
  int limit = minint(n, destatep->next_interesting_pair[whatset]);
  for (int i = 0; i < limit; ++i) {
    printf("%02x%02x ",
           destatep->interesting_pairs[whatset][i * 2 + 0],
           destatep->interesting_pairs[whatset][i * 2 + 1]);
    if ((i & 7) == 7) {printf("  ");}
  }
  printf("\n");
}

void BeginDetail(DetectEncodingState* destatep) {
  fprintf(stderr, "%d [", NUM_RANKEDENCODING);
  for (int e = 0; e < NUM_RANKEDENCODING; ++e) {
    fprintf(stderr, "(%s)",  MyRankedEncName(e));
    if ((e % 10) == 9) {fprintf(stderr, "\n    ");}
  }
  fprintf(stderr, "] size-detail\n");
  destatep->next_detail_entry = 0;
}

// Single character to represent (printable ASCII) gap between bigrams
char DetailOffsetChar(int delta) {
  if (delta == 0) {return ' ';}
  if (delta <= 2) {return '=';}
  if (delta <= 15) {return '_';}
  if (delta <= 31) {return '+';}
  {return ' ';}
}

void DumpDetail(DetectEncodingState* destatep) {
  // Turn all counts into delta from previous entry
  fprintf(stderr, "%d count-detail\n", destatep->next_detail_entry);
  // Rewrite, recording deltas
  for (int z = destatep->next_detail_entry - 1; z > 0; --z) {
    destatep->debug_data[z].offset -= destatep->debug_data[z - 1].offset;
    for (int e = 0; e < NUM_RANKEDENCODING; ++e) {
      destatep->debug_data[z].detail_enc_prob[e] -=
        destatep->debug_data[z - 1].detail_enc_prob[e];
    }
  }
  // Now print
  for (int z = 0; z < destatep->next_detail_entry; ++z) {
    // Highlight some entries ending in '!' with light red underbar
    auto len = static_cast<int>(destatep->debug_data[z].label.size());
    if (destatep->debug_data[z].label[len - 1] == '!') {
      fprintf(stderr, "1 0.9 0.9 do-flag\n");
    }
    fprintf(stderr, "(%c%s) %d [",
            DetailOffsetChar(destatep->debug_data[z].offset),
            destatep->debug_data[z].label.c_str(),
            destatep->debug_data[z].best_enc);
    for (int e = 0; e < NUM_RANKEDENCODING; ++e) {
      fprintf(stderr, "%d ", destatep->debug_data[z].detail_enc_prob[e]);
      if ((e % 10) == 9) {fprintf(stderr, "  ");}
    }
    fprintf(stderr, "] do-detail-e\n");
  }
  // Get ready for next time,if any
  destatep->next_detail_entry = 0;
}

void PsRecurse(const char* buff) {
  fprintf(stderr, "() end-detail (%s) start-detail\n\n", buff);
}

void DumpReliable(DetectEncodingState* destatep) {
  printf("Not reliable: ");

  // Find center of gravity of OtherPair list
  int x_sum = 0;
  int y_sum = 0;
  int count = destatep->next_interesting_pair[OtherPair];
  for (int i = 0; i < count; ++i) {
    uint8 byte1 = destatep->interesting_pairs[OtherPair][i * 2 + 0];
    uint8 byte2 = destatep->interesting_pairs[OtherPair][i * 2 + 1];
    x_sum += byte2;
    y_sum += byte1;
  }
  if (count == 0) {count = 1;}    // adoid zdiv
  int x_bar = x_sum / count;
  int y_bar = y_sum / count;
  printf("center %02X,%02X\n", x_bar, y_bar);

  double closest_dist = 999.0;
  int closest = 0;
  for (int j = 0; j < destatep->rankedencoding_list_len; j++) {
    int rankedencoding = destatep->rankedencoding_list[j];
    const UnigramEntry* ue = &unigram_table[rankedencoding];
    printf("  %8s = %4d at %02x,%02x +/- %02X,%02X ",
           MyEncodingName(kMapToEncoding[rankedencoding]),
           destatep->enc_prob[rankedencoding],
           ue->x_bar, ue->y_bar,
           ue->x_stddev, ue->y_stddev);
    double x_diff = x_bar - ue->x_bar;
    double y_diff = y_bar - ue->y_bar;
    double dist = sqrt((x_diff * x_diff) + (y_diff * y_diff));
    printf("(%3.1f)\n", dist);

    if (closest_dist > dist) {
      closest_dist = dist;
      closest = rankedencoding;
    }
  }
  printf("Closest=%s (%3.1f)\n",
         MyEncodingName(kMapToEncoding[closest]), closest_dist);

  for (int i = 0; i < 8; ++i) {
    // Demote by distance to CG and see if that helps, or just quit
  }
}

// Scan short single lines quickly for all printable ASCII
// Return true if all bytes are in [20..7F], false otherwise
bool QuickPrintableAsciiScan(const char* text, int text_length) {
  const auto* src = reinterpret_cast<const uint8*>(text);
  const uint8* srclimit = src + text_length;
  const uint8* srclimit8 = srclimit - 7;
  while (src < srclimit8) {
    // Exits on any byte outside [0x20..0x7E] range (HT LF CR exit)
    uint8 mask = 0;
    for (int i = 0; i < 8; ++i) mask |= (src[i]-0x20)|(src[i]+0x01);
    if ((mask & 0x80) != 0) break;
    src += 8;
  }
  while (src < srclimit) {
    uint8 uc = *src++;
    if (kIsPrintableAscii[uc] == 0) {return false;}
  }
  return true;
}

static const int kMaxScanBack = 192;

// Return true if text is inside a tag or JS comment
bool TextInsideTag(const uint8* isrc, const uint8* src, const uint8* srclimit) {
  const uint8* srcbacklimit = src - kMaxScanBack;
  if (srcbacklimit < isrc) {
    srcbacklimit = isrc;
  }
  const uint8* ss = src - 1;
  while (srcbacklimit <= ss) {
    uint8 c = *ss--;
    if ((c & ~0x02) == '<') {
      // We found preceding < 3C or > 3E nearby
      // Even cheaper: if inside a tag, we don't care what tag; return true
      if (c == '<') {
        return true;
      }
      // See if we are just after <title>...
      if ((c == '>') && (isrc <= (ss - 5)) &&
          (ss[-5] == '<') &&
          ((ss[-4] | 0x20) == 't') &&
          ((ss[-3] | 0x20) == 'i') &&
          ((ss[-2] | 0x20) == 't') &&
          ((ss[-1] | 0x20) == 'l') &&
          ((ss[-0] | 0x20) == 'e')) {
        return true;
      }
      // See if we are just after <SCRIPT language=javascript>...
      if ((c == '>') && (isrc <= (ss - 5)) &&
          (ss[-5] == 's') &&
          ((ss[-4] | 0x20) == 'c') &&
          ((ss[-3] | 0x20) == 'r') &&
          ((ss[-2] | 0x20) == 'i') &&
          ((ss[-1] | 0x20) == 'p') &&
          ((ss[-0] | 0x20) == 't')) {
        return true;
      }
      // Not in a tag
      return false;
    // See if we are just after JavaScript comment /* ...
    } else if (c == '/') {
      if (((ss + 2) < srclimit) && (ss[2] == '*')) {
        // We backscanned to /*
        return true;
      }
    }
  }

  return false;
}

const uint8* SkipToTagEnd(const uint8* src, const uint8* srclimit) {
  const uint8* ss = src + 1;
  while (ss <= srclimit) {
    uint8 c = *ss++;
    if ((c == '<') || (c == '>')) {
      return ss;
    }
  }
  return src + 2;     // Always make progress, Otherwise we get an infinite loop
}


// Take a watch string and map to a ranked encoding. If no match, return -1
int LookupWatchEnc(const string& watch_str) {
  int watchval = -1;
  // Mixed encoding maps to enc=UTF8UTF8
  if (watch_str == "UTF8UTF8") {
    watchval = F_UTF8UTF8;
  } else {
    Encoding enc;
    if (EncodingFromName(watch_str.c_str(), &enc)) {
      watchval = CompactEncDet::BackmapEncodingToRankedEncoding(enc);
    }
  }
  return watchval;
}

// Return true if enc and enc2 are equal or one is a subset of the other
// or either is UNKNOWN
// also UTF8UTF8 is compatible with both Latin1 and UTF8
bool CompatibleEnc(Encoding enc, Encoding enc2) {
  if (enc < 0) {return false;}
  if (NUM_ENCODINGS <= enc) {return false;}
  if (enc2 < 0) {return false;}
  if (NUM_ENCODINGS <= enc2) {return false;}
  if (enc == enc2) {return true;}
  if (kMapEncToBaseEncoding[enc] == kMapEncToBaseEncoding[enc2]) {return true;}

  if (enc == ASCII_7BIT) {return true;}
  if (enc2 == ASCII_7BIT) {return true;}
  if (enc == UNKNOWN_ENCODING) {return true;}
  if (enc2 == UNKNOWN_ENCODING) {return true;}
  if (enc == UTF8UTF8) {
    if (enc2 == UTF8) {return true;}
    if (kMapEncToBaseEncoding[enc2] == ISO_8859_1) {return true;}
  }
  if (enc2 == UTF8UTF8) {
    if (enc == UTF8) {return true;}
    if (kMapEncToBaseEncoding[enc] == ISO_8859_1) {return true;}
  }

  return false;
}

// Return superset of enc and enc2, which must be compatible
Encoding SupersetEnc(Encoding enc, Encoding enc2) {
  //printf("  SupersetEnc (%s, ", MyEncodingName(enc)); // TEMP
  //printf("%s) ", MyEncodingName(enc2));
  //printf("= %s\n",
  //       MyEncodingName(kMapEncToSuperLevel[enc] >= kMapEncToSuperLevel[enc2] ?
  //                      enc :enc2));
  if (kMapEncToSuperLevel[enc] >= kMapEncToSuperLevel[enc2]) {
    return enc;
  }
  return enc2;
}


// If unreliable, try rescoring to separate some encodings
Encoding Rescore(Encoding enc, const uint8* isrc,
                 const uint8* srctextlimit, DetectEncodingState* destatep) {
  if (FLAGS_counts) {++rescore_used;}
  Encoding new_enc = enc;

  bool rescore_change = false;

  int count = destatep->next_interesting_pair[OtherPair];
  auto text_length = static_cast<int>(srctextlimit - isrc);
  for (int i = 0; i < count; ++i) {
    int bigram_offset = destatep->interesting_offsets[OtherPair][i];
    uint8 byte0 = (0 < bigram_offset) ?
        isrc[bigram_offset - 1] : 0x20;
    uint8 byte1 = isrc[bigram_offset + 0];  // Known to have high bit on
    uint8 byte2 = ((bigram_offset + 1) < text_length) ?
        isrc[bigram_offset + 1] : 0x20;
    uint8 byte3 = ((bigram_offset + 2) < text_length) ?
        isrc[bigram_offset + 2] : 0x20;
    int high_hash = ((byte0 & 0xc0) >> 0) |
                    ((byte1 & 0xc0) >> 1) |
                    ((byte2 & 0xc0) >> 4) |
                    ((byte3 & 0xc0) >> 6);    // 00112233

    // Boost HighAccent encodings for Ascii bit patterns
    //  0x1x  0x0x
    //  1010  1010
    //  0010  0000
    //
    if ((high_hash & 0xaa) == 0x20) {
      for (int j = 0; j < destatep->rankedencoding_list_len; j++) {
        int rankedencoding = destatep->rankedencoding_list[j];
        if (HighAccentEncoding(kMapToEncoding[rankedencoding])) {
          // TODO: also want to boost Shift-JIS here if byte1 is Ax..Dx
          // TEMP
          //printf("  Rescore[%02x] %s +%d\n",
          //       high_hash, MyRankedEncName(rankedencoding), kGentlePairBoost);
          Boost(destatep, rankedencoding, kGentlePairBoost);
          rescore_change = true;
        }
      }
    }

    // Whack HighAccent encodings for high bit patterns
    //  1x1x  1x1x
    //  1010  1010
    //  1010  1010
    //
    if ((high_hash & 0xaa) == 0xaa) {
      for (int j = 0; j < destatep->rankedencoding_list_len; j++) {
        int rankedencoding = destatep->rankedencoding_list[j];
        if (HighAccentEncoding(kMapToEncoding[rankedencoding])) {
          // TEMP
          //printf("  Rescore[%02x] %s -%d\n",
          //       high_hash, MyRankedEncName(rankedencoding), kGentlePairBoost);
          Whack(destatep, rankedencoding, kGentlePairBoost);
          rescore_change = true;
        }
      }
    }

  }

  if (rescore_change) {
    ReRank(destatep);
    new_enc = kMapToEncoding[destatep->top_rankedencoding];

    if (destatep->debug_data != nullptr) {
      char buff[32];
      snprintf(buff, sizeof(buff), "=Rescore %s", MyEncodingName(new_enc));
      SetDetailsEncProb(destatep,
                        0,
                        CompactEncDet::BackmapEncodingToRankedEncoding(new_enc),
                        buff);
      //// DumpDetail(destatep);
    }

    SimplePrune(destatep, kFinalPruneDifference);
    CalcReliable(destatep);
  }

  //if (new_enc != enc) {
  //  // TEMP
  //  printf("  Rescore new top encoding = %s\n",
  //         MyRankedEncName(destatep->top_rankedencoding));
  //}

  return new_enc;
}


// Given an encoding, add its corresponding ranked encoding to the set
void AddToSet(Encoding enc, int* list_len, int* list) {
  // TEMP print
  int item = CompactEncDet::BackmapEncodingToRankedEncoding(enc);
  for (int i = 0; i < *list_len; ++i) {
    if (list[i] == item) {
      return;                 // Already in the set; don't add again
    }
  }
  list[(*list_len)++] = item;
}


static const int kMinRobustBigramCount = 1000;
static const int kMinKBToRobustScan =  64;
static const int kMaxKBToRobustScan = 256;

// Scan the first 64K or so, just doing raw bigram increments on given
// probability list.
// No fancy duplicate filtering or anything else here.
// Returns number of bigrams counted
int RobustScan(const char* text,
                int text_length,
                int robust_renc_list_len,
                int* robust_renc_list,
                int* robust_renc_probs) {
  if (FLAGS_counts) {++robust_used;}
  // Zero all the result probabilities
  for (int i = 0; i < robust_renc_list_len; ++i) {
    robust_renc_probs[i] = 0;
  }
  int max_fast_len = minint(text_length, (kMaxKBToRobustScan << 10));
  const auto* isrc = reinterpret_cast<const uint8*>(text);
  const uint8* src = isrc;
  const uint8* srclimitfast2 = isrc + max_fast_len - 1;
  const uint8* srclimitfast4 = isrc + max_fast_len - 3;

  int min_fast_len = minint(text_length, (kMinKBToRobustScan << 10));
  const uint8* srclimitmin = isrc + min_fast_len - 1;

  int bigram_count = 0;

  if (FLAGS_enc_detect_source) {
    PsSourceInit(kPsSourceWidth);
    fprintf(stderr, "(RobustScan) do-src\n");
  }

  // Sum over a big chunk of the input
  // Faster loop, no 7-bit-encodings possible, approx 3000 GB/sec
  //====================================
  while (src < srclimitfast2) {
    // Skip to next interesting bigram

    while (src < srclimitfast4) {
      if (((src[0] | src[1] | src[2] | src[3]) & 0x80) != 0) break;
      src += 4;
    }

    while (src < srclimitfast2) {
      if ((src[0] & 0x80) != 0) break;
      src++;
    }

    if (src < srclimitfast2) {
      // We found a bigram with high bit on
      // Next 5 lines commented out so we don't show all the source.
      //const uint8* srctextlimit = isrc + text_length;
      //if (FLAGS_enc_detect_source) {
      //  PsSource(src, isrc, srctextlimit);
      //  PsMark(src, 2, isrc, 0);
      //}

      uint8 byte1 = src[0];
      uint8 byte2 = src[1];
      uint8 byte1x2x = (byte1 & 0xf0) | ((byte2 >> 4) & 0x0f);
      uint8 byte1f = byte1;
      // Flip top bit of subscript to better separate quadrant 4 (esp. for Hebrew)
      byte1f ^= (byte2 & 0x80);

      // The real increments
      for (int j = 0; j < robust_renc_list_len; ++j) {
        int rankedencoding = robust_renc_list[j];
        const UnigramEntry* ue = &unigram_table[rankedencoding];
        int incr = ue->b1[byte1f] + ue->b2[byte2] + ue->b12[byte1x2x];
        if ((ue->b12[byte1x2x] & 0x01) != 0) {
          // Use a more-precise table
          int byte32x32 = ((byte1 & 0x1f) << 5) | (byte2 & 0x1f);
          int hiressub = (byte2 & 0x60) >> 5;   // select w/bits 5&6 of byte 2
          DCHECK(ue->hires[hiressub] != NULL);
          incr += ue->hires[hiressub][byte32x32];
        } else {
          // Default final offset
          incr += ue->so;
        }
        robust_renc_probs[j] += incr;
      }

      src += 2;       // Continue after this bigram
      ++bigram_count;

      // Stop after 1000 bigrams reached, if at least 64KB scanned
      if ((bigram_count > kMinRobustBigramCount) && (src > srclimitmin)) {
        break;
      }

    }
  }

  if (FLAGS_enc_detect_source) {
    fprintf(stderr, "(  bigram_count = %d) do-src\n", bigram_count);
    if (bigram_count == 0) {bigram_count = 1;}    // zdiv
    for (int i = 0; i < robust_renc_list_len; ++i) {
      fprintf(stderr, "(  enc[%-12.12s] = %7d (avg %d)) do-src\n",
              MyRankedEncName(robust_renc_list[i]), robust_renc_probs[i],
              robust_renc_probs[i] / bigram_count);
    }
    PsSourceFinish();
  }

  return bigram_count;
}

// If unreliable, rescan middle of document to see if we can get a better
// answer. Rescan is only worthwhile if there are ~200 bytes or more left,
// since the detector takes as much as 96 bytes of bigrams to decide.
Encoding Rescan(Encoding enc,
                const uint8* isrc,
                const uint8* src,
                const uint8* srctextlimit,
                const char* url_hint,
                const char* http_charset_hint,
                const char* meta_charset_hint,
                const int encoding_hint,
                const Language language_hint,
                const CompactEncDet::TextCorpusType corpus_type,
                bool ignore_7bit_mail_encodings,
                DetectEncodingState* destatep) {
  bool enc_is_reliable = destatep->reliable;
  Encoding new_enc = enc;
  Encoding second_best_enc =
    kMapToEncoding[destatep->second_top_rankedencoding];

  if (FLAGS_counts) {++rescan_used;}

  auto scanned_bytes = static_cast<int>(src - isrc);
  auto unscanned_bytes = static_cast<int>(srctextlimit - src);
  auto text_length = static_cast<int>(srctextlimit - isrc);

  // See if enough bytes left to bother doing rescan
  if (kMinRescanLength < unscanned_bytes) {
    const auto* text = reinterpret_cast<const char*>(isrc);

    Encoding one_hint = destatep->http_hint;
    if ((one_hint == UNKNOWN_ENCODING) &&
        (destatep->meta_hint != UNKNOWN_ENCODING)) {
      one_hint = destatep->meta_hint;
    }
    if ((one_hint == UNKNOWN_ENCODING) &&
        (destatep->bom_hint != UNKNOWN_ENCODING)) {
      one_hint = destatep->bom_hint;
    }

    // Go to an even offset to keep UTF-16 in synch
    int middle_offset = (scanned_bytes + (unscanned_bytes / 2)) & ~1;
    CHECK(middle_offset <= text_length);

    // Look back a bit for a low byte to synchronize, else hope for the best.
    const uint8* srcbacklimit = isrc + middle_offset - kMaxScanBack;
    if (srcbacklimit < src) {
      srcbacklimit = src;
    }
    const uint8* ss = isrc + middle_offset - 1;
    while (srcbacklimit <= ss) {
      if ((*ss & 0x80) == 0) {break;}
      --ss;
    }
    // Leave middle offset unchanged unless we found a low byte
    if (srcbacklimit <= ss) {
      // Align to low byte or high byte just after it, whichever is even
      middle_offset = (ss - isrc + 1) & ~1;     // Even to keep UTF-16 in sync
    }
    CHECK(middle_offset <= text_length);

    if (destatep->debug_data != nullptr) {
      SetDetailsEncLabel(destatep, ">> Rescan");
      // Print the current chart before recursive call
      DumpDetail(destatep);

      char buff[32];
      snprintf(buff, sizeof(buff), ">> Rescan[%d..%d]",
               middle_offset, text_length);
      PsRecurse(buff);
    }

    int mid_bytes_consumed;
    bool mid_is_reliable;
    Encoding mid_second_best_enc;
    auto newflags = static_cast<CEDInternalFlags>(
      kCEDRescanning + kCEDForceTags);
    // Recursive call for rescan of half of remaining
    Encoding mid_enc = InternalDetectEncoding(
                             newflags,
                             text + middle_offset,
                             text_length - middle_offset,
                             url_hint,
                             http_charset_hint,
                             meta_charset_hint,
                             encoding_hint,
                             language_hint,   // User interface lang
                             corpus_type,
                             ignore_7bit_mail_encodings,
                             &mid_bytes_consumed,
                             &mid_is_reliable,
                             &mid_second_best_enc);
    destatep->reliable = mid_is_reliable;

    bool const empty_rescan = (mid_enc == ASCII_7BIT);

    // Not the right decision if, e.g. enc=Greek, mid=ASCII7, one=KSC
    // hence the !empty_rescan term
    if (!empty_rescan && CompatibleEnc(one_hint, mid_enc)) {
      // Encoding we just found is compatible with the
      // single hint (if any); return superset
      new_enc = SupersetEnc(one_hint, mid_enc);
    }

    // If original and mid are compatible, and both reliable,
    // return new_enc = SupersetEnc(enc, mid_enc)
    //
    // This avoids too much weight on a bogus hint causing a RobustScan
    // that gets the wrong answer
    if (!empty_rescan && mid_is_reliable && enc_is_reliable &&
        CompatibleEnc(enc, mid_enc)) {
      new_enc = SupersetEnc(enc, mid_enc);
      return new_enc;
    }

    // if mid unreliable, robustscan
    // if mid empty, robustscan
    // if original and mid not compatible, robustscan
    // if mid and one_hint not compatible, robustscan

    // If we found conflicting data, drop back and do a robust scan of a big
    // chunk of the input over a set of candidate encodings
    //
    if (!mid_is_reliable ||
        empty_rescan ||
        !CompatibleEnc(enc, mid_enc) ||
        !CompatibleEnc(one_hint, mid_enc)) {
      int robust_renc_list_len;         // Number of active encodings
      int robust_renc_list[NUM_RANKEDENCODING];   // List of ranked encodings
      int robust_renc_probs[NUM_RANKEDENCODING];  // List of matching probs

      robust_renc_list_len = 0;
      AddToSet(enc, &robust_renc_list_len, robust_renc_list);
      AddToSet(second_best_enc, &robust_renc_list_len, robust_renc_list);
      AddToSet(mid_enc, &robust_renc_list_len, robust_renc_list);
      AddToSet(mid_second_best_enc, &robust_renc_list_len, robust_renc_list);
      if (destatep->http_hint != UNKNOWN_ENCODING) {
        AddToSet(destatep->http_hint, &robust_renc_list_len, robust_renc_list);
      }
      if (destatep->meta_hint != UNKNOWN_ENCODING) {
        AddToSet(destatep->meta_hint, &robust_renc_list_len, robust_renc_list);
      }
      if (destatep->bom_hint != UNKNOWN_ENCODING) {
        AddToSet(destatep->bom_hint, &robust_renc_list_len, robust_renc_list);
      }
      if (destatep->tld_hint != UNKNOWN_ENCODING) {
        AddToSet(destatep->tld_hint, &robust_renc_list_len, robust_renc_list);
      }

      // Separate simple scan
      // =====================
      if (destatep->debug_data != nullptr) {
        SetDetailsEncLabel(destatep, ">> RobustScan");
        // Print the current chart before recursive call
        DumpDetail(destatep);

        char buff[32];
        snprintf(buff, sizeof(buff), ">> RobustScan[0..%d]", text_length);
        PsRecurse(buff);
      }

      int bigram_count = RobustScan(text, text_length,
                 robust_renc_list_len, robust_renc_list, robust_renc_probs);

      // Default to new_enc and update if something better was found
      int best_prob = -1;
      // TEMP print
      for (int i = 0; i < robust_renc_list_len; ++i) {
        if (best_prob < robust_renc_probs[i]) {
          best_prob = robust_renc_probs[i];
          new_enc = kMapToEncoding[robust_renc_list[i]];
        }
      }

      if (destatep->debug_data != nullptr) {
        char buff[32];
        snprintf(buff, sizeof(buff), "=Robust[%d] %s",
                 bigram_count, MyEncodingName(new_enc));
        SetDetailsEncProb(destatep,
                          0,
                          CompactEncDet::BackmapEncodingToRankedEncoding(new_enc),
                          buff);
      }
    }
  }     // End if enough bytes

  return new_enc;
}

// With no hints at all, and perhaps on rescan, we relax our pickiness
// and go ahead and accept the top multibyte encodings, even though
// strictly their web pages should have declared an explicit encoding to
// avoid the HTML standard's default ISO-8859-1.
bool NoHintsCloseEnoughCompatible(Encoding top_enc) {
  // First test accepts degenerate cases plus UTF8 and UTF8UTF8
  if (CompatibleEnc(UTF8, top_enc)) {return true;}

  // The rest look for exact match of base encoding
  Encoding base_enc = kMapEncToBaseEncoding[top_enc];
  if (base_enc == JAPANESE_EUC_JP) {return true;}
  if (base_enc == JAPANESE_SHIFT_JIS) {return true;}
  if (base_enc == CHINESE_BIG5) {return true;}
  if (base_enc == CHINESE_GB) {return true;}
  if (base_enc == KOREAN_EUC_KR) {return true;}
  return false;
}



// Scan raw bytes and detect most likely encoding
// Design goals:
//   Skip over big initial stretches of seven-bit ASCII bytes very quickly
//   Thread safe
//   Works equally well on
//    50-byte queries,
//    5000-byte email and
//    50000-byte web pages
// Length 0 input returns ISO_8859_1 (ASCII) encoding
// Setting ignore_7bit_mail_encodings effectively turns off detection of
//  UTF-7, HZ, and ISO-2022-xx
Encoding InternalDetectEncoding(
    CEDInternalFlags flags, const char* text, int text_length,
    const char* url_hint, const char* http_charset_hint,
    const char* meta_charset_hint, const int encoding_hint,
    const Language language_hint,  // User interface lang
    const CompactEncDet::TextCorpusType corpus_type,
    bool ignore_7bit_mail_encodings, int* bytes_consumed, bool* is_reliable,
    Encoding* second_best_enc) {
  *bytes_consumed = 0;
  *is_reliable = false;
  *second_best_enc = ASCII_7BIT;

  if (text_length == 0) {
    // Follow the spec. Text might be NULL.
    *is_reliable = true;
    return ISO_8859_1;
  }

  // For very short (20-50 byte) input strings that are highly likely to be
  // all printable ASCII, our startup overhead might dominate. We have to do the
  // full detection if the ISO-2022-xx, HZ, or UTF-7 encodings are possible.
  // Otherwise, we can do a quick scan for printable ASCII.
  if ((text_length <= 500) && ignore_7bit_mail_encodings &&
      QuickPrintableAsciiScan(text, text_length)) {
    *is_reliable = true;
    return ASCII_7BIT;
  }

  // Go for the full boat detection
  DetectEncodingState destate;
  InitDetectEncodingState(&destate);

  std::unique_ptr<DetailEntry[]> scoped_debug_data;
  if (FLAGS_enc_detect_detail) {
    // Allocate max 10 details per bigram
    scoped_debug_data = std::make_unique<DetailEntry[]>(kMaxPairs * 10);
    destate.debug_data = scoped_debug_data.get();
    // NOTE: destate and scoped_debug_data have exactly the same scope
    // All other FLAGS_enc_detect_detail tests use destate.debug_data != NULL
  }

  // Get text length limits
  // Typically, we scan the first 16KB looking for all encodings, then
  // scan the rest (up to 256KB) a bit faster by no longer looking for
  // interesting bytes below 0x80. This allows us to skip over runs of
  // 7-bit-ASCII much more quickly.
  int slow_len = minint(text_length, (FLAGS_enc_detect_slow_max_kb << 10));
  int fast_len = minint(text_length, (FLAGS_enc_detect_fast_max_kb << 10));

  // Initialize pointers.
  // In general, we do not look at last 3 bytes of input in the fast scan
  // We do, however want to look at the last byte or so in the slow scan,
  // especilly in the case of a very short text whose only interesting
  // information is a 3-byte UTF-8 character in the last three bytes.
  // If necessary, we fake a last bigram with 0x20 space as a pad byte.
  const auto* isrc = reinterpret_cast<const uint8*>(text);
  const uint8* src = isrc;
  const uint8* srctextlimit = isrc + text_length;
  const uint8* srclimitslow2 = isrc + slow_len - 1;
  const uint8* srclimitfast2 = isrc + fast_len - 1;
  const uint8* srclimitfast4 = isrc + fast_len - 3;
  if (srclimitslow2 > srclimitfast2) {
    srclimitslow2 = srclimitfast2;
  }
  destate.initial_src = isrc;
  destate.limit_src = srclimitfast2 + 1;      // May include last byte
  destate.prior_src = isrc;
  destate.last_pair = isrc - 2;

  const char* scan_table = kTestPrintableAsciiTildePlus;
  if (ignore_7bit_mail_encodings) {
    // Caller wants to ignore UTF-7, HZ, ISO-2022-xx
    // Don't stop on + (for UTF-7), nor on ~ (for HZ)
    scan_table = kTestPrintableAscii;
  }
  int exit_reason = 0;

  if (destate.debug_data != nullptr) {
    BeginDetail(&destate);
    // Take any incoming watch encoding name and backmap to the corresponding
    // ranked enum value
    watch1_rankedenc = LookupWatchEnc(FLAGS_enc_detect_watch1);
    if (watch1_rankedenc >= 0) {
      fprintf(stderr, "/track-me %d def\n", watch1_rankedenc);
    }

    watch2_rankedenc = LookupWatchEnc(FLAGS_enc_detect_watch2);
    if (watch2_rankedenc >= 0) {
      fprintf(stderr, "/track-me2 %d def\n", watch2_rankedenc);
    }

    fprintf(stderr, "%% kDerateHintsBelow = %d\n", kDerateHintsBelow);
  }
  if (FLAGS_enc_detect_source) {
    PsSourceInit(kPsSourceWidth);
    PsSource(src, isrc, srctextlimit);
    PsMark(src, 4, isrc, 0);
  }

  // Apply hints, if any, to probabilities
  // NOTE: Encoding probabilites are all zero at this point
  ApplyHints(url_hint,
             http_charset_hint,
             meta_charset_hint,
             encoding_hint,
             language_hint,
             corpus_type,
             &destate);

  // NOTE: probabilities up to this point are subject to derating for
  // small numbers of bigrams.
  // Probability changes after this point are not derated.

  // Do first 4 bytes to pick off strong markers
  InitialBytesBoost(isrc, text_length, &destate);

  bool ignored_some_tag_text = false;
  int tag_text_bigram_count = 0;

  // Slower loop, approx 500 MB/sec (2.8 GHz P4)
  // ASSERT(srclimitslow2 <= srclimitfast2);
  //====================================
 DoMoreSlowLoop:
  while (src < srclimitslow2) {
    // Skip to next interesting byte (this is the slower part)
    while (src < srclimitslow2) {
      uint8 uc = *src++;
      if (scan_table[uc] != 0) {exit_reason = scan_table[uc]; src--; break;}
    }

    if (src < srclimitslow2) {
      if (FLAGS_enc_detect_source) {
        PsSource(src, isrc, srctextlimit);    // don't mark yet
      }

      int weightshift = 0;
      // In the first 16KB, derate new text run inside <title>...</title> and
      // inside <!-- ... -->
      if (////((destate.last_pair + 6) <= src) &&             // if beyond last one
          ////(tag_text_bigram_count < kMaxBigramsTagTitleText) &&
          (corpus_type == CompactEncDet::WEB_CORPUS) &&   // and web page
          !CEDFlagForceTags(flags)) {                     // and OK to skip
        ////if (TextInsideTag(destate.last_pair + 2, src, srclimitslow2)) {
        if (TextInsideTag(isrc, src, srclimitslow2)) {
          if (tag_text_bigram_count >= kMaxBigramsTagTitleText) {
            ignored_some_tag_text = true;
            src = SkipToTagEnd(src, srclimitslow2);
            continue;
          } else {
            weightshift = kWeightshiftForTagTitleText;
            ++tag_text_bigram_count;
          }
        }
      }
      if (FLAGS_enc_detect_source) {
        PsMark(src, 2, isrc, weightshift);
      }
      // Saves byte pair and offset
      bool pruned = IncrementAndBoostPrune(src, static_cast<int>(srctextlimit - src),
                                           &destate, weightshift, exit_reason);
      // Advance; if inside tag, advance to end of tag
      if (weightshift == 0) {
        src += exit_reason;               // 1 Ascii, 2 other
      } else {
        src += exit_reason;               // 1 Ascii, 2 other
        //// src = SkipToTagEnd(src, srclimitslow2);
      }

      if (pruned) {
        // Scoring and active encodings have been updated
        if (destate.done) {break;}
        // Check if all the reasons for the slow loop have been pruned
        // If so, go to fast loop
        if (!SevenBitActive(&destate)) {break;}
      }
    }
  }
  //====================================

  // We reached the end of a slow scan, possibly because no more SevenBitActive,
  // or possibly are at end of source.
  // If we are exactly at the end of the source, make sure we look at the very
  // last byte.
  bool very_last_byte_incremented = false;
  if (src == (srctextlimit - 1)) {
    exit_reason = scan_table[*src];
    if (exit_reason != 0) {
      // The very last byte is an interesting byte
      // Saves byte pair and offset
      //printf("Interesting very last slow byte = 0x%02x\n", *src);
      IncrementAndBoostPrune(src, static_cast<int>(srctextlimit - src), &destate, 0, exit_reason);
      very_last_byte_incremented = true;
    }
  }

  if (FLAGS_enc_detect_source) {
    PsSource(src, isrc, srctextlimit);
    PsMark(src, 2, isrc, 0);
  }
  // Force a pruning based on whatever we have
  // Delete the seven-bit encodings if there is no evidence of them so far
  BoostPrune(src, &destate, PRUNE_SLOWEND);

  if (!destate.done) {
    // If not clear yet on 7-bit-encodings and more bytes, do more slow
    if (SevenBitActive(&destate) && (src < srclimitfast2)) {
      // Increment limit by another xxxK
      slow_len += (FLAGS_enc_detect_slow_max_kb << 10);
      srclimitslow2 = isrc + slow_len - 1;
      if (srclimitslow2 > srclimitfast2) {
        srclimitslow2 = srclimitfast2;
      }
      if (!UTF7OrHzActive(&destate)) {
        // We can switch to table that does not stop on + ~
        scan_table = kTestPrintableAscii;
      }
      goto DoMoreSlowLoop;
    }


    exit_reason = 2;
    // Faster loop, no 7-bit-encodings possible, approx 3000 GB/sec
    //====================================
    while (src < srclimitfast2) {
      // Skip to next interesting byte (this is the faster part)
      while (src < srclimitfast4) {
        if (((src[0] | src[1] | src[2] | src[3]) & 0x80) != 0) break;
        src += 4;
      }

      while (src < srclimitfast2) {
        if ((src[0] & 0x80) != 0) break;
        src++;
      }

      if (src < srclimitfast2) {
        if (FLAGS_enc_detect_source) {
          PsSource(src, isrc, srctextlimit);
          PsMark(src, 2, isrc, 0);
        }
        // saves byte pair and offset
        bool pruned = IncrementAndBoostPrune(src, static_cast<int>(srctextlimit - src),
                                             &destate, 0, exit_reason);
        src += exit_reason;               // 1 Ascii, 2 other
        if (pruned) {
          // Scoring and active encodings have been updated
          if (destate.done) {break;}
        }
      }
    }
    //====================================
    // We reached the end of fast scan

    // If we are exactly at the end of the source, make sure we look at the very
    // last byte.
    if (src == (srctextlimit - 1) && !very_last_byte_incremented) {
      exit_reason = scan_table[*src];
      if (exit_reason != 0) {
        // The very last byte is an interesting byte
        // Saves byte pair and offset
        //printf("Interesting very last fast byte = 0x%02x\n", *src);
        IncrementAndBoostPrune(src, static_cast<int>(srctextlimit - src), &destate, 0, exit_reason);
        very_last_byte_incremented = true;
      }
    }

  }     // End if !done

  if (FLAGS_enc_detect_source) {
    PsSource(src, isrc, srctextlimit);
    PsMark(src, 2, isrc, 0);
  }
  // Force a pruning based on whatever we have
  BoostPrune(src, &destate, PRUNE_FINAL);

  if (FLAGS_enc_detect_summary) {
    DumpSummary(&destate, AsciiPair, 32);
    DumpSummary(&destate, OtherPair, 32);
  }
  if (FLAGS_enc_detect_source) {
    PsSourceFinish();
  }
  if (destate.debug_data != nullptr) {
    //// DumpDetail(&destate);
  }


  if (ignored_some_tag_text &&
      (kMapToEncoding[destate.top_rankedencoding] == ASCII_7BIT)) {
    // There were some interesting bytes, but only in tag text.
    // Recursive call to reprocess looking at the tags this time.

    if (destate.debug_data != nullptr) {
      SetDetailsEncLabel(&destate, ">> Recurse/tags");
      // Print the current chart before recursive call
      DumpDetail(&destate);

      char buff[32];
      snprintf(buff, sizeof(buff), ">> Recurse for tags");
      PsRecurse(buff);
    }

    // Recursive call for high bytes in tags [no longer used, 1/16 tag score]
    Encoding enc2 = InternalDetectEncoding(
                             kCEDForceTags,  // force
                             text,
                             text_length,
                             url_hint,
                             http_charset_hint,
                             meta_charset_hint,
                             encoding_hint,
                             language_hint,
                             corpus_type,
                             ignore_7bit_mail_encodings,
                             bytes_consumed,
                             is_reliable,
                             second_best_enc);

    if (destate.debug_data != nullptr) {
      // Show winning encoding and dump PostScript
      char buff[32];
      snprintf(buff, sizeof(buff), "=2 %s", MyEncodingName(enc2));
      SetDetailsEncProb(&destate,
                        0,
                        CompactEncDet::BackmapEncodingToRankedEncoding(enc2),
                        buff);
      DumpDetail(&destate);
    }

    return enc2;
  }


  // If the detected encoding does not match default/hints, or if the hints
  // conflict with each other, mark as unreliable. This can be used to trigger
  // further scoring.
  // Three buckets of input documents;
  // ~19% of the web no hints, and top == 7bit, Latin1, or CP1252
  // ~79% of the web one or more hints, all same encoding X and top == X
  // ~ 2% of the web one or more hints that are inconsistent

  Encoding top_enc = kMapToEncoding[destate.top_rankedencoding];
  Encoding one_hint = destate.http_hint;
  if ((one_hint == UNKNOWN_ENCODING) &&
      (destate.meta_hint != UNKNOWN_ENCODING)) {
    one_hint = destate.meta_hint;
  }
  if ((one_hint == UNKNOWN_ENCODING) &&
      (destate.bom_hint != UNKNOWN_ENCODING)) {
    one_hint = destate.bom_hint;
  }

  bool found_compatible_encoding = true;
  if (one_hint == UNKNOWN_ENCODING) {
    // [~14% of the web] No hints, and top == 7bit, Latin1, or CP1252
    if (!CompatibleEnc(ISO_8859_1, top_enc)) {
      found_compatible_encoding = false;
      // If there is nothing but a TLD hint and its top encoding matches, OK
      if ((destate.tld_hint != UNKNOWN_ENCODING) &&
          CompatibleEnc(destate.tld_hint, top_enc)) {
        found_compatible_encoding = true;
      }
    }
  } else if (CompatibleEnc(one_hint, destate.http_hint) &&
             CompatibleEnc(one_hint, destate.meta_hint) &&
             CompatibleEnc(one_hint, destate.bom_hint)) {
    // [~83% of the web] One or more hints, all same encoding X and top == X
    if (!CompatibleEnc(one_hint, top_enc)) {
      // [~ 2% of the web] Oops, not the declared encoding
      found_compatible_encoding = false;
    }
  } else {
    // [~ 3% of the web] Two or more hints that are inconsistent
    one_hint = UNKNOWN_ENCODING;
    found_compatible_encoding = false;
  }

  // If we turned Latin1 into Latin2 or 7 via trigrams, don't fail it here
  if (destate.do_latin_trigrams) {
    if (CompatibleEnc(kMapToEncoding[F_Latin1], top_enc) ||
        CompatibleEnc(kMapToEncoding[F_Latin2], top_enc) ||
        CompatibleEnc(kMapToEncoding[F_CP1250], top_enc) ||
        CompatibleEnc(kMapToEncoding[F_ISO_8859_13], top_enc)) {
      found_compatible_encoding = true;
      destate.reliable = true;
    }
  }

  // If top encoding is not compatible with the hints, but it is reliably
  // UTF-8, accept it anyway.
  // This will perform badly with mixed UTF-8 prefix plus another encoding in
  // the body if done too early, so we want to be rescanning.
  if (!found_compatible_encoding &&
      destate.reliable &&
      NoHintsCloseEnoughCompatible(top_enc) &&
      (destate.next_interesting_pair[OtherPair] >= kStrongPairs) &&
      CEDFlagRescanning(flags)) {
    found_compatible_encoding = true;
  }

  // Hold off on this so Rescan() can see if the original encoding was reliable
  //if (!found_compatible_encoding) {
  //  destate.reliable = false;
  //}

  // If unreliable, try rescoring to separate some encodings
  if (!destate.reliable || !found_compatible_encoding) {
    top_enc = Rescore(top_enc, isrc, srctextlimit, &destate);
  }

  *second_best_enc = kMapToEncoding[destate.second_top_rankedencoding];

  // If unreliable, and not already rescanning,
  // rescan middle of document to see if we can get a better
  // answer. Rescan is only worthwhile if there are ~200 bytes or more left,
  // since the detector takes as much as 96 bytes of bigrams to decide.
  //
  // CANNOT retry ISO-2022-xx HZ etc. because no declaration escape at the front
  // or we may land in the middle of some partial state. Skip them all.
  //
  if ((!destate.reliable || !found_compatible_encoding) &&
      !CEDFlagRescanning(flags) &&
      !SevenBitEncoding(top_enc)) {
    top_enc = Rescan(top_enc,
                     isrc,
                     src,
                     srctextlimit,
                     url_hint,
                     http_charset_hint,
                     meta_charset_hint,
                     encoding_hint,
                     language_hint,
                     corpus_type,
                     ignore_7bit_mail_encodings,
                     &destate);
  } else {
    if (!found_compatible_encoding) {
      destate.reliable = false;
    }
  }

  if (destate.debug_data != nullptr) {
    // Dump PostScript
    DumpDetail(&destate);
  }

  *bytes_consumed = (int)(src - isrc + 1);       // We looked 1 byte beyond src
  *is_reliable = destate.reliable;
  return top_enc;
}

Encoding CompactEncDet::DetectEncoding(
    const char* text, int text_length, const char* url_hint,
    const char* http_charset_hint, const char* meta_charset_hint,
    const int encoding_hint,
    const Language language_hint,  // User interface lang
    const TextCorpusType corpus_type, bool ignore_7bit_mail_encodings,
    int* bytes_consumed, bool* is_reliable) {
  if (FLAGS_ced_echo_input) {
    string temp(text, text_length);
    fprintf(stderr, "CompactEncDet::DetectEncoding()\n%s\n\n", temp.c_str());
  }

  if (FLAGS_counts) {
    encdet_used = 0;
    rescore_used = 0;
    rescan_used = 0;
    robust_used = 0;
    looking_used = 0;
    doing_used = 0;
    ++encdet_used;
  }
  if (FLAGS_dirtsimple) {
    // Just count first 64KB bigram encoding probabilities for each encoding
    int robust_renc_list_len;         // Number of active encodings
    int robust_renc_list[NUM_RANKEDENCODING];   // List of ranked encodings
    int robust_renc_probs[NUM_RANKEDENCODING];  // List of matching probs

    for (int i = 0; i < NUM_RANKEDENCODING; ++i) {
      robust_renc_list[i] = i;
    }
    robust_renc_list_len = NUM_RANKEDENCODING;

    RobustScan(text, text_length,
                 robust_renc_list_len, robust_renc_list, robust_renc_probs);

    // Pick off best encoding
    int best_prob = -1;
    Encoding enc = UNKNOWN_ENCODING;
    for (int i = 0; i < robust_renc_list_len; ++i) {
      if (best_prob < robust_renc_probs[i]) {
        best_prob = robust_renc_probs[i];
        enc = kMapToEncoding[robust_renc_list[i]];
      }
    }

    *bytes_consumed = minint(text_length, (kMaxKBToRobustScan << 10));
    *is_reliable = true;
    if (FLAGS_counts) {
      printf("CEDcounts ");
      while (encdet_used--) {printf("encdet ");}
      while (rescore_used--) {printf("rescore ");}
      while (rescan_used--) {printf("rescan ");}
      while (robust_used--) {printf("robust ");}
      while (looking_used--) {printf("looking ");}
      while (doing_used--) {printf("doing ");}
      printf("\n");
    }

    return enc;
  }

  Encoding second_best_enc;
  Encoding enc = InternalDetectEncoding(kCEDNone,
                           text,
                           text_length,
                           url_hint,
                           http_charset_hint,
                           meta_charset_hint,
                           encoding_hint,
                           language_hint,   // User interface lang
                           corpus_type,
                           ignore_7bit_mail_encodings,
                           bytes_consumed,
                           is_reliable,
                           &second_best_enc);
  if (FLAGS_counts) {
    printf("CEDcounts ");
    while (encdet_used--) {printf("encdet ");}
    while (rescore_used--) {printf("rescore ");}
    while (rescan_used--) {printf("rescan ");}
    while (robust_used--) {printf("robust ");}
    while (looking_used--) {printf("looking ");}
    while (doing_used--) {printf("doing ");}
    printf("\n");
  }

#if defined(HTML5_MODE)
  // Map all the Shift-JIS variants to Shift-JIS when used in Japanese locale.
  if (language_hint == JAPANESE && IsShiftJisOrVariant(enc)) {
    enc = JAPANESE_SHIFT_JIS;
  }

  // 7-bit encodings (except ISO-2022-JP), and some obscure encodings not
  // supported in WHATWG encoding standard are marked as ASCII to keep the raw
  // bytes intact.
  switch (enc) {
    case ISO_2022_KR:
    case ISO_2022_CN:
    case HZ_GB_2312:
    case UTF7:
    case UTF16LE:
    case UTF16BE:

    case CHINESE_EUC_DEC:
    case CHINESE_CNS:
    case CHINESE_BIG5_CP950:
    case JAPANESE_CP932:
    case MSFT_CP874:
    case TSCII:
    case TAMIL_MONO:
    case TAMIL_BI:
    case JAGRAN:
    case BHASKAR:
    case HTCHANAKYA:
    case BINARYENC:
    case UTF8UTF8:
    case TAM_ELANGO:
    case TAM_LTTMBARANI:
    case TAM_SHREE:
    case TAM_TBOOMIS:
    case TAM_TMNEWS:
    case TAM_WEBTAMIL:
    case KDDI_SHIFT_JIS:
    case DOCOMO_SHIFT_JIS:
    case SOFTBANK_SHIFT_JIS:
    case KDDI_ISO_2022_JP:
    case SOFTBANK_ISO_2022_JP:
      enc = ASCII_7BIT;
      break;
    default:
      break;
  }
#endif

  return enc;
}


// Return top encoding hint for given string
Encoding CompactEncDet::TopEncodingOfLangHint(const char* name) {
  string normalized_lang = MakeChar8(string(name));
  int n = HintBinaryLookup8(kLangHintProbs, kLangHintProbsSize,
                           normalized_lang.c_str());
  if (n < 0) {return UNKNOWN_ENCODING;}

  // Charset is eight bytes, probability table is eight bytes
  int toprankenc =
    TopCompressedProb(&kLangHintProbs[n].key_prob[kMaxLangKey],
                      kMaxLangVector);
  return kMapToEncoding[toprankenc];
}

// Return top encoding hint for given string
Encoding CompactEncDet::TopEncodingOfTLDHint(const char* name) {
  string normalized_tld = MakeChar4(string(name));
  int n = HintBinaryLookup4(kTLDHintProbs, kTLDHintProbsSize,
                           normalized_tld.c_str());
  if (n < 0) {return UNKNOWN_ENCODING;}

  // TLD is four bytes, probability table is 12 bytes
  int toprankenc =
    TopCompressedProb(&kTLDHintProbs[n].key_prob[kMaxTldKey],
                      kMaxTldVector);
  return kMapToEncoding[toprankenc];
}

// Return top encoding hint for given string
Encoding CompactEncDet::TopEncodingOfCharsetHint(const char* name) {
  string normalized_charset = MakeChar44(string(name));
  int n = HintBinaryLookup8(kCharsetHintProbs, kCharsetHintProbsSize,
                           normalized_charset.c_str());
  if (n < 0) {return UNKNOWN_ENCODING;}

  // Charset is eight bytes, probability table is eight bytes
  int toprankenc =
    TopCompressedProb(&kCharsetHintProbs[n].key_prob[kMaxCharsetKey],
                      kMaxCharsetVector);
  return kMapToEncoding[toprankenc];
}

const char* CompactEncDet::Version() {
  return kVersion;
}

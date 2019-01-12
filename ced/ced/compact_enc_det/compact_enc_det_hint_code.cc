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

#include "compact_enc_det/compact_enc_det_hint_code.h"

#include <ctype.h>                      // for isalpha
#include <string.h>                     // for NULL, memchr, strlen, etc

#include "util/basictypes.h"            // for uint8, uint32
#include "util/string_util.h"

// Upper to lower, keep digits, everything else to minus '-' (2d)
static const char kCharsetToLowerTbl[256] = {
  0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d, 0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,
  0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d, 0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,
  0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d, 0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,
  0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37, 0x38,0x39,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,

  0x2d,0x61,0x62,0x63,0x64,0x65,0x66,0x67, 0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
  0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77, 0x78,0x79,0x7a,0x2d,0x2d,0x2d,0x2d,0x2d,
  0x2d,0x61,0x62,0x63,0x64,0x65,0x66,0x67, 0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
  0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77, 0x78,0x79,0x7a,0x2d,0x2d,0x2d,0x2d,0x2d,

  0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d, 0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,
  0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d, 0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,
  0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d, 0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,
  0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d, 0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,

  0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d, 0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,
  0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d, 0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,
  0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d, 0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,
  0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d, 0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,0x2d,
};


static const char kIsAlpha[256] = {
  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
  0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,0,0,0,0,0,
  0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,0,0,0,0,0,

  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
};

static const char kIsDigit[256] = {
  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1, 1,1,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,

  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
};

static const char* kFakeEncodingName[] = {
  "FakeEnc100", "FakeEnc101", "FakeEnc102", "FakeEnc103", "FakeEnc104",
  "FakeEnc105", "FakeEnc106", "FakeEnc107", "FakeEnc108", "FakeEnc109",
  "FakeEnc110", "FakeEnc111", "FakeEnc112", "FakeEnc113", "FakeEnc114",
  "FakeEnc115", "FakeEnc116", "FakeEnc117", "FakeEnc118", "FakeEnc119",
};
static const char* kFakeEncodingName2[] = {
  "FakeEnc_0", "FakeEnc_1", "FakeEnc_2", "FakeEnc_3", "FakeEnc_4",
};

// Return name for extended encoding
const char* MyEncodingName(Encoding enc) {
  if (enc < 0) {
    return "~";
  }
  if (enc == ISO_8859_1) {
    return "Latin1";   // I can't stand "ASCII" for this
  }
  if (enc < NUM_ENCODINGS) {
    return EncodingName(enc);
  }
  // allow fake names, for exploration
  if ((NUM_ENCODINGS <= enc) && (enc < (NUM_ENCODINGS + 4))) {
    return kFakeEncodingName2[enc - NUM_ENCODINGS];
  }
  if ((100 <= enc) && (enc < 120)) {
    return kFakeEncodingName[enc - 100];
  }
  return "~";
}


// Normalize ASCII string to first 4 alphabetic chars and last 4 digit chars
// Letters are forced to lowercase ASCII
// Used to normalize charset= values
string MakeChar44(const string& str) {
  string res("________");     // eight underscores
  int l_ptr = 0;
  size_t d_ptr = 0;
  for (char ch : str) {
    auto uc = static_cast<uint8>(ch);
    if (kIsAlpha[uc]) {
      if (l_ptr < 4) {                  // Else ignore
        res[l_ptr] = kCharsetToLowerTbl[uc];
        l_ptr++;
      }
    } else if (kIsDigit[uc]) {
      if (d_ptr < 4) {
        res[4 + d_ptr] = kCharsetToLowerTbl[uc];
      } else {
        // Keep last 4 digits by shifting left
        res[4] = res[5];
        res[5] = res[6];
        res[6] = res[7];
        res[7] = kCharsetToLowerTbl[uc];
      }
      d_ptr++;
    }   // If neither letter nor digit, drop entirely
  }
  return res;
}

// Normalize ASCII string to first 8 alphabetic/digit chars
// Letters are forced to lowercase ASCII
// Used to normalize TLD values
string MakeChar4(const string& str) {
  string res("____");     // four underscores
  int l_ptr = 0;
  for (char ch : str) {
    auto uc = static_cast<uint8>(ch);
    if (kIsAlpha[uc] | kIsDigit[uc]) {
      if (l_ptr < 4) {                  // Else ignore
        res[l_ptr] = kCharsetToLowerTbl[uc];
        l_ptr++;
      }
    }
  }
  return res;
}

// Normalize ASCII string to first 8 alphabetic/digit chars
// Letters are forced to lowercase ASCII
// Used to normalize TLD values
string MakeChar8(const string& str) {
  string res("________");     // eight dots
  int l_ptr = 0;
  for (char ch : str) {
    auto uc = static_cast<uint8>(ch);
    if (kIsAlpha[uc] | kIsDigit[uc]) {
      if (l_ptr < 8) {                  // Else ignore
        res[l_ptr] = kCharsetToLowerTbl[uc];
        l_ptr++;
      }
    }
  }
  return res;
}

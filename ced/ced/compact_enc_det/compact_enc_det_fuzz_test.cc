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

#include <stddef.h>
#include <stdlib.h>
#include <memory>

#include "compact_enc_det/compact_enc_det.h"
#include "util/encodings/encodings.h"
#include "util/languages/languages.h"
#include "util/port.h"
#include "gtest/gtest.h"

namespace {

class CompactEncDetFuzzTest : public testing::Test {};

TEST_F(CompactEncDetFuzzTest, TestRandom) {
  for (size_t i = 0; i < 16384; ++i) {
    unsigned int seed = i;
    srand(seed);
    size_t length = static_cast<size_t>(rand()) % 1024;
    std::unique_ptr<char[]> text(new char[length]);

    for (size_t j = 0; j < length; ++j) text[j] = rand();

    int bytes_consumed;
    bool is_reliable;

    CompactEncDet::DetectEncoding(text.get(), length, nullptr,  // URL hint
                                  nullptr,                      // HTTP hint
                                  nullptr,                      // Meta hint
                                  UNKNOWN_ENCODING,
                                  UNKNOWN_LANGUAGE,
                                  CompactEncDet::WEB_CORPUS,
                                  false,  // Include 7-bit encodings?
                                  &bytes_consumed, &is_reliable);
  }
}

}  // namespace

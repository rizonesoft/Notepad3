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

#include "util/encodings/encodings.h"

#include "gtest/gtest.h"

TEST(EncodingsTest, EncodingNameAliasToEncoding) {
  // Test that cases, non-alpha-numeric chars are ignored.
  EXPECT_EQ(ISO_8859_1, EncodingNameAliasToEncoding("iso_8859_1"));
  EXPECT_EQ(ISO_8859_1, EncodingNameAliasToEncoding("iso-8859-1"));

  // Test that spaces are ignored.
  EXPECT_EQ(UTF8, EncodingNameAliasToEncoding("UTF8"));
  EXPECT_EQ(UTF8, EncodingNameAliasToEncoding("UTF 8"));
  EXPECT_EQ(UTF8, EncodingNameAliasToEncoding("UTF-8"));

  // Test alphanumeric differences are counted.
  EXPECT_NE(UTF8, EncodingNameAliasToEncoding("UTF-7"));
  EXPECT_NE(KOREAN_EUC_KR, EncodingNameAliasToEncoding("euc-jp"));
}

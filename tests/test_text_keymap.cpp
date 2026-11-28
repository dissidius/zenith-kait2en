#include <gtest/gtest.h>

#include "src/platform/linux/input/text_keymap.h"

using platf::keyboard::text::lookup;

TEST(TextKeymap, CommonLettersAndDigits) {
  for (auto layout : {"us", "it"}) {
    EXPECT_EQ(lookup(U'a', layout)->key, 0x41);
    EXPECT_FALSE(lookup(U'a', layout)->shift);
    EXPECT_TRUE(lookup(U'A', layout)->shift);
    EXPECT_EQ(lookup(U'9', layout)->key, 0x39);
    EXPECT_EQ(lookup(U' ', layout)->key, 0x20);
  }
}

TEST(TextKeymap, LayoutSpecificSymbols) {
  EXPECT_EQ(lookup(U'@', "us")->key, 0x32);
  EXPECT_TRUE(lookup(U'@', "us")->shift);
  EXPECT_EQ(lookup(U'@', "it")->key, 0xBA);
  EXPECT_TRUE(lookup(U'@', "it")->altgr);
  EXPECT_EQ(lookup(U'?', "it")->key, 0xBD);
  EXPECT_EQ(lookup(U'ù', "it")->key, 0xDC);
  EXPECT_EQ(lookup(U'\\', "it")->key, 0xC0);
  EXPECT_EQ(lookup(U'"', "it")->key, 0x32);
  EXPECT_EQ(lookup(U'<', "it")->key, 0xE2);
}

TEST(TextKeymap, UnsupportedAndControlCharacters) {
  EXPECT_FALSE(lookup(U'ë', "us"));
  EXPECT_FALSE(lookup(U'🙂', "it"));
  EXPECT_FALSE(lookup(U'a', "unicode"));
  EXPECT_FALSE(lookup(U'\n', "us"));
  EXPECT_FALSE(lookup(U'\t', "it"));
}

#include "gtest/gtest.h"

#include "pk_font.hpp"

TEST(PKFontTest, opening_font_file) {
  PKFont * pk_font;

  pk_font = new PKFont("fonts/cmr10.166pk");
  EXPECT_TRUE(pk_font != nullptr);
  if (pk_font != nullptr) {
    EXPECT_TRUE(pk_font->is_initialized());
    delete pk_font;
  }
}

TEST(PKFontTest, access_a_glyph) {
  PKFont * pk_font;

  pk_font = new PKFont("fonts/cmr10.166pk");
  EXPECT_TRUE(pk_font != nullptr);
  if (pk_font != nullptr) {
    EXPECT_TRUE(pk_font->is_initialized());

    PKFont::Glyph glyph;

    EXPECT_TRUE(pk_font->get_glyph('A', glyph, true));
    EXPECT_TRUE(glyph.bitmap != nullptr);
    EXPECT_TRUE(glyph.char_code == 'A');
    delete [] glyph.bitmap;
    delete pk_font;
  }
}
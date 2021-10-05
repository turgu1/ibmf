#include "gtest/gtest.h"

#include "pk_font.hpp"

TEST(PKFontTest, opening_font_file) {
  PKFont * pk_font;

  pk_font = new PKFont("fonts/cmr12.166pk");
  EXPECT_TRUE(pk_font != nullptr);
  if (pk_font != nullptr) {
    EXPECT_TRUE(pk_font->is_initialized());
    delete pk_font;
  }
}

TEST(PKFontTest, access_a_glyph) {
  PKFont * pk_font;

  pk_font = new PKFont("fonts/cmr12.166pk");
  EXPECT_TRUE(pk_font != nullptr);
  if (pk_font != nullptr) {
    EXPECT_TRUE(pk_font->is_initialized());

    PKFont::Glyph glyph;

    // EXPECT_TRUE(pk_font->get_glyph('a', glyph, true));
    // EXPECT_TRUE(glyph.bitmap != nullptr);

    for (uint8_t i = 0; i < 128; i++) {
      pk_font->get_glyph(i, glyph, true);
      if (glyph.bitmap != nullptr) pk_font->show_glyph(glyph);
      delete [] glyph.bitmap;
    }

    // EXPECT_TRUE(glyph.char_code == 'a');
    // pk_font->show_glyph(glyph);
    // delete [] glyph.bitmap;
    delete pk_font;
  }
}
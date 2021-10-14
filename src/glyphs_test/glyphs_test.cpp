// This is a simple example of using the IBMFFont class to get the
// bitmap related to a character code shown as a matrix of Xs
//
// The font being used is ComputerModern-Regular_212.ibmf
//
// It must be run with the main project folder as the current folder.
// For example:
//
// .pio/build/ascii_example/program À
//
// It is expected that UTF-8 is used to pass parameters from the shell.

#include "ibmf_font.hpp"

IBMFFont font("/home/turgu1/Dev/ibmf/fonts/ComputerModernSans-Italic_212.ibmf", IBMFFont::PixelResolution::ONE_BIT);

uint32_t 
to_unicode(const char *str)
{
  const uint8_t  * c    = (uint8_t *) str;
  uint32_t         u    = 0;
  bool             done = false;

  while (true) {
    if ((*c & 0x80) == 0x00) {
      u = *c++;
      done = true;
    }
    else if ((*str & 0xF1) == 0xF0) {
      u = *c++ & 0x07;
      if (*c == 0) break; else u = (u << 6) + (*c++ & 0x3F);
      if (*c == 0) break; else u = (u << 6) + (*c++ & 0x3F);
      if (*c == 0) break; else u = (u << 6) + (*c++ & 0x3F);
      done = true;
    }
    else if ((*str & 0xF0) == 0xE0) {
      u = *c++ & 0x0F;
      if (*c == 0) break; else u = (u << 6) + (*c++ & 0x3F);
      if (*c == 0) break; else u = (u << 6) + (*c++ & 0x3F);
      done = true;
    }
    else if ((*str & 0xE0) == 0xC0) {
      u = *c++ & 0x1F;
      if (*c == 0) break; else u = (u << 6) + (*c++ & 0x3F);
      done = true;
    }
    break;
  }

  return done ? u : ' ';
}

int
main(int argc, char **argv)
{
  IBMFFont::Glyph glyph;

  font.set_font_size(12);

  // // for (int i = 0; i <= 32; i++) {
  // //   font.get_glyph(i, glyph, true, true);
  // //   font.show_glyph(glyph);
  // // }
  
  // font.get_glyph(0x5E, glyph, true, true);
  // font.show_glyph(glyph);

  // font.get_glyph(0xE9, glyph, true);
  // font.show_glyph(glyph);

  font.get_glyph(to_unicode("Î"), glyph, true);
  font.show_glyph(glyph);

  // font.get_glyph(to_unicode("Ì"), glyph, true);
  // font.show_glyph(glyph);

  // font.get_glyph(to_unicode("Ï"), glyph, true);
  // font.show_glyph(glyph);

  font.get_glyph(to_unicode("î"), glyph, true);
  font.show_glyph(glyph);

  // font.get_glyph(to_unicode("ì"), glyph, true);
  // font.show_glyph(glyph);

  // font.get_glyph(to_unicode("ï"), glyph, true);
  // font.show_glyph(glyph);
}
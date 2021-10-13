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

IBMFFont font("/home/turgu1/Dev/ibmf/fonts/ComputerModern-Regular_212.ibmf", IBMFFont::PixelResolution::ONE_BIT);

int
main(int argc, char **argv)
{
  IBMFFont::Glyph glyph;

  font.set_font_size(24);

  for (int i = 0; i <= 32; i++) {
    font.get_glyph(i, glyph, true, true);
    font.show_glyph(glyph);
  }
  
  font.get_glyph(0x5E, glyph, true, true);
  font.show_glyph(glyph);

  font.get_glyph(0xE9, glyph, true);
  font.show_glyph(glyph);

}
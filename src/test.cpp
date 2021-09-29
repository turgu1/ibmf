#include <iostream>
#include <fstream>

#include "pk_font.hpp"

int 
main(int argc, char **argv)
{
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <pk filename>" << std::endl;
    return 1;
  }

  PKFont pk_font(argv[1]);

  if (pk_font.is_initialized()) {
    PKFont::Glyph glyph;
    if (pk_font.read_glyph('A', &glyph, true)) pk_font.show_glyph(glyph);
  }

  return 1;
}
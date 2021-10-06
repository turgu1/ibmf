#include "pk_font.hpp"
#include "tfm.hpp"
#include "ibmf_gener.hpp"

int 
main(int argc, char **argv)
{
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << " <prefix> <dpi> <tex_font_name>" << std::endl;
    return 1;
  }

  std::string filename = "../fonts/";
  filename.append(argv[3]).append(".").append(argv[2]).append("pk");

  PKFont pk_font(filename);

  filename = "../fonts/";
  filename.append(argv[3]).append(".tfm");
  TFM tfm(filename, atoi(argv[2]));

  if (pk_font.is_initialized() && tfm.is_initialized()) {
    IBMFGener ibmf_gener(argv[1], argv[2], tfm, pk_font);
  }

  // tfm.show();

  // PKFont * pk_font;

  // pk_font = new PKFont("fonts/cmr12.166pk");
  // if (pk_font != nullptr) {
  //   PKFont::Glyph glyph;
  //   for (uint8_t i = 0; i < 128; i++) {
  //     pk_font->get_glyph(i, glyph, true);
  //     if (glyph.bitmap != nullptr) pk_font->show_glyph(glyph);
  //     delete [] glyph.bitmap;
  //   }
  //   delete pk_font;
  // }

  return 0;
}
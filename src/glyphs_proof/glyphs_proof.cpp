#include "ibmf_font.hpp"

IBMFFont font("/home/turgu1/Dev/ibmf/fonts/ComputerModern-Regular_166.ibmf", IBMFFont::PixelResolution::EIGHT_BITS);

char matrix[100][200];
int  col = 0;
int  max_row = 0;

inline void clear_matrix() { memset(matrix, ' ', 100 * 200); }


void show() 
{
  if (col > 200) col = 200;

  for (int j = 0; j < max_row; j++) {
    for (int i = 0; i < col; i++) {
      std::cout << matrix[j][i];
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

void
put(const IBMFFont::Glyph & glyph)
{
  if (col + glyph.bitmap_width >= 200) {
    show();
    col = 0;
    max_row = 0;
    clear_matrix();
  }

  int first_row = (font.get_line_height() * 0.8) - font.get_descender_height() + glyph.vertical_offset;

  int row_size = glyph.bitmap_width;
  uint8_t * rowp = glyph.bitmap;

  for (int j = 0; j < glyph.bitmap_height; j++) {
    for (int i = 0; i < glyph.bitmap_width; i++) {
      matrix[first_row + j][col + i] = rowp[i] ? 'X' : ' ';
    }
    rowp += row_size;
  }
  if ((first_row + glyph.bitmap_height) > max_row) max_row = first_row + glyph.bitmap_height;
  col += glyph.bitmap_width + 3;
}

int
main()
{

  font.set_font_size(12);

  IBMFFont::Glyph glyph;

  clear_matrix();

  for (int i = 0; i < 0x17F; i++) {
    font.get_glyph(i, glyph, true);
    if (glyph.char_code != ' ') {
      put(glyph);
    }
  }

  if (col > 0) show();

  return 0;
}
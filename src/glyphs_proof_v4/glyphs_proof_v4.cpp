#include "ibmf_font_v4.hpp"

IBMFFont font("/home/turgu1/Dev/ibmf/fonts/EC-Regular_212.ibmf", IBMFFont::PixelResolution::EIGHT_BITS);

char matrix[100][200];
int  col     = 0;
int  max_row = 0;


const uint32_t others[16] = {
  0x02BB, // reverse apostrophe
  0x02BC, // apostrophe
  0x02C6, // circumflex
  0x02DA, // ring
  0x02DC, // tilde ~
  0x2013, // endash
  0x2014, // emdash
  0x2018, // quote left
  0x2019, // quote right
  0x201C, // quoted left "
  0x201D, // quoted right
  0x201A, // comma like ,
  0x2032, // minute '
  0x2033, // second "
  0x2044, // fraction /
  0x20AC  // Euro
};

inline void clear_matrix() { 
  memset(matrix, ' ', 100 * 200); 
  col     = 0;
  max_row = 0;
}

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

/*
void
put(const IBMFFont::Glyph & glyph, int code)
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

  sprintf(&matrix[first_row][col], "%04x:", code);
  matrix[first_row++][col+5] = ' ';

  for (int j = 0; j < glyph.bitmap_height; j++) {
    for (int i = 0; i < glyph.bitmap_width; i++) {
      matrix[first_row + j][col + i] = rowp[i] ? 'X' : ' ';
    }
    rowp += row_size;
  }
  if ((first_row + glyph.bitmap_height) > max_row) max_row = first_row + glyph.bitmap_height;
  col += glyph.bitmap_width + 3;
}
*/
int
main()
{

  if (!font.set_font_size(14)) {
    std::cerr << "Font size 14 not available!" << std::endl;
    return 1;
  }

  font.show_font();
  
  IBMFFont::BitmapPtr bitmap = font.get_glyph_bitmap('A');

  font.show_bitmap(bitmap);
/*  
  if (!font.set_font_size(12)) {
    std::cerr << "Font size 12 not available!" << std::endl;
    return 1;
  }

  IBMFFont::Glyph glyph;

  clear_matrix();

  for (int i = 0; i < 0x17F; i++) {
    font.get_glyph(i, glyph, true);
    if (glyph.glyph_info.char_code != ' ') {
      put(glyph, i);
    }
  }

  for (int i = 0; i < 16; i++) {
    font.get_glyph(others[i], glyph, true);
    if (glyph.char_code != ' ') {
      put(glyph, others[i]);
    }
  }

  if (col > 0) show();

  std::cout << std::endl << std::endl << "----------" << std::endl << std::endl;

  clear_matrix();

  for (int i = 0; i < font.get_glyph_count(); i++) {
    font.get_glyph(i, glyph, true, true);
    put(glyph, i);
  }
*/
  return 0;
}
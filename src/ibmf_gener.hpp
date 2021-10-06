#pragma once

#include <cstdio>

#include "pk_font.hpp"
#include "tfm.hpp"

class IBMFGener
{
  private:
    FILE * file;

    typedef int16_t FIX16;

    #pragma pack(push, 1)
      struct Header {
        uint8_t  point_size;
        uint8_t  line_height;
        uint16_t dpi;
        FIX16    space_size;
      };

      struct Glyph {
        uint32_t  tfm_width;
        uint32_t  packet_length;
        uint32_t  char_code;
        uint32_t  horizontal_escapement;
        uint32_t  vertical_escapement;
        uint32_t  bitmap_width;
        uint32_t  bitmap_height;
        int32_t   horizontal_offset;
        int32_t   vertical_offset;
        uint32_t  repeat_count;
        uint32_t  raster_size;
        uint8_t   dyn_f;
        bool      first_nibble_is_black;
        bool      is_a_bitmap;
        uint8_t * bitmap;
        Glyph() { bitmap = nullptr; }
      };
    #pragma pack(pop)
    

  public:
    IBMFGener(char * prefix, char * dpi, TFM & tfm, PKFont & pk_font) {
      std::string filename = prefix;
      filename.append(".ibmf");

      if ((file = fopen(filename.c_str(), "wb")) != nullptr) {
        header();
        glyphs();
        lig_kern();
        fclose(file);
      }
    }
};
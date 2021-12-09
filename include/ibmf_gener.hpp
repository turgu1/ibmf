#pragma once

#include <cstdio>

#include "pk_font.hpp"
#include "tfm.hpp"

/**
 * @brief Generate a single IBMF font entry
 * 
 * This class is used to genereate a single IBMF font entry that will
 * be part of a IBMF file. The font is retrieved from a `.tfm` and `.pk`,
 * classes, processed to extract the required fields and appended to
 * the file.
 * 
 */

class IBMFGener {
  private:
    FILE   * file;

    TFM    & tfm;
    TFM    & tfm2;

    PKFont & pk;
    PKFont & pk2;

    double   factor;
    uint8_t  char_set;

    #pragma pack(push, 1)
      struct Header {
        uint8_t    point_size;
        uint8_t    line_height;
        uint16_t   dpi;
        TFM::FIX16 x_height;
        TFM::FIX16 em_size;
        TFM::FIX16 slant_correction;
        uint8_t    descender_height;
        uint8_t    space_size;
        uint16_t   glyph_count;
        uint8_t    lig_kern_pgm_count;
        uint8_t    kern_count;
      };

      struct GlyphMetric {
        unsigned int dyn_f:4;
        unsigned int first_is_black:1;
        unsigned int filler:3;
      };

      struct LigKernStep {
        unsigned int next_char_code:8;
        union {
          unsigned int char_code:8;
          unsigned int kern_idx:8;  // Ligature: replacement char code, kern: displacement
        } u;
        unsigned int tag:1;         // 0 = Ligature, 1 = Kern
        unsigned int stop:1;
        unsigned int filler:6;
      };

      struct Glyph {
        uint8_t     char_code;
        uint8_t     bitmap_width;
        uint8_t     bitmap_height;
        int8_t      horizontal_offset;
        int8_t      vertical_offset;
        uint8_t     lig_kern_pgm_index; // = 255 if none
        uint16_t    packet_length;
        TFM::FIX16  advance;
        GlyphMetric glyph_metric;
      };
    #pragma pack(pop)

    // uint8_t compute_max_descender_height() {
    //   uint8_t max = 0;

    //   for (int i = tfm.get_first_glyph_code(); i <= tfm.get_last_glyph_code(); i++) {
    //     PKFont::Glyph pk_glyph;
    //     if (pk.get_glyph(i, pk_glyph, false)) {
    //       pk_glyph.
    //     }
    //   }
    //   return max;
    // }    

    Header header;

    void put_header(uint16_t dpi, uint16_t glyph_count) {
      header.dpi         = dpi;
      header.point_size  = trunc(tfm.to_double(tfm.get_design_size(), 20));
      header.line_height = trunc(((double)(header.dpi * header.point_size)) / 72.27 + 0.5);

      factor = tfm.to_double(tfm.get_design_size(), 20) * dpi / 72.27;

      header.x_height           = tfm.to_fix16(tfm.to_double(tfm.get_x_height(), 20) * factor, 6);
      header.em_size            = tfm.to_fix16(tfm.to_double(tfm.get_em_size(),  20) * factor, 6);
      header.space_size         = trunc(tfm.to_double(tfm.get_space_size(), 20) * factor);
      header.glyph_count        = glyph_count;
      header.lig_kern_pgm_count = tfm.get_lig_kern_pgm_count();
      header.kern_count         = tfm.get_kern_count();
      header.slant_correction   = tfm.to_fix16(tfm.to_double(tfm.get_slant_correction(), 20), 6);
      header.descender_height   = tfm.to_fix16(tfm.to_double(tfm.get_max_depth(), 20) * factor, 6) >> 6;

      fwrite(&header, sizeof(Header), 1, file);
    }

    void put_glyph(uint8_t pk_char_code, uint8_t ibmf_char_code) {
      Glyph glyph;
      PKFont::Glyph pk_glyph;
      if (pk.get_glyph(pk_char_code, pk_glyph, false)) {
        glyph.char_code                   = ibmf_char_code;
        glyph.bitmap_width                = pk_glyph.bitmap_width;
        glyph.bitmap_height               = pk_glyph.bitmap_height;
        glyph.horizontal_offset           = pk_glyph.horizontal_offset;
        glyph.vertical_offset             = pk_glyph.vertical_offset;
        glyph.packet_length               = pk_glyph.raster_size;
        glyph.advance                     = tfm.to_fix16(tfm.to_double(tfm.get_advance(pk_char_code), 20) * factor, 6);
        glyph.glyph_metric.dyn_f          = pk_glyph.dyn_f;
        glyph.glyph_metric.first_is_black = pk_glyph.first_nibble_is_black ? 1 : 0;
        glyph.glyph_metric.filler         = 0;
        glyph.lig_kern_pgm_index          = tfm.get_lig_kern_pgm_index(pk_char_code);

        fwrite(&glyph, sizeof(Glyph), 1, file);
        fwrite(pk_glyph.raster, glyph.packet_length, 1, file);
      }
    }

    void put2_glyph(uint8_t pk_char_code, uint8_t ibmf_char_code) {
      Glyph glyph;
      PKFont::Glyph pk_glyph;
      if (pk2.get_glyph(pk_char_code, pk_glyph, false)) {
        glyph.char_code                   = ibmf_char_code;
        glyph.bitmap_width                = pk_glyph.bitmap_width;
        glyph.bitmap_height               = pk_glyph.bitmap_height;
        glyph.horizontal_offset           = pk_glyph.horizontal_offset;
        glyph.vertical_offset             = pk_glyph.vertical_offset;
        glyph.packet_length               = pk_glyph.raster_size;
        glyph.advance                     = tfm2.to_fix16(tfm2.to_double(tfm2.get_advance(pk_char_code), 20) * factor, 6);
        glyph.glyph_metric.dyn_f          = pk_glyph.dyn_f;
        glyph.glyph_metric.first_is_black = pk_glyph.first_nibble_is_black ? 1 : 0;
        glyph.glyph_metric.filler         = 0;
        glyph.lig_kern_pgm_index          = 255;

        fwrite(&glyph, sizeof(Glyph), 1, file);
        fwrite(pk_glyph.raster, glyph.packet_length, 1, file);
      }
    }

    void put_glyphs() {
      if (char_set == 0) {
        uint8_t ibmf_char_code = 0;
        for (int i = 0; i < 0x17; i++) put_glyph(i, ibmf_char_code++);
        put_glyph(0xBE, ibmf_char_code++); // ¿  0x17
        for (int i = 0x18; i < 0x20; i++) put_glyph(i, ibmf_char_code++);
        put_glyph(0xBD, ibmf_char_code++); // ¡  0x20
        for (int i = 0x21; i < 0x7F; i++) put_glyph(i, ibmf_char_code++);

        put_glyph(0x89, ibmf_char_code++); // Ľ  0x7F
        put_glyph(0x8A, ibmf_char_code++); // Ł  0x80
        put_glyph(0x8D, ibmf_char_code++); // Ŋ  0x81
        put_glyph(0x9C, ibmf_char_code++); // Ĳ  0x82
        put_glyph(0x9E, ibmf_char_code++); // đ  0x83
        put_glyph(0x9F, ibmf_char_code++); // §  0x84
        put_glyph(0xA4, ibmf_char_code++); // ď  0x85
        put_glyph(0xA9, ibmf_char_code++); // ľ  0x86
        put_glyph(0xAA, ibmf_char_code++); // ł  0x87
        put_glyph(0xAD, ibmf_char_code++); // ŋ  0x88
        put_glyph(0xB4, ibmf_char_code++); // ť  0x89
        put_glyph(0xBC, ibmf_char_code++); // ĳ  0x8A
        put_glyph(0xBF, ibmf_char_code++); // £  0x8B
        put_glyph(0xC6, ibmf_char_code++); // Æ  0x8C
        put_glyph(0xD0, ibmf_char_code++); // Ð  0x8D
        put_glyph(0xD7, ibmf_char_code++); // Œ  0x8E
        put_glyph(0xD8, ibmf_char_code++); // Ø  0x8F
        put_glyph(0xDE, ibmf_char_code++); // Þ  0x90
        put_glyph(0xDF, ibmf_char_code++); // SS 0x91
        put_glyph(0xE6, ibmf_char_code++); // æ  0x92
        put_glyph(0xF0, ibmf_char_code++); // ð  0x93
        put_glyph(0xF7, ibmf_char_code++); // œ  0x94
        put_glyph(0xF8, ibmf_char_code++); // ø  0x95
        put_glyph(0xFE, ibmf_char_code++); // þ  0x96
        put_glyph(0xFF, ibmf_char_code++); // ß  0x97
        put2_glyph(0xA2, ibmf_char_code++); // ¢  0x98
        put2_glyph(0xA4, ibmf_char_code++); // ¤  0x99
        put2_glyph(0xA5, ibmf_char_code++); // ¥  0x9A
        put2_glyph(0xA6, ibmf_char_code++); // ¦  0x9B
        put2_glyph(0xA9, ibmf_char_code++); // ©  0x9C
        put2_glyph(0xAA, ibmf_char_code++); // ª  0x9D
        put2_glyph(0xAC, ibmf_char_code++); // ¬  0x9E
        put2_glyph(0xAE, ibmf_char_code++); // ®  0x9F
        put2_glyph(0xB1, ibmf_char_code++); // ±  0xA0
        put2_glyph(0xB2, ibmf_char_code++); // ²  0xA1
        put2_glyph(0xB3, ibmf_char_code++); // ³  0xA2
        put2_glyph(0xB5, ibmf_char_code++); // µ  0xA3
        put2_glyph(0xB6, ibmf_char_code++); // ¶  0xA4
        put2_glyph(0xB7, ibmf_char_code++); // middle dot  0xA5
        put2_glyph(0xB9, ibmf_char_code++); // ¹  0xA6
        put2_glyph(0xBA, ibmf_char_code++); // º  0xA7
        put2_glyph(0xD6, ibmf_char_code++); // ×  0xA8
        put2_glyph(0xBC, ibmf_char_code++); // ¼  0xA9
        put2_glyph(0xBD, ibmf_char_code++); // ½  0xAA
        put2_glyph(0xBE, ibmf_char_code++); // ¾  0xAB
        put2_glyph(0xF6, ibmf_char_code++); // ÷  0xAC
        put2_glyph(0xBF, ibmf_char_code++); // Euro  0xAD
      }
      else {
        for (int i = tfm.get_first_glyph_code(); i <= tfm.get_last_glyph_code(); i++) {
          put_glyph(i, i);
        }
      }
    }

    void put_lig_kerns() {
      TFM::LigKernStep tfm_lks;
      LigKernStep lks;

      for (int i = 0; i < header.lig_kern_pgm_count; i++) {
        tfm_lks = tfm.get_lig_kern_step(i);

        lks.next_char_code = tfm_lks.next_char_code;
        lks.u.char_code    = tfm_lks.char_code_or_index;
        lks.stop           = tfm_lks.stop;
        lks.tag            = tfm_lks.tag;

        fwrite(&lks, sizeof(LigKernStep), 1, file);
      }

      for (int i = 0; i < header.kern_count; i++) {
        TFM::FIX kern = tfm.get_kern(i);
        TFM::FIX16 k = tfm.to_fix16(tfm.to_double(kern, 20) * factor, 6);

        fwrite(&k, sizeof(TFM::FIX16), 1, file);
      }
    }

  public:
    IBMFGener(FILE * file, char * dpi, TFM & tfm, TFM & tfm2, PKFont & pk, PKFont & pk2, uint8_t char_set) 
      : file(file), tfm(tfm), tfm2(tfm2), pk(pk), pk2(pk2), char_set(char_set) {
      put_header(atoi(dpi), char_set == 0 ? 0xAE : tfm.get_glyph_count());
      put_glyphs();
      put_lig_kerns();
    }
};

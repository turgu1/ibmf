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
    PKFont & pk;
    double   factor;

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
        uint8_t    glyph_count;
        uint8_t    lig_kern_pgm_count;
        uint8_t    kern_count;
        uint8_t    version;
      };

      struct GlyphMetric {
        unsigned int dyn_f:4;
        unsigned int first_is_black:1;
        unsigned int filler:3;
      };

      struct LigKernStep {
        unsigned int next_char_code:7;
        unsigned int stop:1;
        union {
          unsigned int char_code:7;
          unsigned int kern_idx:7;  // Ligature: replacement char code, kern: displacement
        } u;
        unsigned int tag:1;         // 0 = Ligature, 1 = Kern
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

    void put_header(uint16_t dpi) {
      header.version     = 1;
      header.dpi         = dpi;
      header.point_size  = trunc(tfm.to_double(tfm.get_design_size(), 20));
      header.line_height = trunc(((double)(header.dpi * header.point_size)) / 72.27 + 0.5);

      factor = tfm.to_double(tfm.get_design_size(), 20) * dpi / 72.27;

      header.x_height           = tfm.to_fix16(tfm.to_double(tfm.get_x_height(), 20) * factor, 6);
      header.em_size            = tfm.to_fix16(tfm.to_double(tfm.get_em_size(),  20) * factor, 6);
      header.space_size         = trunc(tfm.to_double(tfm.get_space_size(), 20) * factor);
      header.glyph_count        = tfm.get_glyph_count();
      header.lig_kern_pgm_count = tfm.get_lig_kern_pgm_count();
      header.kern_count         = tfm.get_kern_count();
      header.slant_correction   = tfm.to_fix16(tfm.to_double(tfm.get_slant_correction(), 20), 6);
      header.descender_height   = tfm.to_fix16(tfm.to_double(tfm.get_max_depth(), 20) * factor, 6) >> 6;

      fwrite(&header, sizeof(Header), 1, file);
    }

    void put_glyphs() {
      for (int i = tfm.get_first_glyph_code(); i <= tfm.get_last_glyph_code(); i++) {
        PKFont::Glyph pk_glyph;
        if (pk.get_glyph(i, pk_glyph, false)) {
          Glyph glyph;

          glyph.char_code          = i;
          glyph.bitmap_width       = pk_glyph.bitmap_width;
          glyph.bitmap_height      = pk_glyph.bitmap_height;
          glyph.horizontal_offset  = pk_glyph.horizontal_offset;
          glyph.vertical_offset    = pk_glyph.vertical_offset;
          glyph.packet_length      = pk_glyph.raster_size;
          glyph.advance            = tfm.to_fix16(tfm.to_double(tfm.get_advance(i), 20) * factor, 6);
          glyph.glyph_metric.dyn_f          = pk_glyph.dyn_f;
          glyph.glyph_metric.first_is_black = pk_glyph.first_nibble_is_black ? 1 : 0;
          glyph.glyph_metric.filler         = 0;
          glyph.lig_kern_pgm_index          = tfm.get_lig_kern_pgm_index(i);

          fwrite(&glyph, sizeof(Glyph), 1, file);
          fwrite(pk_glyph.raster, glyph.packet_length, 1, file);
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
    IBMFGener(FILE * file, char * dpi, TFM & tfm, PKFont & pk) : file(file), tfm(tfm), pk(pk) {
      put_header(atoi(dpi));
      put_glyphs();
      put_lig_kerns();
    }
};

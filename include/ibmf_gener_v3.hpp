#pragma once

#include <cstdio>
#include <vector>
#include <set>

#include "pk_font.hpp"
#include "tfm_v3.hpp"

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

    struct GlyphInfo {
      Glyph glyph;
      uint8_t old_char_code;
      uint8_t old_lig_kern_idx;
      int new_lig_kern_idx; // need to be larger for cases that goes beyond 255
    };

    std::vector<TFM::LigKernStep> lig_kerns;

    std::vector<GlyphInfo> glyphs;

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

    void read_header(uint16_t dpi, uint16_t glyph_count) {
      std::cout << "Reading Header...." << std::endl << std::flush;

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

      //fwrite(&header, sizeof(Header), 1, file);
    }

    void read_glyph(uint8_t pk_char_code, uint8_t ibmf_char_code) {
      GlyphInfo glyph_info;
      PKFont::Glyph pk_glyph;
      if (pk.get_glyph(pk_char_code, pk_glyph, false)) {
        glyph_info.glyph.char_code                   = ibmf_char_code;
        glyph_info.glyph.bitmap_width                = pk_glyph.bitmap_width;
        glyph_info.glyph.bitmap_height               = pk_glyph.bitmap_height;
        glyph_info.glyph.horizontal_offset           = pk_glyph.horizontal_offset;
        glyph_info.glyph.vertical_offset             = pk_glyph.vertical_offset;
        glyph_info.glyph.packet_length               = pk_glyph.raster_size;
        glyph_info.glyph.advance                     = tfm.to_fix16(tfm.to_double(tfm.get_advance(pk_char_code), 20) * factor, 6);
        glyph_info.glyph.glyph_metric.dyn_f          = pk_glyph.dyn_f;
        glyph_info.glyph.glyph_metric.first_is_black = pk_glyph.first_nibble_is_black ? 1 : 0;
        glyph_info.glyph.glyph_metric.filler         = 0;
        glyph_info.glyph.lig_kern_pgm_index          = tfm.get_lig_kern_pgm_index(pk_char_code);

        glyph_info.old_char_code = pk_char_code;
        
        glyphs.push_back(glyph_info);
        // fwrite(&glyph, sizeof(Glyph), 1, file);
        // fwrite(pk_glyph.raster, glyph.packet_length, 1, file);
      }
    }

    void read2_glyph(uint8_t pk_char_code, uint8_t ibmf_char_code) {
      GlyphInfo glyph_info;
      PKFont::Glyph pk_glyph;
      if (pk2.get_glyph(pk_char_code, pk_glyph, false)) {
        glyph_info.glyph.char_code                   = ibmf_char_code;
        glyph_info.glyph.bitmap_width                = pk_glyph.bitmap_width;
        glyph_info.glyph.bitmap_height               = pk_glyph.bitmap_height;
        glyph_info.glyph.horizontal_offset           = pk_glyph.horizontal_offset;
        glyph_info.glyph.vertical_offset             = pk_glyph.vertical_offset;
        glyph_info.glyph.packet_length               = pk_glyph.raster_size;
        glyph_info.glyph.advance                     = tfm2.to_fix16(tfm2.to_double(tfm2.get_advance(pk_char_code), 20) * factor, 6);
        glyph_info.glyph.glyph_metric.dyn_f          = pk_glyph.dyn_f;
        glyph_info.glyph.glyph_metric.first_is_black = pk_glyph.first_nibble_is_black ? 1 : 0;
        glyph_info.glyph.glyph_metric.filler         = 0;
        glyph_info.glyph.lig_kern_pgm_index          = 255;

        glyph_info.old_char_code = pk_char_code;

        glyphs.push_back(glyph_info);
        // fwrite(&glyph, sizeof(Glyph), 1, file);
        // fwrite(pk_glyph.raster, glyph.packet_length, 1, file);
      }
    }

    void read_glyphs() {
      std::cout << "Reading Glyphs...." << std::endl << std::flush;

      if (char_set == 0) {
        uint8_t ibmf_char_code = 0;
        for (int i = 0; i < 0x17; i++) read_glyph(i, ibmf_char_code++);
        read_glyph(0xBE, ibmf_char_code++); // ¿  0x17
        for (int i = 0x18; i < 0x20; i++) read_glyph(i, ibmf_char_code++);
        read_glyph(0xBD, ibmf_char_code++); // ¡  0x20
        for (int i = 0x21; i < 0x7F; i++) read_glyph(i, ibmf_char_code++);

        read_glyph(0x89, ibmf_char_code++); // Ľ  0x7F
        read_glyph(0x8A, ibmf_char_code++); // Ł  0x80
        read_glyph(0x8D, ibmf_char_code++); // Ŋ  0x81
        read_glyph(0x9C, ibmf_char_code++); // Ĳ  0x82
        read_glyph(0x9E, ibmf_char_code++); // đ  0x83
        read_glyph(0x9F, ibmf_char_code++); // §  0x84
        read_glyph(0xA4, ibmf_char_code++); // ď  0x85
        read_glyph(0xA9, ibmf_char_code++); // ľ  0x86
        read_glyph(0xAA, ibmf_char_code++); // ł  0x87
        read_glyph(0xAD, ibmf_char_code++); // ŋ  0x88
        read_glyph(0xB4, ibmf_char_code++); // ť  0x89
        read_glyph(0xBC, ibmf_char_code++); // ĳ  0x8A
        read_glyph(0xBF, ibmf_char_code++); // £  0x8B
        read_glyph(0xC6, ibmf_char_code++); // Æ  0x8C
        read_glyph(0xD0, ibmf_char_code++); // Ð  0x8D
        read_glyph(0xD7, ibmf_char_code++); // Œ  0x8E
        read_glyph(0xD8, ibmf_char_code++); // Ø  0x8F
        read_glyph(0xDE, ibmf_char_code++); // Þ  0x90
        read_glyph(0xDF, ibmf_char_code++); // SS 0x91
        read_glyph(0xE6, ibmf_char_code++); // æ  0x92
        read_glyph(0xF0, ibmf_char_code++); // ð  0x93
        read_glyph(0xF7, ibmf_char_code++); // œ  0x94
        read_glyph(0xF8, ibmf_char_code++); // ø  0x95
        read_glyph(0xFE, ibmf_char_code++); // þ  0x96
        read_glyph(0xFF, ibmf_char_code++); // ß  0x97
        read2_glyph(0xA2, ibmf_char_code++); // ¢  0x98
        read2_glyph(0xA4, ibmf_char_code++); // ¤  0x99
        read2_glyph(0xA5, ibmf_char_code++); // ¥  0x9A
        read2_glyph(0xA6, ibmf_char_code++); // ¦  0x9B
        read2_glyph(0xA9, ibmf_char_code++); // ©  0x9C
        read2_glyph(0xAA, ibmf_char_code++); // ª  0x9D
        read2_glyph(0xAC, ibmf_char_code++); // ¬  0x9E
        read2_glyph(0xAE, ibmf_char_code++); // ®  0x9F
        read2_glyph(0xB1, ibmf_char_code++); // ±  0xA0
        read2_glyph(0xB2, ibmf_char_code++); // ²  0xA1
        read2_glyph(0xB3, ibmf_char_code++); // ³  0xA2
        read2_glyph(0xB5, ibmf_char_code++); // µ  0xA3
        read2_glyph(0xB6, ibmf_char_code++); // ¶  0xA4
        read2_glyph(0xB7, ibmf_char_code++); // middle dot  0xA5
        read2_glyph(0xB9, ibmf_char_code++); // ¹  0xA6
        read2_glyph(0xBA, ibmf_char_code++); // º  0xA7
        read2_glyph(0xD6, ibmf_char_code++); // ×  0xA8
        read2_glyph(0xBC, ibmf_char_code++); // ¼  0xA9
        read2_glyph(0xBD, ibmf_char_code++); // ½  0xAA
        read2_glyph(0xBE, ibmf_char_code++); // ¾  0xAB
        read2_glyph(0xF6, ibmf_char_code++); // ÷  0xAC
        read2_glyph(0xBF, ibmf_char_code++); // Euro  0xAD
      }
      else {
        for (int i = tfm.get_first_glyph_code(); i <= tfm.get_last_glyph_code(); i++) {
          read_glyph(i, i);
        }
      }
    }

    int get_char_idx(uint8_t pk_char_code) {
      for (int idx = 0; idx < glyphs.size(); idx++) {
        if (glyphs[idx].old_char_code == pk_char_code) return idx;
      }
      return -1;
    }

    int get_new_lig_kern_idx(int last_idx, int old_lig_kern_idx) {
      for (int i = 0; i < last_idx; i++) {
        if (glyphs[i].old_lig_kern_idx == old_lig_kern_idx) return glyphs[i].new_lig_kern_idx;
      }
      return -1;
    }

    int 
    read_lig_kerns() {
      TFM::LigKernStep lks;

      lig_kerns.clear();
      lig_kerns.reserve(500);

      std::cout << "Reading and reshuffling Lig/Kern...." << std::endl << std::flush;

      for (int i = 0; i < glyphs.size(); i++) {
        int idx;
        if ((idx = glyphs[i].glyph.lig_kern_pgm_index) < 255) {
          glyphs[i].old_lig_kern_idx = glyphs[i].glyph.lig_kern_pgm_index;

          int new_idx;

          // Check if the same pgm has been used in the preceeding glyphs. If so,
          // point to that pgm again
          if ((new_idx = get_new_lig_kern_idx(i, glyphs[i].old_lig_kern_idx)) != -1) {
            glyphs[i].new_lig_kern_idx = new_idx;
            std::cout << "Pgm at " << new_idx << " is being used again!" << std::endl;
          }
          else {
            lks = tfm.get_lig_kern_step(idx);
            if (lks.skip.whole > 128) {
              idx = ((lks.op_code.d.displ_high << 8) + lks.remainder.displ_low);
              lks = tfm.get_lig_kern_step(idx);
            }
            std::cout << "OLD LKS idx: " << idx;
            bool first = true;
            do {
              int next_char_idx;
              int replacement_char_idx;

              // We get rid of:
              // 1. steps that relate of next-chars that were not retrieved
              // 2. steps that relate to replacement-chars that are not retrieved

              if (((next_char_idx = get_char_idx(lks.next_char)) != -1) &&
                  (lks.op_code.d.is_a_kern || 
                  ((replacement_char_idx = get_char_idx(lks.remainder.replacement_char)) != -1))) { 
              
                lks.next_char = next_char_idx;
                lks.remainder.replacement_char = replacement_char_idx;
                if (first) {
                  first = false;
                  glyphs[i].new_lig_kern_idx = lig_kerns.size();
                }
                lig_kerns.push_back(lks);
              }

              if (!lks.skip.s.stop)  {
                lks = tfm.get_lig_kern_step(++idx);
              }
            } while (!lks.skip.s.stop);

            if (first) {
              glyphs[i].new_lig_kern_idx = -1;
            }

            std::cout << "... New LKS idx: " << +glyphs[i].new_lig_kern_idx << std::endl;
          }
        }
        else {
          glyphs[i].new_lig_kern_idx = -1;
        }
      }

      std::set<int> overflow_list;

      for (auto & g : glyphs) {
        if (g.new_lig_kern_idx > 254) overflow_list.insert(g.new_lig_kern_idx);
      }

      std::cout << "Number of resulting Ligature/Kern entries: " 
                << lig_kerns.size() 
                << " Overflow Count: "
                << overflow_list.size()
                << std::endl;

      if (overflow_list.size() > 0) {

        // Build a unique list of all pgms start indexes.

        std::set<int> all_pgms;
        for (auto & g : glyphs) {
          if (g.new_lig_kern_idx >= 0) all_pgms.insert(g.new_lig_kern_idx);
        }

        // Put them in a vector such that we can access them through indices.

        std::vector<int> all;
        std::copy(all_pgms.begin(), all_pgms.end(), std::back_inserter(all));

        // Compute how many entries we need to add to the lig/kern vector to
        // redirect over the limiting 255 indexes, and where to add them.

        int space_required = overflow_list.size();
        int i = all.size() - (space_required + 1);
        
        while (true) {
          if ((all[i] + space_required) >= 255) {
            i -= 1;
            space_required += 1;
            overflow_list.insert(all[i]);
          }
          else break;
        }

        // Starting at index i, all items must go down for an amount of space_required
        // The corresponding indices in the glyphs table must be adjusted accordingly.

        int lig_kern_idx = all[i];

        for (auto idx = overflow_list.rbegin(); idx != overflow_list.rend(); idx++) {
          std::cout << *idx << " treatment: " << std::endl;
          TFM::LigKernStep lks;
          memset(&lks, 0, sizeof(TFM::LigKernStep));
          lks.skip.whole = 255;
          lks.op_code.d.displ_high = (*idx + space_required) >> 8;
          lks.remainder.displ_low = (*idx + space_required) & 0xFF;

          lig_kerns.insert(lig_kerns.begin() + i, lks);
          for (auto & g : glyphs) {
            if (g.new_lig_kern_idx == *idx) {
              std::cout << "Char Code " << +g.glyph.char_code << "With index in lig/kern " << +g.new_lig_kern_idx << " is modified for " << i << std::endl;
              g.new_lig_kern_idx = i;
            }
          }
          i++;
        }
        std::cout << std::endl;
      }
      return lig_kerns.size();
    }

    void write_everything() {

      // Header
      header.lig_kern_pgm_count = lig_kerns.size();
      fwrite(&header, sizeof(Header), 1, file);

      // Glyphs data
      for (auto & g : glyphs) {
        PKFont::Glyph pk_glyph;
        pk.get_glyph(g.old_char_code, pk_glyph, false);

        fwrite(&g.glyph, sizeof(Glyph), 1, file);
        fwrite(pk_glyph.raster, g.glyph.packet_length, 1, file);
      }

      // ligature / kerns struct

      for (auto & lks : lig_kerns) {
        fwrite(&lks, sizeof(TFM::LigKernStep), 1, file);
      }

      // Kerns

      for (int i = 0; i < header.kern_count; i++) {
        TFM::FIX kern = tfm.get_kern(i);
        TFM::FIX16 k = tfm.to_fix16(tfm.to_double(kern, 20) * factor, 6);

        fwrite(&k, sizeof(TFM::FIX16), 1, file);
      }
    }

  public:
    IBMFGener(FILE * file, char * dpi, TFM & tfm, TFM & tfm2, PKFont & pk, PKFont & pk2, uint8_t char_set) 
      : file(file), tfm(tfm), tfm2(tfm2), pk(pk), pk2(pk2), char_set(char_set) {
      read_header(atoi(dpi), char_set == 0 ? 0xAE : tfm.get_glyph_count());
      read_glyphs();
      int count;
      if ((count = read_lig_kerns()) < 255) {
        write_everything();
      }
      else {
        std::cout << "ABORTED!!! TOO MANY LIGATURE/KERN STRUCT!!!" << std::endl;
      }
    }
};

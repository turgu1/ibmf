#pragma once

#include <cstdio>
#include <vector>
#include <bitset>
#include <set>

#include "pk_font.hpp"
#include "tfm_v4.hpp"

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

    int last_idx_to_check;
    typedef uint16_t GlyphCode;
    typedef int16_t FIX14;

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
        uint16_t   lig_kern_step_count;
        uint8_t    max_height;
        uint8_t    filler;
      };

      struct GlyphMetric {
        uint8_t dyn_f:4;
        uint8_t first_is_black:1;
        uint8_t filler:3;
      };

      struct Glyph {
        GlyphCode   char_code;
        uint8_t     bitmap_width;
        uint8_t     bitmap_height;
        int8_t      horizontal_offset;
        int8_t      vertical_offset;
        uint16_t    packet_length;
        TFM::FIX16  advance;
        GlyphMetric glyph_metric;
        uint8_t     lig_kern_pgm_index; // = 255 if none
      };

      // Ligature and Kerning table
      
      struct Nxt {
        GlyphCode nextGlyphCode : 15;
        bool      stop          : 1;
      };

      union ReplDisp {
        struct  Repl {
          GlyphCode replGlyphCode : 15;
          bool      isAKern       : 1;
        } repl;
        struct Kern {
          FIX14 kerningValue : 14;
          bool  isAGoTo      : 1;
          bool  isAKern      : 1;
        } kern;
        struct GoTo {
          uint16_t displacement : 14;
          bool     isAGoTo      : 1;
          bool     isAKern      : 1;
        } goTo;
      };

      struct NewLigKernStep {
        Nxt      a;
        ReplDisp b;
      };      

    #pragma pack(pop)

    struct GlyphInfo {
      Glyph glyph;
      uint8_t * pixels;
      uint16_t old_char_code;
      uint8_t old_lig_kern_idx;
      int new_lig_kern_idx; // need to be larger for cases that goes beyond 255
    };

    std::vector<TFM::LigKernStep *> lig_kerns;
    std::vector<TFM::FIX16> kerns;
    std::vector<GlyphInfo *> glyphs;

    std::vector<NewLigKernStep *> new_lig_kerns;

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
      // std::cout << "Reading Header...." << std::endl << std::flush;

      header.dpi         = dpi;
      header.point_size  = trunc(tfm.to_double(tfm.get_design_size(), 20));
      header.line_height = trunc(((double)(header.dpi * header.point_size)) / 72.27 + 0.5);

      factor = tfm.to_double(tfm.get_design_size(), 20) * dpi / 72.27;

      header.x_height           = tfm.to_fix16(tfm.to_double(tfm.get_x_height(), 20) * factor, 6);
      header.em_size            = tfm.to_fix16(tfm.to_double(tfm.get_em_size(),  20) * factor, 6);
      header.space_size         = trunc(tfm.to_double(tfm.get_space_size(), 20) * factor);
      header.glyph_count        = glyph_count;
      header.lig_kern_step_count = tfm.get_lig_kern_step_count();
      header.slant_correction   = tfm.to_fix16(tfm.to_double(tfm.get_slant_correction(), 20), 6);
      header.descender_height   = tfm.to_fix16(tfm.to_double(tfm.get_max_depth(), 20) * factor, 6) >> 6;

      //fwrite(&header, sizeof(Header), 1, file);
    }

    void read_glyph(uint8_t pk_char_code, uint16_t ibmf_char_code) {
      GlyphInfo * glyph_info = new GlyphInfo;
      PKFont::Glyph pk_glyph;
      if (pk.get_glyph(pk_char_code, pk_glyph, false)) {
        glyph_info->glyph.char_code                   = ibmf_char_code;
        glyph_info->glyph.bitmap_width                = pk_glyph.bitmap_width;
        glyph_info->glyph.bitmap_height               = pk_glyph.bitmap_height;
        glyph_info->glyph.horizontal_offset           = pk_glyph.horizontal_offset;
        glyph_info->glyph.vertical_offset             = pk_glyph.vertical_offset;
        glyph_info->glyph.packet_length               = pk_glyph.raster_size;
        glyph_info->glyph.advance                     = tfm.to_fix16(tfm.to_double(tfm.get_advance(pk_char_code), 20) * factor, 6);
        glyph_info->glyph.glyph_metric.dyn_f          = pk_glyph.dyn_f;
        glyph_info->glyph.glyph_metric.first_is_black = pk_glyph.first_nibble_is_black ? 1 : 0;
        glyph_info->glyph.glyph_metric.filler         = 0;
        glyph_info->glyph.lig_kern_pgm_index          = tfm.get_lig_kern_pgm_index(pk_char_code);

        glyph_info->old_char_code    = pk_char_code;
        glyph_info->old_lig_kern_idx = glyph_info->glyph.lig_kern_pgm_index;
        
        glyph_info->pixels = new uint8_t[glyph_info->glyph.packet_length];
        memcpy(glyph_info->pixels, pk_glyph.raster, glyph_info->glyph.packet_length);
        glyphs.push_back(glyph_info);
        // fwrite(&glyph, sizeof(Glyph), 1, file);
        // fwrite(pk_glyph.raster, glyph.packet_length, 1, file);
      }
    }

    void read2_glyph(uint8_t pk_char_code, uint16_t ibmf_char_code) {
      GlyphInfo * glyph_info = new GlyphInfo;
      PKFont::Glyph pk_glyph;
      if (pk2.get_glyph(pk_char_code, pk_glyph, false)) {
        glyph_info->glyph.char_code                   = ibmf_char_code;
        glyph_info->glyph.bitmap_width                = pk_glyph.bitmap_width;
        glyph_info->glyph.bitmap_height               = pk_glyph.bitmap_height;
        glyph_info->glyph.horizontal_offset           = pk_glyph.horizontal_offset;
        glyph_info->glyph.vertical_offset             = pk_glyph.vertical_offset;
        glyph_info->glyph.packet_length               = pk_glyph.raster_size;
        glyph_info->glyph.advance                     = tfm2.to_fix16(tfm2.to_double(tfm2.get_advance(pk_char_code), 20) * factor, 6);
        glyph_info->glyph.glyph_metric.dyn_f          = pk_glyph.dyn_f;
        glyph_info->glyph.glyph_metric.first_is_black = pk_glyph.first_nibble_is_black ? 1 : 0;
        glyph_info->glyph.glyph_metric.filler         = 0;
        glyph_info->glyph.lig_kern_pgm_index          = 255;

        glyph_info->old_char_code = pk_char_code;
        glyph_info->old_lig_kern_idx = 255;

        glyph_info->pixels = new uint8_t[glyph_info->glyph.packet_length];
        memcpy(glyph_info->pixels, pk_glyph.raster, glyph_info->glyph.packet_length);
        glyphs.push_back(glyph_info);
        // fwrite(&glyph, sizeof(Glyph), 1, file);
        // fwrite(pk_glyph.raster, glyph.packet_length, 1, file);
      }
    }

    void read_glyphs() {
      // std::cout << "Reading Glyphs...." << std::endl << std::flush;

      if (char_set == 0) {
        uint16_t ibmf_char_code = 0;
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

        // For the  lig/kern table, this is the last glyph index to consider
        last_idx_to_check = 0x97;

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
        if (glyphs[idx]->old_char_code == pk_char_code) return idx;
      }
      return -1;
    }

    int get_new_lig_kern_idx(int last_idx, int old_lig_kern_idx) {
      for (int i = 0; i < last_idx; i++) {
        if (glyphs[i]->old_lig_kern_idx == old_lig_kern_idx) return glyphs[i]->new_lig_kern_idx;
      }
      return -1;
    }

    int 
    read_lig_kerns() 
    {
      TFM::LigKernStep * lks;

      lig_kerns.clear();
      lig_kerns.reserve(500);

      for (auto & g : glyphs) {
        g->new_lig_kern_idx = -1;
      }

      // std::cout << "Reading and reshuffling Lig/Kern...." << std::endl << std::flush;

      int glyph_idx = 0;
      bool found_it = false;
      // Look at targetted glyphs for pointers to the TFM lig/kern pgm array, retrieve
      // the steps into the lig_kern array
      for (auto & g : glyphs) {
        // std::cout << "Now doing glyph " << glyph_idx << " (0x" << std::hex << glyph_idx << std::dec << ")" << std::endl;

        if (glyph_idx == 102) {
          std::cout << "Found it!" << std::endl;
          found_it = true;
        }
        int tfm_idx;
        if ((tfm_idx = g->glyph.lig_kern_pgm_index) < 255) { // 255 means no lig/kern pgm
          //std::cout << tfm_idx << " a..." << std::endl;
          // save the old pointer for future use below
          g->old_lig_kern_idx = g->glyph.lig_kern_pgm_index;

          int new_idx;

          // Check if the same pgm has been used in the preceeding glyphs. If so,
          // point to that pgm again instead of poluting the new steps list
          if ((new_idx = get_new_lig_kern_idx(glyph_idx, g->old_lig_kern_idx)) != -1) {
            g->new_lig_kern_idx = new_idx;
          }
          else {
            // Was not used before, so we create one
            lks = new TFM::LigKernStep;
            *lks = tfm.get_lig_kern_step(tfm_idx);
            if (lks->skip.whole > 128) { // if > 128, this is a redirect
              tfm_idx = (((int)lks->op_code.d.displ_high << 8) + lks->remainder.displ_low);
              *lks = tfm.get_lig_kern_step(tfm_idx);
              //std::cout << tfm_idx << " b..." << std::endl;
            }

            bool first_to_be_saved = true;
            do {
next:
              // std::cout << "loop start" << std::endl;
              int next_char_idx;
              int replacement_char_idx;

              // We get rid of:
              // 1. steps that relate of next-chars that were not retrieved
              // 2. steps that relate to replacement-chars that are not retrieved

              bool kept = false;
              if (((next_char_idx = get_char_idx(lks->next_char)) != -1) &&
                  (lks->op_code.d.is_a_kern || 
                   ((replacement_char_idx = get_char_idx(lks->remainder.replacement_char)) != -1))) { 
              
                lks->next_char = next_char_idx;
                if (!lks->op_code.d.is_a_kern) lks->remainder.replacement_char = replacement_char_idx;

                if (first_to_be_saved) {
                  first_to_be_saved = false;
                  g->new_lig_kern_idx = lig_kerns.size();
                }

                //std::cout << "Old tfm idx " << tfm_idx << " Saved at new idx " << lig_kerns.size() << "..." << std::endl;
                lig_kerns.push_back(lks);
                kept = true;
              }

              if (!lks->skip.s.stop) {
                if (kept) lks = new TFM::LigKernStep; // If kept, create a new step, else reuse the old one
                *lks = tfm.get_lig_kern_step(++tfm_idx);
                //std::cout << tfm_idx << " c..." << " stop:" << (lks->skip.s.stop ? "YES" : "no") <<  std::endl;
                goto next;
              }
              else if (!kept && !first_to_be_saved) {
                lig_kerns[lig_kerns.size() - 1]->skip.s.stop = true;
                //std::cout << "Last mark as stop step... " << (lig_kerns.size() - 1) << std::endl;
              }

              if (tfm_idx >= tfm.get_lig_kern_step_count()) std::cout << "=============> GOING BEYOND THE END OF THE ARRAY!!! <=================" << tfm_idx << std::endl;

            } while (!lks->skip.s.stop);
          }
        }

        if (glyph_idx == last_idx_to_check) break;
        glyph_idx += 1;
      }

      std::set<int> overflow_list;

      for (auto & g : glyphs) {
        if (g->new_lig_kern_idx > 254) overflow_list.insert(g->new_lig_kern_idx);
      }

      // std::cout << "Number of resulting Ligature/Kern entries: " 
      //           << lig_kerns.size() 
      //           << " Overflow Count: "
      //           << overflow_list.size()
      //           << std::endl;

      if (overflow_list.size() > 0) {

        // Build a unique list of all pgms start indexes.

        std::set<int> all_pgms;
        for (auto & g : glyphs) {
          if (g->new_lig_kern_idx >= 0) all_pgms.insert(g->new_lig_kern_idx);
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
            space_required += 1;
            overflow_list.insert(all[i]);
            i -= 1;
          }
          else break;
        }

        overflow_list.insert(all[i]);

        // Starting at index all[i], all items must go down for an amount of space_required
        // The corresponding indices in the glyphs table must be adjusted accordingly.

        int first_idx = all[i];
        int new_lig_kern_idx = all[i];

        for (auto idx = overflow_list.rbegin(); idx != overflow_list.rend(); idx++) {
          // std::cout << *idx << " treatment: " << std::endl;
          TFM::LigKernStep * lks = new TFM::LigKernStep;
          memset(lks, 0, sizeof(TFM::LigKernStep));
          lks->skip.whole = 255;
          lks->op_code.d.displ_high = (*idx + space_required + 1) >> 8;
          lks->remainder.displ_low = (*idx + space_required + 1) & 0xFF;

          lig_kerns.insert(lig_kerns.begin() + new_lig_kern_idx, lks);
          for (auto g : glyphs) {
            if (g->new_lig_kern_idx == *idx) {
              // std::cout << "   Char Code " 
              //           << +g->glyph.char_code 
              //           << " with index in lig/kern " 
              //           << +g->new_lig_kern_idx 
              //           << " is modified for " 
              //           << new_lig_kern_idx 
              //           << std::endl;
              g->new_lig_kern_idx = -new_lig_kern_idx;
            }
          }
          new_lig_kern_idx++;
        }
      
        for (auto g : glyphs) {
          if (g->new_lig_kern_idx == -1) {
            g->glyph.lig_kern_pgm_index = 255;
          }
          else if (g->new_lig_kern_idx < 0) {
            g->glyph.lig_kern_pgm_index = -g->new_lig_kern_idx;
          }
          else {
            g->glyph.lig_kern_pgm_index = g->new_lig_kern_idx;
          }
        }
      }

      header.lig_kern_step_count = lig_kerns.size();
      return lig_kerns.size();
    }

    void 
    read_kerns() 
    {
      kerns.clear();

      for (int i = 0; i < tfm.get_kern_count(); i++) {
        TFM::FIX kern = tfm.get_kern(i);
        TFM::FIX16 k = tfm.to_fix16(tfm.to_double(kern, 20) * factor, 6);

        kerns.push_back(k);
      }
    }

    bool
    write_everything() {
      uint8_t max_height;
      for (auto g : glyphs) {
        if (max_height < g->glyph.vertical_offset) max_height = g->glyph.vertical_offset;
      }
      header.max_height = max_height;
      
      // Header
      header.lig_kern_step_count = lig_kerns.size();
      fwrite(&header, sizeof(Header), 1, file);

      // Glyphs data
      for (auto g : glyphs) {
        fwrite(&g->glyph, sizeof(Glyph), 1, file);
        if (g->pixels != nullptr) {
          fwrite(g->pixels, g->glyph.packet_length, 1, file);
        }
        else {
          return false;
        }
      }

      // ligature / kerns struct

      NewLigKernStep *nlks;

      for (auto lks : lig_kerns) {
        nlks = new NewLigKernStep;
        memset(nlks, 0, sizeof(NewLigKernStep));
        nlks->a.nextGlyphCode = lks->next_char;
        nlks->a.stop = lks->skip.s.stop;
        nlks->b.kern.isAKern = lks->op_code.op.is_a_kern;
        if (nlks->b.kern.isAKern) {
          int16_t idx = ((int16_t)(lks->op_code.d.displ_high << 8)) + lks->remainder.displ_low;
          nlks->b.kern.kerningValue = kerns[idx];
        }
        else {
          if (lks->skip.whole > 128) {
            nlks->b.goTo.isAGoTo = true;
            nlks->b.goTo.isAKern = true; // For a goTo, isAKern must also be true!!!
            nlks->b.goTo.displacement = ((int16_t)(lks->op_code.d.displ_high << 8)) + lks->remainder.displ_low;
          }
          else {
            nlks->b.repl.replGlyphCode = lks->remainder.replacement_char;
          }
        }
        fwrite(nlks, sizeof(NewLigKernStep), 1, file);
        new_lig_kerns.push_back(nlks);
      }

      // Kerns

      // for (auto kern : kerns) {
      //   fwrite(&kern, sizeof(TFM::FIX16), 1, file);
      // }

      return true;
    }

    void
    show() {
      int i = 0;

      std::cout << std::endl << "----------- Header: ---------- " << sizeof(Header) << std::endl;

      std::cout << "DPI: " << header.dpi
                << ", point size: "       << +header.point_size
                << ", line height: "      << +header.line_height
                << ", max height: "       << +header.max_height      
                << ", x height: "         << +((float) header.x_height   / 64.0)        
                << ", em size: "          << +((float) header.em_size    / 64.0)        
                << ", space size: "       << +((float) header.space_size / 64.0)  
                << ", glyph count: "      << +header.glyph_count        
                << ", lig kern count: "   << +header.lig_kern_step_count 
                << ", slant corr: "       << +header.slant_correction   
                << ", descender height: " << +header.descender_height
                << std::endl;

      std::cout << std::endl << "----------- Glyphs: ---------- " << sizeof(Glyph) << std::endl;
      for (auto & g : glyphs) {

        std::cout << "  [" << i << "]: "
                  << "char_code : "     << +g->glyph.char_code                   
                  << ", width: "        << +g->glyph.bitmap_width                
                  << ", height: "       << +g->glyph.bitmap_height               
                  << ", hoffset: "      << +g->glyph.horizontal_offset           
                  << ", voffset: "      << +g->glyph.vertical_offset             
                  << ", pkt_len: "      << +g->glyph.packet_length               
                  << ", advance: "      << +((float)g->glyph.advance / 64.0)                    
                  << ", dyn_f: "        << +g->glyph.glyph_metric.dyn_f          
                  << ", first_black: "  << +g->glyph.glyph_metric.first_is_black 
                  << ", Lig_kern_idx: " << +g->glyph.lig_kern_pgm_index  
                  << ", Old Index: "    << +g->old_lig_kern_idx        
                  << std::endl;
        i++;
      }

      i = 0;
      std::cout << std::endl << "----------- Ligature / Kern programs: ----------" << std::endl;
      for (auto entry : lig_kerns) {
        if (entry->skip.whole > 128) {
          std::cout << "  [" << i << "]:  "
                    << "Whole skip: " << +entry->skip.whole << ", "
                    << "Goto: " << +((entry->op_code.d.displ_high << 8) + entry->remainder.displ_low);
        }
        else {
          std::cout << "  [" << i << "]:  "
                    << "Whole skip: " << +entry->skip.whole << ", "
                    << "Stop: "       << (entry->skip.s.stop ? "Yes" : "No") << ", "
                    << "NxtChar: "    << +entry->next_char << ", "
                    << "IsKern: "     << (entry->op_code.d.is_a_kern ? "Yes" : "No") << ", "
                    << (entry->op_code.d.is_a_kern ? "Kern Idx: " : "Lig char: ")
                    << (entry->op_code.d.is_a_kern
                            ? +((entry->op_code.d.displ_high << 8) + entry->remainder.displ_low) 
                            : +entry->remainder.replacement_char)
                    << std::dec;
          if (!entry->op_code.d.is_a_kern) {
            std::cout << ", OpCodes: a:" 
                      << +entry->op_code.op.a_op
                      << ", b:"
                      << +entry->op_code.op.b_op
                      << ", c:"
                      << +entry->op_code.op.c_op;
          }
        }

        std::cout << std::endl;

        i++;       
      }

      i = 0;
      std::cout << std::endl << "----------- Kerns: ----------" << std::endl;

      for (auto kern : kerns) {
        std::cout << "  [" << i << "]:  "
                  << ((float) kern / 64.0)
                  << std::endl;

        i += 1;
      }

      i = 0;
      std::cout << std::endl << "----------- NEW Ligature / Kern programs: ---------- " << sizeof(NewLigKernStep) << std::endl;
      for (auto entry : new_lig_kerns) {
        if (entry->b.goTo.isAGoTo) {
          std::cout << "  [" << i << "]:  "
                    << "Goto: " << entry->b.goTo.displacement;
        }
        else {
          std::cout << "  [" << i << "]:  "
                    << "Stop: "       << (entry->a.stop ? "Yes" : "No") << ", "
                    << "NxtGlyphCode: "    << +entry->a.nextGlyphCode << ", "
                    << "IsKern: "     << (entry->b.kern.isAKern ? "Yes" : "No") << ", "
                    << (entry->b.kern.isAKern ? "Kern Value: " : "Lig char: ");
          if (entry->b.kern.isAKern) {
            std::cout << ((float)entry->b.kern.kerningValue / 64.0);
          }
          else {
            std::cout << entry->b.repl.replGlyphCode;
          }
        }
        std::cout << std::endl;
        i++;
      }
    }

  public:

    IBMFGener(FILE * file, char * dpi, TFM & tfm, TFM & tfm2, PKFont & pk, PKFont & pk2, uint8_t char_set) 
      : file(file), tfm(tfm), tfm2(tfm2), pk(pk), pk2(pk2), char_set(char_set) {
      read_header(atoi(dpi), char_set == 0 ? 0xAE : tfm.get_glyph_count());
      read_glyphs();
      //show();
      read_lig_kerns();
      read_kerns();
      if (!write_everything()) {
        std::cout << "PROBLEM GENERATING THE OUTPUT FILE!" << std::endl;
      }
      else {
        show();
      }
    }
};

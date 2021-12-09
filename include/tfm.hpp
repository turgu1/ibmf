#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cmath>

#include "sys/stat.h"

/**
 * @brief Access to a TFM file.
 * 
 * This is a class to allow acces to a TFM (TeX Font Metric) file generated through the METAFONT.
 * 
 * This class is to validate the comprehension of the author about the TFM file
 * format.
 */
class TFM
{
  public:
    typedef int32_t FIX;
    typedef int16_t FIX16;

    #pragma pack(push, 1)
    struct LigKernStep {
      uint8_t next_char_code;
      uint8_t char_code_or_index;  // Ligature: replacement char code, kern: index in kern array
      unsigned int     stop:1;
      unsigned int     tag:1;               // 0 = Ligature, 1 = Kern
    };
    #pragma pack(pop)

  private:
    bool initialized;
    bool memory_owner_is_the_instance;

    uint8_t  * memory;
    uint32_t   memory_length;

    uint8_t  * memory_ptr;
    uint8_t  * memory_end;

    uint16_t   font_dpi;

    #pragma pack(push, 1)
    struct TFMSizes {
      uint16_t lf; // Length of file in 32 bits words
      uint16_t lh; // Length of header data
      uint16_t bc; // First Character code in font
      uint16_t ec; // Last Character code in font
      uint16_t nw; // Number of words in width table
      uint16_t nh; // Number of words in height table
      uint16_t nd; // Number of words in depth table
      uint16_t ni; // Number of words in italic correction table
      uint16_t nl; // Number of words in ligature/kern program
      uint16_t nk; // Number of words in kern table
      uint16_t ne; // Number of words in extensible character table
      uint16_t np; // Number of font parameters 
    } sizes;

    
    union Other {
      uint32_t data;
      struct {
        unsigned int seven_bit_safe:1;
        unsigned int filler:23;
        unsigned int parc_face_type:8;
      } info;
    };

    struct Header {
      uint32_t checksum;
      FIX      design_size;
      char     char_coding_scheme[40];
      char     parc_font_identifier[20];
      Other    other;
    } header;

    struct FInfoEntry {
      unsigned int   width_index:8;
      unsigned int  height_index:4;
      unsigned int   depth_index:4;
      unsigned int char_ic_index:6;
      unsigned int     tag_field:2;
      unsigned int     remainder:8;
    } * font_info_entries;

    #pragma pack(pop)

    FIX * widths;
    FIX * heights;
    FIX * depths;
    FIX * char_ics;
    LigKernStep * lig_kern_steps;
    FIX * kerns;
    FIX * params;

    void
    extract_info_entry(int idx, uint32_t data) {
      font_info_entries[idx].width_index   = (data & 0xFF000000) >> 24;
      font_info_entries[idx].height_index  = (data & 0x00F00000) >> 20;
      font_info_entries[idx].depth_index   = (data & 0x000F0000) >> 16;
      font_info_entries[idx].char_ic_index = (data & 0x0000FC00) >> 10;
      font_info_entries[idx].tag_field     = (data & 0x00000300) >>  8;
      font_info_entries[idx].remainder     = (data & 0x000000FF);
    }

    bool
    getnextch(char & ch) {
      if (memory_ptr >= memory_end) return false;  
      ch = *memory_ptr++;
      return true;
    }

    bool
    getnext8(uint8_t & val) {
      if (memory_ptr >= memory_end) return false;  
      val = *memory_ptr++;
      return true;
    }

    bool
    getnext16(uint16_t & val) {
      if ((memory_ptr + 1) >= memory_end) return false;
      val  = *memory_ptr++ << 8;
      val |= *memory_ptr++;
      return true;
    }

    bool
    getnext24(uint32_t & val) {
      if ((memory_ptr + 2) >= memory_end) return false;
      val  = *memory_ptr++ << 16;
      val |= *memory_ptr++ << 8;
      val |= *memory_ptr++;
      return true;
    }

    bool
    getnext32(uint32_t & val) {
      if ((memory_ptr + 3) >= memory_end) return false;
      val  = *memory_ptr++ << 24;
      val |= *memory_ptr++ << 16;
      val |= *memory_ptr++ << 8;
      val |= *memory_ptr++;
      return true;
    }

    bool
    getnextfix(FIX & val) {
      if ((memory_ptr + 3) >= memory_end) return false;
      val  = *memory_ptr++ << 24;
      val |= *memory_ptr++ << 16;
      val |= *memory_ptr++ << 8;
      val |= *memory_ptr++;
      return true;
    }

    bool
    getnextstr(char * str, uint8_t length) {
      uint8_t l;
      if (!getnext8(l)) return false;
      if (l >= (length - 1)) return false;
      for (int i = 0; i < (length - 1); i++) {
        if (!getnextch(str[i])) return false;
      }
      str[l] = 0;

      return true;
    }

    bool
    load_tfm() {
      memory_ptr = memory;
      bool result = 
        getnext16(sizes.lf) &&
        getnext16(sizes.lh) &&
        getnext16(sizes.bc) &&
        getnext16(sizes.ec) &&
        getnext16(sizes.nw) &&
        getnext16(sizes.nh) &&
        getnext16(sizes.nd) &&
        getnext16(sizes.ni) &&
        getnext16(sizes.nl) &&
        getnext16(sizes.nk) &&
        getnext16(sizes.ne) &&
        getnext16(sizes.np);

      if (result) {
        memset(&header, 0, sizeof(Header));
        result =
          getnext32(header.checksum) && 
            ((sizes.lh <=1) || (
             getnextfix(header.design_size) &&
             ((sizes.lh <= 2) ||
               getnextstr(header.char_coding_scheme,   sizeof(header.char_coding_scheme)) &&
               ((sizes.lh <= 12) ||
                 getnextstr(header.parc_font_identifier, sizeof(header.parc_font_identifier)) &&
                 ((sizes.lh <= 17)) ||
                   getnext32(header.other.data)
             ))));
        if (result) {
          int32_t size = sizes.ec - sizes.bc + 1;
          if ((font_info_entries = new FInfoEntry[size]) == nullptr) return false;
          for (int i = 0; i < size; i++) {
            uint32_t data;
            if (!getnext32(data)) return false;
            extract_info_entry(i, data);
          }

          if ((widths = new FIX[sizes.nw]) == nullptr) return false;
          for (int i = 0; i < sizes.nw; i++) {
            if (!getnextfix(widths[i])) return false;
          }

          if ((heights = new FIX[sizes.nh]) == nullptr) return false;
          for (int i = 0; i < sizes.nh; i++) {
            if (!getnextfix(heights[i])) return false;
          }

          if ((depths = new FIX[sizes.nd]) == nullptr) return false;
          for (int i = 0; i < sizes.nd; i++) {
            if (!getnextfix(depths[i])) return false;
          }

          if ((char_ics = new FIX[sizes.ni]) == nullptr) return false;
          for (int i = 0; i < sizes.ni; i++) {
            if (!getnextfix(char_ics[i])) return false;
          }

          if ((lig_kern_steps = new LigKernStep[sizes.nl]) == nullptr) return false;
          for (int i = 0; i < sizes.nl; i++) {
            uint32_t data;
            if (!getnext32(data)) return false;
            lig_kern_steps[i].stop               = (data & 0x80000000) >> 31;
            lig_kern_steps[i].next_char_code     = (data & 0x00FF0000) >> 16;
            lig_kern_steps[i].tag                = (data & 0x00008000) >> 15;
            lig_kern_steps[i].char_code_or_index = (data & 0x000000FF);
          }

          if ((kerns = new FIX[sizes.nk]) == nullptr) return false;
          for (int i = 0; i < sizes.nk; i++) {
            if (!getnextfix(kerns[i])) return false;
          }

          for (int i = 0; i < sizes.ne; i++) {
            uint32_t dummy;
            if (!getnext32(dummy)) return false;
          }

          if ((params = new FIX[sizes.np]) == nullptr) return false;
          for (int i = 0; i < sizes.np; i++) {
            if (!getnextfix(params[i])) return false;
          }
        }
      }

      return result;
    }
  public:

    TFM(uint8_t * memory_font, uint32_t size) 
      : memory(memory_font), 
        memory_length(size) {
          memory_end = memory + memory_length;
          initialized = load_tfm();
          memory_owner_is_the_instance = false;
        }

    TFM(const std::string filename, uint16_t dpi) {
      struct stat file_stat;
      initialized = false;
      memory = nullptr;
      font_dpi    = dpi;
      if (stat(filename.c_str(), &file_stat) != -1) {
        memory = new uint8_t[memory_length = file_stat.st_size];
        memory_end = (memory == nullptr) ? nullptr : memory + memory_length;
        memory_owner_is_the_instance = true;
        if (memory != nullptr) {
          FILE * file = fopen(filename.c_str(), "rb");
          if (fread(memory, memory_length, 1, file) == 1) {
            initialized = load_tfm();
          }
          else {
            std::cerr << "Unable to read file " << filename.c_str() << std::endl;
          }
          fclose(file);
        }
      }
      else {
        std::cerr << "Unable to stat file " << filename.c_str() << std::endl;
      }
    }

   ~TFM() {
      if (memory_owner_is_the_instance && (memory != nullptr)) {
        delete [] memory;
        memory = nullptr;
      }
    }

    inline bool            is_initialized() { return initialized;                              }
    inline FIX            get_design_size() { return header.design_size;                       }
    inline FIX             get_space_size() { return params[1];                                }
    inline FIX               get_x_height() { return params[4];                                }
    inline FIX                get_em_size() { return params[5];                                }
    inline uint16_t       get_glyph_count() { return sizes.ec - sizes.bc + 1;                  }
    inline uint8_t get_lig_kern_pgm_count() { return sizes.nl;                                 }
    inline uint8_t         get_kern_count() { return sizes.nk;                                 }
    inline uint8_t   get_first_glyph_code() { return sizes.bc;                                 }
    inline uint8_t    get_last_glyph_code() { return sizes.ec;                                 }
    inline FIX       get_advance(uint8_t i) { return widths[font_info_entries[i - sizes.bc].width_index]; }
    inline uint8_t   get_lig_kern_pgm_index(uint8_t i) { return (font_info_entries[i - sizes.bc].tag_field == 1) ? font_info_entries[i - sizes.bc].remainder : 255; }
    inline LigKernStep    get_lig_kern_step(uint8_t i) { return lig_kern_steps[i]; }
    inline FIX          get_kern(uint8_t i) { return kerns[i]; }
    inline FIX       get_slant_correction() { return params[0]; }
    
    inline FIX get_max_depth() {
      FIX max = 0;
      for (int i = 0; i < sizes.nd; i++) {
        if (depths[i] > max) max = depths[i];
      }
      return max;
    }

    double 
    to_double(FIX input, uint8_t fractional_bits) {
      return ((double)input / (double)(1 << fractional_bits));
    }

    double 
    to_double(FIX16 input, uint8_t fractional_bits) {
      return ((double)input / (double)(1 << fractional_bits));
    }

    FIX16
    to_fix16(double val, uint8_t fractional_bits) {
      return trunc(val * (1 << fractional_bits));
    }

    std::string
    to_string(FIX val) {
      std::stringstream ss; 
      ss << to_double(val, 20);
      return ss.str();
    }

    std::string
    to_string(FIX16 val) {
      std::stringstream ss; 
      ss << to_double(val, 6);
      return ss.str();
    }

    void
    show() {
      std::cout << "Sizes:"                                                        << std::endl
                << "                Length of file in 32 bits words: " << sizes.lf << std::endl
                << "                          Length of header data: " << sizes.lh << std::endl
                << "                   First Character code in font: " << sizes.bc << std::endl
                << "                    Last Character code in font: " << sizes.ec << std::endl
                << "                 Number of words in width table: " << sizes.nw << std::endl
                << "                Number of words in height table: " << sizes.nh << std::endl
                << "                 Number of words in depth table: " << sizes.nd << std::endl
                << "     Number of words in italic correction table: " << sizes.ni << std::endl
                << "       Number of words in ligature/kern program: " << sizes.nl << std::endl
                << "                  Number of words in kern table: " << sizes.nk << std::endl
                << "  Number of words in extensible character table: " << sizes.ne << std::endl
                << "                     Number of font parameters : " << sizes.np << std::endl 
                                                                                   << std::endl;
      std::cout << "Header:" << std::endl 
                << "              Checksum: " << header.checksum                   << std::endl
                << "           Design Size: " << to_string(header.design_size) << std::endl
                << "    Char Coding Scheme: " << header.char_coding_scheme         << std::endl
                << "  Parc Font Identifier: " << header.parc_font_identifier       << std::endl<< std::endl;

      double factor = to_double(header.design_size, 20) * font_dpi / 72.27;

      for (int i = 0; i < (sizes.ec - sizes.bc + 1); i++) {
        std::cout << "Char Code: " << (i + sizes.bc) 
                  << " [" 
                  << to_string(widths[font_info_entries[i].width_index])
                  << ", "
                  << to_string(heights[font_info_entries[i].height_index])
                  << "] "
                  << to_string(depths[font_info_entries[i].depth_index])
                  << " --> ["
                  << to_double(widths[font_info_entries[i].width_index], 20) * factor
                  << ", "
                  << to_double(heights[font_info_entries[i].height_index], 20) * factor
                  << "] "
                  << to_double(depths[font_info_entries[i].depth_index], 20) * factor
                  << " " << ((font_info_entries[i].tag_field == 1) ? +font_info_entries[i].remainder : -1)
                  << std::endl;
      }

      std::cout << std::endl << "Ligature / Kern programs:" << std::endl;
      for (int i = 0; i < sizes.nl; i++) {
        std::cout << "  [" << i << "]:  "
                  << "Stop: "  << (lig_kern_steps[i].stop ? '1' : '0') << "  "
                  << "NxtChar: " << lig_kern_steps[i].next_char_code << "  "
                  << "Tag: "     << +lig_kern_steps[i].tag << "  "
                  << (lig_kern_steps[i].tag == 1 ? "Kern Idx: " : "Ligature char: 0x")
                  << (lig_kern_steps[i].tag == 1 ?  std::dec    :  std::hex          )
                  << +lig_kern_steps[i].char_code_or_index << std::endl;
      }
      
      std::cout << std::endl << "Kerns:" << std::endl;
      for (int i = 0; i < sizes.nk; i++) {
        std::cout << "  [" << i << "]: " 
                  << to_double(kerns[i], 20) 
                  << " (" << (to_double(kerns[i], 20) * factor) << ")"
                  << std::endl; 
      }

      std::cout << std::endl << "Params:" << std::endl;
      std::cout << "  Slant: "     << to_double(params[0], 20) << " (" << to_fix16(to_double(params[0], 20) * factor, 6) << ")" << std::endl
                << "  Space: "     << to_double(params[1], 20) << " (" << to_fix16(to_double(params[1], 20) * factor, 6) << ")" << std::endl
                << "  SpStretch: " << to_double(params[2], 20) << " (" << to_fix16(to_double(params[2], 20) * factor, 6) << ")" << std::endl
                << "  SpShrink: "  << to_double(params[3], 20) << " (" << to_fix16(to_double(params[3], 20) * factor, 6) << ")" << std::endl
                << "  xHeight: "   << to_double(params[4], 20) << " (" << to_fix16(to_double(params[4], 20) * factor, 6) << ")" << std::endl
                << "  Quad: "      << to_double(params[5], 20) << " (" << to_fix16(to_double(params[5], 20) * factor, 6) << ")" << std::endl
                << "  Extra: "     << to_double(params[6], 20) << " (" << to_fix16(to_double(params[6], 20) * factor, 6) << ")" << std::endl;

      std::cout << std::endl << "static constexpr FIX16 advances[] = {";
      for (int i = 0; i < (sizes.ec - sizes.bc + 1); i++) {
        if ((i % 10) == 0) std::cout << std::endl << "  /* " << i << " */  ";
        std::cout << to_fix16(to_double(widths[font_info_entries[i].width_index], 20) * factor, 6) << ", ";
      }
      std::cout << std::endl << "};" << std::endl << "static constexpr FIX16 depths[] = {";
      for (int i = 0; i < (sizes.ec - sizes.bc + 1); i++) {
        if ((i % 10) == 0) std::cout << std::endl << "  /* " << i << " */  ";
        std::cout << to_fix16(to_double(depths[font_info_entries[i].depth_index], 20) * factor, 6) << ", ";
      }
      std::cout << std::endl << "};" << std::endl;

      std::cout << std::endl << "static constexpr LigKernStep lig_kerns[] = {" << std::endl;
      for (int i = 0; i < sizes.nl; i++) {
        std::cout << "/* " << i << " */  { "
                  << ".next_char_code = 0x" << std::hex << +lig_kern_steps[i].next_char_code << ", ";
        if (lig_kern_steps[i].tag == 1) {
          std::cout << ".u = { .kern_idx = " 
                    << std::dec 
                    << +lig_kern_steps[i].char_code_or_index
                    << " }, ";
        }
        else {
          std::cout << ".u = { .char_code = 0x" 
                    << std::hex 
                    << +lig_kern_steps[i].char_code_or_index 
                    << " }, ";
        }
        std::cout << ".stop = "         << std::dec << (lig_kern_steps[i].stop ? '1' : '0') << ", "
                  << ".tag = "          << std::dec << +lig_kern_steps[i].tag << " }," << std::endl;
      }
      std::cout << "};"<< std::endl;

      std::cout << std::endl << "static constexpr FIX16 kerns[] = {";
      for (int i = 0; i < sizes.nk; i++) {
        if ((i % 10) == 0) std::cout << std::endl << "  /* " << i << " */  ";
        std::cout << to_fix16(to_double(kerns[i], 20) * factor, 6)
                  << ", ";
      };
      std::cout << std::endl << "};" << std::endl;

      std::cout << std::endl << "static constexpr int16_t lig_kern_indexes[] = {";
      for (int i = 0; i < (sizes.ec - sizes.bc + 1); i++) {
        if ((i % 20) == 0) std::cout << std::endl << "  /* " << i << " */  ";
        std::cout << ((font_info_entries[i].tag_field == 1) ? +font_info_entries[i].remainder : -1)
                  << ", ";
      }
      std::cout << std::endl << "};" << std::endl;

    }
};
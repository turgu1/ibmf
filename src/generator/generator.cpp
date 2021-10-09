#include "pk_font.hpp"
#include "tfm.hpp"
#include "ibmf_gener.hpp"

constexpr uint8_t IBMF_VERSION = 1;

constexpr int TEX_FONT_DIR  = 1;
constexpr int IBMF_PREFIX   = 2;
constexpr int DPI           = 3;
constexpr int CHAR_SET      = 4;
constexpr int TEX_PREFIX    = 5;
constexpr int FIRST_POINT   = 6;

constexpr int MIN_ARG_COUNT = 7;

int 
main(int argc, char **argv)
{
  if (argc < MIN_ARG_COUNT) {
    std::cerr << "ERROR! Usage: " << argv[0] << "<tex_fonts_dir> <IBMF_font_prefix> <dpi> <char_Set> <tex_font_prefix> <point_size>..." << std::endl;
    return 1;
  }

  FILE * file;
  std::string out_filename = argv[IBMF_PREFIX];
  out_filename.append("_").append(argv[DPI]).append(".ibmf");

  if ((file = fopen(out_filename.c_str(), "wb")) != nullptr) {

    std::cout << "Creating file " << out_filename << " ..." << std::endl;

    uint16_t offset   = 0;
    uint8_t  count    = argc - FIRST_POINT;

    #pragma pack(push, 1)
    struct {
      uint8_t   version:5;
      uint8_t  char_set:3;
    } bits;
    #pragma pack(pop)

    bits.version  = IBMF_VERSION;
    bits.char_set = atoi(argv[CHAR_SET]);

    fwrite("IBMF",    4, 1, file);
    fwrite(&count,    1, 1, file);
    fwrite(&bits,     1, 1, file);

    for (int i = FIRST_POINT; i < argc; i++) {
      fwrite(&offset, 2, 1, file);
    }

    for (int i = FIRST_POINT; i < argc; i++) {
      uint8_t size = atoi(argv[i]);
      fwrite(&size, 1, 1, file);
    }

    uint8_t filler = 0;
    if (count & 1) fwrite(&filler, 1, 1, file);

    for (int i = FIRST_POINT; i < argc; i++) {
      if (ftell(file) >= 65536) {
        std::cerr << "ERROR! Output file is too large (max 64K). Aborting!" << std::endl;
        fclose(file);
        remove(out_filename.c_str());
        return 1;
      }
      offset = ftell(file);
      fseek(file, (i - FIRST_POINT) * 2 + 6, SEEK_SET);
      fwrite(&offset, 2, 1, file);
      fseek(file, 0, SEEK_END);

      std::string filename = argv[TEX_FONT_DIR];
      filename.append("/")
              .append(argv[TEX_PREFIX])
              .append(argv[i])
              .append(".")
              .append(argv[DPI])
              .append("pk");

      PKFont pk_font(filename);

      filename = argv[TEX_FONT_DIR];
      filename.append("/")
              .append(argv[TEX_PREFIX])
              .append(argv[i])
              .append(".tfm");

      TFM tfm(filename, atoi(argv[DPI]));
 
      if (pk_font.is_initialized() && tfm.is_initialized()) {
        IBMFGener ibmf_gener(file, argv[DPI], tfm, pk_font);
      }
      else {
        std::cerr << "ERROR! TeX font reading error. Aborting!" << std::endl;
        fclose(file);
        remove(out_filename.c_str());
        return 1;
      }
    }

    fclose(file);
  }
  else {
    std::cerr << "ERROR! Unable to create file " << out_filename << ". Aborting!" << std::endl;
    return 1;
  }

  std::cout << "Completed!" << std::endl;

  return 0;
}


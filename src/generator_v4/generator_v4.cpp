#include "pk_font.hpp"
#include "tfm_v4.hpp"
#include "ibmf_gener_v4.hpp"

constexpr uint8_t IBMF_VERSION = 4;

constexpr int TEX_FONT_DIR  = 1;
constexpr int IBMF_PREFIX   = 2;
constexpr int DPI           = 3;
constexpr int CHAR_SET      = 4;
constexpr int TEX_PREFIX    = 5;
constexpr int TEX2_PREFIX   = 6;
constexpr int FIRST_POINT   = 7;

constexpr int MIN_ARG_COUNT = 8;

int 
main(int argc, char **argv)
{
  if (argc < MIN_ARG_COUNT) {
    std::cerr << "ERROR! Usage: " << argv[0] << "<tex_fonts_dir> <IBMF_font_prefix> <dpi> <char_Set> <tex_font_prefix> <tex_companion_prefix> <point_size>..." << std::endl;
    return 1;
  }

  FILE * file;
  std::string out_filename = argv[IBMF_PREFIX];
  out_filename.append("_").append(argv[DPI]).append(".ibmf");

  if ((file = fopen(out_filename.c_str(), "wb")) != nullptr) {

    std::cout << "Creating file " << out_filename << " ..." << std::endl << std::flush;

    uint32_t offset   = 0;
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
      fwrite(&offset, 4, 1, file);
    }

    for (int i = FIRST_POINT; i < argc; i++) {
      uint8_t size = atoi(argv[i]);
      fwrite(&size, 1, 1, file);
    }

    uint8_t filler = 0;
    if (count & 1) fwrite(&filler, 1, 1, file);

    for (int i = FIRST_POINT; i < argc; i++) {
      offset = ftell(file);
      fseek(file, (i - FIRST_POINT) * 4 + 6, SEEK_SET);
      fwrite(&offset, 4, 1, file);
      fseek(file, 0, SEEK_END);

      std::string filename = argv[TEX_FONT_DIR];
      filename.append("/")
              .append(argv[TEX_PREFIX])
              .append(argv[i])
              .append(".")
              .append(argv[DPI])
              .append("pk");

      PKFont pk(filename);

      filename = argv[TEX_FONT_DIR];
      filename.append("/")
              .append(argv[TEX_PREFIX])
              .append(argv[i])
              .append(".tfm");

      TFM tfm(filename, atoi(argv[DPI]));
 
      filename = argv[TEX_FONT_DIR];
      filename.append("/")
              .append(argv[TEX2_PREFIX])
              .append(argv[i])
              .append(".")
              .append(argv[DPI])
              .append("pk");

      PKFont pk2(filename);

      filename = argv[TEX_FONT_DIR];
      filename.append("/")
              .append(argv[TEX2_PREFIX])
              .append(argv[i])
              .append(".tfm");

      TFM tfm2(filename, atoi(argv[DPI]));

      if (pk.is_initialized() && tfm.is_initialized() && pk2.is_initialized() && tfm2.is_initialized()) {
        IBMFGener ibmf_gener(file, argv[DPI], tfm, tfm2, pk, pk2, atoi(argv[CHAR_SET]));
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


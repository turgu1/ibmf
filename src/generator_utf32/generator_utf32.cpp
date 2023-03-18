
// "args": [
//     ".", "EC-Regular", "75", "0", "ecrm", "tcrm", "12"
// ]

#include "ibmf_gener_utf32.hpp"
#include "pk_font.hpp"
#include "tfm_v4.hpp"

constexpr uint8_t IBMF_VERSION = 4;

constexpr int TEX_FONT_DIR = 1;
constexpr int IBMF_PREFIX  = 2;
constexpr int DPI          = 3;
constexpr int CHAR_SET     = 4;
constexpr int TEX_PREFIX   = 5;
constexpr int TEX2_PREFIX  = 6;
constexpr int FIRST_POINT  = 7;

constexpr int MIN_ARG_COUNT = 8;

typedef uint16_t GlyphCode;

#pragma pack(push, 1)

struct Plane {
  uint16_t  codePointBundlesIdx; // Index of the plane in the CodePointBundles table
  uint16_t  entriesCount;        // The number of entries in the CodePointBungles table
  GlyphCode firstGlyphCode;      // glyphCode corresponding to the first codePoint in the bundles
};

struct CodePointBundle {
  char16_t firstCodePoint; // The first UTF16 codePoint of the bundle
  char16_t
      endCodePoint; // Codepoint corresponding to the one after the last codePoint of that bundle
};

typedef Plane Planes[4];

#pragma pack(pop)

typedef CodePointBundle (*CodePointBundlesPtr)[];
typedef Planes *PlanesPtr;

auto main(int argc, char **argv) -> int {
  if (argc < MIN_ARG_COUNT) {
    std::cerr << "ERROR! Usage: " << argv[0]
              << "<tex_fonts_dir> <IBMF_font_prefix> <dpi> <char_Set> <tex_font_prefix> "
                 "<tex_companion_prefix> <point_size>..."
              << std::endl;
    return 1;
  }

  FILE       *file;
  std::string outFilename = argv[IBMF_PREFIX];
  outFilename.append("_").append(argv[DPI]).append(".ibmf");

  if ((file = fopen(outFilename.c_str(), "wb")) != nullptr) {

    std::cout << "Creating file " << outFilename << " ..." << std::endl << std::flush;

    uint32_t offset = 0;
    uint8_t  count  = argc - FIRST_POINT;

#pragma pack(push, 1)
    struct {
      uint8_t version : 5;
      uint8_t charSet : 3;
    } bits;
#pragma pack(pop)

    bits.version = IBMF_VERSION;
    bits.charSet = 1;

    fwrite("IBMF", 4, 1, file);
    fwrite(&count, 1, 1, file);
    fwrite(&bits, 1, 1, file);

    for (int i = FIRST_POINT; i < argc; i++) {
      uint8_t size = atoi(argv[i]);
      fwrite(&size, 1, 1, file);
    }

    uint8_t filler = 0;
    int     offst  = count + 6;
    while (offst & 3) {
      fwrite(&filler, 1, 1, file);
      offst++;
    }

    long facesIdxLoc = ftell(file);

    for (int i = FIRST_POINT; i < argc; i++) { fwrite(&offset, 4, 1, file); }

    // Write simple UTF32 table

    Planes planes;
    planes[0] = {.codePointBundlesIdx = 0, .entriesCount = 1, .firstGlyphCode = 0};
    planes[1] = {.codePointBundlesIdx = 1, .entriesCount = 0, .firstGlyphCode = 0x7F - 0x21};
    planes[2] = {.codePointBundlesIdx = 1, .entriesCount = 0, .firstGlyphCode = 0x7F - 0x21};
    planes[3] = {.codePointBundlesIdx = 1, .entriesCount = 0, .firstGlyphCode = 0x7F - 0x21};

    fwrite(&planes, sizeof(Planes), 1, file);

    CodePointBundle bundle = {.firstCodePoint = static_cast<char16_t>(0x21),
                              .endCodePoint   = static_cast<char16_t>(0x7F)};

    fwrite(&bundle, sizeof(CodePointBundle), 1, file);

    for (int i = FIRST_POINT; i < argc; i++) {
      offset = ftell(file);
      fseek(file, (i - FIRST_POINT) * 4 + facesIdxLoc, SEEK_SET);
      fwrite(&offset, 4, 1, file);
      fseek(file, offset, SEEK_SET);

      std::string filename = argv[TEX_FONT_DIR];
      filename.append("/")
          .append(argv[TEX_PREFIX])
          .append(argv[i])
          .append(".")
          .append(argv[DPI])
          .append("pk");

      PKFont pk(filename);

      filename = argv[TEX_FONT_DIR];
      filename.append("/").append(argv[TEX_PREFIX]).append(argv[i]).append(".tfm");

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
      filename.append("/").append(argv[TEX2_PREFIX]).append(argv[i]).append(".tfm");

      TFM tfm2(filename, atoi(argv[DPI]));

      if (pk.is_initialized() && tfm.is_initialized() && pk2.is_initialized() &&
          tfm2.is_initialized()) {
        IBMFGener ibmf_gener(file, argv[DPI], tfm, tfm2, pk, pk2, atoi(argv[CHAR_SET]));
      } else {
        std::cerr << "ERROR! TeX font reading error. Aborting!" << std::endl;
        fclose(file);
        remove(outFilename.c_str());
        return 1;
      }
    }

    fclose(file);
  } else {
    std::cerr << "ERROR! Unable to create file " << outFilename << ". Aborting!" << std::endl;
    return 1;
  }

  std::cout << "Completed!" << std::endl;

  return 0;
}

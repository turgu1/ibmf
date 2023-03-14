#include <cstdarg>
#include <iostream>

#define TEST_IBMF 1

#ifdef TEST_IBMF

  #include "UI/Fonts/Fonts.h"
  #include "UI/Fonts/IBMFDriver/IBMFDefs.hpp"

Font *font = (Font *)&FONT_ECSANS_REGULAR_75BPI14PT;

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: %s <string>", argv[0]);
    return 1;
  }
  IBMFFont          *fnt  = static_cast<IBMFFont *>(font);
  const IBMFFaceLow *face = fnt->getFace();
  Bitmap             canvas;
  canvas.dim = Dim(90, 40);
  if (default_resolution == PixelResolution::ONE_BIT) {
    canvas.pixels = new uint8_t[((canvas.dim.width + 7) >> 3) * canvas.dim.height];
    memset(canvas.pixels, WHITE_EIGHT_BITS, ((canvas.dim.width + 7) >> 3) * canvas.dim.height);
  } else {
    canvas.pixels = new uint8_t[canvas.dim.width * canvas.dim.height];
    memset(canvas.pixels, WHITE_EIGHT_BITS, (canvas.dim.width * canvas.dim.height));
  }
  fnt->drawSingleLineOfText(canvas, Pos(010, 25), argv[1]);
  face->showBitmap(canvas);

  delete [] canvas.pixels;
}

#endif

#ifdef TEST_UTF8

  #include "UI/Fonts/IBMFDriver/Utf8Iterator.hpp"

int main() {
  const std::string testStr = "Alélo";

  int i;
  for (Utf8Iterator iter = testStr.begin(); iter != testStr.end(); iter++) {
    std::cout << "[" << i++ << "] " << *iter << std::endl;
  }

  return 0;
}

#endif

char *formatStr(const std::string &format, ...) {
  static char buffer[256];
  va_list     args;
  va_start(args, format);
  std::vsnprintf(buffer, 256, format.c_str(), args);

  return buffer;
}
#include <cstdarg>
#include <iostream>

#define TEST_IBMF 1

#ifdef TEST_IBMF

#include "UI/Fonts/Fonts.hpp"
#include "UI/Fonts/IBMFDriver/IBMFDefs.hpp"

Font *font = static_cast<Font *>(&fontTahoma75BPI12PT);

auto main(int argc, char **argv) -> int {
    if (argc != 2) {
        printf("Usage: %s <string>", argv[0]);
        return 1;
    }
    auto *fnt = static_cast<IBMFFont *>(font);
    const IBMFFaceLow *face = fnt->getFace();
    ibmf_defs::Bitmap canvas;
    canvas.dim = ibmf_defs::Dim(90, 40);
    if (DEFAULT_RESOLUTION == PixelResolution::ONE_BIT) {
        canvas.pixels = new uint8_t[static_cast<int32_t>(
            static_cast<int16_t>((canvas.dim.width + 7) >> 3) * canvas.dim.height)];
        memset(canvas.pixels, WHITE_EIGHT_BITS,
               static_cast<int32_t>(static_cast<int16_t>((canvas.dim.width + 7) >> 3) *
                                    canvas.dim.height));
    } else {
        canvas.pixels = new uint8_t[static_cast<int32_t>(canvas.dim.width * canvas.dim.height)];
        memset(canvas.pixels, WHITE_EIGHT_BITS,
               static_cast<int32_t>(canvas.dim.width * canvas.dim.height));
    }
    fnt->drawSingleLineOfText(canvas, ibmf_defs::Pos(010, 25), argv[1]);
    face->showBitmap(canvas);

    delete[] canvas.pixels;
}

#endif

#ifdef TEST_UTF8

#include "UI/Fonts/IBMFDriver/UTF8Iterator.hpp"

int main() {
    const std::string testStr = "Alélo";

    int i;
    for (Utf8Iterator iter = testStr.begin(); iter != testStr.end(); iter++) {
        std::cout << "[" << i++ << "] " << *iter << std::endl;
    }

    return 0;
}

#endif

auto formatStr(const std::string &format, ...) -> char * {
    static char buffer[256];
    va_list args;
    va_start(args, format);
    if (std::vsnprintf(buffer, 256, format.c_str(), args) > 256) {
        std::cout << "Internal error!!" << std::endl;
    }
    va_end(args);
    return buffer;
}
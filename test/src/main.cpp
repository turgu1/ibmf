#include <iostream>

#include "UI/Fonts/Fonts.h"
#include "UI/Fonts/IBMFDriver/IBMFDefs.hpp"

Font * font = (Font *)&FONT_EC_REGULAR_75BPI14PT;

char * formatStr(const std::string &format, ...)
{
    static char buffer[256];
    va_list args;
    va_start(args, format);
    std::vsnprintf(buffer, 256, format.c_str(), args);

    return buffer;
}


int main(int argc, char **argv) {
    IBMFFont * fnt = static_cast<IBMFFont *>(font);
    const IBMFFaceLow * face = fnt->getFace();
    Bitmap canvas;
    canvas.dim = Dim(80, 30);
    if (default_resolution == PixelResolution::ONE_BIT) {
        canvas.pixels = new uint8_t[((canvas.dim.width + 7) >> 3) * canvas.dim.height];
        memset(canvas.pixels, 0x00, ((canvas.dim.width + 7) >> 3) * canvas.dim.height);
    }
    else {
        canvas.pixels = new uint8_t[canvas.dim.width * canvas.dim.height];
        memset(canvas.pixels, WHITE_EIGHT_BITS, (canvas.dim.width * canvas.dim.height));
    }
    fnt->drawSingleLineOfText(canvas, Pos(010,15), argv[1]);
    face->showBitmap(canvas);
}
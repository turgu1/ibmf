#include <iostream>

#include "UI/Fonts/Fonts.h"
#include "UI/Fonts/IBMFDriver/IBMFDefs.hpp"
Font * font = (Font *)&FONT_EC_REGULAR_75BPI17PT;

char * formatStr(const std::string &format, ...)
{
    static char buffer[256];
    va_list args;
    va_start(args, format);
    std::vsnprintf(buffer, 256, format.c_str(), args);

    return buffer;
}


int main() {
    std::cout << "allo!" << std::endl;

    Bitmap canvas;
    canvas.dim = Dim(256, 256);
    canvas.pixels = new uint8_t[((canvas.dim.width + 7) >> 3) * canvas.dim.height];

    static_cast<IBMFFont *>(font)->drawSingleLineOfText(canvas, Pos(010,15), "ABCDEFGH");
}
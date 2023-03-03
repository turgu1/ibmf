#pragma once

enum FontType { GFX, IBMF };

class Font {
private:
    FontType type;

public:
    inline bool is_a(FontType font_type) const { return font_type == type; };
    virtual int yAdvance() const = 0;

protected:
    Font(FontType font_type) : type(font_type) {}
};

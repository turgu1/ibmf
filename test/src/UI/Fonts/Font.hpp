#pragma once

enum FontType { GFX, IBMF };

class Font {
private:
    FontType type_;

public:
    [[nodiscard]] inline auto isA(FontType fontType) const -> bool { return fontType == type_; }
    [[nodiscard]] virtual auto yAdvance() const -> int = 0;

protected:
    Font(FontType fontType) : type_(fontType) {}
};

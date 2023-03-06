#pragma once

#include <cstring>

#include "IBMFDefs.hpp"
#include "RLEExtractor.hpp"

using namespace IBMFDefs;

/**
 * @brief Access to a IBMF face.
 *
 * This is a low-level class to allow acces to a IBMF faces generated through METAFONT
 *
 */
class IBMFFaceLow {

private:
    static constexpr char const *TAG = "IBMFFaceLow";

    bool initialized;
    uint8_t charSet;
    PixelResolution resolution;

    FaceHeaderPtr faceHeader;
    RLEBitmap bitmaps[MAX_GLYPH_COUNT];
    GlyphInfoPtr glyphs[MAX_GLYPH_COUNT];
    LigKernStep (*ligKernSteps)[];
    FIX16 (*kerns)[];

public:
    IBMFFaceLow() : initialized(false), charSet(0), resolution(default_resolution) {}

    ~IBMFFaceLow() {}

    inline static float fromFIX16(FIX16 val) { return (float)val / 64.0; }
    inline static FIX16 toFIX16(float val) { return (FIX16)(val * 64.0); }

    // ---

    bool load(const MemoryPtr dataStart, const int dataLength, uint8_t chSet) {
        memset(glyphs, 0, sizeof(glyphs));

        MemoryPtr memoryPtr = dataStart;
        MemoryPtr memoryEnd = dataStart + dataLength;

        faceHeader = (FaceHeaderPtr)memoryPtr;
        charSet = chSet;

        if (faceHeader->glyphCount >= MAX_GLYPH_COUNT) {
            LOGE("Too many glyphs in face. Maximum is %d in IBMFDefs.h", MAX_GLYPH_COUNT);
            return false;
        }
        memoryPtr += sizeof(FaceHeader);
        for (int i = 0; i < faceHeader->glyphCount; i++) {
            glyphs[i] = (GlyphInfoPtr)memoryPtr;
            memoryPtr += sizeof(GlyphInfo);
            if (memoryPtr > memoryEnd) {
                LOGE("IBMFFace: End of memory (0x%p, 0x%p, %d) reached reading glyph #%d!!",
                     dataStart, memoryEnd, dataLength, i);
                return false;
            }

            bitmaps[i] = RLEBitmap{.pixels = memoryPtr,
                                   .dim = Dim(glyphs[i]->bitmapWidth, glyphs[i]->bitmapHeight),
                                   .length = glyphs[i]->packetLength};
            memoryPtr += glyphs[i]->packetLength;

            if (memoryPtr > memoryEnd) {
                LOGE("IBMFFace: End of memory (0x%p, 0x%p, %d) reached reading bitmap #%d of "
                     "length %d!!",
                     dataStart, memoryEnd, dataLength, i, glyphs[i]->packetLength);
                return false;
            }
        }

        ligKernSteps = (LigKernStep(*)[])memoryPtr;
        memoryPtr += sizeof(LigKernStep) * faceHeader->ligKernStepCount;
        if (memoryPtr > memoryEnd) {
            LOGE("IBMFFace: End of memory reached reading lig/kern struct!!");
            return false;
        }

        kerns = (FIX16(*)[])memoryPtr;

        memoryPtr += sizeof(FIX16) * faceHeader->kernCount;
        if (memoryPtr > memoryEnd) {
            LOGE("IBMFFace: End of memory reached reading kerns!!");
            return false;
        }

        // showFace();
        initialized = true;
        return true;
    }

    inline bool isInitialized() const { return initialized; }
    inline uint8_t getFacePtSize() const { return faceHeader->pointSize; }
    inline uint16_t getLineHeight() const { return faceHeader->lineHeight; }
    inline uint16_t getEmHeight() const { return faceHeader->emHeight >> 6; }
    inline uint8_t getMaxHight() const { return faceHeader->maxHight; }
    inline int16_t getDescenderHeight() const { return -(int16_t)faceHeader->descenderHeight; }
    inline const LigKernStep &getLigKern(uint8_t idx) const { return (*ligKernSteps)[idx]; }
    inline FIX16 getKern(uint8_t i) const { return (*kerns)[i]; }
    inline const GlyphInfo &getGlyphInfo(uint8_t glyphCode) const { return *glyphs[glyphCode]; }
    inline void setResolution(PixelResolution res) { resolution = res; }
    inline PixelResolution getResolution() { return resolution; }

    /**
     * @brief Translate latin in an internal glyph code
     *
     * The class allow for latin1+ characters to be plotted into a bitmap. As the
     * font doesn't contains all accented glyphs, a combination of glyphs must be
     * used to draw a single bitmap. This method translate some of the supported
     * unicode values to that combination. The least significant byte will contains
     * the main glyph code and the next byte will contain the accent code.
     *
     * The supported UTF16 character codes supported are the following:
     *
     *      U+0020 to U+007F
     *      U+00A1 to U+017f
     *
     *   and the following:
     *
     * |  UTF16  | Description          |
     * |:-------:|----------------------|
     * | U+02BB  | reverse apostrophe
     * | U+02BC  | apostrophe
     * | U+02C6  | circumflex
     * | U+02DA  | ring
     * | U+02DC  | tilde ~
     * | U+2013  | endash (Not available with CM Typewriter)
     * | U+2014  | emdash (Not available with CM Typewriter)
     * | U+2018  | quote left
     * | U+2019  | quote right
     * | U+201A  | comma like ,
     * | U+201C  | quoted left "
     * | U+201D  | quoted right
     * | U+2032  | minute '
     * | U+2033  | second "
     * | U+2044  | fraction /
     * | U+20AC  | euro
     *
     * @param charcode The UTF16 character code
     * @return The internal representation of a character (glyphCode)
     */
    uint16_t translateFromLatin(char32_t charcode) const {
        uint16_t glyphCode;

        if ((charcode > 0x20) && (charcode < 0x7F)) {
            glyphCode = charcode; // ASCII codes No accent
        } else if ((charcode >= 0xA1) && (charcode <= 0x1FF)) {
            glyphCode = latinTranslationSet[charcode - 0xA1];
        } else {
            switch (charcode) {
            case 0x2013: // endash
                glyphCode = 0x0015;
                break;
            case 0x2014: // emdash
                glyphCode = 0x0016;
                break;
            case 0x2018: // quote left
            case 0x02BB: // reverse apostrophe
                glyphCode = 0x0060;
                break;
            case 0x2019: // quote right
            case 0x02BC: // apostrophe
                glyphCode = 0x0027;
                break;
            case 0x201C: // quoted left "
                glyphCode = 0x0010;
                break;
            case 0x201D: // quoted right
                glyphCode = 0x0011;
                break;
            case 0x02C6: // circumflex
                glyphCode = 0x005E;
                break;
            case 0x02DA: // ring
                glyphCode = 0x0006;
                break;
            case 0x02DC: // tilde ~
                glyphCode = 0x007E;
                break;
            case 0x201A: // comma like ,
                glyphCode = 0x000D;
                break;
            case 0x2032: // minute '
                glyphCode = 0x0027;
                break;
            case 0x2033: // second "
                glyphCode = 0x0022;
                break;
            case 0x2044: // fraction /
                glyphCode = 0x002F;
                break;
            case 0x20AC: // euro
                glyphCode = 0x00AD;
                break;
            default:
                glyphCode = SPACE_CODE;
            }
        }

        return glyphCode;
    }

    bool getGlyph(char32_t charCode, Glyph &appGlyph, bool loadBitmap, bool caching = true,
                  Pos atPos = Pos(0, 0)) {

        LOGD("charCode: %d, loadBitmap: %s, caching: %s, pos: [%d, %d]", charCode,
             loadBitmap ? "YES" : "NO", caching ? "YES" : "NO", atPos.x, atPos.y);

        uint16_t glyphCode;
        bool accentIsPresent = false;
        uint8_t accentIdx = 0;
        GlyphInfoPtr accentInfo = nullptr;

        if (caching) appGlyph.clear();

        if (charSet == CharSet::LATIN_CHARACTER_SET) {
            glyphCode = translateFromLatin(charCode);

            accentIsPresent = (glyphCode & ACCENT_MASK) != 0;
            if (accentIsPresent) {
                accentIdx = (glyphCode & ACCENT_MASK) >> ACCENT_SHIFTR;
                if (accentIdx == CODED_GRAVE_ACCENT)
                    accentIdx = GRAVE_ACCENT;
                else if (accentIdx == CODED_APOSTROPHE)
                    accentIdx = APOSTROPHE;
                accentInfo = glyphs[accentIdx];
            }
        } else if (charSet == UTF16_TABLE_SET) {
            // Next release of the IBMF file format will have a table to support
            // direct usage of predefined UTF-16 characters. Up to 1022 characters
            // will be possible, unless extended to use more space in memory.
            //
            // It will be known as UTF16_TABLE_SET
            LOGW("Support for UTF16_TABLE_SET not ready.");
            return false;
        } else {
            LOGE("Unknown character set: %d", charSet);
        }

        uint8_t glyphIdx = (glyphCode & GLYPH_CODE_MASK) - faceHeader->firstCode;
        if (((glyphCode & GLYPH_CODE_MASK) == NO_GLYPH_CODE) ||
            ((glyphIdx < faceHeader->glyphCount) && (glyphs[glyphIdx] == nullptr))) {
            LOGW("No entry for char code %d (0x%x) [glyph code %d (0x%x)]", charCode, charCode,
                 glyphCode, glyphCode);
            glyphCode = SPACE_CODE;
        }

        if (glyphCode == SPACE_CODE) { // send as a space character
            appGlyph.metrics.lineHeight = faceHeader->lineHeight;
            appGlyph.metrics.advance = faceHeader->spaceSize;
            appGlyph.pointSize = faceHeader->pointSize;
            return true;
        }

        if (glyphIdx >= faceHeader->glyphCount) return false;

        GlyphInfoPtr glyphInfo = glyphs[glyphIdx];

        if (glyphInfo == nullptr) return false;

        Dim dim = Dim(glyphInfo->bitmapWidth, glyphInfo->bitmapHeight);
        Pos glyphOffsets = Pos(0, 0);
        Pos accentOffsets = Pos(0, 0);

        uint8_t addedLeft = 0;
        int16_t addedHeightBelow = 0;

        if (accentIsPresent) {

            if (glyphCode == 0xE06E) { // Apostrophe n
                // offsets.x = 0; // already set
                glyphOffsets.x = accentInfo->bitmapWidth + 1 -
                                 (((faceHeader->xHeight >> 6) * faceHeader->slantCorrection) >> 6);
                dim.width = glyphOffsets.x + glyphInfo->bitmapWidth;
            } else {
                // Horizontal adjustment
                if (glyphCode == 0xC041) { // Ą
                    accentOffsets.x = glyphInfo->bitmapWidth - accentInfo->bitmapWidth;
                } else if ((glyphCode == 0xC061) || (glyphCode == 0xC045)) { // ą or Ę
                    accentOffsets.x =
                        glyphInfo->bitmapWidth - accentInfo->bitmapWidth -
                        ((((int32_t)glyphInfo->bitmapHeight) * faceHeader->slantCorrection) >> 6);
                } else {
                    accentOffsets.x =
                        ((glyphInfo->bitmapWidth > accentInfo->bitmapWidth)
                             ? ((glyphInfo->bitmapWidth - accentInfo->bitmapWidth) >> 1)
                             : 0) +
                        ((accentInfo->verticalOffset < 5)
                             ? -((faceHeader->xHeight * faceHeader->slantCorrection) >> 6)
                             : ((((int32_t)glyphInfo->bitmapHeight) *
                                 faceHeader->slantCorrection) >>
                                6));
                    /*- (accentInfo->horizontalOffset - glyphInfo->horizontalOffset)*/;
                }
                if ((accentOffsets.x == 0) && (glyphInfo->bitmapWidth < accentInfo->bitmapWidth)) {
                    glyphOffsets.x = (accentInfo->bitmapWidth - glyphInfo->bitmapWidth) >> 1;
                    dim.width = accentInfo->bitmapWidth;
                }
                if (dim.width < (accentOffsets.x + accentInfo->bitmapWidth)) {
                    dim.width = accentOffsets.x + accentInfo->bitmapWidth;
                }
            }

            // Vertical adjustment
            if (caching) {
                if (accentInfo->verticalOffset >= (faceHeader->xHeight >> 6)) {
                    // Accents that are on top of a main glyph
                    glyphOffsets.y = accentInfo->verticalOffset - (faceHeader->xHeight >> 6);
                    dim.height += glyphOffsets.y;
                } else if (accentInfo->verticalOffset < 5) {
                    // Accents below the main glyph (cedilla)
                    addedHeightBelow = (glyphInfo->bitmapHeight - glyphInfo->verticalOffset) -
                                       ((-accentInfo->verticalOffset) + accentInfo->bitmapHeight);
                    if (addedHeightBelow < 0) dim.height += -addedHeightBelow;
                    accentOffsets.y = glyphInfo->verticalOffset - accentInfo->verticalOffset;
                }
            } else {
                if (accentInfo->verticalOffset >= (faceHeader->xHeight >> 6)) {
                    accentOffsets.y = -(accentInfo->verticalOffset - (faceHeader->xHeight >> 6) +
                                        glyphInfo->verticalOffset);
                    glyphOffsets.y = -glyphInfo->verticalOffset;
                } else if (accentInfo->verticalOffset < 5) {
                    glyphOffsets.y = -glyphInfo->verticalOffset;
                }
            }
            // else if ((glyphCode == 0x1648) || (glyphCode == 0x1568)) { // Ħ or ħ
            //   offsets.y = (glyphInfo->bitmapHeight * 1) >> 2;
            //   offsets.x = ((glyphInfo->bitmapHeight - offsets.y) *
            //   faceHeader->slantCorrection) >> 6;
            // }
            // else if ((glyphCode == 0x1554) || (glyphCode == 0x1574)) { // Ŧ or ŧ
            //   offsets.y = (glyphInfo->bitmapHeight * 1) >> 1;
            //   offsets.x = (offsets.y * faceHeader->slantCorrection) >> 6;
            // }
        } else {
            glyphOffsets.y = -glyphInfo->verticalOffset;
        }

        if (loadBitmap) {
            if (caching) {
                uint16_t size = (resolution == PixelResolution::ONE_BIT)
                                    ? dim.height * ((dim.width + 7) >> 3)
                                    : dim.height * dim.width;

                uint8_t white;
                if (resolution == PixelResolution::ONE_BIT) {
                    white = WHITE_ONE_BIT ? 0xFF : 0;
                } else {
                    white = WHITE_EIGHT_BITS;
                }

                appGlyph.bitmap.pixels = new uint8_t[size];
                memset(appGlyph.bitmap.pixels, white, size);
                appGlyph.bitmap.dim = dim;
            }

            RLEBitmapPtr glyphBitmap = &bitmaps[glyphIdx];
            if (glyphBitmap != nullptr) {
                RLEExtractor rle(resolution);

                Pos outPos = Pos(atPos.x + accentOffsets.x, atPos.y + accentOffsets.y);
                if (accentIsPresent) {
                    RLEBitmapPtr accentBitmap = &bitmaps[accentIdx];
                    rle.retrieveBitmap(*accentBitmap, appGlyph.bitmap, outPos,
                                       accentInfo->rleMetrics);

                    showBitmap(appGlyph.bitmap);
                }

                outPos = Pos(atPos.x + glyphOffsets.x, atPos.y + glyphOffsets.y);
                rle.retrieveBitmap(*glyphBitmap, appGlyph.bitmap, outPos, glyphInfo->rleMetrics);
            }
        }

        appGlyph.metrics = {.xoff = (int16_t) - (glyphOffsets.x + glyphInfo->horizontalOffset),
                            .yoff = (int16_t) - (glyphOffsets.y + glyphInfo->verticalOffset),
                            .advance = (int16_t)((glyphInfo->advance + 32) >> 6),
                            .lineHeight = faceHeader->lineHeight,
                            .ligatureAndKernPgmIndex = glyphInfo->ligKernPgmIndex};

        // showGlyph(appGlyph, glyphCode & 0xFF);
        return true;
    }

    void showBitmap(const Bitmap &bitmap) const {
        if constexpr (DEBUG) {
            uint32_t row, col;
            MemoryPtr rowPtr;

            std::cout << "   +";
            for (col = 0; col < bitmap.dim.width; col++)
                std::cout << '-';
            std::cout << '+' << std::endl;

            if (resolution == PixelResolution::ONE_BIT) {
                uint32_t rowSize = (bitmap.dim.width + 7) >> 3;
                for (row = 0, rowPtr = bitmap.pixels; row < bitmap.dim.height;
                     row++, rowPtr += rowSize) {
                    std::cout << "   |";
                    for (col = 0; col < bitmap.dim.width; col++) {
                        if constexpr (BLACK_ONE_BIT) {
                            std::cout << ((rowPtr[col >> 3] & (0x80 >> (col & 7))) ? 'X' : ' ');
                        } else {
                            std::cout << ((rowPtr[col >> 3] & (0x80 >> (col & 7))) ? ' ' : 'X');
                        }
                    }
                    std::cout << '|';
                    std::cout << std::endl;
                }
            } else {
                uint32_t rowSize = bitmap.dim.width;
                for (row = 0, rowPtr = bitmap.pixels; row < bitmap.dim.height;
                     row++, rowPtr += rowSize) {
                    std::cout << "   |";
                    for (col = 0; col < bitmap.dim.width; col++) {
                        if constexpr (BLACK_EIGHT_BITS) {
                            std::cout << ((rowPtr[col] == BLACK_EIGHT_BITS) ? 'X' : ' ');
                        } else {
                            std::cout << ((rowPtr[col] == BLACK_EIGHT_BITS) ? 'X' : ' ');
                        }
                    }
                    std::cout << '|';
                    std::cout << std::endl;
                }
            }

            std::cout << "   +";
            for (col = 0; col < bitmap.dim.width; col++) {
                std::cout << '-';
            }
            std::cout << '+' << std::endl << std::endl;
        }
    }

    void showGlyph(const Glyph &glyph, uint16_t glyphCode, char16_t charCode = ' ') const {
        if constexpr (DEBUG) {
            std::wcout << "Glyph Base Code: 0x" << std::hex << glyphCode << std::dec << "("
                       << charCode << ")" << std::endl
                       << "  Metrics: [" << std::dec << glyph.bitmap.dim.width << ", "
                       << glyph.bitmap.dim.height << "] " << std::endl
                       << "  Position: [" << glyph.metrics.xoff << ", " << glyph.metrics.yoff << ']'
                       << std::endl
                       << "  Bitmap available: "
                       << ((glyph.bitmap.pixels == nullptr) ? "No" : "Yes") << std::endl;

            if (glyph.bitmap.pixels != nullptr) {
                showBitmap(glyph.bitmap);
            }
        }
    }

    void showGlyph2(const Glyph &glyph, char16_t charCode) const { showGlyph(glyph, 0, charCode); }

    void showGlyphInfo(uint16_t i, GlyphInfo &g) const {
        if constexpr (DEBUG) {
            std::cout << "  [" << i << "]: "
                      << "charCode : " << +g.charCode << ", width: " << +g.bitmapWidth
                      << ", height: " << +g.bitmapHeight << ", hoffset: " << +g.horizontalOffset
                      << ", voffset: " << +g.verticalOffset << ", pktLen: " << +g.packetLength
                      << ", advance: " << +((float)g.advance / 64.0)
                      << ", dynF: " << +g.rleMetrics.dynF
                      << ", firstIsBlack: " << +g.rleMetrics.firstIsBlack
                      << ", ligKernPgmIndex: " << +g.ligKernPgmIndex << std::endl;
        }
    }

    void showLigKerns() const {
        if constexpr (DEBUG) {
            std::cout << std::endl
                      << "----------- Ligature / Kern programs: ----------" << std::endl;
            uint16_t i;
            for (i = 0; i < faceHeader->ligKernStepCount; i++) {
                LigKernStepPtr entry = &(*ligKernSteps)[i];
                if (entry->skip.whole > 128) {
                    std::cout << "  [" << i << "]:  "
                              << "Whole skip: " << +entry->skip.whole << ", "
                              << "Goto: "
                              << +((entry->opCode.d.displHigh << 8) + entry->remainder.displLow);
                } else {
                    std::cout << "  [" << i << "]:  "
                              << "Whole skip: " << +entry->skip.whole << ", "
                              << "Stop: " << (entry->skip.s.stop ? "Yes" : "No") << ", "
                              << "NxtChar: " << +entry->nextChar << ", "
                              << "IsKern: " << (entry->opCode.d.isAKern ? "Yes" : "No") << ", "
                              << (entry->opCode.d.isAKern ? "Kern Idx: " : "Lig char: ")
                              << (entry->opCode.d.isAKern ? +((entry->opCode.d.displHigh << 8) +
                                                              entry->remainder.displLow)
                                                          : +entry->remainder.replacementChar)
                              << std::dec;
                    if (!entry->opCode.d.isAKern) {
                        std::cout << ", OpCodes: a:" << +entry->opCode.op.aOp
                                  << ", b:" << +entry->opCode.op.bOp
                                  << ", c:" << +entry->opCode.op.cOp;
                    }
                }

                std::cout << std::endl;
            }
        }
    }

    void showKerns() const {
        if constexpr (DEBUG) {
            std::cout << std::endl << "----------- Kerns: ----------" << std::endl;

            for (int i = 0; i < faceHeader->kernCount; i++) {
                std::cout << "  [" << i << "]:  " << ((*kerns)[i] / 64.0) << std::endl;
            }
        }
    }

    void showFace() const {
        if constexpr (DEBUG) {

            std::cout << std::endl << "----------- Header: ----------" << std::endl;

            std::cout << "DPI: " << faceHeader->dpi << ", point size: " << +faceHeader->pointSize
                      << ", first char code: " << +faceHeader->firstCode
                      << ", last char code: " << +faceHeader->lastCode
                      << ", line height: " << +faceHeader->lineHeight
                      << ", max height: " << +faceHeader->maxHight
                      << ", x height: " << +((float)faceHeader->xHeight / 64.0)
                      << ", em size: " << +((float)faceHeader->emHeight / 64.0)
                      << ", space size: " << +((float)faceHeader->spaceSize / 64.0)
                      << ", glyph count: " << +faceHeader->glyphCount
                      << ", lig kern count: " << +faceHeader->ligKernStepCount
                      << ", kernCount: " << +faceHeader->kernCount
                      << ", slant corr: " << +faceHeader->slantCorrection
                      << ", descender height: " << +faceHeader->descenderHeight << std::endl;

            std::cout << std::endl << "----------- Glyphs: ----------" << std::endl;

            for (int i = 0; i < faceHeader->glyphCount; i++) {
                showGlyphInfo(i, *glyphs[i]);
            }

            showLigKerns();

            showKerns();
        }
    }
};

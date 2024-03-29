#pragma once

#include <cstring>

#include "IBMFDefs.hpp"
#include "RLEExtractor.hpp"

using namespace ibmf_defs;

/**
 * @brief Access to a IBMF face.
 *
 * This is a low-level class to allow acces to a IBMF faces generated through METAFONT
 *
 */
class IBMFFaceLow {

private:
    static constexpr char const *TAG = "IBMFFaceLow";

    bool initialized_{false};
    FontFormat fontFormat_{FontFormat::UNKNOWN};
    PixelResolution resolution_{DEFAULT_RESOLUTION};

    FaceHeaderPtr faceHeader_{nullptr};
    GlyphsPixelPoolIndexes glyphsPixelPoolIndexes_{nullptr};
    GlyphsInfoPtr glyphsInfo_{nullptr};
    PixelsPoolPtr pixelsPool_{nullptr};
    LigKernStepsPtr ligKernSteps_{nullptr};

public:
    IBMFFaceLow() = default;

    ~IBMFFaceLow() = default;

    inline static auto fromFIX16(FIX16 val) -> float { return (float)val / 64.0; }
    inline static auto toFIX16(float val) -> FIX16 { return (FIX16)(val * 64.0); }
    inline static auto fromFIX14(FIX14 val) -> float { return (float)toFIX16(val) / 64.0; }

    // ---

    auto load(const MemoryPtr dataStart, const int dataLength, FontFormat fontFmt) -> bool {

        MemoryPtr memoryPtr = dataStart;
        MemoryPtr memoryEnd = dataStart + dataLength;

        faceHeader_ = reinterpret_cast<FaceHeaderPtr>(memoryPtr);
        fontFormat_ = fontFmt;

        uint16_t maxCount =
            (fontFormat_ == FontFormat::LATIN) ? LATIN_MAX_GLYPH_COUNT : UTF32_MAX_GLYPH_COUNT;
        if (faceHeader_->glyphCount >= maxCount) {
            LOGE("Too many glyphs in face. Maximum is %d in ibmf_defs.h", maxCount);
            return false;
        }

        memoryPtr += sizeof(FaceHeader);
        glyphsPixelPoolIndexes_ = reinterpret_cast<GlyphsPixelPoolIndexes>(memoryPtr);

        memoryPtr += (sizeof(PixelPoolIndex) * faceHeader_->glyphCount);
        glyphsInfo_ = reinterpret_cast<GlyphsInfoPtr>(memoryPtr);

        memoryPtr += (sizeof(GlyphInfo) * faceHeader_->glyphCount);
        pixelsPool_ = reinterpret_cast<PixelsPoolPtr>(memoryPtr);

        memoryPtr += faceHeader_->pixelsPoolSize;
        ligKernSteps_ = reinterpret_cast<LigKernStepsPtr>(memoryPtr);

        memoryPtr += (sizeof(LigKernStep) * faceHeader_->ligKernStepCount);

        if (memoryPtr != memoryEnd) {
            LOGE("IBMFFace: Memory synch issue reading lig/kern struct!!");
            return false;
        }

        // showFace();
        initialized_ = true;
        return true;
    }

    [[nodiscard]] inline auto isInitialized() const -> bool { return initialized_; }
    [[nodiscard]] inline auto getFacePtSize() const -> uint8_t { return faceHeader_->pointSize; }
    [[nodiscard]] inline auto getLineHeight() const -> uint16_t { return faceHeader_->lineHeight; }
    [[nodiscard]] inline auto getEmHeight() const -> uint16_t { return faceHeader_->emHeight >> 6; }
    [[nodiscard]] inline auto getDescenderHeight() const -> int16_t {
        return -(int16_t)faceHeader_->descenderHeight;
    }
    [[nodiscard]] inline auto getLigKernStep(uint16_t idx) const -> LigKernStep * {
        return &(*ligKernSteps_)[idx];
    }
    inline auto setResolution(PixelResolution res) -> void { resolution_ = res; }
    [[nodiscard]] inline auto getResolution() const -> PixelResolution { return resolution_; }

    [[nodiscard]] inline auto getGlyphInfo(GlyphCode glyphCode) const -> const GlyphInfo & {
        return (*glyphsInfo_)[glyphCode];
    }

    [[nodiscard]] inline auto getLigKernPgmIndex(GlyphCode glyphCode) const -> uint16_t {

#if LATIN_SUPPORT
        if ((fontFormat_ == FontFormat::LATIN) && (glyphCode != SPACE_CODE)) {
            glyphCode &= LATIN_GLYPH_CODE_MASK;
        }
#endif
        return (glyphCode < SPACE_CODE) ? (*glyphsInfo_)[glyphCode].ligKernPgmIndex
                                        : NO_LIG_KERN_PGM;
    }

    /// @brief Search Ligature and Kerning table
    ///
    /// Using the LigKern program of **glyphCode1**, find the first entry in the
    /// program for which **glyphCode2** is the next character. If a ligature is
    /// found, sets **glyphCode2** with the new code and returns *true*. If a
    /// kerning entry is found, it sets the kern parameter with the value
    /// in the table and return *false*. If the LigKern pgm is empty or there
    /// is no entry for **glyphCode2**, it returns *false*.
    ///
    /// Note: character codes have to be translated to internal GlyphCode before
    /// calling this method.
    ///
    /// @param glyphCode1 In. The GlyhCode for which to find a LigKern entry in its program.
    /// @param glyphCode2 InOut. The GlyphCode that must appear in the program as the next
    ///                   character in sequence. Will be replaced with the target
    ///                   ligature GlyphCode if found.
    /// @param kern Out. When a kerning entry is found in the program, kern will receive the value.
    /// @return True if a ligature was found, false otherwise.
    ///
    auto ligKern(const GlyphCode glyphCode1, GlyphCode *glyphCode2, FIX16 *kern) -> bool {
        if ((glyphCode1 == NO_GLYPH_CODE) || (glyphCode1 == SPACE_CODE) ||
            (*glyphCode2 == NO_GLYPH_CODE) || (*glyphCode2 == SPACE_CODE)) {
            *kern = 0;
            return false;
        }
        if ((glyphCode1 >= faceHeader_->glyphCount) || (*glyphCode2 >= faceHeader_->glyphCount)) {
            LOGE("Bad Glyphcode glyphCode1: %d, glyphCode2: %d, max: %d", glyphCode1, *glyphCode2,
                 faceHeader_->glyphCount);
            *kern = 0;
            return false;
        }

        // Check for ligatures
        uint16_t lkIdx = getLigKernPgmIndex(glyphCode1);
        if (lkIdx != NO_LIG_KERN_PGM) {
            LigKernStep *lk = getLigKernStep(lkIdx);
            if (lk->b.goTo.isAKern && lk->b.goTo.isAGoTo) {
                lkIdx = lk->b.goTo.displacement;
                lk = getLigKernStep(lkIdx);
            }

            GlyphCode code = (*glyphsInfo_)[*glyphCode2].mainCode;
#if LATIN_SUPPORT
            if (fontFormat_ == FontFormat::LATIN) {
                code &= LATIN_GLYPH_CODE_MASK;
            }
#endif
            bool first = true;

            do {
                if (!first) {
                    lk++;
                } else {
                    first = false;
                }

                if (lk->a.nextGlyphCode == code) {
                    if (lk->b.kern.isAKern) {
                        *kern = lk->b.kern.kerningValue;
                        return false; // No other iteration to be done
                    } else {
                        *glyphCode2 = lk->b.repl.replGlyphCode;
                        return true;
                    }
                }
            } while (!lk->a.stop);
        }

#if OPTICAL_KERNING
        const constexpr size_t K_BUFF_PITCH = ((K_BUFF_WIDTH + 7) >> 3);
        const constexpr size_t K_BUFF_SIZE = K_BUFF_PITCH * K_BUFF_HEIGHT;
        static uint8_t buff[K_BUFF_SIZE];
        Glyph glyph;

        glyph.bitmap =
            Bitmap{.pixels = static_cast<MemoryPtr>(buff), .dim = Dim(K_BUFF_WIDTH, K_BUFF_HEIGHT)};
        memset(buff, WHITE_EIGHT_BITS, K_BUFF_SIZE);
        Pos pos = Pos(K_ORIGIN_X, K_ORIGIN_Y);

        // Position the first glyph in the buffer
        bool result = getGlyph(glyphCode1, glyph, true, false, pos);
        if (!result) {
            LOGE("Unable to load glyphCode %d related glyph bitmap.", glyphCode1);
            return false;
        }

        int voff = (*glyphsInfo_)[*glyphCode2].verticalOffset;
        int hoff = (*glyphsInfo_)[*glyphCode2].horizontalOffset;

        Glyph glyph2;
        result = getGlyph(*glyphCode2, glyph2, true, true, Pos(0, voff));
        if (!result) {
            LOGE("Unable to load glyphCode %d related glyph bitmap.", *glyphCode2);
            return false;
        }
        // showBitmap(glyph2.bitmap);
        int advance =
            ((glyph.metrics.advance + 32) >> 6); // + (*glyphsInfo_)[*glyphCode2].horizontalOffset;
        if (advance == 0) advance = glyph.bitmap.dim.width + 1;

        pos.x += advance;

        int max = advance;
        int kerning = 0;
        int pitch2 = (glyph2.bitmap.dim.width + 7) >> 3;

        bool first = true;
        int col, row;
        uint8_t mask, mask2;
        int idx, idx2, ridx, ridx2;

        while (max > 0) {

            mask = 0x80 >> ((pos.x - hoff) & 7);
            mask2 = 0x80;
            idx = ((pos.y - voff) * K_BUFF_PITCH) + ((pos.x - hoff) >> 3);
            idx2 = 0;

            for (col = 0; col < glyph2.bitmap.dim.width; col++) {

                for (row = 0, ridx = idx, ridx2 = idx2; row < glyph2.bitmap.dim.height;
                     row++, ridx += K_BUFF_PITCH, ridx2 += pitch2) {
                    if constexpr (BLACK_ONE_BIT) {
                        if ((glyph2.bitmap.pixels[ridx2] & mask2) != 0) {
                            if ((buff[ridx] & mask) != 0) {
                                goto fin;
                            } else if ((buff[ridx - K_BUFF_PITCH] & mask) != 0) {
                                goto fin;
                            } else if ((buff[ridx + K_BUFF_PITCH] & mask) != 0) {
                                goto fin;
                            }
                        }
                    } else {
                        if ((glyph2.bitmap.pixels[ridx2] & mask2) == 0) {
                            if ((buff[ridx] & mask) == 0) {
                                goto fin;
                            } else if ((buff[ridx - K_BUFF_PITCH] & mask) == 0) {
                                goto fin;
                            } else if ((buff[ridx + K_BUFF_PITCH] & mask) == 0) {
                                goto fin;
                            }
                        }
                    }
                }
                mask >>= 1;
                if (mask == 0) {
                    mask = 0x80;
                    idx += 1;
                }
                mask2 >>= 1;
                if (mask2 == 0) {
                    mask2 = 0x80;
                    idx2 += 1;
                }
            }

            first = false;
            pos.x -= 1;
            kerning -= 1;
            max -= 1;
        }
    fin:
        delete[] glyph2.bitmap.pixels;
        *kern = static_cast<FIX16>((max > 0) ? ((kerning + KERNING_SIZE + 1) << 6) : 0);
        // LOGD("Computed kern: %d (%d).", static_cast<int>((float)*kern / 64.0), kerning);
#endif
        return false;
    }

    auto getGlyph(GlyphCode glyphCode, Glyph &appGlyph, bool loadBitmap, bool caching = true,
                  Pos atPos = Pos(0, 0)) -> bool {

        // LOGD("glyphCode: %04x, loadBitmap: %s, caching: %s, pos: [%d, %d]", glyphCode,
        //      loadBitmap ? "YES" : "NO", caching ? "YES" : "NO", atPos.x, atPos.y);

        if (caching) {
            appGlyph.clear();
        }

#if LATIN_SUPPORT
        bool accentIsPresent = false;
        uint16_t accentIdx = 0;
        GlyphCode latinCode;
        GlyphInfo *accentInfo = nullptr;

        if (fontFormat_ == FontFormat::LATIN) {
            latinCode = glyphCode;
            if (glyphCode != SPACE_CODE) {
                accentIsPresent = (glyphCode & ACCENT_MASK) != 0;
                if (accentIsPresent) {
                    accentIdx = (glyphCode & ACCENT_MASK) >> ACCENT_SHIFTR;
                    if (accentIdx == CODED_GRAVE_ACCENT) {
                        accentIdx = GRAVE_ACCENT;
                    } else if (accentIdx == CODED_APOSTROPHE) {
                        accentIdx = APOSTROPHE;
                    }
                    accentInfo = &(*glyphsInfo_)[accentIdx];
                }
                glyphCode &= LATIN_GLYPH_CODE_MASK;
            }
        } else if (fontFormat_ != FontFormat::UTF32) {
            LOGE("Unknown character set: %d", fontFormat_);
            return false;
        }
#else
        if (fontFormat_ != FontFormat::UTF32) {
            LOGE("Unknown character set: %d", fontFormat_);
            return false;
        }
#endif

        if (glyphCode == SPACE_CODE) { // send as a space character
            appGlyph.metrics.lineHeight = faceHeader_->lineHeight;
            appGlyph.metrics.advance = (int16_t)faceHeader_->spaceSize << 6;
            appGlyph.pointSize = faceHeader_->pointSize;
            return true;
        }

        if (glyphCode >= faceHeader_->glyphCount) {
            return false;
        }

        GlyphInfo *glyphInfo = &(*glyphsInfo_)[glyphCode];

        if (glyphInfo == nullptr) {
            return false;
        }

        Dim dim = Dim(glyphInfo->bitmapWidth, glyphInfo->bitmapHeight);
        Pos glyphOffsets = Pos(0, 0);

#if LATIN_SUPPORT
        Pos accentOffsets = Pos(0, 0);
        if (accentIsPresent) {

            if (latinCode == 0xE06E) { // Apostrophe n
                // offsets.x = 0; // already set
                glyphOffsets.x =
                    accentInfo->bitmapWidth + 1 -
                    (((faceHeader_->xHeight >> 6) * faceHeader_->slantCorrection) >> 6);
                dim.width = glyphOffsets.x + glyphInfo->bitmapWidth;
            } else {
                // Horizontal adjustment
                if (latinCode == 0xC041) { // Ą
                    accentOffsets.x = glyphInfo->bitmapWidth - accentInfo->bitmapWidth;
                } else if ((latinCode == 0xC061) || (latinCode == 0xC045)) { // ą or Ę
                    accentOffsets.x =
                        glyphInfo->bitmapWidth - accentInfo->bitmapWidth -
                        ((((int32_t)glyphInfo->bitmapHeight) * faceHeader_->slantCorrection) >> 6);
                } else {
                    accentOffsets.x =
                        ((glyphInfo->bitmapWidth > accentInfo->bitmapWidth)
                             ? ((glyphInfo->bitmapWidth - accentInfo->bitmapWidth) >> 1)
                             : 0) +
                        ((accentInfo->verticalOffset < 5)
                             ? -((faceHeader_->xHeight * faceHeader_->slantCorrection) >> 6)
                             : ((((int32_t)glyphInfo->bitmapHeight) *
                                 faceHeader_->slantCorrection) >>
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
                if (accentInfo->verticalOffset >= (faceHeader_->xHeight >> 6)) {
                    // Accents that are on top of a main glyph
                    glyphOffsets.y = accentInfo->verticalOffset - (faceHeader_->xHeight >> 6);
                    dim.height += glyphOffsets.y;
                } else if (accentInfo->verticalOffset < 5) {
                    // Accents below the main glyph (cedilla)
                    int16_t addedHeightBelow =
                        (glyphInfo->bitmapHeight - glyphInfo->verticalOffset) -
                        ((-accentInfo->verticalOffset) + accentInfo->bitmapHeight);
                    if (addedHeightBelow < 0) {
                        dim.height += -addedHeightBelow;
                    }
                    accentOffsets.y = glyphInfo->verticalOffset - accentInfo->verticalOffset;
                }
            } else {
                if (accentInfo->verticalOffset >= (faceHeader_->xHeight >> 6)) {
                    accentOffsets.y = -(accentInfo->verticalOffset - (faceHeader_->xHeight >> 6) +
                                        glyphInfo->verticalOffset);
                    glyphOffsets.y = -glyphInfo->verticalOffset;
                } else if (accentInfo->verticalOffset < 5) {
                    glyphOffsets.y = -glyphInfo->verticalOffset;
                }
            }
            // else if ((glyphCode == 0x1648) || (glyphCode == 0x1568)) { // Ħ or ħ
            //   offsets.y = (glyphInfo->bitmapHeight * 1) >> 2;
            //   offsets.x = ((glyphInfo->bitmapHeight - offsets.y) *
            //   faceHeader_->slantCorrection) >> 6;
            // }
            // else if ((glyphCode == 0x1554) || (glyphCode == 0x1574)) { // Ŧ or ŧ
            //   offsets.y = (glyphInfo->bitmapHeight * 1) >> 1;
            //   offsets.x = (offsets.y * faceHeader_->slantCorrection) >> 6;
            // }
        } else {
            glyphOffsets.y = -glyphInfo->verticalOffset;
        }
        if (!caching) {
            glyphOffsets.x -= glyphInfo->horizontalOffset;
            accentOffsets.x -= glyphInfo->horizontalOffset;
        }
#else
        glyphOffsets.y = -glyphInfo->verticalOffset;
        if (!caching) {
            glyphOffsets.x -= glyphInfo->horizontalOffset;
        }
#endif

        if (loadBitmap) {
            if (caching) {
                uint16_t size = (resolution_ == PixelResolution::ONE_BIT)
                                    ? dim.height * ((dim.width + 7) >> 3)
                                    : dim.height * dim.width;

                uint8_t white;
                if (resolution_ == PixelResolution::ONE_BIT) {
                    white = WHITE_ONE_BIT ? 0xFF : 0;
                } else {
                    white = WHITE_EIGHT_BITS;
                }

                appGlyph.bitmap.pixels = new uint8_t[size];
                memset(appGlyph.bitmap.pixels, white, size);
                appGlyph.bitmap.dim = dim;
            }

            RLEBitmap glyphBitmap = {.pixels =
                                         &(*pixelsPool_)[(*glyphsPixelPoolIndexes_)[glyphCode]],
                                     .dim = Dim(glyphInfo->bitmapWidth, glyphInfo->bitmapHeight),
                                     .length = glyphInfo->packetLength};
            RLEExtractor rle(resolution_);

            Pos outPos;
#if LATIN_SUPPORT
            outPos = Pos(atPos.x + accentOffsets.x, atPos.y + accentOffsets.y);
            if (accentIsPresent) {
                RLEBitmap accentBitmap = {
                    .pixels = &(*pixelsPool_)[(*glyphsPixelPoolIndexes_)[accentIdx]],
                    .dim = Dim(accentInfo->bitmapWidth, accentInfo->bitmapHeight),
                    .length = accentInfo->packetLength};
                rle.retrieveBitmap(accentBitmap, appGlyph.bitmap, outPos, accentInfo->rleMetrics);

                // showBitmap(appGlyph.bitmap);
            }
#endif
            outPos = Pos(atPos.x + glyphOffsets.x, atPos.y + glyphOffsets.y);
            // std::cout << "inPos: [" << atPos.x << ", " << atPos.y << "], outPos: [" << outPos.x
            //           << ", " << outPos.y << "] " << std::endl;
            rle.retrieveBitmap(glyphBitmap, appGlyph.bitmap, outPos, glyphInfo->rleMetrics);
        }

        appGlyph.metrics = {
            .xoff = (int16_t) - (glyphOffsets.x + ((caching) ? glyphInfo->horizontalOffset : 0)),
            .yoff = (int16_t) - (glyphOffsets.y + ((caching) ? glyphInfo->verticalOffset : 0)),
            .advance = glyphInfo->advance,
            .lineHeight = faceHeader_->lineHeight,
            .ligatureAndKernPgmIndex = glyphInfo->ligKernPgmIndex};

        // showGlyph(appGlyph, glyphCode & 0xFF);
        return true;
    }

    auto showBitmap(const Bitmap &bitmap) const -> void {
        if constexpr (DEBUG) {
            uint32_t row, col;
            MemoryPtr rowPtr;

            std::cout << "   +";
            for (col = 0; col < bitmap.dim.width; col++) {
                std::cout << '-';
            }
            std::cout << '+' << std::endl;

            if (resolution_ == PixelResolution::ONE_BIT) {
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
                            std::cout << ((rowPtr[col] == BLACK_EIGHT_BITS) ? ' ' : 'X');
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

    auto showGlyph(const Glyph &glyph, GlyphCode glyphCode, char32_t codePoint = ' ') const
        -> void {
        if constexpr (DEBUG) {
            std::cout << "Glyph Base Code: 0x" << std::hex << glyphCode << std::dec << "("
                      << +codePoint << ")" << std::endl
                      << "  Metrics: [" << std::dec << glyph.bitmap.dim.width << ", "
                      << glyph.bitmap.dim.height << "] " << std::endl
                      << "  Position: [" << glyph.metrics.xoff << ", " << glyph.metrics.yoff << ']'
                      << std::endl
                      << "  Bitmap available: " << ((glyph.bitmap.pixels == nullptr) ? "No" : "Yes")
                      << std::endl;

            if (glyph.bitmap.pixels != nullptr) {
                showBitmap(glyph.bitmap);
            }
        }
    }

    auto showGlyph2(const Glyph &glyph, char32_t codePoint) const -> void {
        showGlyph(glyph, 0, codePoint);
    }

    auto showGlyphInfo(GlyphCode i, const GlyphInfo &g) const -> void {
        if constexpr (DEBUG) {
            std::cout << "  [" << i << "]: w: " << +g.bitmapWidth << ", h: " << +g.bitmapHeight
                      << ", hoff: " << +g.horizontalOffset << ", voff: " << +g.verticalOffset
                      << ", pktLen: " << +g.packetLength << ", adv: " << +((float)g.advance / 64.0)
                      << ", dynF: " << +g.rleMetrics.dynF
                      << ", 1stBlack: " << +g.rleMetrics.firstIsBlack
                      << ", lKPgmIdx: " << +g.ligKernPgmIndex
                      << ", poolIdx: " << +(*glyphsPixelPoolIndexes_)[i];
            if (g.mainCode != i) {
                std::cout << ", mainCode: " << g.mainCode;
            }
            std::cout << std::endl;
        }
    }

    auto showLigKerns() const -> void {
        if constexpr (DEBUG) {
            std::cout << std::endl
                      << "----------- Ligature / Kern programs: ----------" << std::endl;
            uint16_t i;
            for (i = 0; i < faceHeader_->ligKernStepCount; i++) {
                LigKernStep *entry = &(*ligKernSteps_)[i];
                if (entry->b.goTo.isAKern && entry->b.goTo.isAGoTo) {
                    std::cout << "  [" << i << "]:  "
                              << "Goto: " << entry->b.goTo.displacement;
                } else {
                    std::cout << "  [" << i << "]:  "
                              << "Stop: " << (entry->a.stop ? "Yes" : "No") << ", "
                              << "NxtGlyphCode: " << +entry->a.nextGlyphCode << ", "
                              << "IsKern: " << (entry->b.kern.isAKern ? "Yes" : "No") << ", "
                              << (entry->b.kern.isAKern ? "Kern Value: " : "Lig char: ");
                    if (entry->b.kern.isAKern) {
                        std::cout << (float)(entry->b.kern.kerningValue / 64.0);
                    } else {
                        std::cout << entry->b.repl.replGlyphCode;
                    }
                }

                std::cout << std::endl;
            }
        }
    }

    auto showFace() const -> void {
        if constexpr (DEBUG) {

            std::cout << std::endl << "----------- Face Header: ----------" << std::endl;

            std::cout << "DPI: " << faceHeader_->dpi << ", point siz: " << +faceHeader_->pointSize
                      << ", linHght: " << +faceHeader_->lineHeight
                      << ", xHght: " << +((float)faceHeader_->xHeight / 64.0)
                      << ", emSiz: " << +((float)faceHeader_->emHeight / 64.0)
                      << ", spcSiz: " << +faceHeader_->spaceSize
                      << ", glyphCnt: " << +faceHeader_->glyphCount
                      << ", LKCnt: " << +faceHeader_->ligKernStepCount
                      << ", PixPoolSiz: " << +faceHeader_->pixelsPoolSize
                      << ", slantCorr: " << +((float)faceHeader_->slantCorrection / 64.0)
                      << ", descHght: " << +faceHeader_->descenderHeight << std::endl;

            std::cout << std::endl << "----------- Glyphs: ----------" << std::endl;

            for (int i = 0; i < faceHeader_->glyphCount; i++) {
                showGlyphInfo(i, (*glyphsInfo_)[i]);
            }

            showLigKerns();
        }
    }
};

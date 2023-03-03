#pragma once

#include <cstring>
#include <fstream>
#include <iostream>

#include "IBMFDefs.hpp"
#include "IBMFFaceLow.hpp"

using namespace IBMFDefs;

/**
 * @brief Access to a IBMF font.
 *
 * This is a low-level class to allow acces to a IBMF font generated through METAFONT
 *
 */
class IBMFFontLow {

private:
    static constexpr char const *TAG = "IBMFFontLow";

    typedef IBMFFaceLow *IBMFFaceLowPtr;

    bool initialized;
    PreamblePtr preamble;
    IBMFFaceLow *faces;

public:
    IBMFFontLow(MemoryPtr fontData, uint32_t length) {

        initialized = load(fontData, length);
        if (!initialized) {
            log_e("%s: Font data not recognized!", TAG);
        }
    }

    IBMFFontLow() : initialized(false) {}
    ~IBMFFontLow() {}

    inline uint8_t getCharSet() const { return preamble->bits.char_set; }
    inline bool isInitialized() const { return initialized; }

    inline IBMFFaceLowPtr getFace(int idx) const {
        if (isInitialized() && (idx < preamble->face_count)) {
            return &faces[idx];
        }
        else {
            return nullptr;
        }
    }

    bool load(const MemoryPtr fontData, uint32_t length) {

        MemoryPtr data = fontData;

        preamble = (Preamble *)data;
        data += sizeof(Preamble);

        LOGI("IBMFFont: Loading font at location 0x%p of length %d", fontData, length);

        if (strncmp("IBMF", preamble->marker, 4) != 0) {
            log_e("IBMFFont: Preamble in error: IBMF marker absent!!");
            return false;
        }
        if (preamble->bits.version != IBMF_VERSION) {
            LOGE("IBMFFont: Font is of a wrong version. Expected V4, got V%d",
                 preamble->bits.version);
            return false;
        }

        uint32_t *bin_face_offsets = (uint32_t *)data;

        uint32_t bin_face_lengths[10];
        for (int i = 0; i < preamble->face_count; i++) {
            bin_face_lengths[i] =
                ((i < (preamble->face_count - 1)) ? bin_face_offsets[i + 1] : length) -
                bin_face_offsets[i];
        }

        faces = new IBMFFaceLow[preamble->face_count];
        for (int i = 0; i < preamble->face_count; i++) {
            if (!faces[i].load(fontData + bin_face_offsets[i], bin_face_lengths[i])) {
                LOGE("IBMFFont: Unable to load face %d", i);
                return false;
            }
        }
        initialized = true;
        return true;
    }
};

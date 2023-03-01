#pragma once

#include <cstring>
#include <fstream>
#include <iostream>

#include "IBMFDefs.h"
#include "IBMFFaceLow.h"

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

    bool initialized;
    PreamblePtr preamble;
    IBMFFaceLow *faces;
    uint32_t *bin_face_offsets;
    uint32_t *bin_face_end_offsets;

    bool load(MemoryPtr font_data, uint32_t length) {

        MemoryPtr data = font_data;

        preamble = (Preamble *)data;
        data += sizeof(Preamble);

        if (strncmp("IBMF", preamble->marker, 4) != 0) return false;
        if (preamble->bits.version != IBMF_VERSION) return false;

        bin_face_end_offsets = (uint32_t *)data;
        data += 4 * preamble->face_count;

        bin_face_end_offsets = new uint32_t[preamble->face_count];
        for (int i = 0; i < preamble->face_count; i++) {
            bin_face_end_offsets[i] =
                (i < (preamble->face_count - 1)) ? bin_face_offsets[i + 1] : length;
        }

        faces = new IBMFFaceLow[preamble->face_count];
        for (int i = 0; i < preamble->face_count; i++) {
            if (!faces[i].load(font_data + bin_face_offsets[i],
                               font_data + bin_face_end_offsets[i]))
                return false;
        }

        initialized = true;
        return true;
    }

public:
    IBMFFontLow(MemoryPtr font_data, uint32_t length) {

        initialized = load(font_data, length);
        if (!initialized) {
            std::cerr << "Font data not recognized!" << std::endl;
        }
    }

    ~IBMFFontLow() {}

    inline uint8_t get_char_set() { return preamble->bits.char_set; }
    inline bool is_initialized() { return initialized; }
};
#pragma once

#include "../Font.h"
#include "IBMFFontLow.h"

class IBMFFont {
private:
  static constexpr char const *TAG = "IBMF";

  IBMFFont                    *face;
  IBMFDefs::Glyph             *glyph_data;

public:
  IBMFFont(const std::string &filename);
  IBMFFont(unsigned char *buffer, int32_t size);
  ~IBMFFont();

  /**
   * @brief Get a glyph object
   *
   * Retrieve a glyph object and convert it to it's bitmap représentation.
   * The cache will be updated to contain the new glyph, if it's not
   * already there.
   *
   * @param charcode Character code as a unicode number.
   * @return Glyph The glyph associated to the unicode character.
   */

  IBMFDefs::Glyph *get_glyph(uint32_t charcode, int16_t glyph_size) override;

  IBMFDefs::Glyph *get_glyph(uint32_t charcode, uint32_t next_charcode, int16_t glyph_size,
                             int16_t &kern, bool &ignore_next) override;

  IBMFDefs::Glyph *adjust_ligature_and_kern(Glyph *glyph, uint16_t glyph_size,
                                            uint32_t next_charcode, int16_t &kern,
                                            bool &ignore_next);

  /**
   * @brief Face normal line height
   *
   *
   * @return int32_t Normal line height of the face in pixels
   */
  int32_t get_line_height(int16_t glyph_size) {
    if (current_font_size != glyph_size)
      set_font_size(glyph_size);
    return (face == nullptr) ? 0 : (face->get_line_height());
  }

  /**
   * @brief Face descender height
   *
   * @return int32_t The face descender height in pixels related to
   *                 the current font size.
   */
  int32_t get_descender_height(int16_t glyph_size) {
    if (current_font_size != glyph_size)
      set_font_size(glyph_size);
    return (face == nullptr) ? 0 : (face->get_descender_height());
  }

private:
  void clear_face();

  /**
   * @brief Set the font face object
   *
   * Get a font from memory loaded and ready to supply glyphs. Note
   * that the buffer will be freed when the face will be removed.
   *
   * @param buffer The buffer containing the font.
   * @param size   The buffer size in bytes.
   * @return true The font was found and retrieved.
   * @return false Some error (file not found, unsupported format).
   */
  bool set_font_face_from_memory(unsigned char *buffer, int32_t size);

  /**
   * @brief Set the font size
   *
   * Set the font size. This will be used to set various general metrics
   * required from the font structure.
   *
   * @param size The size of the glyphs in points (1/72th of an inch).
   * @return true The font was resized.
   * @return false Not able to resize the font.
   */
  bool            set_font_size(int16_t size);

  Glyph          *get_glyph_internal(uint32_t charcode, int16_t glyph_size);

  inline uint32_t translate(uint32_t charcode) { return face->translate(charcode); }
};

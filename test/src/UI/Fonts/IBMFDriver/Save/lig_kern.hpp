Font::Glyph * 
IBMF::adjust_ligature_and_kern(Glyph   * glyph, 
                               uint16_t  glyph_size, 
                               uint32_t  next_charcode, 
                               int16_t & kern,
                               bool    & ignore_next)
{
  ignore_next = false;
  kern = 0;

  if ((next_charcode != 0) && (glyph->ligature_and_kern_pgm_index != 255)) {
    const IBMFFont::LigKernStep * step = face->get_lig_kern(glyph->ligature_and_kern_pgm_index);
    while (true) {
      if (step->tag == 1) {
        if (step->next_char_code == next_charcode) {
          kern = face->get_kern(step->u.kern_idx);
          LOG_D("Kerning between %c and %c: %d", (char) glyph_data->char_code, (char) next_charcode, kern);
          break;
        }
      }
      else {
        if (step->next_char_code == next_charcode) {
          LOG_D("Ligature between %c and %c", (char) glyph_data->char_code, (char) next_charcode);
          glyph = get_glyph_internal(step->u.char_code | 0xFF00, glyph_size);
          ignore_next = true;
          break;
        }
      }
      if (step->stop == 1) break;
      step++;
    }
  }
  return glyph;
}


// 00000000 -- 0000007F: 	0xxxxxxx
// 00000080 -- 000007FF: 	110xxxxx 10xxxxxx
// 00000800 -- 0000FFFF: 	1110xxxx 10xxxxxx 10xxxxxx
// 00010000 -- 001FFFFF: 	11110xxx 10xxxxxx 10xxxxxx 10xxxxxx

int32_t 
Page::to_unicode(const char *str, CSS::TextTransform transform, bool first, const char **str2) const
{
  const uint8_t * c    = (uint8_t *) str;
  int32_t         u    = 0;
  bool            done = false;

  if (*c == '&') {
    const uint8_t * s = ++c;
    uint8_t len = 0;
    while ((len < 7) && (*s != 0) && (*s != ';')) { s++; len++; }
    if (*s == ';') {
      if      (strncmp("nbsp;",  (const char *) c, 5) == 0) u =    160;
      else if (strncmp("lt;",    (const char *) c, 3) == 0) u =     60;
      else if (strncmp("gt;",    (const char *) c, 3) == 0) u =     62;
      else if (strncmp("amp;",   (const char *) c, 4) == 0) u =     38;
      else if (strncmp("quot;",  (const char *) c, 5) == 0) u =     34;
      else if (strncmp("apos;",  (const char *) c, 5) == 0) u =     39;
      else if (strncmp("mdash;", (const char *) c, 6) == 0) u = 0x2014;
      else if (strncmp("ndash;", (const char *) c, 6) == 0) u = 0x2013;
      else if (strncmp("lsquo;", (const char *) c, 6) == 0) u = 0x2018;
      else if (strncmp("rsquo;", (const char *) c, 6) == 0) u = 0x2019;
      else if (strncmp("ldquo;", (const char *) c, 6) == 0) u = 0x201C;
      else if (strncmp("rdquo;", (const char *) c, 6) == 0) u = 0x201D;
      else if (strncmp("euro;",  (const char *) c, 5) == 0) u = 0x20AC;
      else if (strncmp("dagger;",(const char *) c, 7) == 0) u = 0x2020;
      else if (strncmp("Dagger;",(const char *) c, 7) == 0) u = 0x2021;
      else if (strncmp("copy;",  (const char *) c, 5) == 0) u =   0xa9;
      if (u == 0) {
        u = '&';
      }
      else {
        c = ++s;
      }
    }
    else {
      u = '&';
    }
    done = true;
  } 
  else while (true) {
    if ((*c & 0x80) == 0x00) {
      u = *c++;
      done = true;
    }
    else if ((*str & 0xF1) == 0xF0) {
      u = *c++ & 0x07;
      if (*c == 0) break; else u = (u << 6) + (*c++ & 0x3F);
      if (*c == 0) break; else u = (u << 6) + (*c++ & 0x3F);
      if (*c == 0) break; else u = (u << 6) + (*c++ & 0x3F);
      done = true;
    }
    else if ((*str & 0xF0) == 0xE0) {
      u = *c++ & 0x0F;
      if (*c == 0) break; else u = (u << 6) + (*c++ & 0x3F);
      if (*c == 0) break; else u = (u << 6) + (*c++ & 0x3F);
      done = true;
    }
    else if ((*str & 0xE0) == 0xC0) {
      u = *c++ & 0x1F;
      if (*c == 0) break; else u = (u << 6) + (*c++ & 0x3F);
      done = true;
    }
    break;
  }
  *str2 = (char *) c;
  if (transform != CSS::TextTransform::NONE) {
    if      (transform == CSS::TextTransform::UPPERCASE) u = toupper(u);
    else if (transform == CSS::TextTransform::LOWERCASE) u = tolower(u);
    else if (first && (transform == CSS::TextTransform::CAPITALIZE)) u = toupper(u);
  }
  return done ? u : ' ';
}

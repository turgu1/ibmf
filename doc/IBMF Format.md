# IBMF - Integrated BitMap Font Format, Version 1

This document presents the **IBMF** file format. This file's content has been designed to integrate individual METAFONT generated bitmap fonts of a single typeface style that were generated for a specific physical display resolution (DPI, or Dots Per Inch). Each individual font is of a different points in size (a point is a measure of the size of a font. There is 72.27 points per inch, per the TeX/METAFONT definition).

Each IBMF file contains a preamble followed by each individual fonts. 

Each METAFONT generated font is described using two files: a `PK` font file (the content is taylored to a physical display definition, mainly in dots per inch) and a `TFM` font file (the content are independant metrics of the glyphs, mainly in points). The specific format description of these files can be found in the `pk.pdf` and `tfm.pdf` documents. The IBMF file integrates part of the `PK` and `TFM` files in a single structure described below.

The following subsections describe each of these elements. All values are stored in little-endian order. All *fractional dots count* are a 2-bytes ones-conplement fixed point value using 6 bits fractional part. Unless mentionned otherwise, all values are unsigned numbers.

## 1. IBMF Preamble

- 4 bytes: 'IBMF'
- 1 byte: Number of individual fonts (NbrFonts).
- 1 byte:
    - 5 least significant bits: IBMF Format version number. Must be equal to 1.
    - 3 most significant bits: Character set number (see the Character Tables below).
- 2 bytes * NbrFonts: byte offset from the beginning of the file to the indifidual fonts, in the same order as the point sizes.
- 1 byte * NbrFonts: point size of the individual fonts, sorted in ascending point size.
- [optional 1 byte]: 0, present if NbrFonts is odd, to align on 2-bytes offsets.

## 2. Individual fonts format

Following the preamble, each individual fonts content are appended to the file, one after the other. Each font contains the following structures:

- Header
- Glyphs definition
- Ligature and Kerning description
- Kerning adjustements

### 2.1 Header

- 1 byte: font size in point.
- 1 byte: line height as dots count.
- 2 bytes: display resolution in dots per inch.
- 2 bytes: height of a lowercase `x` in fractional dots count.
- 2 bytes: height of an uppercase `M` in fractional dots count.
- 2 bytes: slant correction, in fractional dots count. Used to adjust accent placement on top of italic characters.
- 1 byte: descender height as dots count.
- 1 byte: space character size as dots count.
- 1 byte: glyph count. The number of glyphs that follows.
- 1 byte: ligature and kerning description count.
- 1 byte: kerning adjustment count.
- 1 byte: font format version number. Current value is 1.

### 2.2 Glyphs definition

Each glyph is defined using a fixed size glyph preamble, followed by the bitmap definition. Two formats of bitmap are used to minimize the amount of memory required: 1) a run-length encoding pattern or 2) a simple bitmap format.

The run-length encoding is the same as presented in the `pk.pdf` document and the description is reproduced below.

#### 2.2.1 Glyph preamble

- 1 byte: The character code.
- 1 byte: The bitmap width.
- 1 byte: The bitmap height.
- 1 byte: Horizontal offset to reference pixel. Two complement value.
- 1 byte: Vertical offset to reference pixel. Two complement value.
- 1 byte: packet length. That is the size in bytes of the bitmap definition that follows the preamble.
- 2 bytes: The horizontal character advance to the next character as a fractional dots count.
- 1 byte with the following bits values:
    - least significant 4 bits unsigned: the dyn_f value as described for the bitmap run-lenght encoding below. If = 14, the bitmap is stored as is, no run-lenght encoding.
    - next 1 bit: = 1 if the first value in the run-length encoding designates black pixels (= 0 if white pixels).
- 1 byte: ligature and kerning description index.

#### 2.2.2 Bitmap encoding

The bitmap size is the minimum bounding box. The minimum bounding box is the smallest rectangle which encloses all 'black' pixels of a character.

If dyn_f = 14 (in the glyph's preamble), the bitmap is stored as is. Otherwise, it is stored as described below:

For the run-length encoding, instead of packing the individual bits of the character, we instead count the number of consecutive 'black' or 'white' pixels in a horizontal raster row, and then encode this number. Run counts are found for each row, from the top of the character to the bottom. The great majority of run counts will fit in a four-bit nybble.    

When consecutive rows are the same, a repeat count is encoded to reflect the fact and minimize the amount of data to keep. The following algorithm is used to decode the information, giving back the next number of black or white pixels, intertwine with repeat counts for the current row being decoded:

``` pascal
function next_packed_num: integer;
 var i,j,k: integer;
 begin 
   i := get_nyb;
   if i = 0 then begin 
     repeat 
       j := getnyb; incr(i);
     until j != 0;
     while i > 0 do begin 
       j := j * 16 + get_nyb; 
       decr(i);
     end;
     pk_packed_num := j - 15 + (13 - dyn_f) * 16 + dyn_f;
   end
   else if i <= dyn_f then 
     pk_packed_num := i
   else if i < 14 then 
     pk_packed_num := (i - dyn_f - 1) * 16 + get_nyb + dyn_f + 1
   else begin 
     if repeat_count != 0 then abort('Extra repeat count!');
     if i = 14 then
        repeat_count := pk_packed_num
     else
        repeat_count := 1;
     send_out(true, repeat_count);
     pk_packed_num := pk_packed_num;
   end;
 end;
```

More details can be found in the `pk.pdf` document.

### 2.3 Ligature and Kerning Definition

This is a list of steps defining how to apply ligature or Kerning to glyphs. Each glyph that is associated with a Ligature or Kerning is having an index pointing at one of the step in this list. The last step in a serie will have a stop bit to 1 indicating the end of the *program*.

Ligature or kerning application depends on the glyph following the current one. Every step indicates which character code must follow the current one to apply the associated ligature or kerning. Ligature indicates a character code that will replace both the current and the following flyph. Kerning indicates an index into the Kerning Adjustments vector with a value that will be added (or subtracted) to the advance of the current glyph.

The fields are the following (They are all bit-sized fields):

- The least snigicant 7 bits represents the glyph that must follow the current one to apply the lifature or kerning change. There is a maximum of 128 glyphs in a font, so 7 bits are enough to represent the next character.
- The next bit if = 1, indicates that it is the last step in the program.

### 2.4 Kerning Adjustments
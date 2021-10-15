# IBMF (Integrated Bitmap Font) Font manipulation tools

(Updated 2021.10.13)

- [x] Some issues with glyph raster size corrected.
- [x] Issues with accent extraction corrected.
- [x] Issue when accents are larger than the character.
- [x] Accents position on Italic characters adjusted.
- [x] Fonts re-generated.


This is a tool to generate IBMF fonts.

IBMF fonts are created from the need of having better-looking fonts to use on low-resolution monochrome displays (i.e. displays with a DPI lower than 250). This is an opiniatre font format that is based on the METAFONT available toolkit that generates bitmap fonts. The format integrates multiple font point sizes in the same file. It is loosely based on the PK font format that uses run-length encoding of bitmaps to compress the information in a small characters bundle. The IBMF font format is described in the file `doc/IBMF Format.md`.

A C++ class (`ibmf_font.hpp`) that allows for the extraction of characters is provided, allowing the retrieval of glyphs from the various point bitmaps size available in the font. Please look at the `latin_example` program on how to use it.

The `fonts\gener.sh` is used to generate IBMF fonts by:

1) Generating PK fonts from the METAFONT foundry.
2) Extracting the information from the PK fonts to generate IBMFsubfonts.
3) Packing the information in IBMF fonts through PlatformIO built program named `generator`. The resulting fonts are located in the `fonts` folder.

The targetted fonts suites are:

  - Computer Modern Roman Serif
  - Computer Modern Sans-Serif
  - Typewriter

Each IBMF font contains 8, 9, 10, 12, 14, 17, and 24 points bitmaps.
Each Computer Modern font suit produces Regular, Bold, Italic, and Bold-Italic fonts. Only Regular and Italic are available for the Typewriter fonts.

To generate the METAFONT files through the `gener.sh` script, you need to have `TeXlive` installed, as well as the `sauter` package from the `sauter.zip` file available on CTAN. The `TeXlive` is usually available through packages for your operating system.

The `sauter` package must be installed manually. A copy of it is supplied in the main folder (file `sauter.zip`). The location to install it on Linux is usually `/usr/share/texlive/texmf-dist/fonts/source/public/`. After that, you must run the `mktexlsr` application from a shell to integrate it to the METAFONT search path.

The `fonts/gener.sh` content can be updated to take into account other DPI than the one generated. As it was built for some devices named `inkplate6`, `inkplate10` and `inkplate6plus`, there are files with extension `.mf` for those devices in the `fonts/` folder that can be renamed and modified to accommodate your needs. You can then modify the content of `fonts/gener.sh` accordingly. 

The `IBMFFont` class currently support the follwing characters from Latin9, ISO 8859-15 (all other characters will return a space character, no bitmap), for the all Computer Modern (Serif, SansSerif, Typewriter) fonts:

|Dec|Oct|Hex|Ch|Description
|---|---|--|-|-----------------------------------
| 33|041|21|!|Exclamation mark
| 34|042|22|"|Double quotes (or speech marks)
| 35|043|23|#|Number
| 36|044|24|$|Dollar
| 37|045|25|%|Per cent sign
| 38|046|26|&|Ampersand
| 39|047|27|'|Single quote
| 40|050|28|(|Open parenthesis (or open bracket)
| 41|051|29|)|Close parenthesis (or close bracket)
| 42|052|2A|*|Asterisk
| 43|053|2B|+|Plus
| 44|054|2C|,|Comma
| 45|055|2D|-|Hyphen
| 46|056|2E|.|Period, dot or full stop
| 47|057|2F|/|Slash or divide
| 48|060|30|0|Zero
| 49|061|31|1|One
| 50|062|32|2|Two
| 51|063|33|3|Three
| 52|064|34|4|Four
| 53|065|35|5|Five
| 54|066|36|6|Six
| 55|067|37|7|Seven
| 56|070|38|8|Eight
| 57|071|39|9|Nine
| 58|072|3A|:|Colon
| 59|073|3B|;|Semicolon
| 61|075|3D|=|Equals
| 63|077|3F|?|Question mark
| 64|100|40|@|At symbol
| 65|101|41|A|Uppercase A
| 66|102|42|B|Uppercase B
| 67|103|43|C|Uppercase C
| 68|104|44|D|Uppercase D
| 69|105|45|E|Uppercase E
| 70|106|46|F|Uppercase F
| 71|107|47|G|Uppercase G
| 72|110|48|H|Uppercase H
| 73|111|49|I|Uppercase I
| 74|112|4A|J|Uppercase J
| 75|113|4B|K|Uppercase K
| 76|114|4C|L|Uppercase L
| 77|115|4D|M|Uppercase M
| 78|116|4E|N|Uppercase N
| 79|117|4F|O|Uppercase O
| 80|120|50|P|Uppercase P
| 81|121|51|Q|Uppercase Q
| 82|122|52|R|Uppercase R
| 83|123|53|S|Uppercase S
| 84|124|54|T|Uppercase T
| 85|125|55|U|Uppercase U
| 86|126|56|V|Uppercase V
| 87|127|57|W|Uppercase W
| 88|130|58|X|Uppercase X
| 89|131|59|Y|Uppercase Y
| 90|132|5A|Z|Uppercase Z
| 91|133|5B|[|Opening bracket
| 93|135|5D|]|Closing bracket
| 94|136|5E|^|Caret - circumflex
| 96|140|60|`|Grave accent
| 97|141|61|a|Lowercase a
| 98|142|62|b|Lowercase b
| 99|143|63|c|Lowercase c
|100|144|64|d|Lowercase d
|101|145|65|e|Lowercase e
|102|146|66|f|Lowercase f
|103|147|67|g|Lowercase g
|104|150|68|h|Lowercase h
|105|151|69|i|Lowercase i
|106|152|6A|j|Lowercase j
|107|153|6B|k|Lowercase k
|108|154|6C|l|Lowercase l
|109|155|6D|m|Lowercase m
|110|156|6E|n|Lowercase n
|111|157|6F|o|Lowercase o
|112|160|70|p|Lowercase p
|113|161|71|q|Lowercase q
|114|162|72|r|Lowercase r
|115|163|73|s|Lowercase s
|116|164|74|t|Lowercase t
|117|165|75|u|Lowercase u
|118|166|76|v|Lowercase v
|119|167|77|w|Lowercase w
|120|170|78|x|Lowercase x
|121|171|79|y|Lowercase y
|122|172|7A|z|Lowercase z
|126|176|7E|~|Equivalency sign - tilde
|241|161|A1|¡|INVERTED EXCLAMATION MARK
|246|166|A6|Š|LATIN CAPITAL LETTER S WITH CARON
|250|168|A8|š|LATIN SMALL LETTER S WITH CARON
|260|176|B0|°|DEGREE SIGN
|264|180|B4|Ž|LATIN CAPITAL LETTER Z WITH CARON
|270|184|B8|ž|LATIN SMALL LETTER Z WITH CARON
|274|188|BC|Œ|LATIN CAPITAL LIGATURE OE
|275|189|BD|œ|LATIN SMALL LIGATURE OE
|276|190|BE|Ÿ|LATIN CAPITAL LETTER Y WITH DIAERESIS
|277|191|BF|¿|INVERTED QUESTION MARK
|300|192|C0|À|LATIN CAPITAL LETTER A WITH GRAVE
|301|193|C1|Á|LATIN CAPITAL LETTER A WITH ACUTE
|302|194|C2|Â|LATIN CAPITAL LETTER A WITH CIRCUMFLEX
|303|195|C3|Ã|LATIN CAPITAL LETTER A WITH TILDE
|304|196|C4|Ä|LATIN CAPITAL LETTER A WITH DIAERESIS
|305|197|C5|Å|LATIN CAPITAL LETTER A WITH RING ABOVE
|306|198|C6|Æ|LATIN CAPITAL LETTER AE
|307|199|C7|Ç|LATIN CAPITAL LETTER C WITH CEDILLA
|310|200|C8|È|LATIN CAPITAL LETTER E WITH GRAVE
|311|201|C9|É|LATIN CAPITAL LETTER E WITH ACUTE
|312|202|CA|Ê|LATIN CAPITAL LETTER E WITH CIRCUMFLEX
|313|203|CB|Ë|LATIN CAPITAL LETTER E WITH DIAERESIS
|314|204|CC|Ì|LATIN CAPITAL LETTER I WITH GRAVE
|315|205|CD|Í|LATIN CAPITAL LETTER I WITH ACUTE
|316|206|CE|Î|LATIN CAPITAL LETTER I WITH CIRCUMFLEX
|317|207|CF|Ï|LATIN CAPITAL LETTER I WITH DIAERESIS
|321|209|D1|Ñ|LATIN CAPITAL LETTER N WITH TILDE
|322|210|D2|Ò|LATIN CAPITAL LETTER O WITH GRAVE
|323|211|D3|Ó|LATIN CAPITAL LETTER O WITH ACUTE
|324|212|D4|Ô|LATIN CAPITAL LETTER O WITH CIRCUMFLEX
|325|213|D5|Õ|LATIN CAPITAL LETTER O WITH TILDE
|326|214|D6|Ö|LATIN CAPITAL LETTER O WITH DIAERESIS
|330|216|D8|Ø|LATIN CAPITAL LETTER O WITH STROKE
|331|217|D9|Ù|LATIN CAPITAL LETTER U WITH GRAVE
|332|218|DA|Ú|LATIN CAPITAL LETTER U WITH ACUTE
|333|219|DB|Û|LATIN CAPITAL LETTER U WITH CIRCUMFLEX
|334|220|DC|Ü|LATIN CAPITAL LETTER U WITH DIAERESIS
|335|221|DD|Ý|LATIN CAPITAL LETTER Y WITH ACUTE
|337|223|DF|ß|LATIN SMALL LETTER SHARP S
|340|224|E0|à|LATIN SMALL LETTER A WITH GRAVE
|341|225|E1|á|LATIN SMALL LETTER A WITH ACUTE
|342|226|E2|â|LATIN SMALL LETTER A WITH CIRCUMFLEX
|343|227|E3|ã|LATIN SMALL LETTER A WITH TILDE
|344|228|E4|ä|LATIN SMALL LETTER A WITH DIAERESIS
|345|229|E5|å|LATIN SMALL LETTER A WITH RING ABOVE
|346|230|E6|æ|LATIN SMALL LETTER AE
|347|231|E7|ç|LATIN SMALL LETTER C WITH CEDILLA
|350|232|E8|è|LATIN SMALL LETTER E WITH GRAVE
|351|233|E9|é|LATIN SMALL LETTER E WITH ACUTE
|352|234|EA|ê|LATIN SMALL LETTER E WITH CIRCUMFLEX
|353|235|EB|ë|LATIN SMALL LETTER E WITH DIAERESIS
|354|236|EC|ì|LATIN SMALL LETTER I WITH GRAVE
|355|237|ED|í|LATIN SMALL LETTER I WITH ACUTE
|356|238|EE|î|LATIN SMALL LETTER I WITH CIRCUMFLEX
|357|239|EF|ï|LATIN SMALL LETTER I WITH DIAERESIS
|361|241|F1|ñ|LATIN SMALL LETTER N WITH TILDE
|362|242|F2|ò|LATIN SMALL LETTER O WITH GRAVE
|363|243|F3|ó|LATIN SMALL LETTER O WITH ACUTE
|364|244|F4|ô|LATIN SMALL LETTER O WITH CIRCUMFLEX
|365|245|F5|õ|LATIN SMALL LETTER O WITH TILDE
|366|246|F6|ö|LATIN SMALL LETTER O WITH DIAERESIS
|370|248|F8|ø|LATIN SMALL LETTER O WITH STROKE
|371|249|F9|ù|LATIN SMALL LETTER U WITH GRAVE
|372|250|FA|ú|LATIN SMALL LETTER U WITH ACUTE
|373|251|FB|û|LATIN SMALL LETTER U WITH CIRCUMFLEX
|374|252|FC|ü|LATIN SMALL LETTER U WITH DIAERESIS
|375|253|FD|ý|LATIN SMALL LETTER Y WITH ACUTE
|377|255|FF|ÿ|LATIN SMALL LETTER Y WITH DIAERESIS
|

Available only with the Computer Modern Typewriter font:

|Dec|Oct|Hex|Ch|Description
|---|---|--|-|-----------------------------------
| 60|074|3C|<|Less than (or open angled bracket)
| 62|076|3E|>|Greater than (or close angled bracket)
| 92|134|5C|'\\'|Backslash
| 95|137|5F|_|Underscore
|123|173|7B|{|Opening brace
|124|174|7C|'\|'|Vertical bar
|125|175|7D|}|Closing brace
|

The following character are currently not supported, as the METAFONT generators doesn't supply the appropriate glyphs. It is possible that some of these will appears in the future.

|Dec|Oct|Hex|Ch|Description
|---|---|--|-|-----------------------------------
|242|162|A2|¢|CENT SIGN
|243|163|A3|£|POUND SIGN
|244|164|A4|€|EURO SIGN
|245|165|A5|¥|YEN SIGN
|247|167|A7|§|SECTION SIGN
|251|169|A9|©|COPYRIGHT SIGN
|252|170|AA|ª|FEMININE ORDINAL INDICATOR
|253|171|AB|«|LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
|254|172|AC|¬|NOT SIGN
|255|173|AD|­|SOFT HYPHEN
|256|174|AE|®|REGISTERED SIGN
|257|175|AF|¯|MACRON
|261|177|B1|±|PLUS-MINUS SIGN
|262|178|B2|²|SUPERSCRIPT TWO
|263|179|B3|³|SUPERSCRIPT THREE
|265|181|B5|µ|MICRO SIGN
|266|182|B6|¶|PILCROW SIGN
|267|183|B7|·|MIDDLE DOT
|271|185|B9|¹|SUPERSCRIPT ONE
|272|186|BA|º|MASCULINE ORDINAL INDICATOR
|273|187|BB|»|RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
|320|208|D0|Ð|LATIN CAPITAL LETTER ETH
|327|215|D7|×|MULTIPLICATION SIGN
|336|222|DE|Þ|LATIN CAPITAL LETTER THORN
|360|240|F0|ð|LATIN SMALL LETTER ETH
|367|247|F7|÷|DIVISION SIGN
|376|254|FE|þ|LATIN SMALL LETTER THORN
|
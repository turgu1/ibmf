# IBMF (Integrated Bitmap Font) Font manipulation tools

(Updated 2021.10.12)

[Still some issue with accents...]

This is a tool to generate IBMF fonts.

IBMF fonts are created from the need of having better-looking fonts to use on low-resolution monochrome displays (i.e. displays with a DPI lower than 250). This is an opiniatre font format that is based on the METAFONT available toolkit that generates bitmap fonts. The format integrates multiple font point sizes in the same file. It is loosely based on the PK font format that uses run-length encoding of bitmaps to compress the information in a small characters bundle. The IBMF font format is described in the file `doc/IBMF Format.md`.

A C++ class (`ibmf_font.hpp`) that allows for the extraction of characters is provided, allowing the retrieval of glyphs from the various point bitmaps size available in the font. Please look at the `latin1_example` program on how to use it.

The `fonts\gener.sh` is used to generate IBMF fonts by:

1) Generating PK fonts from the METAFONT foundry.
2) Extracting the information from the PK fonts to generate IBMFsubfonts.
3) Packing the information in IBMF fonts through PlatformIO built program named `generator`. The resulting fonts are located in the `fonts` folder.

To generate the METAFONT files, you need to have `TeXlive` installed, as well as the `sauter` package from the `sauter.zip` file available on CTAN.

The targetted fonts suites are:

  - Computer Modern Roman Serif
  - Computer Modern Sans-Serif
  - Typewriter

Each IBMF font contains 8, 9, 10, 12, 14, and 17 points bitmaps.
Each Computer Modern font suit produces Regular, Bold, Italic, and Bold-Italic fonts. Only Regular and Italic are available for the Typewriter fonts.



# IBMF (Integrated Bitmap Font) Font manipulation tools

(Updated 2021.09.29)

(Work in progress)

This is going to be a suite of tools to generate IBMF fonts.

IBMF fonts are created from the need of having better-looking fonts to use on low-resolution displays (i.e. displays with a DPI lower than 250). This is an opiniatre font format that is based on the METAFONT available toolkit that generates bitmap fonts. The format integrates multiple font point sizes in the same file. It is loosely based on the PK font format that uses run-length encoding of bitmaps to compress the information in a small characters bundle.

A C++ class hierarchy that allows for the extraction of characters will be provided, allowing the retrieval of glyphs from the various point bitmaps size available in the font.

The `fonts\gener.sh` will be used to generate IBMF fonts by:

1) Generating PK fonts from the METAFONT foundry.
2) Extracting the information from the PK fonts to generate IBMFsubfonts.
3) Packing the information in IBMF fonts

The targetted fonts suites are:
  - Computer Modern Roman Serif
  - Computer Modern Sans-Serif
  - Typewriter

Each font will contain 8, 9, 10, 12, and 17 points bitmaps.
Each font suit will produce Regular, Bold, Italic, and Bold-Italic fonts.


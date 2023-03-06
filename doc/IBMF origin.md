## Where does it came from (by Guy Turcotte)

The IBMF Font is composed of elements that were retrieved from the TeX/METAFONT typesetting system. This is an open source environment. You can find futher information (here)[https://en.wikipedia.org/wiki/TeX] and (here)[https://ctan.org/].

The overal IBMF Font format structure is my own definition. The individual "faces" that are part of a font are loosely based on a mix of the PK Font content (mainly the glyphs' compressed bitmap) and TFM (Font metrics) content that are produced using some of the TeX tools (mainly the `Metafont` [named `mf`] and `GFtoPK` [named `gftopk`] tools). 

The glyphs' bitmap are using the exact same compression mechanism (RLE, or Run Length Encoding) used in the Pk Font files. The ligature/kerning table and the list of kerning distance vector formats are the same as defined in the TFM files, but the internal numbers have been adapted to the DPI associated to an IBMF Font (The TFM file metrics are independant of the DPI of glyphs).

All the source code (mainly C++ and shell scripts) that is part of the font generator, the font drivers, and the font editor are my own creation. The RLEExtractor and RLEGenerator classes content are loosely based on the TeX algorithm for bitmap compression and extraction, as described in the `GFtoPK` application document that you can find (here)[https://ctan.mines-albi.fr/info/knuth-pdf/mfware/gftopk.pdf]

## Licensing

The following is a paragraph coming for the Wikipedia source indicated above:

Donald Knuth, the license owner of the main TeX suite, has indicated several times that the source code of TeX has been placed into the "public domain", and he strongly encourages modifications or experimentations with this source code. However, since Knuth highly values the reproducibility of the output of all versions of TeX, any changed version must not be called TeX, or anything confusingly similar. To enforce this rule, any implementation of the system must pass a test suite called the TRIP test before being allowed to be called TeX. The question of license is somewhat confused by the statements included at the beginning of the TeX source code, which indicate that "all rights are reserved. Copying of this file is authorized only if ... you make absolutely no changes to your copy". This restriction should be interpreted as a prohibition to change the source code as long as the file is called tex.web. The copyright note at the beginning of tex.web (and mf.web) was changed in 2021 to explicitly state this. This interpretation is confirmed later in the source code when the TRIP test is mentioned ("If this program is changed, the resulting system should not be called 'TeX'"). The American Mathematical Society tried in the early 1980s to claim a trademark for TeX. This was rejected because at the time "TEX" (all caps) was registered by Honeywell for the "Text EXecutive" text processing system.

Here is the licensing text appearing at the main TeX Web Archive (a copy can be found (here)[https://ctan.org/license/knuth]):

```
This software is copyrighted. Unlimited copying and redistribution of this package and/or its individual files are permitted as long as there are no modifications. Modifications, and redistribution of modifications, are also permitted, but only if the resulting package and/or files are renamed. 
```

1. Fonts file integration. There is various methods to use to add fonts to the application. Which one is priotitized?

    1. As a binary file loaded at boot or as needed. Requires loading from flash and processing to extract the information and rebuild an internal structure.
    2. As a header file similar to GFX (a raw bytes vector in hexadecimal notation). No loading required but the processing must be done to rebuild the internal structure. Would require an application to produce the header file (may already exists).
    3. As a binary file loaded in the application .rodata segment through platformio configuration as explained here: https://docs.platformio.org/en/latest/platforms/espressif32.html#embedding-binary-data. Again, processing must be done to rebuild the internal structure.
    4. As a header file, already processed in proper structs and vectors/arrays, ready to access. This would requires the development of an application that would read the binary file and generate the header file.

The options 2, 3 and 4 have the advantage of simplifying the releases updates as everything is in the application binary. Option 1 main advantage is the replacement of fonts is easy IF their exists a mean to replace them through updates, without having to update the main application.

2. Glyphs bitmap preprocessing or not. The IBMF Font format generally contains glyphs compressed using a RLE (Run Length Encoding) format. The glyphs can be extracted and expanded to their bitmap representation at two moment. Which one is prioritized?

    1. At load time. That means that the font will take more memory space but will be accessed faster when a glyph is required.
    2. At the moment a glyph is required. Less space in memory but a bit longuer when a glyph is required. to mitigate the processing time, a cache can be used to keep the last glyphs processed. THe length of the cache can be adjusted.

3. IBMF Driver integration. Keeping the GFX fonts or not?

    1. The GFX fonts are not used anymore. They can be regenerated as IBMF fonts with some restrictions (no european glyphs, or partial support, or regenerated through TTF/OTF).
    2. The GFX fonts are kept.

4. If the GFX are kept, what method of access is prioritized for them and the IBMF fonts?

    1. Through inheritance (Interface) using a single interface for all access. If so, is there an abstract class already available or there will be a need to create one?
    2. Through a template (Generics)
    3. Other means?

5. How are GFX fonts are selected and accessed in the application at the moment?

6. For kerning and ligature, words must be sent to methods that will replace characters and apply spacing adjustments. Where in the source code can we locate the changes to be done?

7. Which point sizes are needed to be generated for the IBMF Fonts? Normally, points are independant of the resolution (one point = 1/72 inch). The example IBMF Fonts where produced using 100 bpi and 75 bpi resolutions in 12pt and 15pt sizes.

8. Are Serif and SansSerif required? 

9. Are bold and italic required? Would be a bit more difficult to achieve. We can try generating one and adjust some glyphs to see the potential result.
 
10. Are bold-italic required? Would be more difficult...


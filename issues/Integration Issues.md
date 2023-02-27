1. Fonts file integration. There is various methods to use to add fonts to the application. Which one is priotitized?

    1. As a binary file loaded at boot or as needed. Requires loading from flash and processing to extract the information and rebuild an internal structure.
    2. As a header file similar to GFX (a raw bytes vector in hexadecimal notation). No loading required but the processing must be done to rebuild the internal structure. Would require an application to produce the header file (may already exists).
    3. As a binary file loaded in the application .rodata segment through platformio configuration as explained here: https://docs.platformio.org/en/latest/platforms/espressif32.html#embedding-binary-data. Again, processing must be done to rebuild the internal structure.
    4. As a header file, already processed in proper structs and vectors/arrays, ready to access. This would requires the development of an application that would read the binary file and generate the header file.


The options 2, 3 and 4 have the advantage of simplifying the releases updates as everything is in the application binary. Option 1 main advantage is the replacement of fonts is easy IF their exists a mean to replace them through updates, without having to update the main application.

John: My preference is to include the fonts in the application binary (as a header or .rodata) then the fonts get updated with our existing OTA flow. I think a font file on the filesystem would be more of a headache than it's worth. Currently we reserve up to 4MB for firmware and we're using <2MB so we have some margin for fonts.

2. Glyphs bitmap preprocessing or not. The IBMF Font format generally contains glyphs compressed using a RLE (Run Length Encoding) format. The glyphs can be extracted and expanded to their bitmap representation at two moment. Which one is prioritized?

    1. At load time. That means that the font will take more memory space but will be accessed faster when a glyph is required.
    2. At the moment a glyph is required. Less space in memory but a bit longuer when a glyph is required. to mitigate the processing time, a cache can be used to keep the last glyphs processed. THe length of the cache can be adjusted.

John:  I could go either way on this one depending on the load time. Seems like loading them all at boot would be the simplest to implement. I'd start with whatever is easiest to implement and we can work on performance, or partial loading + caching if necessary.

3. IBMF Driver integration. Keeping the GFX fonts or not?

    1. The GFX fonts are not used anymore. They can be regenerated as IBMF fonts with some restrictions (no european glyphs, or partial support, or regenerated through TTF/OTF).
    2. The GFX fonts are kept.

John: We want to be able to switch back to our existing Tahoma font for comparison, but I don't have a preference between converting our existing GFX fonts to IBMF or keeping GFX font support as well. Initially I'd go with whatever's simplest. I think it's a near-certainty we will drop Adafruit_GFX fonts once your bitmap font work is integrated. But for comparison sake it might be fairly simple to just add IMBF support to DisplaySystem and use a bool to switch between the Adafruit's text methods (gfxClass_->setCursor, gfxClass_->printf, gfxClass_->getTextBounds) and your new text methods for IBMF.

4. From what I can see in the project, the Style class is the location where the GFX fonts are integrated. Please confirm.

John: Yes that's where the available fonts are specified. The font family is set on the DisplaySystem's instance here:
https://github.com/sindarin-inc/embedded-app/blob/be237e3db5a65f3b199bd9f1fadb4b84532ec8fd/src/main.cpp#L391
Then you'll find a bunch of spots where getFontFamily is called to get the currently selected font family

5. If the GFX are kept, what method of access is prioritized for them and the IBMF fonts?

    1. Through inheritance (Interface) using a single interface for all access. If so, is there an abstract class already available or there will be a need to create one? The Style class could become the abstract class, the GFXfont and GFXglyph may have to change their name and potentially their content (fields).
    2. Through a template (Generics)
    3. Other means?

John: I think it'd be simple to have a bool in DisplaySystem that switches the implementation of the text-related methods between Adafruit_GFX and your new IBMF implementation

Q: Is this switching implication is in the DisplaySystem only or elsewere?

John: If we punt on ligatures and kerning (writing up the response to Q 6 now) then i think just DisplaySystem

Guy: A switch will have another impact: accessing the fields in different structures (GFX related structs are specific to GFX). I can certainly adapt the fields of IBMFFont to be similar to GFX but their will be additional fields.

Guy: My own point of view on your answer on question 5: a bool switching would require the same level of effort than using an abstract class. An abstract class would disconnect the DisplaySystem from the specific aspects of the fonts format (GFX or IBMF) and would simplify the future integration of any kind of font format (including TTF or others).

I can build a mockup of such an abstraction if you would like to check it at first

John: I'm fine with either! Keep in mind DisplaySystem is already an abstract superclass for DisplayEinkET013TT1 so you'll need to swap between them in the class definition for DisplayEinkET013TT1

Guy: I don't have too much insights on the classes for the moment :slightly_smiling_face:  I will look at them now and see how all this will fits and return with a suggestion if any. You certainly know more than me on the internals of the application!!

John: I think totally fine to start with whatever's easiest to implement and we can dial in optimal architecture as needed for performance/maintainability/testability.

6. For kerning and ligature, words must be sent to methods that will replace characters and apply spacing adjustments. Where in the source code can we locate the changes to be done? If the answer to the last question above is (1), the methods would become part of the Style class replacement.

John: The code for splitting on words and doing layout for paragraphs is in Renderer.cpp https://github.com/sindarin-inc/embedded-app/blob/main/src/Renderers/Renderer.cpp#L115
Since we don't have any code support for ligatures / kerning right now there might be some bigger changes needed. Nothing is sacred in our codebase so if you think we should throw out all of Renderer.cpp and start over that's ok too. I'd propose you stage this as phase 2 of the project -- maybe get IMBF working without ligatures/kerning and then work on handling entire words.

ligatures and kerning will mostly make text shorter right? Perhaps the simple way to do this would be apply kerning/ligatures at draw time in DisplaySystem::drawSingleLineOfText. It means the layout math in Renderer wouldn't be 100% accurate but you could silo the IBMF changes to only DisplaySystem

Guy: Don't know your processing yet, Do you compute the pages location before drawing? If so, there is a need to apply ligature and kerning before drawing as there will be an impact on the page content.

John: We don't pre-compute all the page locations like you do in your code -- do it as needed based on the current position in the readable. Paging backwards is pretty complex because of this but eliminates any full processing of the readable.

It would definitely be ideal to include kerning/ligatures when doing the text layout, but if you can hack it together by ignoring kerning/ligatures for text sizing & layout, and then applying them at draw time, I'm open to that hack at least initially

John: 
7. Which point sizes are needed to be generated for the IBMF Fonts? Normally, points are independant of the resolution (one point ~= 1/72 inch). The example IBMF Fonts where produced using 100 bpi and 75 bpi resolutions in 12pt and 14pt sizes.

John: Depends how you define an inch when it's behind magnification :slightly_smiling_face:. Once you get our simulator running you can see what we're currently using and compare against that.

8. Are Serif and SansSerif required? 

John: Whichever looks the best really! We'd be fine with one or the other. I'm personally doubtful we'll get much out of serif fonts at our low resolution.

Ben Cheff: Pretty sure just SansSerif -- the chances of a Serif font looking good is small.

9. Are bold and italic required? Would be a bit more difficult to achieve. We can try generating one and adjust some glyphs to see the potential result.



10. Are bold-italic required? Would be more difficult at the lowest point sizes...

Ans to the 2 last questions: These are all bonus. It would be very nice to have eventually. But it's ok to start without them. Probably bold would be a different font only used for headers. Not actually inline in the text. italics inline would definitely be nice.

--------

I've seen that you have integrated the complete Adafruit GFX Library. I have ported that library to ESP-IDF 2 years ago. May not be up to date with the current version but could be usefull, if you need it beyond the fonts.
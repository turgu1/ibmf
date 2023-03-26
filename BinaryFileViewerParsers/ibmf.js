const fontFormat0CharacterCodes = [
	0x0060, // `
	0x00B4, // ´
	0x02C6, // ˆ
	0x02DC, // ˜
	0x00A8, // ¨
	0x02DD, // ˝
	0x02DA, // ˚
	0x02C7, // ˇ
	0x02D8, // ˘
	0x00AF, // ¯
	0x02D9, // ˙
	0x00B8, // ¸
	0x02DB, // ˛
	0x201A, // ‚
	0x2039, // ‹
	0x203A, // ›

	0x201C, // “
	0x201D, // ”
	0x201E, // „
	0x00AB, // «
	0x00BB, // »
	0x2013, // –
	0x2014, // —
	0x00BF, // ¿
	0x2080, // ₀
	0x0131, // ı
	0x0237, // ȷ
	0xFB00, // ﬀ
	0xFB01, // ﬁ
	0xFB02, // ﬂ
	0xFB03, // ﬃ
	0xFB04, // ﬄ

	0x00A1, // ¡
	0x0021, // !
	0x0022, // "
	0x0023, // #
	0x0024, // $
	0x0025, // %
	0x0026, // &
	0x2019, // ’
	0x0028, // (
	0x0029, // )
	0x002A, // *
	0x002B, // +
	0x002C, // ,
	0x002D, // .
	0x002E, // -
	0x002F, // /

	0x0030, // 0
	0x0031, // 1
	0x0032, // 2
	0x0033, // 3
	0x0034, // 4
	0x0035, // 5
	0x0036, // 6
	0x0037, // 7
	0x0038, // 8
	0x0039, // 9
	0x003A, // :
	0x003B, // ;
	0x003C, // <
	0x003D, // =
	0x003E, // >
	0x003F, // ?

	0x0040, // @
	0x0041, // A
	0x0042, // B
	0x0043, // C
	0x0044, // D
	0x0045, // E
	0x0046, // F
	0x0047, // G
	0x0048, // H
	0x0049, // I
	0x004A, // J
	0x004B, // K
	0x004C, // L
	0x004D, // M
	0x004E, // N
	0x004F, // O

	0x0050, // P
	0x0051, // Q
	0x0052, // R
	0x0053, // S
	0x0054, // T
	0x0055, // U
	0x0056, // V
	0x0057, // W
	0x0058, // X
	0x0059, // Y
	0x005A, // Z
	0x005B, // [
	0x005C, // \ .
	0x005D, // ]
	0x005E, // ^
	0x005F, // _

	0x2018, // ‘
	0x0061, // a
	0x0062, // b
	0x0063, // c
	0x0064, // d
	0x0065, // e
	0x0066, // f
	0x0067, // g
	0x0068, // h
	0x0069, // i
	0x006A, // j
	0x006B, // k
	0x006C, // l
	0x006D, // m
	0x006E, // n
	0x006F, // o

	0x0070, // p
	0x0071, // q
	0x0072, // r
	0x0073, // s
	0x0074, // t
	0x0075, // u
	0x0076, // v
	0x0077, // w
	0x0078, // x
	0x0079, // y
	0x007A, // z
	0x007B, // {
	0x007C, // |
	0x007D, // }
	0x007E, // ~
	0x013D, // Ľ

	0x0141, // Ł
	0x014A, // Ŋ
	0x0132, // Ĳ
	0x0111, // đ
	0x00A7, // §
	0x010F, // ď
	0x013E, // ľ
	0x0142, // ł
	0x014B, // ŋ
	0x0165, // ť
	0x0133, // ĳ
	0x00A3, // £
	0x00C6, // Æ
	0x00D0, // Ð
	0x0152, // Œ
	0x00D8, // Ø

	0x00DE, // Þ
	0x1E9E, // ẞ
	0x00E6, // æ
	0x00F0, // ð
	0x0153, // œ
	0x00F8, // ø
	0x00FE, // þ
	0x00DF, // ß
	0x00A2, // ¢
	0x00A4, // ¤
	0x00A5, // ¥
	0x00A6, // ¦
	0x00A9, // ©
	0x00AA, // ª
	0x00AC, // ¬
	0x00AE, // ®

	0x00B1, // ±
	0x00B2, // ²
	0x00B3, // ³
	0x00B5, // µ
	0x00B6, // ¶
	0x00B7, // ·
	0x00B9, // ¹
	0x00BA, // º
	0x00D7, // ×
	0x00BC, // ¼
	0x00BD, // ½
	0x00BE, // ¾
	0x00F7, // ÷
	0x20AC  // €
];

registerFileType((fileExt, filePath, fileData) => {
	if (fileExt == 'ibmf') {
		const headerArray = fileData.getBytesAt(0, 4);
		const header = String.fromCharCode(...headerArray)
		if (header == 'IBMF')
			return true;
	}
	return false;
});

var planes = [];
var codePointBundles = [];
var isUTF32 = false;

function decimalToHex(d, padding) {
	var hex = Number(d).toString(16);
	padding = typeof (padding) === "undefined" || padding === null ? padding = 2 : padding;
	while (hex.length < padding) hex = "0" + hex;
	return hex;
}

function fix16(v) {
	f = v / 64.0;
	return f.toString();
}

function utf32(glyphCode) {
	dbgLog("utf32... glyphCode: ", glyphCode);
	let codePoint = 0;
	if (isUTF32) {
		let i = 0;
		while (i < 3) {
			if (planes[i+1].firstGlyphCode > glyphCode) break;
			i += 1;
		}
		if (i < 4) {
			let planeMask = i << 16;
			let bundleIdx = planes[i].codePointBundlesIdx;
			let idx = planes[i].firstGlyphCode;
			let entriesCount = planes[i].entriesCount;
			let j = 0;
			while (j < entriesCount) {
				if (idx + (codePointBundles[bundleIdx].size) > glyphCode) break;
				idx += codePointBundles[bundleIdx].size;
				bundleIdx += 1;
				j += 1;
			}
			if (j < entriesCount) {
				codePoint = (codePointBundles[bundleIdx].first + (glyphCode - idx)) | planeMask;
			}
		}
	}
	else {
		dbgLog("table size: ", fontFormat0CharacterCodes.length);
		if (glyphCode < fontFormat0CharacterCodes.length) {
			codePoint = fontFormat0CharacterCodes[glyphCode];
		}
	}
	dbgLog("CodePoint:", codePoint);
	return "U+" + decimalToHex(codePoint, 5) + "  [" + String.fromCodePoint(codePoint) + "](" + codePoint.toString() + ")";
}

registerParser(() => {
	var faceCount = 0;
	var fontFormat = 0;

	addStandardHeader();

	readRowWithDetails('PREAMBLE', () => {
		read(4);
		addRow('Marker', getStringValue(), 'Must be the string "IBMF"');
		read(1);
		faceCount = getNumberValue();
		addRow('Face Count', faceCount, 'How many face there is in the file');
		readBits(5);
		addRow('File Version', getNumberValue(), 'Font structure version number');
		readBits(3);
		fontFormat = getNumberValue()
		addRow("Font Format", fontFormat == 0 ? "LATIN" : (fontFormat == 1 ? "UTF32" : "ERROR"), '0 = LATIN, 1 = UTF32');
		return {description: 'IBMF Preamble'};
	});

	let fillerCount = (4 - (6 + faceCount) & 3) & 3;
	read(faceCount + fillerCount);
	addRow('POINT SIZES', ' ', 'Each Face Point Size');
	addDetails(() => {
		for (let i = 0; i < faceCount; i++) {
			read(1); addRow("Face " + (i+1).toString(), getNumberValue(), 'Point size');
		}
		while (fillerCount > 0) {
			read(1); addRow('Filler', getNumberValue(), 'Make it 32bits padded');
			fillerCount -= 1;
		}
	});

	read(4 * faceCount);
	addRow('FACES OFFSETS', ' ', 'Faces Offsets');
	addDetails(() => {
		for (let i = 0; i < faceCount; i++) {
			read(4); addRow("Face " + (i+1), getNumberValue(), 'Offset from the beginning of the file');
		}
	});

	if (fontFormat == 1) {
		isUTF32 = true;
		readRowWithDetails('PLANES Info', () => {
			for (let i = 0; i < 4; i++) {
				readRowWithDetails("PLANE " + i.toString(), () => {
					read(2); addRow("codePointBundlesIdx", idx = getNumberValue(), 'Index of the first codePoint Bundle');
					read(2); addRow("entriesCount", count = getNumberValue(), "The number of bundle entries");
					read(2); addRow("firstGlyphCode", code = getNumberValue(), "The first glyph Code corresponding to the first bundle in the list");
					planes.push({codePointBundlesIdx: idx, entriesCount: count, firstGlyphCode: code});
					return {description: "A Plane"};
				});
			}
			return {description: "Plane headers"};
		});
		readRowWithDetails('PLANES CodePoints', () => {
			for (let i = 0; i < 4; i++) {
				readRowWithDetails('PLANES ' + i.toString(), () => {
					for (let j = 0; j < planes[i].entriesCount; j++) {
						read(2); let first = getNumberValue();
						read(2); let last = getNumberValue();
						addRow("firstCodePoint", "U+" + decimalToHex(first, 4), "First UTF16 CodePoint");
						addRow("lastCodePoint", "U+" + decimalToHex(last, 4), "Last UTF16 CodePoint");
						addRow("Length", last - first + 1, "(Computed) Number of codePoints");
						codePointBundles.push({first: first, last: last, size: (last - first + 1)}); 
					}
				});
			}
		});
	}

	readRowWithDetails('FACES Info', () => {
		for (let i = 0; i < faceCount; i++) {
			readRowWithDetails('FACE ' + (i+1).toString(), () => {
				var glyphCount;
				var poolSize;
				var ligKernCount;
				readRowWithDetails('HEADER', () => {
					read(1); addRow("pointSize", getNumberValue(), "In points (pt) a point is 1 / 72.27 of an inch");
					read(1); addRow("lineHeight", getNumberValue(), "In pixels");
					read(2); addRow("dpi", getNumberValue(), "Pixels per inch");
					read(2); addRow("xHeight", fix16(getNumberValue()), "Hight of character 'x' in pixels");
					read(2); addRow("emHeight", fix16(getNumberValue()), "em size in pixels");
					read(2); addRow("slantCorrection", fix16(getNumberValue()), "When an italic face");
					read(1); addRow("descenderHeight", getNumberValue(), "The height of the descending below the origin");
					read(1); addRow("spaceSize", getNumberValue(), "Size of a space character in pixels");
					read(2); addRow("glyphCount", glyphCount = getNumberValue(), "Must be the same for all face");
					read(2); addRow("ligKernStepCount", ligKernCount = getNumberValue(), "Length of the Ligature/Kerning table");
					read(4); addRow("pixelsPoolSize", poolSize = getNumberValue(), "Size of the Pixels Pool");
					read(1); addRow("maxHight", getNumberValue(), "The maximum hight in pixels of every glyph in the face");
					read(3); addRow("filler", getNumberValue(), "To keep the struct to be at a frontier of 32 bits");
					return {};
				});

				readRowWithDetails("GLYPHS PIXELS POOL INDEXES", () => {
					for (let j = 0; j < glyphCount; j++) {
						read(4); addRow(j.toString() + " Index", getNumberValue());
					}
				});

				readRowWithDetails("GLYPHS INFO", () => {
					for (let j = 0; j < glyphCount; j++) {
						readRowWithDetails("GlyphCode " + j.toString(), () => {
							read(1); addRow("bitmapWidth", getNumberValue(), "Width of bitmap once decompressed");
							read(1); addRow("bitmapHeight", getNumberValue(), "Height of bitmap once decompressed");
							read(1); addRow("horizontalOffset", getSignedNumberValue(), "Horizontal offset from the orign");
							read(1); addRow("verticalOffset", getSignedNumberValue(), "Vertical offset from the origin");
							read(2); addRow("packetLength", getNumberValue(), "Length of the compressed bitmap");
							read(2); addRow("advance", fix16(getNumberValue()), "Normal advance to the next glyph position in line");
							read(1); addRow("rleMetrics", "RLE Compression information");
							addDetails(() => {
								readBits(4); addRow("dynF", getNumberValue(), "Compression factor");
								readBits(1); addRow("firstIsBlack", getNumberValue() == 1 ? "YES" : "NO", "First pixels in compressed format are black");
								readBits(3); addRow("filler", getNumberValue(), "To keep it at byte boundary");
							})
							read(1); addRow("ligKernPgmIndex", (val = getNumberValue()) == 255 ? "NONE" : val);
							let info = (val != 255) && (val >= ligKernCount) ? 
								" LigKern: INDEX OUT OF BOUNDS! (" + val.toString() + ")" : 
								((val != 255) ? " LigKernIdx: " + val.toString() : "");
							return {description: utf32(j) + info };
						});
					}
					return {};
				});
				
				read(poolSize);
				addRow('PIXELS POOL');
				addDetails(() => {
					read(poolSize);
					addMemDump();
				});

				readRowWithDetails("LIGATURE AND KERN", () => {
					for (let i = 0; i < ligKernCount; i++) {
						read(2); a = getNumberValue();
						read(2); b = getNumberValue();
						let descrip = "";
						let value = "";
						if ((b & 0x8000) == 0x8000) { // kern or goto
							if ((b & 0xC000) == 0xC000) { // goto
								descrip = "GOTO " + (b & 0x3FFF).toString();
								value = "GOTO";
							}
							else { // kern
								descrip = "NEXT: " + utf32(a & 0x7FFF);
								k = (b & 0x3FFF);
								if (k & 0x2000) k |= 0xFFFFC000;
								descrip += " KERN: " + fix16(k) + " (" + decimalToHex(b & 0x3FFF, 4) + ")";
								if ((a & 0x8000) != 0) descrip += " STOP";
								value = "KERN";
							}
						}
						else {
							descrip = "NEXT: " + utf32(a & 0x7FFF);
							descrip += " LIGATURE: " + utf32(b);
							if ((a & 0x8000) != 0) descrip += " STOP";
							value = "LIG";
						}
						addRow(i.toString(), value, descrip);
					}
				});
				return {};
			});
		}
		return {};
	});

});

/*
  Copyright 2008 Brad Hards <bradh@frogmouth.net>
  Copyright 2009-2011 Inge Wallin <inge@lysator.liu.se>
  Copyright 2015 Ion Vasilief <ion_vasilief@yahoo.fr>
*/

#ifndef EMFENUMS_H
#define EMFENUMS_H

/**
   \file

   Enumerations used in various parts of EMF files
*/

/**
   Namespace for Enhanced Metafile (EMF) classes
*/
namespace QEmf
{

	/**
	   Background fill mode
	See [MS-EMF] Section 2.1.4
	*/
	enum BackgroundMode {
		TRANSPARENT = 0x01, ///< Equivalent to Qt::TransparentMode
		OPAQUE      = 0x02  ///< Equivalent to Qt::OpaqueMode
	};

	/**
	   Parameters for text output.

	   See [MS-EMF] Section 2.1.11
	*/
	enum TextOutOptions {
		//ETO_OPAQUE            = 0x000002,    // Already defined in WmfEnums.h
		//ETO_CLIPPED           = 0x000004,
		//ETO_GLYPH_INDEX       = 0x000010,
		//ETO_RTLREADING        = 0x000080,
		ETO_NO_RECT           = 0x000100,
		ETO_SMALL_CHARS       = 0x000200,
		//ETO_NUMERICSLOCAL     = 0x000400,
		//ETO_NUMERICSLATIN     = 0x000800,
		ETO_IGNORELANGUAGE    = 0x001000,
		//ETO_PDY               = 0x002000,
		ETO_REVERSE_INDEX_MAP = 0x010000
	};

	/**
	   Graphics mode, used to interpret shape data such as rectangles

	   See [MS-EMF] Section 2.1.16
	*/
	enum GraphicsMode {
		GM_COMPATIBLE = 0x01,
		GM_ADVANCED   = 0x02
	};

	/**
	   MapModes

	   See [MS-EMF] Section 2.1.21
	*/
	typedef enum {
		MM_TEXT        = 0x01,
		MM_LOMETRIC    = 0x02,
		MM_HIMETRIC    = 0x03,
		MM_LOENGLISH   = 0x04,
		MM_HIENGLISH   = 0x05,
		MM_TWIPS       = 0x06,
		MM_ISOTROPIC   = 0x07,
		MM_ANISOTROPIC = 0x08
	} MapMode;

	/**
	   World Transform modification modes

	   See [MS-EMF] Section 2.1.24
	*/
	enum ModifyWorldTransformMode {
		MWT_IDENTITY            = 0x01,
		MWT_LEFTMULTIPLY        = 0x02,
		MWT_RIGHTMULTIPLY       = 0x03,
		MWT_SET                 = 0x04
	};

	/**
	   Pen Styles

	   See [MS-EMF] Section 2.1.25
	*/
	enum PenStyle {
	PS_COSMETIC      = 0x00000000,
	PS_ENDCAP_ROUND  = 0x00000000,
	PS_JOIN_ROUND    = 0x00000000,
	PS_SOLID         = 0x00000000,
	PS_DASH          = 0x00000001,
	PS_DOT           = 0x00000002,
	PS_DASHDOT       = 0x00000003,
	PS_DASHDOTDOT    = 0x00000004,
	PS_NULL          = 0x00000005,
	PS_INSIDEFRAME   = 0x00000006,
	PS_USERSTYLE     = 0x00000007,
	PS_ALTERNATE     = 0x00000008,
	PS_STYLE_MASK    = 0x0000000f,
	PS_ENDCAP_SQUARE = 0x00000100,
	PS_ENDCAP_FLAT   = 0x00000200,
	PS_ENDCAP_MASK   = 0x00000f00,
	PS_JOIN_BEVEL    = 0x00001000,
	PS_JOIN_MITER    = 0x00002000,
	PS_JOIN_MASK     = 0x0000f000,
	PS_GEOMETRIC     = 0x00010000,
	PS_TYPE_MASK     = 0x000f0000
	};

	/**
	   Stock Objects

	   See [MS-EMF] Section 2.1.31
	*/
	enum StockObject {
	WHITE_BRUSH	= 0x80000000,
	LTGRAY_BRUSH	= 0x80000001,
	GRAY_BRUSH	= 0x80000002,
	DKGRAY_BRUSH	= 0x80000003,
	BLACK_BRUSH	= 0x80000004,
	NULL_BRUSH	= 0x80000005,
	WHITE_PEN	= 0x80000006,
	BLACK_PEN	= 0x80000007,
	NULL_PEN	= 0x80000008,
	OEM_FIXED_FONT	= 0x8000000A,
	ANSI_FIXED_FONT	= 0x8000000B,
	ANSI_VAR_FONT	= 0x8000000C,
	SYSTEM_FONT	= 0x8000000D,
	DEVICE_DEFAULT_FONT = 0x8000000E,
	DEFAULT_PALETTE = 0x8000000F,
	SYSTEM_FIXED_FONT = 0x80000010,
	DEFAULT_GUI_FONT = 0x80000011,
	DC_BRUSH	= 0x80000012,
	DC_PEN		= 0x80000013
	};

	/**
	   Fill mode

	   See [MS-EMF] Section 2.1.27
	*/
	enum PolygonFillMode {
		ALTERNATE = 0x01, ///< Equivalent to Qt::OddEvenFill
		WINDING   = 0x02  ///< Equivalent to Qt::WindingFill
	};

	/**
	  Clipping region mode

	  See [MS-EMF] Section 2.1.29
	*/
	enum RegionMode {
		RGN_AND = 0x01,   ///< Equivalent to Qt::IntersectClip
		RGN_OR = 0x02,    ///< Equivalent to Qt::UniteClip
		RGN_XOR = 0x03,
		RGN_DIFF = 0x04,
		RGN_COPY = 0x05   ///< Equivalent to Qt::ReplaceClip
	};

	/**
	   Comment type as defined for the EMR_COMMENT record.

	   See [MS-EMF] section 2.3.3
	 */
	enum CommentType {
		EMR_COMMENT_EMFSPOOL = 0x00000000,
		EMR_COMMENT_EMFPLUS  = 0x2B464D45, // The string "EMF+"
		EMR_COMMENT_PUBLIC   = 0x43494447,

		// The following value is not defined in [MS-EMF].pdf, but
		// according to google it means that the file was created by
		// Microsoft Graph.  It is present in one test file
		// (Presentation_tips.ppt).
		EMR_COMMENT_MSGR     = 0x5247534d // The string MSGR
	};

	/**
	   WMF 2.1.1.3 BitCount Enumeration

	   The BitCount Enumeration specifies the number of bits that define
	   each pixel and the maximum number of colors in a device-independent
	   bitmap (DIB).
	*/
	enum WmfBitCount {
		BI_BITCOUNT_0 = 0x0000,
		BI_BITCOUNT_1 = 0x0001,
		BI_BITCOUNT_2 = 0x0004,
		BI_BITCOUNT_3 = 0x0008,
		BI_BITCOUNT_4 = 0x0010,
		BI_BITCOUNT_5 = 0x0018,
		BI_BITCOUNT_6 = 0x0020
	};

	/**
	   MS-WMF 2.1.1.7 Compression Enumeration

	   The Compression Enumeration specifies the type of compression for a
	   bitmap image.
	*/
	enum WmfCompression {
		BI_RGB       = 0x0000,
		BI_RLE8      = 0x0001,
		BI_RLE4      = 0x0002,
		BI_BITFIELDS = 0x0003,
		BI_JPEG      = 0x0004,
		BI_PNG       = 0x0005,
		BI_CMYK      = 0x000B,
		BI_CMYKRLE8  = 0x000C,
		BI_CMYKRLE4  = 0x000D
	};

	/**
	   MS-WMF 2.1.1.4 BrushStyle Enumeration

	   The BrushStyle Enumeration specifies the different possible brush
	   types that can be used in graphics operations. For more
	   information, see the specification of the Brush Object (section 2.2.1.1).
	*/
	enum WmfBrushStyle {
		BS_SOLID         = 0x0000,
		BS_NULL          = 0x0001,
		BS_HATCHED       = 0x0002,
		BS_PATTERN       = 0x0003,
		BS_INDEXED       = 0x0004,
		BS_DIBPATTERN    = 0x0005,
		BS_DIBPATTERNPT  = 0x0006,
		BS_PATTERN8X8    = 0x0007,
		BS_DIBPATTERN8X8 = 0x0008,
		BS_MONOPATTERN   = 0x0009
	};

	/**
	   MS-WMF 2.1.1.12 HatchStyle Enumeration

	   The HatchStyle Enumeration specifies the hatch pattern.
	*/
	enum WmfHatchStyle {
		HS_HORIZONTAL = 0x0000,
		HS_VERTICAL   = 0x0001,
		HS_FDIAGONAL  = 0x0002,
		HS_BDIAGONAL  = 0x0003,
		HS_CROSS      = 0x0004,
		HS_DIAGCROSS  = 0x0005
	};

	/**
	   MS-WMF 2.1.1.13 Layout Enumeration

	   The Layout Enumeration defines options for controlling the
	   direction in which text and graphics are drawn.
	*/
	enum WmfLayout {
		LAYOUT_LTR = 0x0000,
		LAYOUT_RTL = 0x0001,
		LAYOUT_BTT = 0x0002,
		LAYOUT_VBH = 0x0004,
		LAYOUT_BITMAPORIENTATIONPRESERVED = 0x0008
	};

	/**
	   MS-WMF 2.1.2.3 TextAlignmentMode Flags

	   TextAlignmentMode Flags specify the relationship between a
	   reference point and a bounding rectangle, for text alignment. These
	   flags can be combined to specify multiple options, with the
	   restriction that only one flag can be chosen that alters the
	   drawing position in the playback device context.

	   Horizontal text alignment is performed when the font has a
	   horizontal default baseline.
	*/

	#define TA_NOUPDATECP 0x0000  /// Do not update Current Point (default)
	#define TA_LEFT       0x0000  /// The reference point is on the left edge of the bounding rectangle
	#define TA_TOP        0x0000  /// The reference point is on the top edge of the bounding rectangle
	#define TA_UPDATECP   0x0001  /// Use Current Point. The Current Point must be updated
	#define TA_RIGHT      0x0002  /// The reference point is on the right edge of the bounding rectangle
	#define TA_CENTER     0x0006  /// The reference point is at the center of the bounding rectangle
	#define TA_BOTTOM     0x0008  /// The reference point is on the bottom edge of the bounding rectangle
	#define TA_BASELINE   0x0018  /// The reference point is on the baseline

	// Some useful masks, not part of the specification:
	#define TA_HORZMASK 0x0006
	#define TA_VERTMASK 0x0018
}


#endif

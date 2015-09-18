/** \class QgsRaster
 */

class QgsRaster
{
%TypeHeaderCode
#include <qgsraster.h>
%End

  public:
    // This is modified copy of GDALColorInterp
    enum ColorInterpretation
    {
      UndefinedColorInterpretation,
      /** Greyscale */                                      GrayIndex,
      /** Paletted (see associated color table) */          PaletteIndex, // indexed color table
      /** Red band of RGBA image */                         RedBand,
      /** Green band of RGBA image */                       GreenBand,
      /** Blue band of RGBA image */                        BlueBand,
      /** Alpha (0=transparent, 255=opaque) */              AlphaBand,
      /** Hue band of HLS image */                          HueBand,
      /** Saturation band of HLS image */                   SaturationBand,
      /** Lightness band of HLS image */                    LightnessBand,
      /** Cyan band of CMYK image */                        CyanBand,
      /** Magenta band of CMYK image */                     MagentaBand,
      /** Yellow band of CMYK image */                      YellowBand,
      /** Black band of CMLY image */                       BlackBand,
      /** Y Luminance */                                    YCbCr_YBand,
      /** Cb Chroma */                                      YCbCr_CbBand,
      /** Cr Chroma */                                      YCbCr_CrBand,
      /** Continuous palette, QGIS addition, GRASS */       ContinuousPalette,
    };

    enum IdentifyFormat
    {
      IdentifyFormatUndefined,
      IdentifyFormatValue,
      IdentifyFormatText,
      IdentifyFormatHtml,
      IdentifyFormatFeature,
    };

    // Progress types
    enum RasterProgressType
    {
      ProgressHistogram,
      ProgressPyramids,
      ProgressStatistics,
    };

    enum RasterBuildPyramids
    {
      PyramidsFlagNo,
      PyramidsFlagYes,
      PyramidsCopyExisting,
    };

    enum RasterPyramidsFormat
    {
      PyramidsGTiff,
      PyramidsInternal,
      PyramidsErdas,
    };

    /** \brief Contrast enhancement limits */
    enum ContrastEnhancementLimits
    {
      ContrastEnhancementNone,
      ContrastEnhancementMinMax,
      ContrastEnhancementStdDev,
      ContrastEnhancementCumulativeCut
    };

    /** \brief This enumerator describes the different kinds of drawing we can do */
    enum DrawingStyle
    {
      UndefinedDrawingStyle,
      SingleBandGray,                 // a single band image drawn as a range of gray colors
      SingleBandPseudoColor,          // a single band image drawn using a pseudocolor algorithm
      PalettedColor,                  // a "Palette" image drawn using color table
      PalettedSingleBandGray,         // a "Palette" layer drawn in gray scale
      PalettedSingleBandPseudoColor,  // a "Palette" layerdrawn using a pseudocolor algorithm
      PalettedMultiBandColor,         // currently not supported
      MultiBandSingleBandGray,        // a layer containing 2 or more bands, but a single band drawn as a range of gray colors
      MultiBandSingleBandPseudoColor, // a layer containing 2 or more bands, but a single band drawn using a pseudocolor algorithm
      MultiBandColor,                 // a layer containing 2 or more bands, mapped to RGB color space. In the case of a multiband with only two bands, one band will be mapped to more than one color.
      SingleBandColorDataStyle        // ARGB values rendered directly
    };

    static QString contrastEnhancementLimitsAsString( QgsRaster::ContrastEnhancementLimits theLimits );
    static ContrastEnhancementLimits contrastEnhancementLimitsFromString( QString theLimits );

    /** Get value representable by given data type.
     * Supported are numerical types Byte, UInt16, Int16, UInt32, Int32, Float32, Float64.
     * @param value
     * @param dataType
     * @note added in version 2.1 */
    static double representableValue( double value, QGis::DataType dataType );
};


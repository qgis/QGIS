/*! \class QgsRaster
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
      UndefinedColorInterpretation = 0,
      /*! Greyscale */                                      GrayIndex = 1,
      /*! Paletted (see associated color table) */          PaletteIndex = 2, // indexed color table
      /*! Red band of RGBA image */                         RedBand = 3,
      /*! Green band of RGBA image */                       GreenBand = 4,
      /*! Blue band of RGBA image */                        BlueBand = 5,
      /*! Alpha (0=transparent, 255=opaque) */              AlphaBand = 6,
      /*! Hue band of HLS image */                          HueBand = 7,
      /*! Saturation band of HLS image */                   SaturationBand = 8,
      /*! Lightness band of HLS image */                    LightnessBand = 9,
      /*! Cyan band of CMYK image */                        CyanBand = 10,
      /*! Magenta band of CMYK image */                     MagentaBand = 11,
      /*! Yellow band of CMYK image */                      YellowBand = 12,
      /*! Black band of CMLY image */                       BlackBand = 13,
      /*! Y Luminance */                                    YCbCr_YBand = 14,
      /*! Cb Chroma */                                      YCbCr_CbBand = 15,
      /*! Cr Chroma */                                      YCbCr_CrBand = 16,
      /*! Continuous palette, QGIS addition, GRASS */       ContinuousPalette = 17
    };
    enum IdentifyFormat
    {
      IdentifyFormatUndefined = 0,
      IdentifyFormatValue     = 1, // numerical pixel value
      IdentifyFormatText      = 0x2, // WMS text
      IdentifyFormatHtml      = 0x4, // WMS HTML
      IdentifyFormatFeature   = 0x8  // WMS GML -> feature
    };

    // Progress types
    enum RasterProgressType
    {
      ProgressHistogram = 0,
      ProgressPyramids  = 1,
      ProgressStatistics = 2
    };

    enum RasterBuildPyramids
    {
      PyramidsFlagNo = 0,
      PyramidsFlagYes = 1,
      PyramidsCopyExisting = 2
    };

    enum RasterPyramidsFormat
    {
      PyramidsGTiff = 0,
      PyramidsInternal = 1,
      PyramidsErdas = 2
    };

    /** \brief Contrast enhancement limits */
    enum ContrastEnhancementLimits
    {
      ContrastEnhancementNone,
      ContrastEnhancementMinMax,
      ContrastEnhancementStdDev,
      ContrastEnhancementCumulativeCut
    };

    static QString contrastEnhancementLimitsAsString( QgsRaster::ContrastEnhancementLimits theLimits );
    static ContrastEnhancementLimits contrastEnhancementLimitsFromString( QString theLimits );

};


/***************************************************************************
              qgsraster.h - Raster namespace
     --------------------------------------
    Date                 : Apr 2013
    Copyright            : (C) 2013 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTER_H
#define QGSRASTER_H

#include <QString>

#include "qgis.h"

/** \ingroup core
 * Raster namespace.
 */
class CORE_EXPORT QgsRaster
{
  public:
    // This is modified copy of GDALColorInterp
    enum ColorInterpretation
    {
      UndefinedColorInterpretation = 0,
      /** Greyscale */                                      GrayIndex = 1,
      /** Paletted (see associated color table) */          PaletteIndex = 2, // indexed color table
      /** Red band of RGBA image */                         RedBand = 3,
      /** Green band of RGBA image */                       GreenBand = 4,
      /** Blue band of RGBA image */                        BlueBand = 5,
      /** Alpha (0=transparent, 255=opaque) */              AlphaBand = 6,
      /** Hue band of HLS image */                          HueBand = 7,
      /** Saturation band of HLS image */                   SaturationBand = 8,
      /** Lightness band of HLS image */                    LightnessBand = 9,
      /** Cyan band of CMYK image */                        CyanBand = 10,
      /** Magenta band of CMYK image */                     MagentaBand = 11,
      /** Yellow band of CMYK image */                      YellowBand = 12,
      /** Black band of CMLY image */                       BlackBand = 13,
      /** Y Luminance */                                    YCbCr_YBand = 14,
      /** Cb Chroma */                                      YCbCr_CbBand = 15,
      /** Cr Chroma */                                      YCbCr_CrBand = 16,
      /** Continuous palette, QGIS addition, GRASS */       ContinuousPalette = 17
    };

    enum IdentifyFormat
    {
      IdentifyFormatUndefined =      0,
      IdentifyFormatValue     =      1, // numerical pixel value
      IdentifyFormatText      = 1 << 1, // WMS text
      IdentifyFormatHtml      = 1 << 2, // WMS HTML
      IdentifyFormatFeature   = 1 << 3, // WMS GML/JSON -> feature
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
    static ContrastEnhancementLimits contrastEnhancementLimitsFromString( const QString& theLimits );

    /** Check if the specified value is representable in the given data type.
     * Supported are numerical types Byte, UInt16, Int16, UInt32, Int32, Float32, Float64.
     * @param value
     * @param dataType
     * @note added in version 2.16
     *  @note not available in Python bindings */
    static bool isRepresentableValue( double value, QGis::DataType dataType );

    /** Get value representable by given data type.
     * Supported are numerical types Byte, UInt16, Int16, UInt32, Int32, Float32, Float64.
     * This is done through C casting, so you have to be sure that the provided value is
     * representable in the output data type. This can be checked with isRepresentableValue().
     * @param value
     * @param dataType
     * @note added in version 2.1 */
    static double representableValue( double value, QGis::DataType dataType );
};

#endif



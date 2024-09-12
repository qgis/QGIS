/***************************************************************************
  qgselevationmap.h
  --------------------------------------
  Date                 : August 2022
  Copyright            : (C) 2022 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSELEVATIONMAP_H
#define QGSELEVATIONMAP_H

#include "qgis.h"
#include "qgis_sip.h"

#include <QImage>
#include <memory>

class QgsRasterBlock;

/**
 * \ingroup core
 * \brief Stores digital elevation model in a raster image which may get updated
 * as a part of map layer rendering process.
 *
 * Afterwards the elevations can be used for post-processing effects of the
 * rendered color map image.
 *
 * Elevations are encoded as colors in QImage, thanks to this it is not
 * only possible to set elevation for each pixel, but also to use QPainter
 * for more complex updates of elevations. We encode elevations to 24 bits
 * in range of [-7900, 8877] with precision of three decimal digits, which
 * should give millimiter precision and enough range for elevation values
 * in meters.
 *
 * \since QGIS 3.28
 */
class CORE_EXPORT QgsElevationMap
{
  public:

    QgsElevationMap() = default;

    //! Constructs an elevation map with the given width and height
    explicit QgsElevationMap( const QSize &size, float devicePixelRatio = 1.0 );

    /**
     * Constructs an elevation map from an existing raw elevation \a image.
     * The image must have ARGB32 format and obtained by the rawElevationImage() method.
     *
     * \see rawElevationImage()
     * \since QGIS 3.30
     */
    explicit QgsElevationMap( const QImage &image );

    QgsElevationMap( const QgsElevationMap &other );

    /**
     * Applies eye dome lighting effect to the given \a image. The effect makes
     * angled surfaces darker and adds silhouettes in case of larger differences
     * of elevations between neighboring pixels.
     *
     * The \a distance parameter tells how many pixels away from the original pixel
     * to sample neighboring pixels. Normally distance of 2 pixels gives good results.
     *
     * The \a strength parameter adjusts how strong the added shading will be.
     * Good default for this value seems to be 1000.
     *
     * The \a rendererScale parameter adjusts scale of elevation values. It is recommended
     * to set this to the map's scale denominator to get similarly looking results
     * at different map scales.
     */
    void applyEyeDomeLighting( QImage &image, int distance, float strength, float rendererScale ) const;

    /**
     * Applies hill shading effect to the given \a image.
     *
     * If the \a multidirectinal parameter is TRUE, the algorithm will considered a
     * multi horizontal directional light to apply the shading.
     *
     * The parameter \a altitude (could also be named zenith) is the vertical direction of the light.
     *
     * The parameter \a azimuth is the horizontal direction of the light considered if
     * \a multidirectional is FALSE.
     *
     * The parameter \a zFactor is the vertical exageration of the terrain.
     *
     * The parameters \a cellSizeX and \a cellSizeY are the sizes of the elevation map cells in unit consistent
     * with the unit of the encoded elevation in this elevation map.
     *
     * \since QGIS 3.30
     */
    void applyHillshading( QImage &image, bool multiDirectional, double altitude, double azimuth, double zFactor, double cellSizeX, double cellSizeY ) const;

    //! Returns raw elevation image with elevations encoded as color values
    QImage rawElevationImage() const { return mElevationImage; }

#ifndef SIP_RUN

    /**
     * Returns pointer to the actual elevation image data
     * \since QGIS 3.36
     */
    QRgb *rawElevationImageData() { return reinterpret_cast<QRgb *>( mElevationImage.bits() ); }
#endif

    //! Returns painter to the underlying QImage with elevations
    QPainter *painter() const;

    /**
     * Combines this elevation map with \a otherElevationMap.
     * This elevation map keeps its size and takes elevation values of otherElevationMap that
     * is not null for same row and column following the combine \a method.
     * The other elevation map can have a different size, only rows and columns contained in
     * this elevation map will be considered.
     *
     * \since QGIS 3.30
     */
    void combine( const QgsElevationMap &otherElevationMap, Qgis::ElevationMapCombineMethod method );

    /**
     * Fills the elevation map with values contains in a raster \a block starting from position
     * defined by \a top and \a left. The z scale \a zScale and vertical \a offset are applied if provided.
     */
    void fillWithRasterBlock( QgsRasterBlock *block, int top, int left, double zScale = 1.0, double offset = 0.0 ) SIP_SKIP;

    /**
     * Returns whether the elevation map is valid.
     *
     * \since QGIS 3.30
     */
    bool isValid() const;

    //! Converts elevation value to an actual color
    static QRgb encodeElevation( float z );
    //! Converts a color back to elevation value
    static float decodeElevation( QRgb colorRaw );

    //! Creates an elevation map based on data from the given raster block.
    static std::unique_ptr<QgsElevationMap> fromRasterBlock( QgsRasterBlock *block ) SIP_SKIP;

    //! Returns whether the encoded value is a no data value
    inline bool isNoData( QRgb colorRaw ) const {return colorRaw == 0;}

    //! Returns the no data value for the elevation map
    inline float noDataValue() const {return decodeElevation( 0 );}

    QgsElevationMap &operator=( const QgsElevationMap &other );

  private:

    mutable QImage mElevationImage;
    mutable std::unique_ptr<QPainter> mPainter;
};

#endif // QGSELEVATIONMAP_H

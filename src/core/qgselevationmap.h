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
 * as a part of map layer rendering process. Afterwards the elevations can
 * be used for post-processing effects of the rendered color map image.
 *
 * Elevations are encoded as colors in QImage, thanks to this it is not
 * only possible to set elevation for each pixel, but also to use QPainter
 * for more complex updates of elevations. We encode elevations to 24 bits
 * in range of [-8000, 8777] with precision of three decimal digits, which
 * should give millimiter precision and enough range for elevation values
 * in meters.
 *
 * \since QGIS 3.28
 */
class CORE_EXPORT QgsElevationMap
{
  public:
    //! Constructs an elevation map with the given width and height
    explicit QgsElevationMap( const QSize &size );

    /**
     * Applies eye dome lighting effect to the given image. The effect makes
     * angled surfaces darker and adds silhouettes in case of larger differences
     * of elevations between neighboring pixels.
     *
     * The distance parameter tells how many pixels away from the original pixel
     * to sample neighboring pixels. Normally distance of 2 pixels gives good results.
     *
     * The strength parameter adjusts how strong the added shading will be.
     * Good default for this value seems to be 1000.
     *
     * The zScale parameter adjusts scale of elevation values. It is recommended
     * to set this to the map's scale denominator to get similarly looking results
     * at different map scales.
     */
    void applyEyeDomeLighting( QImage &img, int distance, float strength, float rendererScale );

    //! Returns raw elevation image with elevations encoded as color values
    QImage rawElevationImage() const { return mElevationImage; }

    //! Returns painter to the underlying QImage with elevations
    QPainter *painter() const { return mPainter.get(); }

    //! Converts elevation value to an actual color
    static QRgb encodeElevation( float z );
    //! Converts a color back to elevation value
    static float decodeElevation( QRgb colorRaw );

    //! Creates an elevation map based on data from the given raster block.
    static std::unique_ptr<QgsElevationMap> fromRasterBlock( QgsRasterBlock *block ) SIP_SKIP;

  private:

#ifdef SIP_RUN
    QgsElevationMap( const QgsElevationMap & );
#endif

    Q_DISABLE_COPY( QgsElevationMap )

    QImage mElevationImage;
    std::unique_ptr<QPainter> mPainter;
};

#endif // QGSELEVATIONMAP_H

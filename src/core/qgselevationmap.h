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

/**
 * Stores digital elevation model in a raster image which may get updated
 * as a part of map layer rendering process. Afterwards the elevations can
 * be used for post-processing effects of the rendered color map image.
 *
 * \since QGIS 3.28
 */
class CORE_EXPORT QgsElevationMap
{
  public:
    explicit QgsElevationMap( const QSize &size );

    /**
     * Applies eye dome lighting effect to the given image.
     */
    void applyEyeDomeLighting( QImage &img, int distance, float strength, float zScale );

    //! Returns painter to the underlying QImage with elevations
    QPainter *painter() { return mPainter.get(); }

    //! Converts elevation value to an actual color
    static QColor encodeElevation( float z );
    //! Converts a color back to elevation value
    static float decodeElevation( const QRgb *colorRaw );

  private:

    static const double mZMin;
    static const double mZMax;

    QImage mElevationImage;
    std::unique_ptr<QPainter> mPainter;
};

#endif // QGSELEVATIONMAP_H

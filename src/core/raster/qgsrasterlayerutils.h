/***************************************************************************
                          qgsrasterlayerutils.h
                          -------------------------
    begin                : March 2024
    copyright            : (C) 2024 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTERLAYERUTILS_H
#define QGSRASTERLAYERUTILS_H

#include "qgis.h"
#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgspointxy.h"
#include "qgsrange.h"

#include <QColor>
#include <QString>

using namespace Qt::StringLiterals;

class QgsRasterLayer;
class QgsRasterDataProvider;
class QgsRasterMinMaxOrigin;
class QgsRectangle;

/**
 * \class QgsRasterReliefColor
 * \ingroup core
 * \brief Defines elevation range and color for raster relief coloring.
 *
 * \note Prior to 4.2 this was available as QgsRelief::ReliefColor.
 *
 * \since QGIS 4.2
 */
class CORE_EXPORT QgsRasterReliefColor
{
  public:
    /**
   * Constructor for QgsRasterReliefColor.
   * \param c color
   * \param min elevation range minimum
   * \param max elevation range maximum
   */
    QgsRasterReliefColor( const QColor &c, double min, double max )
      : color( c )
      , minElevation( min )
      , maxElevation( max )
    {}

    //! Color
    QColor color;
    //! Elevation range minimum
    double minElevation = 0;
    //! Elevation range maximum
    double maxElevation = 0;

    bool operator==( const QgsRasterReliefColor &other ) const
    {
      return qgsDoubleNear( minElevation, other.minElevation ) && qgsDoubleNear( maxElevation, other.maxElevation ) && color == other.color;
    }

    bool operator!=( const QgsRasterReliefColor &other ) const { return !( *this == other ); }

#ifdef SIP_RUN
    // clang-format off
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = u"<QgsRasterReliefColor: %1-%2 (%3)>"_s.arg( qgsDoubleToString( sipCpp->minElevation ), qgsDoubleToString( sipCpp->maxElevation ), sipCpp->color.name() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
// clang-format on
#endif
};

/**
 * \class QgsRasterLayerUtils
 * \ingroup core
 * \brief Contains utility functions for working with raster layers.
 *
 * \since QGIS 3.38
 */
class CORE_EXPORT QgsRasterLayerUtils
{
  public:
    /**
     * Given a raster \a layer, returns the band which should be used for
     * rendering the layer for a specified temporal and elevation range,
     * respecting any elevation and temporal settings which affect the rendered band.
     *
     * \param layer Target raster layer
     * \param temporalRange temporal range for rendering
     * \param elevationRange elevation range for rendering
     * \param matched will be set to TRUE if a band matching the temporal and elevation range was found
     *
     * \returns Matched band, or -1 if the layer does not have any elevation or temporal settings which affect the rendered band.
     */
    static int renderedBandForElevationAndTemporalRange( QgsRasterLayer *layer, const QgsDateTimeRange &temporalRange, const QgsDoubleRange &elevationRange, bool &matched SIP_OUT );

    /**
     * Compute the \a min \a max values for \a provider along \a band according to
     * MinMaxOrigin parameters \a mmo and \a extent.
     *
     * \since QGIS 4.0
     */
    static void computeMinMax(
      QgsRasterDataProvider *provider, int band, const QgsRasterMinMaxOrigin &mmo, Qgis::RasterRangeLimit limits, const QgsRectangle &extent, int sampleSize, double &min SIP_OUT, double &max SIP_OUT
    );

    /**
     * Returns a new extent that includes the given \a extent with corners coordinates
     * aligned to the pixel grid defined by the \a origin and \a pixelSizeX and \a pixelSizeY arguments.
     *
     * The resulting extent will be expanded if necessary to ensure that its corners fall on pixel boundaries defined by the origin and pixel sizes.
     *
     * \returns The aligned extent or the original extent if pixel sizes are zero (to avoid division by zero) or if the extent is empty.
     *
     * \since QGIS 4.0
     */
    static QgsRectangle alignRasterExtent( const QgsRectangle &extent, const QgsPointXY &origin, double pixelSizeX, double pixelSizeY );
};

#endif //QGSRASTERLAYERUTILS_H

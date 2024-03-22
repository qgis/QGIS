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

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsrange.h"

class QgsRasterLayer;

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
     * \returns Matched band, or -1 if the layer does not have any elevation
     * or temporal settings which affect the rendered band.
     */
    static int renderedBandForElevationAndTemporalRange(
      QgsRasterLayer *layer,
      const QgsDateTimeRange &temporalRange,
      const QgsDoubleRange &elevationRange,
      bool &matched SIP_OUT );

};

#endif //QGSRASTERLAYERUTILS_H

/***************************************************************************
                           qgswmsutils.h
                          ---------------
    begin                : September 2025
    copyright            : (C) 2025 by Germán Carrillo
    email                : german at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSWMSUTILS_H
#define QGSWMSUTILS_H

#include "qgsrasterlayer.h"

/**
 * \ingroup core
 * \class QgsWmsUtils
 * \brief Contains utility functions for OGC WMS layers.
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsWmsUtils
{
  public:

    /**
     * Returns whether a map \a layer is an OGC WMS layer or not.
     * \param layer Map layer that will be checked.
     * \note This method is strictly checking for OGC WMS sources. It will return FALSE for WMTS or XYZ tile layers.
     */
    static bool isWmsLayer( QgsMapLayer *layer );

    /**
     * Returns the WMS version of a WMS layer as a string (e.g., "1.3.0").
     *
     * Returns an empty string if the layer is not a valid WMS layer.
     *
     * \param layer WMS layer from which the version will be queried.
     */
    static QString wmsVersion( QgsRasterLayer *layer );
};

#endif // QGSWMSUTILS_H

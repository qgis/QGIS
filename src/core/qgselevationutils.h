/***************************************************************************
  qgselevationutils.h
  ------------------
  Date                 : November 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSELEVATIONUTILS_H
#define QGSELEVATIONUTILS_H

#include "qgis_core.h"
#include "qgsrange.h"

class QgsProject;
class QgsMapLayer;

/**
 * \ingroup core
 * \class QgsElevationUtils
 * \brief Contains utility methods for working with elevation from layers and projects.
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsElevationUtils
{
  public:

    /**
     * Calculates the elevation range for a \a project.
     *
     * This method considers the elevation (or z) range available from layers contained within the project and
     * returns the maximal combined elevation range of these layers.
     */
    static QgsDoubleRange calculateZRangeForProject( QgsProject *project );

    /**
     * Returns a list of significant elevation/z-values for the specified \a project, using
     * the values from layers contained by the project.
     *
     * These values will be highlighted in elevation related widgets for the project.
     *
     * \since QGIS 3.38
     */
    static QList< double > significantZValuesForProject( QgsProject *project );

    /**
     * Returns a list of significant elevation/z-values for the specified \a layers.
     *
     * These values will be highlighted in elevation related widgets for the project.
     *
     * \since QGIS 3.38
     */
    static QList< double > significantZValuesForLayers( const QList< QgsMapLayer * > &layers );

    /**
     * Returns TRUE if elevation can be enabled for a map \a layer.
     *
     * \since QGIS 3.32
     */
    static bool canEnableElevationForLayer( QgsMapLayer *layer );

    /**
     * Automatically enables elevation for a map \a layer, using reasonable defaults.
     *
     * Returns TRUE if the elevation was enabled successfully.
     *
     * \since QGIS 3.32
     */
    static bool enableElevationForLayer( QgsMapLayer *layer );

};


#endif // QGSELEVATIONUTILS_H

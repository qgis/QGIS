/***************************************************************************
                             qgscoordinateutils.h
                             --------------------
    begin                : February 2016
    copyright            : (C) 2016 by Nyall Dawson
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

#ifndef QGSCOORDINATEUTILS_H
#define QGSCOORDINATEUTILS_H

#include "qgscoordinatereferencesystem.h"
#include "qgspoint.h"

//not stable api - I plan on reworking this when QgsCoordinateFormatter lands in 2.16
///@cond NOT_STABLE_API

/** \ingroup core
 * \class QgsCoordinateUtils
 * \brief Utilities for handling and formatting coordinates
 * \note added in QGIS 2.14
 */
class CORE_EXPORT QgsCoordinateUtils
{
  public:

    /** Returns the precision to use for displaying coordinates to the user, respecting
     * the user's project settings. If the user has set the project to use "automatic"
     * precision, this function tries to calculate an optimal coordinate precision for a given
     * map units per pixel by calculating the number of decimal places for the coordinates
     * with the aim of always having enough decimal places to show the difference in position
     * between adjacent pixels.
     * @param mapUnitsPerPixel number of map units per pixel
     * @param mapCrs CRS of map
     * @returns optimal number of decimal places for coordinates
     */
    static int calculateCoordinatePrecision( double mapUnitsPerPixel, const QgsCoordinateReferenceSystem& mapCrs );

    static QString formatCoordinateForProject( const QgsPoint& point, const QgsCoordinateReferenceSystem& destCrs, int precision );

};


///@endcond

#endif //QGSCOORDINATEUTILS_H

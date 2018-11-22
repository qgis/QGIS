/***************************************************************************
                             qgsbearingutils.h
                             -----------------
    begin                : October 2016
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

#ifndef QGSBEARINGUTILS_H
#define QGSBEARINGUTILS_H

class QgsCoordinateReferenceSystem;
class QgsCoordinateTransformContext;
class QgsPointXY;

#include "qgis_core.h"

/**
 * \class QgsBearingUtils
 * \ingroup core
 * Utilities for calculating bearings and directions.
 * \since QGIS 2.18
*/
class CORE_EXPORT QgsBearingUtils
{
  public:

    /**
     * Returns the direction to true north from a specified point and for a specified
     * coordinate reference system. The returned value is in degrees clockwise from
     * vertical. An exception will be thrown if the bearing could not be calculated.
     */
    static double bearingTrueNorth( const QgsCoordinateReferenceSystem &crs,
                                    const QgsCoordinateTransformContext  &transformContext,
                                    const QgsPointXY &point );

};

#endif //QGSBEARINGUTILS_H

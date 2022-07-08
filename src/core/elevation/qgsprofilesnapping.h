/***************************************************************************
                         qgsprofilesnapping.h
                         ---------------
    begin                : April 2022
    copyright            : (C) 2022 by Nyall Dawson
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
#ifndef QGSPROFILESNAPPING_H
#define QGSPROFILESNAPPING_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"
#include "qgsprofilepoint.h"

/**
 * \brief Encapsulates the context of snapping a profile point.
 * \ingroup core
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsProfileSnapContext
{
  public:

    //! Maximum allowed snapping delta for the distance values when snapping to a continuous elevation surface
    double maximumSurfaceDistanceDelta = 0;

    //! Maximum allowed snapping delta for the elevation values when snapping to a continuous elevation surface
    double maximumSurfaceElevationDelta = 0;

    //! Maximum allowed snapping delta for the distance values when snapping to a point
    double maximumPointDistanceDelta = 0;

    //! Maximum allowed snapping delta for the elevation values when snapping to a point
    double maximumPointElevationDelta = 0;

    //! Display ratio of elevation vs distance units
    double displayRatioElevationVsDistance = 1;

};

/**
 * \brief Encapsulates results of snapping a profile point.
 * \ingroup core
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsProfileSnapResult
{
  public:

    //! Snapped point
    QgsProfilePoint snappedPoint;

    /**
     * Returns TRUE if the result is a valid point.
     */
    bool isValid() const { return !snappedPoint.isEmpty(); }
};

#endif // QGSPROFILESNAPPING_H

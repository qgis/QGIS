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

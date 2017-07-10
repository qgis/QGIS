/***************************************************************************
                             qgslayoututils.h
                             -------------------
    begin                : July 2017
    copyright            : (C) 2017 by Nyall Dawson
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
#ifndef QGSLAYOUTUTILS_H
#define QGSLAYOUTUTILS_H

#include "qgis_core.h"

/**
 * \ingroup core
 * Utilities for layouts.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutUtils
{
  public:

    /**
     * Ensures that an \a angle (in degrees) is in the range 0 <= angle < 360.
     * If \a allowNegative is true then angles between (-360, 360) are allowed. If false,
     * angles are converted to positive angles in the range [0, 360).
     */
    static double normalizedAngle( const double angle, const bool allowNegative = false );

};

#endif //QGSLAYOUTUTILS_H

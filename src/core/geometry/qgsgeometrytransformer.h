/***************************************************************************
                             qgsgeometrytransformer.h
                             ----------------------
    begin                : February 2021
    copyright            : (C) 2021 by Nyall Dawson
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

#ifndef QGSGEOMETRYTRANSFORMER_H
#define QGSGEOMETRYTRANSFORMER_H

#include "qgis_core.h"
#include "qgis_sip.h"

/**
 * \class QgsAbstractGeometryTransformer
 * \ingroup core
 * \brief An abstract base class for classes which transform geometries by transforming
 * input points to output points.
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsAbstractGeometryTransformer
{
  public:

    virtual ~QgsAbstractGeometryTransformer() = default;

    /**
     * Transforms the point defined by the coordinates (\a x, \a y, \a z) and the specified \a m value.
     *
     * \param x point x coordinate
     * \param y point y coordinate
     * \param z point z coordinate, or NaN if the input point is 2D only
     * \param m point m value, or NaN if not available
     *
     * \returns TRUE if point was transformed (or no transformation was required), or FALSE if point could not be transformed successfully.
     *
     * ### Example
     *
     * A transformer which multiples the x coordinate by 3 and adds 10 to the y coordinate:
     *
     * \code{.py}
     *   class MyTransformer(QgsAbstractGeometryTransformer):
     *
     *     def transformPoint(self, x, y, z, m):
     *       # returns a tuple of True to indicate success, then the modified x/y/z/m values
     *       return True, x*3, y+10, z, m
     * \endcode
     */
    virtual bool transformPoint( double &x SIP_INOUT, double &y SIP_INOUT, double &z SIP_INOUT, double &m  SIP_INOUT ) = 0;

};

#endif // QGSGEOMETRYTRANSFORMER_H

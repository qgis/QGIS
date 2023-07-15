/***************************************************************************
                         qgsorientedbox3d.h
                         --------------------
    begin                : July 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ******************************************************************
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSORIENTEDBOX3D_H
#define QGSORIENTEDBOX3D_H

#include "qgis_core.h"
#include "qgis_sip.h"

#include <QList>
#include <limits>

class QgsBox3d;

/**
 * \brief Represents a oriented (rotated) box in 3 dimensions.
 *
 * \ingroup core
 *
 * \warning Non-stable API, exposed to Python for unit testing only.
 *
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsOrientedBox3D
{
  public:

    /**
     * Constructor for a null oriented box.
     */
    QgsOrientedBox3D();

    /**
     * Constructor for a oriented box, with a specified center and half axes matrix.
     */
    QgsOrientedBox3D( const QList<double> &center, QList< double > &halfAxes );

    /**
     * Returns TRUE if the box is a null box.
     */
    bool isNull() const;

    /**
     * Returns the center x-coordinate.
     *
     * \see centerY()
     * \see centerZ()
     */
    double centerX() const { return mCenter[0]; }

    /**
     * Returns the center y-coordinate.

     * \see centerX()
     * \see centerZ()
     */
    double centerY() const { return mCenter[1]; }

    /**
     * Returns the center z-coordinate.
     *
     * \see centerX()
     * \see centerY()
     */
    double centerZ() const { return mCenter[2]; }

    /**
     * Returns the half axes matrix;
     */
    const double *halfAxes() const SIP_SKIP { return mHalfAxes; }

    /**
     * Returns the half axes matrix;
     */
    QList< double > halfAxesList() const SIP_PYNAME( halfAxes );

    /**
     * Returns the overall bounding box of the object.
     */
    QgsBox3d extent() const;

  private:

    double mCenter[ 3 ] { std::numeric_limits< double >::quiet_NaN(), std::numeric_limits< double >::quiet_NaN(), std::numeric_limits< double >::quiet_NaN() };
    double mHalfAxes[9] { 1, 0, 0, 0, 1, 0, 0, 0, 1 };

    friend class QgsCesiumUtils;

};


#endif // QGSORIENTEDBOX3D_H

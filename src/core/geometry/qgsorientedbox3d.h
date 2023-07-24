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
#include "qgis.h"
#include "qgsvector3d.h"

#include <QList>
#include <limits>

class QgsBox3D;
class QgsCoordinateTransform;
class QgsMatrix4x4;

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
    QgsOrientedBox3D( const QList<double> &center, const QList< double > &halfAxes );

    /**
     * Constructor for a oriented box, with a specified center and half axes matrix.
     */
    QgsOrientedBox3D( const QgsVector3D &center, const QList< QgsVector3D > &halfAxes );

    bool operator==( const QgsOrientedBox3D &other ) const
    {
      return qgsDoubleNear( mCenter[0], other.mCenter[0] )
             && qgsDoubleNear( mCenter[1], other.mCenter[1] )
             && qgsDoubleNear( mCenter[2], other.mCenter[2] )
             && qgsDoubleNear( mHalfAxes[0], other.mHalfAxes[0] )
             && qgsDoubleNear( mHalfAxes[1], other.mHalfAxes[1] )
             && qgsDoubleNear( mHalfAxes[2], other.mHalfAxes[2] )
             && qgsDoubleNear( mHalfAxes[3], other.mHalfAxes[3] )
             && qgsDoubleNear( mHalfAxes[4], other.mHalfAxes[4] )
             && qgsDoubleNear( mHalfAxes[5], other.mHalfAxes[5] )
             && qgsDoubleNear( mHalfAxes[6], other.mHalfAxes[6] )
             && qgsDoubleNear( mHalfAxes[7], other.mHalfAxes[7] )
             && qgsDoubleNear( mHalfAxes[8], other.mHalfAxes[8] );
    }

    bool operator!=( const QgsOrientedBox3D &other ) const
    {
      return !( *this == other );
    }

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
     * Returns the vector to the center of the box.
     */
    QgsVector3D center() const { return QgsVector3D( mCenter[0], mCenter[1], mCenter[2] ); }

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
    QgsBox3D extent() const;

    /**
     * Returns an array of all corners as 3D vectors.
     */
    QVector< QgsVector3D > corners() const;

    /**
     * Returns size of sides of the box.
     */
    QgsVector3D size() const;

    /**
     * Calculates the projection of the box onto a \a vector.
     */
    QgsVector3D projectOnto( const QgsVector3D &vector ) const;

    /**
     * Reprojects corners of this box using the given coordinate \a transform
     * and returns axis-aligned box containing reprojected corners.
     * \throws QgsCsException
     */
    QgsBox3D reprojectedExtent( const QgsCoordinateTransform &ct ) const SIP_THROW( QgsCsException );

    /**
     * Returns box transformed by a 4x4 matrix.
     */
    QgsOrientedBox3D transformed( const QgsMatrix4x4 &transform ) const;

    /**
     * Returns TRUE if the box intersects the \a other box.
     */
    bool intersects( const QgsOrientedBox3D &other ) const;

  private:

    double mCenter[ 3 ] { std::numeric_limits< double >::quiet_NaN(), std::numeric_limits< double >::quiet_NaN(), std::numeric_limits< double >::quiet_NaN() };
    double mHalfAxes[9] { 1, 0, 0, 0, 1, 0, 0, 0, 1 };

    friend class QgsCesiumUtils;

};


#endif // QGSORIENTEDBOX3D_H

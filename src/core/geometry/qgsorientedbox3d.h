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

    /**
     * Constructs an oriented box from an axis-aligned bounding box.
     */
    static QgsOrientedBox3D fromBox3D( const QgsBox3D &box );

    bool operator==( const QgsOrientedBox3D &other ) const SIP_HOLDGIL
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

    bool operator!=( const QgsOrientedBox3D &other ) const SIP_HOLDGIL
    {
      return !( *this == other );
    }

    /**
     * Returns TRUE if the box is a null box.
     */
    bool isNull() const SIP_HOLDGIL;

    /**
     * Returns the center x-coordinate.
     *
     * \see centerY()
     * \see centerZ()
     */
    double centerX() const SIP_HOLDGIL { return mCenter[0]; }

    /**
     * Returns the center y-coordinate.

     * \see centerX()
     * \see centerZ()
     */
    double centerY() const SIP_HOLDGIL { return mCenter[1]; }

    /**
     * Returns the center z-coordinate.
     *
     * \see centerX()
     * \see centerY()
     */
    double centerZ() const SIP_HOLDGIL { return mCenter[2]; }

    /**
     * Returns the vector to the center of the box.
     */
    QgsVector3D center() const SIP_HOLDGIL { return QgsVector3D( mCenter[0], mCenter[1], mCenter[2] ); }

    /**
     * Returns the half axes matrix;
     */
    const double *halfAxes() const SIP_SKIP { return mHalfAxes; }

    /**
     * Returns the half axes matrix;
     */
    QList< double > halfAxesList() const SIP_HOLDGIL SIP_PYNAME( halfAxes );

    /**
     * Returns the overall bounding box of the object.
     */
    QgsBox3D extent() const SIP_HOLDGIL;

    /**
     * Returns an array of all corners as 3D vectors.
     */
    QVector< QgsVector3D > corners() const SIP_HOLDGIL;

    /**
     * Returns size of sides of the box.
     */
    QgsVector3D size() const SIP_HOLDGIL;

    /**
     * Reprojects corners of this box using the given coordinate \a transform
     * and returns axis-aligned box containing reprojected corners.
     * \throws QgsCsException
     */
    QgsBox3D reprojectedExtent( const QgsCoordinateTransform &ct ) const SIP_THROW( QgsCsException ) SIP_HOLDGIL;

    /**
     * Returns box transformed by a 4x4 matrix.
     */
    QgsOrientedBox3D transformed( const QgsMatrix4x4 &transform ) const SIP_HOLDGIL;

    /**
     * Returns TRUE if the box intersects the \a other box.
     */
    bool intersects( const QgsOrientedBox3D &other ) const SIP_HOLDGIL;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsOrientedBox3D([%1, %2, %3], [%4, %5, %6, %7, %8, %9, %10, %11, %12])>" )
                  .arg( sipCpp->centerX() )
                  .arg( sipCpp->centerY() )
                  .arg( sipCpp->centerZ() )
                  .arg( sipCpp->halfAxes()[0] )
                  .arg( sipCpp->halfAxes()[1] )
                  .arg( sipCpp->halfAxes()[2] )
                  .arg( sipCpp->halfAxes()[3] )
                  .arg( sipCpp->halfAxes()[4] )
                  .arg( sipCpp->halfAxes()[5] )
                  .arg( sipCpp->halfAxes()[6] )
                  .arg( sipCpp->halfAxes()[7] )
                  .arg( sipCpp->halfAxes()[8] );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

  private:

    double mCenter[ 3 ] { std::numeric_limits< double >::quiet_NaN(), std::numeric_limits< double >::quiet_NaN(), std::numeric_limits< double >::quiet_NaN() };
    double mHalfAxes[9] { 1, 0, 0, 0, 1, 0, 0, 0, 1 };

    friend class QgsCesiumUtils;

};


#endif // QGSORIENTEDBOX3D_H

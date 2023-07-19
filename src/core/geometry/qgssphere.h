/***************************************************************************
                         qgssphere.h
                         --------------
    begin                : July 2023
    copyright            : (C) 2023 by Nyall Dawson
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

#ifndef QGSSPHERE_H
#define QGSSPHERE_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"
#include <limits>

class QgsPoint;
class QgsCircle;
class QgsBox3D;
class QgsVector3D;

/**
 * \ingroup core
 * \class QgsSphere
 * \brief A spherical geometry object.
 *
 * Represents a simple 3-dimensional sphere.
 *
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsSphere
{

  public:

    /**
     * Constructor for an invalid QgsSphere.
     */
    QgsSphere() SIP_HOLDGIL = default;

    /**
     * Constructor for QgsSphere with the specified center (\a x, \a y, \a z) and \a radius.
     */
    QgsSphere( double x, double y, double z, double radius ) SIP_HOLDGIL;

    bool operator==( const QgsSphere &other ) const
    {
      return qgsDoubleNear( mCenterX, other.mCenterX ) && qgsDoubleNear( mCenterY, other.mCenterY ) && qgsDoubleNear( mCenterZ, other.mCenterZ ) && qgsDoubleNear( mRadius, other.mRadius );
    }
    bool operator!=( const QgsSphere &other ) const { return !( *this == other ); }

    /**
     * Returns TRUE if the sphere is a null (default constructed) sphere.
     */
    bool isNull() const SIP_HOLDGIL;

    /**
     * Returns TRUE if the sphere is considered empty, i.e. it has a radius of 0.
     */
    bool isEmpty() const SIP_HOLDGIL;

    /**
     * Returns the center point of the sphere.
     *
     * \see centerX()
     * \see centerY()
     * \see centerZ()
     * \see setCenter()
     */
    QgsPoint center() const SIP_HOLDGIL;

    /**
     * Returns the vector to the center of the sphere.
     *
     * \see centerX()
     * \see centerY()
     * \see centerZ()
     * \see setCenter()
     */
    QgsVector3D centerVector() const SIP_HOLDGIL;

    /**
     * Returns the x-coordinate of the center of the sphere.
     *
     * \see center()
     * \see centerY()
     * \see centerZ()
     * \see setCenter()
     */
    double centerX() const { return mCenterX; }

    /**
     * Returns the y-coordinate of the center of the sphere.
     *
     * \see center()
     * \see centerX()
     * \see centerZ()
     * \see setCenter()
     */
    double centerY() const { return mCenterY; }

    /**
     * Returns the z-coordinate of the center of the sphere.
     *
     * \see center()
     * \see centerX()
     * \see centerY()
     * \see setCenter()
     */
    double centerZ() const { return mCenterZ; }

    /**
     * Sets the center point of the sphere.
     * \see center()
     */
    void setCenter( const QgsPoint &center ) SIP_HOLDGIL;

    /**
     * Sets the center point of the sphere to (\a x, \a y, \a z).
     * \see center()
     */
    void setCenter( double x, double y, double z ) SIP_HOLDGIL { mCenterX = x; mCenterY = y; mCenterZ = z; }

    /**
     * Returns the radius of the sphere.
     *
     * \see setRadius()
     * \see diameter()
     */
    double radius() const SIP_HOLDGIL { return mRadius; }

    /**
     * Sets the \a radius of the sphere.
     *
     * \see radius()
     */
    void setRadius( double radius ) SIP_HOLDGIL{ mRadius = radius; }

    /**
     * Returns the diameter of the sphere.
     *
     * \see radius()
     */
    double diameter() const SIP_HOLDGIL { return mRadius * 2; }

    /**
     * Returns the volume of the sphere.
     */
    double volume() const SIP_HOLDGIL;

    /**
     * Returns the surface area of the sphere.
     */
    double surfaceArea() const SIP_HOLDGIL;

    /**
     * Converts the sphere to a 2-dimensional circle.
     */
    QgsCircle toCircle() const SIP_HOLDGIL;

    /**
     * Returns the 3-dimensional bounding box containing the sphere.
     */
    QgsBox3D boundingBox() const SIP_HOLDGIL;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str;
    if ( sipCpp->isNull() )
    {
      str = QStringLiteral( "<QgsSphere: null>" ).arg( sipCpp->centerX() ).arg( sipCpp->centerY() ).arg( sipCpp->centerZ() ).arg( sipCpp->radius() );
    }
    else
    {
      str = QStringLiteral( "<QgsSphere: (%1, %2, %3) radius %4>" ).arg( sipCpp->centerX() ).arg( sipCpp->centerY() ).arg( sipCpp->centerZ() ).arg( sipCpp->radius() );
    }
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

  private:
    double mCenterX = std::numeric_limits< double >::quiet_NaN();
    double mCenterY = std::numeric_limits< double >::quiet_NaN();
    double mCenterZ = std::numeric_limits< double >::quiet_NaN();
    double mRadius = 0;

};

#endif // QGSSPHERE_H

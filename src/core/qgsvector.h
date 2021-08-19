/***************************************************************************
  qgsvector.h - QgsVector

 ---------------------
 begin                : 24.2.2017
 copyright            : (C) 2017 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSVECTOR_H
#define QGSVECTOR_H

#include "qgis.h"
#include "qgis_core.h"
#include <QtGlobal>

/**
 * \ingroup core
 * \brief A class to represent a vector.
 * Currently no Z axis / 2.5D support is implemented.
 */

class CORE_EXPORT QgsVector
{

  public:

    /**
     * Default constructor for QgsVector. Creates a vector with length of 0.0.
     */
    QgsVector() = default;

    /**
     * Constructor for QgsVector taking x and y component values.
     * \param x x-component
     * \param y y-component
     */
    QgsVector( double x, double y )
      : mX( x )
      , mY( y )
    {
    }

    //! Swaps the sign of the x and y components of the vector.
    QgsVector operator-() const SIP_HOLDGIL
    {
      return QgsVector( -mX, -mY );
    }

    /**
     * Returns a vector where the components have been multiplied by a scalar value.
     * \param scalar factor to multiply by
     */
    QgsVector operator*( double scalar ) const SIP_HOLDGIL
    {
      return QgsVector( mX * scalar, mY * scalar );
    }

    /**
     * Returns a vector where the components have been divided by a scalar value.
     * \param scalar factor to divide by
     */
    QgsVector operator/( double scalar ) const SIP_HOLDGIL
    {
      return *this * ( 1.0 / scalar );
    }

    /**
     * Returns the dot product of two vectors, which is the sum of the x component
     *  of this vector multiplied by the x component of another
     *  vector plus the y component of this vector multiplied by the y component of another vector.
     */
    double operator*( QgsVector v ) const SIP_HOLDGIL
    {
      return mX * v.mX + mY * v.mY;
    }

    /**
     * Adds another vector to this vector.
     * \since QGIS 3.0
     */
    QgsVector operator+( QgsVector other ) const SIP_HOLDGIL
    {
      return QgsVector( mX + other.mX, mY + other.mY );
    }

    /**
     * Adds another vector to this vector in place.
     * \since QGIS 3.0
     */
    QgsVector &operator+=( QgsVector other ) SIP_HOLDGIL
    {
      mX += other.mX;
      mY += other.mY;
      return *this;
    }

    /**
     * Subtracts another vector to this vector.
     * \since QGIS 3.0
     */
    QgsVector operator-( QgsVector other ) const SIP_HOLDGIL
    {
      return QgsVector( mX - other.mX, mY - other.mY );
    }

    /**
     * Subtracts another vector to this vector in place.
     * \since QGIS 3.0
     */
    QgsVector &operator-=( QgsVector other ) SIP_HOLDGIL
    {
      mX -= other.mX;
      mY -= other.mY;
      return *this;
    }

    /**
     * Returns the length of the vector.
     * \see lengthSquared()
     */
    double length() const SIP_HOLDGIL
    {
      return std::sqrt( mX * mX + mY * mY );
    }

    /**
     * Returns the length of the vector.
     * \see length()
     * \since QGIS 3.2
     */
    double lengthSquared() const SIP_HOLDGIL
    {
      return mX * mX + mY * mY;
    }

    /**
     * Returns the vector's x-component.
     * \see y()
     */
    double x() const SIP_HOLDGIL
    {
      return mX;
    }

    /**
     * Returns the vector's y-component.
     * \see x()
     */
    double y() const SIP_HOLDGIL
    {
      return mY;
    }

    /**
     * Returns the perpendicular vector to this vector (rotated 90 degrees counter-clockwise)
     */
    QgsVector perpVector() const SIP_HOLDGIL
    {
      return QgsVector( -mY, mX );
    }

    /**
     * Returns the angle of the vector in radians.
     */
    double angle() const SIP_HOLDGIL
    {
      const double angle = std::atan2( mY, mX );
      return angle < 0.0 ? angle + 2.0 * M_PI : angle;
    }

    /**
     * Returns the angle between this vector and another vector in radians.
     */
    double angle( QgsVector v ) const SIP_HOLDGIL
    {
      return v.angle() - angle();
    }

    /**
     * Returns the 2D cross product of this vector and another vector \a v. (This is sometimes
     * referred to as a "perpendicular dot product", and equals x1 * y1 - y1 * x2).
     *
     * \since QGIS 3.2
     */
    double crossProduct( QgsVector v ) const SIP_HOLDGIL
    {
      return mX * v.y() - mY * v.x();
    }

    /**
     * Rotates the vector by a specified angle.
     * \param rot angle in radians
     */
    QgsVector rotateBy( double rot ) const SIP_HOLDGIL;

    /**
     * Returns the vector's normalized (or "unit") vector (ie same angle but length of 1.0).
     *
     * \throws QgsException if called on a vector with length of 0.
     */
    QgsVector normalized() const SIP_THROW( QgsException );

    //! Equality operator
    bool operator==( QgsVector other ) const SIP_HOLDGIL
    {
      return qgsDoubleNear( mX, other.mX ) && qgsDoubleNear( mY, other.mY );
    }

    //! Inequality operator
    bool operator!=( QgsVector other ) const
    {
      return !qgsDoubleNear( mX, other.mX ) || !qgsDoubleNear( mY, other.mY );
    }

    /**
    * Returns a string representation of the vector.
    * Members will be truncated to the specified \a precision.
    */
    QString toString( int precision = 17 ) const SIP_HOLDGIL
    {
      QString str = "Vector (";
      str += qgsDoubleToString( mX, precision );
      str += ", ";
      str += qgsDoubleToString( mY, precision );
      str += ')';
      return str;
    }

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsVector: %1>" ).arg( sipCpp->toString() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

  private:
    double mX = 0.0;
    double mY = 0.0;

};

Q_DECLARE_TYPEINFO( QgsVector, Q_MOVABLE_TYPE );

#endif // QGSVECTOR_H

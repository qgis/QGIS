/***************************************************************************
                          qgspoint.h  -  description
                             -------------------
    begin                : Sat Jun 22 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTXY_H
#define QGSPOINTXY_H

#include "qgis_core.h"
#include "qgsvector.h"

#include "qgis.h"

#include <iostream>
#include <QString>
#include <QPoint>
#include <QObject>

class QgsPoint;

/**
 * \ingroup core
 * \brief A class to represent a 2D point.
 *
 * A QgsPointXY represents a strictly 2-dimensional position, with only X and Y coordinates.
 * This is a very lightweight class, designed to minimize the memory requirements of storing
 * millions of points.
 *
 * In many scenarios it is preferable to use a QgsPoint instead which also
 * supports optional Z and M values. QgsPointXY should only be used for situations where
 * a point can only EVER be two dimensional.
 *
 * Some valid use cases for QgsPointXY include:
 *
 * - A mouse cursor location
 * - A coordinate on a purely 2-dimensional rendered map, e.g. a QgsMapCanvas
 * - A coordinate in a raster, vector tile, or other purely 2-dimensional layer
 *
 * Use cases for which QgsPointXY is NOT a valid choice include:
 *
 * - Storage of coordinates for a geometry. Since QgsPointXY is strictly 2-dimensional it should never be used to store coordinates for vector geometries, as this will involve a loss of any z or m values present in the geometry.
 *
 * \see QgsPoint
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsPointXY
{
    Q_GADGET

    Q_PROPERTY( double x READ x WRITE setX )
    Q_PROPERTY( double y READ y WRITE setY )

  public:
    /// Default constructor
    QgsPointXY() = default;

    //! Create a point from another point
    QgsPointXY( const QgsPointXY &p ) SIP_HOLDGIL;

    /**
     * Create a point from x,y coordinates
     * \param x x coordinate
     * \param y y coordinate
     */
    QgsPointXY( double x, double y ) SIP_HOLDGIL
  : mX( x )
    , mY( y )
    , mIsEmpty( false )
    {}

    /**
     * Create a point from a QPointF
     * \param point QPointF source
     * \since QGIS 2.7
     */
    QgsPointXY( QPointF point ) SIP_HOLDGIL
  : mX( point.x() )
    , mY( point.y() )
    , mIsEmpty( false )
    {}

    /**
     * Create a point from a QPoint
     * \param point QPoint source
     * \since QGIS 2.7
     */
    QgsPointXY( QPoint point ) SIP_HOLDGIL
  : mX( point.x() )
    , mY( point.y() )
    , mIsEmpty( false )
    {}

    /**
     * Create a new point.
     * Z and M values will be dropped.
     *
     * \since QGIS 3.0
     */
    QgsPointXY( const QgsPoint &point ) SIP_HOLDGIL;

    // IMPORTANT - while QgsPointXY is inherited by QgsReferencedPointXY, we do NOT want a virtual destructor here
    // because this class MUST be lightweight and we don't want the cost of the vtable here.
    // see https://github.com/qgis/QGIS/pull/4720#issuecomment-308652392
    ~QgsPointXY() = default;

    /**
     * Sets the x value of the point
     * \param x x coordinate
     */
    void setX( double x ) SIP_HOLDGIL
    {
      mX = x;
      mIsEmpty = false;
    }

    /**
     * Sets the y value of the point
     * \param y y coordinate
     */
    void setY( double y ) SIP_HOLDGIL
    {
      mY = y;
      mIsEmpty = false;
    }

    //! Sets the x and y value of the point
    void set( double x, double y ) SIP_HOLDGIL
    {
      mX = x;
      mY = y;
      mIsEmpty = false;
    }

    /**
     * Gets the x value of the point
     * \returns x coordinate
     */
    double x() const SIP_HOLDGIL
    {
      return mX;
    }

    /**
     * Gets the y value of the point
     * \returns y coordinate
     */
    double y() const SIP_HOLDGIL
    {
      return mY;
    }

    /**
     * Converts a point to a QPointF
     * \returns QPointF with same x and y values
     * \since QGIS 2.7
     */
    QPointF toQPointF() const
    {
      return QPointF( mX, mY );
    }

    /**
     * Returns a string representation of the point (x, y) with a preset \a precision.
     * If  \a precision is -1, then a default precision will be used.
     */
    QString toString( int precision = -1 ) const;

    /**
     * Returns the well known text representation for the point (e.g. "POINT(x y)").
     * The wkt is created without an SRID.
     */
    QString asWkt() const;

    /**
     * Returns the squared distance between this point a specified x, y coordinate.
     * \see distance()
    */
    double sqrDist( double x, double y ) const SIP_HOLDGIL
    {
      return ( mX - x ) * ( mX - x ) + ( mY - y ) * ( mY - y );
    }

    /**
     * Returns the squared distance between this point another point.
     * \see distance()
    */
    double sqrDist( const QgsPointXY &other ) const SIP_HOLDGIL
    {
      return sqrDist( other.x(), other.y() );
    }

    /**
     * Returns the distance between this point and a specified x, y coordinate.
     * \param x x-coordniate
     * \param y y-coordinate
     * \see sqrDist()
     * \since QGIS 2.16
    */
    double distance( double x, double y ) const SIP_HOLDGIL
    {
      return std::sqrt( sqrDist( x, y ) );
    }

    /**
     * Returns the distance between this point and another point.
     * \param other other point
     * \see sqrDist()
     * \since QGIS 2.16
    */
    double distance( const QgsPointXY &other ) const SIP_HOLDGIL
    {
      return std::sqrt( sqrDist( other ) );
    }

    //! Returns the minimum distance between this point and a segment
    double sqrDistToSegment( double x1, double y1, double x2, double y2, QgsPointXY &minDistPoint SIP_OUT, double epsilon = DEFAULT_SEGMENT_EPSILON ) const SIP_HOLDGIL;

    //! Calculates azimuth between this point and other one (clockwise in degree, starting from north)
    double azimuth( const QgsPointXY &other ) const SIP_HOLDGIL;

    /**
     * Returns a new point which corresponds to this point projected by a specified distance
     * in a specified bearing.
     * \param distance distance to project
     * \param bearing angle to project in, clockwise in degrees starting from north
     * \since QGIS 2.16
     */
    QgsPointXY project( double distance, double bearing ) const SIP_HOLDGIL;

    /**
     * Returns TRUE if the geometry is empty.
     * Unlike QgsPoint, this class is also used to retrieve graphical coordinates like QPointF.
     * It therefore has the default coordinates (0.0).
     * A QgsPointXY is considered empty, when the coordinates have not been explicitly filled in.
     * \since QGIS 3.10
     */
    bool isEmpty() const SIP_HOLDGIL { return mIsEmpty; }

    /**
     * Compares this point with another point with a fuzzy tolerance
     * \param other point to compare with
     * \param epsilon maximum difference for coordinates between the points
     * \returns TRUE if points are equal within specified tolerance
     * \since QGIS 2.9
     */
    bool compare( const QgsPointXY &other, double epsilon = 4 * std::numeric_limits<double>::epsilon() ) const SIP_HOLDGIL
    {
      return ( qgsDoubleNear( mX, other.x(), epsilon ) && qgsDoubleNear( mY, other.y(), epsilon ) );
    }

    //! equality operator
    bool operator==( const QgsPointXY &other ) SIP_HOLDGIL
    {
      if ( isEmpty() && other.isEmpty() )
        return true;
      if ( isEmpty() && !other.isEmpty() )
        return false;
      if ( ! isEmpty() && other.isEmpty() )
        return false;

      bool equal = true;
      equal &= qgsDoubleNear( other.x(), mX, 1E-8 );
      equal &= qgsDoubleNear( other.y(), mY, 1E-8 );

      return equal;
    }

    //! Inequality operator
    bool operator!=( const QgsPointXY &other ) const SIP_HOLDGIL
    {
      if ( isEmpty() && other.isEmpty() )
        return false;
      if ( isEmpty() && !other.isEmpty() )
        return true;
      if ( ! isEmpty() && other.isEmpty() )
        return true;

      bool equal = true;
      equal &= qgsDoubleNear( other.x(), mX, 1E-8 );
      equal &= qgsDoubleNear( other.y(), mY, 1E-8 );

      return !equal;
    }

    //! Multiply x and y by the given value
    void multiply( double scalar ) SIP_HOLDGIL
    {
      mX *= scalar;
      mY *= scalar;
    }

    //! Assignment
    QgsPointXY &operator=( const QgsPointXY &other ) SIP_HOLDGIL
    {
      if ( &other != this )
      {
        mX = other.x();
        mY = other.y();
        mIsEmpty = other.isEmpty();
      }

      return *this;
    }

    //! Calculates the vector obtained by subtracting a point from this point
    QgsVector operator-( const QgsPointXY &p ) const { return QgsVector( mX - p.mX, mY - p.mY ); }

    //! Adds a vector to this point in place
    QgsPointXY &operator+=( QgsVector v ) { *this = *this + v; return *this; }

    //! Subtracts a vector from this point in place
    QgsPointXY &operator-=( QgsVector v ) { *this = *this - v; return *this; }

    //! Adds a vector to this point
    QgsPointXY operator+( QgsVector v ) const { return QgsPointXY( mX + v.x(), mY + v.y() ); }

    //! Subtracts a vector from this point
    QgsPointXY operator-( QgsVector v ) const { return QgsPointXY( mX - v.x(), mY - v.y() ); }

    //! Multiplies the coordinates in this point by a scalar quantity
    QgsPointXY operator*( double scalar ) const { return QgsPointXY( mX * scalar, mY * scalar ); }

    //! Divides the coordinates in this point by a scalar quantity
    QgsPointXY operator/( double scalar ) const { return QgsPointXY( mX / scalar, mY / scalar ); }

    //! Multiplies the coordinates in this point by a scalar quantity in place
    QgsPointXY &operator*=( double scalar ) { mX *= scalar; mY *= scalar; return *this; }

    //! Divides the coordinates in this point by a scalar quantity in place
    QgsPointXY &operator/=( double scalar ) { mX /= scalar; mY /= scalar; return *this; }

    //! Allows direct construction of QVariants from points.
    operator QVariant() const
    {
      return QVariant::fromValue( *this );
    }

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsPointXY: %1>" ).arg( sipCpp->asWkt() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End

    int __len__();
    % MethodCode
    sipRes = 2;
    % End


    SIP_PYOBJECT __getitem__( int );
    % MethodCode
    if ( a0 == 0 )
    {
      sipRes = Py_BuildValue( "d", sipCpp->x() );
    }
    else if ( a0 == 1 )
    {
      sipRes = Py_BuildValue( "d", sipCpp->y() );
    }
    else
    {
      QString msg = QString( "Bad index: %1" ).arg( a0 );
      PyErr_SetString( PyExc_IndexError, msg.toLatin1().constData() );
    }
    % End

    long __hash__() const;
    % MethodCode
    sipRes = qHash( *sipCpp );
    % End
#endif

  private:

    //! x coordinate
    double mX = 0; //std::numeric_limits<double>::quiet_NaN();

    //! y coordinate
    double mY = 0; //std::numeric_limits<double>::quiet_NaN();

    //! is point empty?
    bool mIsEmpty = true;

    friend uint qHash( const QgsPointXY &pnt );

}; // class QgsPointXY

Q_DECLARE_METATYPE( QgsPointXY )

inline bool operator==( const QgsPointXY &p1, const QgsPointXY &p2 ) SIP_SKIP
{
  const bool nan1X = std::isnan( p1.x() );
  const bool nan2X = std::isnan( p2.x() );
  if ( nan1X != nan2X )
    return false;
  if ( !nan1X && !qgsDoubleNear( p1.x(), p2.x(), 1E-8 ) )
    return false;

  const bool nan1Y = std::isnan( p1.y() );
  const bool nan2Y = std::isnan( p2.y() );
  if ( nan1Y != nan2Y )
    return false;

  if ( !nan1Y && !qgsDoubleNear( p1.y(), p2.y(), 1E-8 ) )
    return false;

  return true;
}

inline std::ostream &operator << ( std::ostream &os, const QgsPointXY &p ) SIP_SKIP
{
  // Use Local8Bit for printouts
  os << p.toString().toLocal8Bit().data();
  return os;
}

inline uint qHash( const QgsPointXY &p ) SIP_SKIP
{
  uint hash;
  const uint h1 = qHash( static_cast< quint64 >( p.mX ) );
  const uint h2 = qHash( static_cast< quint64 >( p.mY ) );
  hash = h1 ^ ( h2 << 1 );
  return hash;
}


#endif //QGSPOINTXY_H

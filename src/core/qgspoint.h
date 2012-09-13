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

#ifndef QGSPOINT_H
#define QGSPOINT_H

#include <qgis.h>

#include <iostream>
#include <QString>
#include <QPoint>

/** \ingroup core
 * A class to represent a vector.
 * Currently no Z axis / 2.5D support is implemented.
 */

class CORE_EXPORT QgsVector
{
    double m_x, m_y;

  public:
    QgsVector();
    QgsVector( double x, double y );

    QgsVector operator-( void ) const;
    QgsVector operator*( double scalar ) const;
    QgsVector operator/( double scalar ) const;
    double operator*( QgsVector v ) const;
    double length() const;

    double x() const;
    double y() const;

    // perpendicular vector (rotated 90° counter-clockwise)
    QgsVector perpVector() const;

    double angle( void ) const;
    double angle( QgsVector v ) const;
    QgsVector rotateBy( double rot ) const;
    QgsVector normal() const;

};

/** \ingroup core
 * A class to represent a point geometry.
 * Currently no Z axis / 2.5D support is implemented.
 */
class CORE_EXPORT QgsPoint
{
  public:
    /// Default constructor
    QgsPoint() : m_x( 0.0 ), m_y( 0.0 )
    {}

    /*! Create a point from another point */
    QgsPoint( const QgsPoint& p );

    /*! Create a point from x,y coordinates
     * @param x x coordinate
     * @param y y coordinate
     */
    QgsPoint( double x, double y )
        : m_x( x ), m_y( y )
    {}

    ~QgsPoint()
    {}

    /*! Sets the x value of the point
     * @param x x coordinate
     */
    void setX( double x )
    {
      m_x = x;
    }

    /*! Sets the y value of the point
     * @param y y coordinate
     */
    void setY( double y )
    {
      m_y = y;
    }

    /*! Sets the x and y value of the point */
    void set( double x, double y )
    {
      m_x = x;
      m_y = y;
    }

    /*! Get the x value of the point
     * @return x coordinate
     */
    double x() const
    {
      return m_x;
    }

    /*! Get the y value of the point
     * @return y coordinate
     */
    double y() const
    {
      return m_y;
    }

    //! String representation of the point (x,y)
    QString toString() const;

    //! As above but with precision for string representation of a point
    QString toString( int thePrecision ) const;

    /** Return a string representation as degrees minutes seconds.
     *  Its up to the calling function to ensure that this point can
     *  be meaningfully represented in this form.
     *  @note added in QGIS 1.4
     */
    QString toDegreesMinutesSeconds( int thePrecision ) const;

    /** Return a string representation as degrees minutes.
     *  Its up to the calling function to ensure that this point can
     *  be meaningfully represented in this form.
     *  @note added in QGIS 1.9
     */
    QString toDegreesMinutes( int thePrecision ) const;


    /*! Return the well known text representation for the point.
     * The wkt is created without an SRID.
     * @return Well known text in the form POINT(x y)
     */
    QString wellKnownText() const;

    /**Returns the squared distance between this point and x,y*/
    double sqrDist( double x, double y ) const;

    /**Returns the squared distance between this and other point*/
    double sqrDist( const QgsPoint& other ) const;

    /**Returns the minimum distance between this point and a segment
    @note added in QGIS 1.5*/
    double sqrDistToSegment( double x1, double y1, double x2, double y2, QgsPoint& minDistPoint, double epsilon = DEFAULT_SEGMENT_EPSILON ) const;

    /**Calculates azimut between this point and other one (clockwise in degree, starting from north)
      @note: this function has been added in version 1.7*/
    double azimuth( const QgsPoint& other );

    //! equality operator
    bool operator==( const QgsPoint &other );

    //! Inequality operator
    bool operator!=( const QgsPoint &other ) const;

    //! Assignment
    QgsPoint & operator=( const QgsPoint &other );

    //! Multiply x and y by the given value
    void multiply( const double& scalar );

    //! Test if this point is on the segment defined by points a, b
    //! @return 0 if this point is not on the open ray through a and b,
    //! 1 if point is on open ray a, 2 if point is within line segment,
    //! 3 if point is on open ray b.
    int onSegment( const QgsPoint& a, const QgsPoint& b ) const;

    QgsVector operator-( QgsPoint p ) const { return QgsVector( m_x - p.m_x, m_y - p.m_y ); }
    QgsPoint &operator+=( const QgsVector &v ) { *this = *this + v; return *this; }
    QgsPoint &operator-=( const QgsVector &v ) { *this = *this - v; return *this; }
    QgsPoint operator+( const QgsVector &v ) const { return QgsPoint( m_x + v.x(), m_y + v.y() ); }
    QgsPoint operator-( const QgsVector &v ) const { return QgsPoint( m_x - v.x(), m_y - v.y() ); }

  private:

    //! x coordinate
    double m_x;

    //! y coordinate
    double m_y;


}; // class QgsPoint


inline bool operator==( const QgsPoint &p1, const QgsPoint &p2 )
{
  if (( p1.x() == p2.x() ) && ( p1.y() == p2.y() ) )
    { return true; }
  else
    { return false; }
}

inline std::ostream& operator << ( std::ostream& os, const QgsPoint &p )
{
  // Use Local8Bit for printouts
  os << p.toString().toLocal8Bit().data();
  return os;
}

#endif //QGSPOINT_H

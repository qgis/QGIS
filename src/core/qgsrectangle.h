/***************************************************************************
                          qgsrectangle.h  -  description
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
/* $Id$ */

#ifndef QGSRECT_H
#define QGSRECT_H

#include <iosfwd>

class QString;

#include "qgspoint.h"


/** \ingroup core
 * A rectangle specified with double values.
 *
 * QgsRectangle is used to store a rectangle when double values are required.
 * Examples are storing a layer extent or the current view extent of a map
 */
class CORE_EXPORT QgsRectangle
{
  public:
    //! Constructor
    QgsRectangle( double xmin = 0, double ymin = 0, double xmax = 0, double ymax = 0 );
    //! Construct a rectangle from two points. The rectangle is normalized after construction.
    QgsRectangle( QgsPoint const & p1, QgsPoint const & p2 );
    //! Copy constructor
    QgsRectangle( const QgsRectangle &other );
    //! Destructor
    ~QgsRectangle();
    //! Set the rectangle from two QgsPoints. The rectangle is
    //normalised after construction.
    void set( const QgsPoint& p1, const QgsPoint& p2 );
    //! Set the rectangle from four points. The rectangle is
    //  normalised after construction.
    void set( double xmin, double ymin, double xmax, double ymax );
    //! Set the minimum x value
    void setXMinimum( double x );
    //! Set the maximum x value
    void setXMaximum( double x );
    //! Set the minimum y value
    void setYMinimum( double y );
    //! Set the maximum y value
    void setYMaximum( double y );
    //! Set a rectangle so that min corner is at max
    // and max corner is at min. It is NOT normalized.
    void setMinimal();
    //! Get the x maximum value (right side of rectangle)
    double xMaximum() const;
    //! Get the x maximum value (right side of rectangle)
    double xMinimum() const;
    //! Get the x minimum value (left side of rectangle)
    double yMaximum() const;
    //! Get the y maximum value (top side of rectangle)
    double yMinimum() const;
    //! Normalize the rectangle so it has non-negative width/height
    void normalize();
    //! Width of the rectangle
    double width() const;
    //! Height of the rectangle
    double height() const;
    //! Center point of the rectangle
    QgsPoint center() const;
    //! Scale the rectangle around its center point
    void scale( double, const QgsPoint *c = 0 );
    //! Expand the rectangle to support zoom out scaling
    void expand( double, const QgsPoint *c = 0 );
    //! return the intersection with the given rectangle
    QgsRectangle intersect( const QgsRectangle *rect ) const;
    //! returns true when rectangle intersects with other rectangle
    bool intersects( const QgsRectangle& rect ) const;
    //! return true when rectangle contains other rectangle
    //! @note added in version 1.1
    bool contains( const QgsRectangle& rect ) const;
    //! expand the rectangle so that covers both the original rectangle and the given rectangle
    void combineExtentWith( QgsRectangle *rect );
    //! expand the rectangle so that covers both the original rectangle and the given point
    void combineExtentWith( double x, double y );
    //! test if rectangle is empty
    bool isEmpty() const;
    //! returns string representation in Wkt form
    QString asWktCoordinates() const;
    //! returns string representation of form xmin,ymin xmax,ymax
    QString toString( bool automaticPrecision = false ) const;
    //! overloaded toString that allows precision of numbers to be set
    QString toString( int thePrecision ) const;
    //! returns rectangle s a polygon
    QString asPolygon() const;
    /*! Comparison operator
      @return True if rectangles are equal
    */
    bool operator==( const QgsRectangle &r1 ) const;
    /*! Comparison operator
    @return False if rectangles are equal
     */
    bool operator!=( const QgsRectangle &r1 ) const;
    /*! Assignment operator
     * @param r1 QgsRectangle to assign from
     */
    QgsRectangle & operator=( const QgsRectangle &r1 );

    /** updates rectangle to include passed argument */
    void unionRect( const QgsRectangle& rect );

    /** Returns true if the rectangle has finite boundaries. Will
        return false if any of the rectangle boundaries are NaN or Inf. */
    bool isFinite() const;

  protected:

    // These are protected instead of private so that things like
    // the QgsPostGisBox3d can get at them.

    double xmin;
    double ymin;
    double xmax;
    double ymax;

};


inline QgsRectangle::~QgsRectangle()
{
}

inline void QgsRectangle::setXMinimum( double x )
{
  xmin = x;
}

inline void QgsRectangle::setXMaximum( double x )
{
  xmax = x;
}

inline void QgsRectangle::setYMinimum( double y )
{
  ymin = y;
}

inline void QgsRectangle::setYMaximum( double y )
{
  ymax = y;
}

inline double QgsRectangle::xMaximum() const
{
  return xmax;
}

inline double QgsRectangle::xMinimum() const
{
  return xmin;
}

inline double QgsRectangle::yMaximum() const
{
  return ymax;
}

inline double QgsRectangle::yMinimum() const
{
  return ymin;
}

inline double QgsRectangle::width() const
{
  return xmax - xmin;
}

inline double QgsRectangle::height() const
{
  return ymax - ymin;
}

inline QgsPoint QgsRectangle::center() const
{
  return QgsPoint( xmin + width() / 2,  ymin + height() / 2 );
}
inline std::ostream& operator << ( std::ostream& os, const QgsRectangle &r )
{
  return os << r.toString().toLocal8Bit().data();
}

#endif // QGSRECT_H

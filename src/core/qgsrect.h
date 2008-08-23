/***************************************************************************
                          qgsrect.h  -  description
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
 * QgsRect is used to store a rectangle when double values are required. 
 * Examples are storing a layer extent or the current view extent of a map
 */
class CORE_EXPORT QgsRect
{
 public:
    //! Constructor
    QgsRect(double xmin=0, double ymin=0, double xmax=0, double ymax=0);
    //! Construct a rectangle from two points. The rectangle is normalized after construction.
    QgsRect(QgsPoint const & p1, QgsPoint const & p2);
    //! Copy constructor
    QgsRect(const QgsRect &other);
    //! Destructor
    ~QgsRect();
    //! Set the rectangle from two QgsPoints. The rectangle is
    //normalised after construction. 
    void set(const QgsPoint& p1, const QgsPoint& p2);
    //! Set the rectangle from four points. The rectangle is
    //  normalised after construction. 
    void set(double xmin, double ymin, double xmax, double ymax);
    //! Set the minimum x value
    void setXMinimum(double x);
    //! Set the maximum x value
    void setXMaximum(double x);
    //! Set the maximum y value
    void setYmin(double y);
    //! Set the maximum y value
    void setYmax(double y);
    //! Set a rectangle so that min corner is at max
    // and max corner is at min. It is NOT normalized.
    void setMinimal();
    //! Get the x maximum value (right side of rectangle)
    double xMax() const;
    //! Get the x maximum value (right side of rectangle)
    double xMin() const;
    //! Get the x minimum value (left side of rectangle)
    double yMax() const;
    //! Get the y maximum value (top side of rectangle)
    double yMin() const;
    //! Normalize the rectangle so it has non-negative width/height
    void normalize();
    //! Width of the rectangle
    double width() const;
    //! Height of the rectangle
    double height() const;
    //! Center point of the rectangle
    QgsPoint center() const;
    //! Scale the rectangle around its center point
    void scale(double, const QgsPoint *c =0);
    //! Expand the rectangle to support zoom out scaling
    void expand(double, const QgsPoint *c = 0);
    //! return the intersection with the given rectangle
    QgsRect intersect(QgsRect *rect) const;
    //! returns true when rectangle intersects with other rectangle
    bool intersects(const QgsRect& rect) const;
    //! expand the rectangle so that covers both the original rectangle and the given rectangle
    void combineExtentWith(QgsRect *rect);
    //! expand the rectangle so that covers both the original rectangle and the given point
    void combineExtentWith(double x, double y);
    //! test if rectangle is empty
    bool isEmpty() const;
    //! returns string representation in WKT form
    QString asWktCoordinates() const;
    //! returns string representation of form xmin,ymin xmax,ymax
    QString toString(bool automaticPrecision = false) const;
    //! overloaded toString that allows precision of numbers to be set
    QString toString(int thePrecision) const;
    //! returns rectangle s a polygon 
    QString asPolygon() const;
    /*! Comparison operator
      @return True if rectangles are equal
    */
    bool operator==(const QgsRect &r1) const;
    /*! Comparison operator
    @return False if rectangles are equal
     */
    bool operator!=(const QgsRect &r1) const;
    /*! Assignment operator
     * @param r1 QgsRect to assign from
     */
    QgsRect & operator=(const QgsRect &r1);
    
    /** updates rectangle to include passed argument */
    void unionRect(const QgsRect& rect);

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


inline QgsRect::~QgsRect()
{
}

inline void QgsRect::setXMinimum(double x)
{
    xmin = x;
}

inline void QgsRect::setXMaximum(double x)
{
    xmax = x;
}

inline void QgsRect::setYmin(double y)
{
    ymin = y;
}

inline void QgsRect::setYmax(double y)
{
    ymax = y;
}

inline double QgsRect::xMax() const
{
    return xmax;
}

inline double QgsRect::xMin() const
{
    return xmin;
}

inline double QgsRect::yMax() const
{
    return ymax;
}

inline double QgsRect::yMin() const
{
    return ymin;
}

inline double QgsRect::width() const
{
    return xmax - xmin;
}

inline double QgsRect::height() const
{
    return ymax - ymin;
}

inline QgsPoint QgsRect::center() const
{
    return QgsPoint(xmin + width() / 2 ,  ymin + height() / 2);
}
inline std::ostream& operator << (std::ostream& os, const QgsRect &r)
{
  return os << r.toString().toLocal8Bit().data();
}
  
#endif // QGSRECT_H

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

#ifndef QSTRING_H
#include <qstring.h>
#endif

#include "qgspoint.h"


/*! \class QgsRect
 * \brief A rectangle specified with double values.
 *
 * QgsRect is used to store a rectangle when double values are required. 
 * Examples are storing a layer extent or the current view extent of a map
 */
class QgsRect
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
    //! Set the minimum x value
    void setXmin(double x);
    //! Set the maximum x value
    void setXmax(double x);
    //! Set the maximum y value
    void setYmin(double y);
    //! Set the maximum y value
    void setYmax(double y);
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
    void scale(double, QgsPoint *c =0);
    //! Expand the rectangle to support zoom out scaling
    void expand(double, QgsPoint *c = 0);
    //! return the intersection with the given rectangle
    QgsRect intersect(QgsRect *rect);
    //! expand the rectangle so that covers both the original rectangle and the given rectangle
    void combineExtentWith(QgsRect *rect);
    //! test if rectangle is empty
    bool isEmpty();
    //! returns string representation in WKT form
    QString asWKTCoords() const;
    //! returns string representation of form xmin,ymin xmax,ymax
    QString stringRep(bool automaticPrecision = false) const;
    //! overloaded stringRep that allows precision of numbers to be set
    QString stringRep(int thePrecision) const;
    //! returns rectangle s a polygon 
    QString asPolygon() const;
    /*! Comparison operator
      @return True if rectangles are equal
    */
    bool operator==(const QgsRect &r1);
    /*! Assignment operator
     * @param r1 QgsRect to assign from
     */
    QgsRect & operator=(const QgsRect &r1);
 
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

inline void QgsRect::setXmin(double x)
{
    xmin = x;
}

inline void QgsRect::setXmax(double x)
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
    return os << r.stringRep();
}
  
#endif // QGSRECT_H

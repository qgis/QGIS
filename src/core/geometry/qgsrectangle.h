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

#ifndef QGSRECTANGLE_H
#define QGSRECTANGLE_H

#include "qgis_core.h"
#include <iosfwd>
#include <QDomDocument>

class QString;
class QRectF;
class QgsBox3d;
#include "qgspointxy.h"


/**
 * \ingroup core
 * A rectangle specified with double values.
 *
 * QgsRectangle is used to store a rectangle when double values are required.
 * Examples are storing a layer extent or the current view extent of a map
 * \see QgsBox3d
 */
class CORE_EXPORT QgsRectangle
{
  public:
    //! Constructor
    QgsRectangle( double xMin = 0, double yMin = 0, double xMax = 0, double yMax = 0 );
    //! Construct a rectangle from two points. The rectangle is normalized after construction.
    QgsRectangle( const QgsPointXY &p1, const QgsPointXY &p2 );
    //! Construct a rectangle from a QRectF. The rectangle is normalized after construction.
    QgsRectangle( const QRectF &qRectF );
    //! Copy constructor
    QgsRectangle( const QgsRectangle &other );

    // IMPORTANT - while QgsRectangle is inherited by QgsReferencedRectangle, we do NOT want a virtual destructor here
    // because this class MUST be lightweight and we don't want the cost of the vtable here.
    // see https://github.com/qgis/QGIS/pull/4720#issuecomment-308652392
    ~QgsRectangle() = default;

    /**
    * Creates a new rectangle from a \a wkt string.
    * The WKT must contain only 5 vertices, representing a rectangle aligned with X and Y axes.
    * \since QGIS 3.0
    */
    static QgsRectangle fromWkt( const QString &wkt );

    /**
     * Creates a new rectangle, given the specified \a center point
     * and \a width and \a height.
     * \since QGIS 3.0
     */
    static QgsRectangle fromCenterAndSize( QgsPointXY center, double width, double height );

    /**
     * Sets the rectangle from two QgsPoints. The rectangle is
     * normalised after construction.
     */
    void set( const QgsPointXY &p1, const QgsPointXY &p2 );

    /**
     * Sets the rectangle from four points. The rectangle is
     * normalised after construction.
     */
    void set( double mXmin, double mYmin, double mXmax, double mYmax );

    /**
     * Set the minimum x value.
     */
    void setXMinimum( double x ) { mXmin = x; }

    /**
     * Set the maximum x value.
     */
    void setXMaximum( double x ) { mXmax = x; }

    /**
     * Set the minimum y value.
     */
    void setYMinimum( double y ) { mYmin = y; }

    /**
     * Set the maximum y value.
     */
    void setYMaximum( double y ) { mYmax = y; }

    /**
     * Set a rectangle so that min corner is at max
     * and max corner is at min. It is NOT normalized.
     */
    void setMinimal();

    /**
     * Returns the x maximum value (right side of rectangle).
     */
    double xMaximum() const { return mXmax; }

    /**
     * Returns the x minimum value (left side of rectangle).
     */
    double xMinimum() const { return mXmin; }

    /**
     * Returns the y maximum value (top side of rectangle).
     */
    double yMaximum() const { return mYmax; }

    /**
     * Returns the y minimum value (bottom side of rectangle).
     */
    double yMinimum() const { return mYmin; }

    /**
     * Normalize the rectangle so it has non-negative width/height.
     */
    void normalize();

    /**
     * Returns the width of the rectangle.
     * \see height()
     * \see area()
     */
    double width() const { return mXmax - mXmin; }

    /**
     * Returns the height of the rectangle.
     * \see width()
     * \see area()
     */
    double height() const { return mYmax - mYmin; }

    /**
     * Returns the area of the rectangle.
     * \since QGIS 3.0
     * \see width()
     * \see height()
     * \see perimeter()
     */
    double area() const { return ( mXmax - mXmin ) * ( mYmax - mYmin ); }

    /**
     * Returns the perimeter of the rectangle.
     * \since QGIS 3.0
     * \see area()
     */
    double perimeter() const { return 2 * ( mXmax - mXmin ) + 2 * ( mYmax - mYmin ); }

    /**
     * Returns the center point of the rectangle.
     */
    QgsPointXY center() const { return QgsPointXY( mXmin + width() / 2, mYmin + height() / 2 ); }

    /**
     * Scale the rectangle around its center point.
     */
    void scale( double scaleFactor, const QgsPointXY *c = nullptr );

    /**
     * Scale the rectangle around its center point.
     */
    void scale( double scaleFactor, double centerX, double centerY );

    /**
     * Grows the rectangle in place by the specified amount.
     * \see buffered()
     */
    void grow( double delta );

    /**
     * Updates the rectangle to include the specified point.
     */
    void include( const QgsPointXY &p );

    /**
     * Get rectangle enlarged by buffer.
     * \note In earlier QGIS releases this method was named buffer().
     * \since QGIS 3.0
     * \see grow()
    */
    QgsRectangle buffered( double width ) const;

    /**
     * Return the intersection with the given rectangle.
     */
    QgsRectangle intersect( const QgsRectangle *rect ) const;

    /**
     * Returns true when rectangle intersects with other rectangle.
     */
    bool intersects( const QgsRectangle &rect ) const;

    /**
     * Return true when rectangle contains other rectangle.
     */
    bool contains( const QgsRectangle &rect ) const;

    /**
     * Return true when rectangle contains a point.
     */
    bool contains( const QgsPointXY &p ) const;

    /**
     * Expand the rectangle so that covers both the original rectangle and the given rectangle.
     */
    void combineExtentWith( const QgsRectangle &rect );

    /**
     * Expand the rectangle so that covers both the original rectangle and the given point.
     */
    void combineExtentWith( double x, double y );

    /**
     * Returns a rectangle offset from this one in the direction of the reversed vector.
     * \since QGIS 3.0
     */
    QgsRectangle operator-( const QgsVector v ) const;

    /**
     * Returns a rectangle offset from this one in the direction of the vector.
     * \since QGIS 3.0
     */
    QgsRectangle operator+( const QgsVector v ) const;

    /**
     * Moves this rectangle in the direction of the reversed vector.
     * \since QGIS 3.0
     */
    QgsRectangle &operator-=( const QgsVector v );

    /**
     * Moves this rectangle in the direction of the vector.
     * \since QGIS 3.0
     */
    QgsRectangle &operator+=( const QgsVector v );

    /**
     * Returns true if the rectangle is empty.
     * An empty rectangle may still be non-null if it contains valid information (e.g. bounding box of a point).
     */
    bool isEmpty() const;

    /**
     * Test if the rectangle is null (all coordinates zero or after call to setMinimal()).
     * A null rectangle is also an empty rectangle.
     * \since QGIS 2.4
     */
    bool isNull() const;

    /**
     * Returns a string representation of the rectangle in WKT format.
     */
    QString asWktCoordinates() const;

    /**
     * Returns a string representation of the rectangle as a WKT Polygon.
     */
    QString asWktPolygon() const;

    /**
     * Returns a QRectF with same coordinates as the rectangle.
     */
    QRectF toRectF() const;

    /**
     * Returns a string representation of form xmin,ymin : xmax,ymax
     * Coordinates will be truncated to the specified precision.
     * If the specified precision is less than 0, a suitable minimum precision is used.
     */
    QString toString( int precision = 16 ) const;

    /**
     * Returns the rectangle as a polygon.
     */
    QString asPolygon() const;

    /**
     * Comparison operator
     * \returns True if rectangles are equal
     */
    bool operator==( const QgsRectangle &r1 ) const;

    /**
     * Comparison operator
     * \returns False if rectangles are equal
     */
    bool operator!=( const QgsRectangle &r1 ) const;

    /**
     * Assignment operator
     * \param r1 QgsRectangle to assign from
     */
    QgsRectangle &operator=( const QgsRectangle &r1 );

    /**
     * Returns true if the rectangle has finite boundaries. Will
     * return false if any of the rectangle boundaries are NaN or Inf.
     */
    bool isFinite() const;

    /**
     * Swap x/y coordinates in the rectangle.
     */
    void invert();

    /**
     * Converts the rectangle to a 3D box, with the specified
     * \a zMin and \a zMax z values.
     * \since QGIS 3.0
     */
    QgsBox3d toBox3d( double zMin, double zMax ) const;

    //! Allows direct construction of QVariants from rectangles.
    operator QVariant() const
    {
      return QVariant::fromValue( *this );
    }

  private:

    double mXmin;
    double mYmin;
    double mXmax;
    double mYmax;

};

Q_DECLARE_METATYPE( QgsRectangle )

#ifndef SIP_RUN

/**
 * Writes the list rectangle to stream out. QGIS version compatibility is not guaranteed.
 */
CORE_EXPORT QDataStream &operator<<( QDataStream &out, const QgsRectangle &rectangle );

/**
 * Reads a rectangle from stream in into rectangle. QGIS version compatibility is not guaranteed.
 */
CORE_EXPORT QDataStream &operator>>( QDataStream &in, QgsRectangle &rectangle );

inline std::ostream &operator << ( std::ostream &os, const QgsRectangle &r )
{
  return os << r.toString().toLocal8Bit().data();
}

#endif

#endif // QGSRECTANGLE_H

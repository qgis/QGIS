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
#include "qgis.h"
#include <iosfwd>
#include <QDomDocument>
#include <QRectF>

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

    //! Constructor for a null rectangle
    QgsRectangle() = default; // optimised constructor for null rectangle - no need to call normalize here

    //! Constructor
    explicit QgsRectangle( double xMin, double yMin = 0, double xMax = 0, double yMax = 0 )
      : mXmin( xMin )
      , mYmin( yMin )
      , mXmax( xMax )
      , mYmax( yMax )
    {
      normalize();
    }

    //! Construct a rectangle from two points. The rectangle is normalized after construction.
    QgsRectangle( const QgsPointXY &p1, const QgsPointXY &p2 )
    {
      set( p1, p2 );
    }

    //! Construct a rectangle from a QRectF. The rectangle is normalized after construction.
    QgsRectangle( const QRectF &qRectF )
    {
      mXmin = qRectF.topLeft().x();
      mYmin = qRectF.topLeft().y();
      mXmax = qRectF.bottomRight().x();
      mYmax = qRectF.bottomRight().y();
    }

    //! Copy constructor
    QgsRectangle( const QgsRectangle &other )
    {
      mXmin = other.xMinimum();
      mYmin = other.yMinimum();
      mXmax = other.xMaximum();
      mYmax = other.yMaximum();
    }

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
    void set( const QgsPointXY &p1, const QgsPointXY &p2 )
    {
      mXmin = p1.x();
      mXmax = p2.x();
      mYmin = p1.y();
      mYmax = p2.y();
      normalize();
    }

    /**
     * Sets the rectangle from four points. The rectangle is
     * normalised after construction.
     */
    void set( double xMin, double yMin, double xMax, double yMax )
    {
      mXmin = xMin;
      mYmin = yMin;
      mXmax = xMax;
      mYmax = yMax;
      normalize();
    }

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
    void setMinimal()
    {
      mXmin = std::numeric_limits<double>::max();
      mYmin = std::numeric_limits<double>::max();
      mXmax = -std::numeric_limits<double>::max();
      mYmax = -std::numeric_limits<double>::max();
    }

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
    void normalize()
    {
      if ( isNull() )
        return;

      if ( mXmin > mXmax )
      {
        std::swap( mXmin, mXmax );
      }
      if ( mYmin > mYmax )
      {
        std::swap( mYmin, mYmax );
      }
    }

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
     * \see width()
     * \see height()
     * \see perimeter()
     * \since QGIS 3.0
     */
    double area() const { return ( mXmax - mXmin ) * ( mYmax - mYmin ); }

    /**
     * Returns the perimeter of the rectangle.
     * \see area()
     * \since QGIS 3.0
     */
    double perimeter() const { return 2 * ( mXmax - mXmin ) + 2 * ( mYmax - mYmin ); }

    /**
     * Returns the center point of the rectangle.
     */
    QgsPointXY center() const { return QgsPointXY( mXmin + width() / 2, mYmin + height() / 2 ); }

    /**
     * Scale the rectangle around its center point.
     */
    void scale( double scaleFactor, const QgsPointXY *c = nullptr )
    {
      // scale from the center
      double centerX, centerY;
      if ( c )
      {
        centerX = c->x();
        centerY = c->y();
      }
      else
      {
        centerX = mXmin + width() / 2;
        centerY = mYmin + height() / 2;
      }
      scale( scaleFactor, centerX, centerY );
    }

    /**
     * Scale the rectangle around its center point.
     */
    void scale( double scaleFactor, double centerX, double centerY )
    {
      double newWidth = width() * scaleFactor;
      double newHeight = height() * scaleFactor;
      mXmin = centerX - newWidth / 2.0;
      mXmax = centerX + newWidth / 2.0;
      mYmin = centerY - newHeight / 2.0;
      mYmax = centerY + newHeight / 2.0;
    }

    /**
     * Scale the rectangle around its \a center point.
     * \since QGIS 3.4
     */
    QgsRectangle scaled( double scaleFactor, const QgsPointXY *center = nullptr ) const;

    /**
     * Grows the rectangle in place by the specified amount.
     * \see buffered()
     */
    void grow( double delta )
    {
      mXmin -= delta;
      mXmax += delta;
      mYmin -= delta;
      mYmax += delta;
    }

    /**
     * Updates the rectangle to include the specified point.
     */
    void include( const QgsPointXY &p )
    {
      if ( p.x() < xMinimum() )
        setXMinimum( p.x() );
      else if ( p.x() > xMaximum() )
        setXMaximum( p.x() );
      if ( p.y() < yMinimum() )
        setYMinimum( p.y() );
      if ( p.y() > yMaximum() )
        setYMaximum( p.y() );
    }

    /**
     * Gets rectangle enlarged by buffer.
     * \note In earlier QGIS releases this method was named buffer().
     * \see grow()
     * \since QGIS 3.0
    */
    QgsRectangle buffered( double width ) const
    {
      return QgsRectangle( mXmin - width, mYmin - width, mXmax + width, mYmax + width );
    }

    /**
     * Returns the intersection with the given rectangle.
     */
    QgsRectangle intersect( const QgsRectangle &rect ) const
    {
      QgsRectangle intersection = QgsRectangle();
      if ( intersects( rect ) )
      {
        intersection.setXMinimum( mXmin > rect.xMinimum() ? mXmin : rect.xMinimum() );
        intersection.setXMaximum( mXmax < rect.xMaximum() ? mXmax : rect.xMaximum() );
        intersection.setYMinimum( mYmin > rect.yMinimum() ? mYmin : rect.yMinimum() );
        intersection.setYMaximum( mYmax < rect.yMaximum() ? mYmax : rect.yMaximum() );
      }
      return intersection;
    }

    /**
     * Returns true when rectangle intersects with other rectangle.
     */
    bool intersects( const QgsRectangle &rect ) const
    {
      double x1 = ( mXmin > rect.mXmin ? mXmin : rect.mXmin );
      double x2 = ( mXmax < rect.mXmax ? mXmax : rect.mXmax );
      if ( x1 > x2 )
        return false;
      double y1 = ( mYmin > rect.mYmin ? mYmin : rect.mYmin );
      double y2 = ( mYmax < rect.mYmax ? mYmax : rect.mYmax );
      return y1 <= y2;
    }

    /**
     * Returns true when rectangle contains other rectangle.
     */
    bool contains( const QgsRectangle &rect ) const
    {
      return ( rect.mXmin >= mXmin && rect.mXmax <= mXmax && rect.mYmin >= mYmin && rect.mYmax <= mYmax );
    }

    /**
     * Returns true when rectangle contains a point.
     */
    bool contains( const QgsPointXY &p ) const
    {
      return mXmin <= p.x() && p.x() <= mXmax &&
             mYmin <= p.y() && p.y() <= mYmax;
    }

    /**
     * Expands the rectangle so that it covers both the original rectangle and the given rectangle.
     */
    void combineExtentWith( const QgsRectangle &rect )
    {
      if ( isNull() )
        *this = rect;
      else if ( !rect.isNull() )
      {
        mXmin = std::min( mXmin, rect.xMinimum() );
        mXmax = std::max( mXmax, rect.xMaximum() );
        mYmin = std::min( mYmin, rect.yMinimum() );
        mYmax = std::max( mYmax, rect.yMaximum() );;
      }
    }

    /**
     * Expands the rectangle so that it covers both the original rectangle and the given point.
     */
    void combineExtentWith( double x, double y )
    {
      if ( isNull() )
        *this = QgsRectangle( x, y, x, y );
      else
      {
        mXmin = ( ( mXmin < x ) ? mXmin : x );
        mXmax = ( ( mXmax > x ) ? mXmax : x );

        mYmin = ( ( mYmin < y ) ? mYmin : y );
        mYmax = ( ( mYmax > y ) ? mYmax : y );
      }
    }

    /**
     * Expands the rectangle so that it covers both the original rectangle and the given point.
     * \since QGIS 3.2
     */
    void combineExtentWith( const QgsPointXY &point )
    {
      combineExtentWith( point.x(), point.y() );
    }

    /**
     * Returns a rectangle offset from this one in the direction of the reversed vector.
     * \since QGIS 3.0
     */
    QgsRectangle operator-( QgsVector v ) const;

    /**
     * Returns a rectangle offset from this one in the direction of the vector.
     * \since QGIS 3.0
     */
    QgsRectangle operator+( QgsVector v ) const;

    /**
     * Moves this rectangle in the direction of the reversed vector.
     * \since QGIS 3.0
     */
    QgsRectangle &operator-=( QgsVector v );

    /**
     * Moves this rectangle in the direction of the vector.
     * \since QGIS 3.0
     */
    QgsRectangle &operator+=( QgsVector v );

    /**
     * Returns true if the rectangle is empty.
     * An empty rectangle may still be non-null if it contains valid information (e.g. bounding box of a point).
     */
    bool isEmpty() const
    {
      return mXmax < mXmin || mYmax < mYmin || qgsDoubleNear( mXmax, mXmin ) || qgsDoubleNear( mYmax, mYmin );
    }

    /**
     * Test if the rectangle is null (all coordinates zero or after call to setMinimal()).
     * A null rectangle is also an empty rectangle.
     * \since QGIS 2.4
     */
    bool isNull() const
    {
      // rectangle created QgsRectangle() or with rect.setMinimal() ?
      return ( qgsDoubleNear( mXmin, 0.0 ) && qgsDoubleNear( mXmax, 0.0 ) && qgsDoubleNear( mYmin, 0.0 ) && qgsDoubleNear( mYmax, 0.0 ) ) ||
             ( qgsDoubleNear( mXmin, std::numeric_limits<double>::max() ) && qgsDoubleNear( mYmin, std::numeric_limits<double>::max() ) &&
               qgsDoubleNear( mXmax, -std::numeric_limits<double>::max() ) && qgsDoubleNear( mYmax, -std::numeric_limits<double>::max() ) );
    }

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
    QRectF toRectF() const
    {
      return QRectF( static_cast< qreal >( mXmin ), static_cast< qreal >( mYmin ), static_cast< qreal >( mXmax - mXmin ), static_cast< qreal >( mYmax - mYmin ) );
    }

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
    bool operator==( const QgsRectangle &r1 ) const
    {
      return qgsDoubleNear( r1.xMaximum(), xMaximum() ) &&
             qgsDoubleNear( r1.xMinimum(), xMinimum() ) &&
             qgsDoubleNear( r1.yMaximum(), yMaximum() ) &&
             qgsDoubleNear( r1.yMinimum(), yMinimum() );
    }

    /**
     * Comparison operator
     * \returns False if rectangles are equal
     */
    bool operator!=( const QgsRectangle &r1 ) const
    {
      return ( ! operator==( r1 ) );
    }

    /**
     * Assignment operator
     * \param r1 QgsRectangle to assign from
     */
    QgsRectangle &operator=( const QgsRectangle &r1 )
    {
      if ( &r1 != this )
      {
        mXmax = r1.xMaximum();
        mXmin = r1.xMinimum();
        mYmax = r1.yMaximum();
        mYmin = r1.yMinimum();
      }

      return *this;
    }

    /**
     * Returns true if the rectangle has finite boundaries. Will
     * return false if any of the rectangle boundaries are NaN or Inf.
     */
    bool isFinite() const
    {
      if ( std::isinf( mXmin ) || std::isinf( mYmin ) || std::isinf( mXmax ) || std::isinf( mYmax ) )
      {
        return false;
      }
      if ( std::isnan( mXmin ) || std::isnan( mYmin ) || std::isnan( mXmax ) || std::isnan( mYmax ) )
      {
        return false;
      }
      return true;
    }

    /**
     * Swap x/y coordinates in the rectangle.
     */
    void invert()
    {
      std::swap( mXmin, mYmin );
      std::swap( mXmax, mYmax );
    }

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

    /**
     * Returns a copy of this rectangle that is snapped to a grid with
     * the specified \a spacing between the grid lines.
     *
     * \since QGIS 3.4
     */
    QgsRectangle snappedToGrid( double spacing ) const;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsRectangle: %1>" ).arg( sipCpp->asWktCoordinates() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

  private:

    double mXmin = 0.0;
    double mYmin = 0.0;
    double mXmax = 0.0;
    double mYmax = 0.0;

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

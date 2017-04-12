/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgsrectangle.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsRectangle
{
%Docstring
 A rectangle specified with double values.

 QgsRectangle is used to store a rectangle when double values are required.
 Examples are storing a layer extent or the current view extent of a map
 \see QgsBox3d
%End

%TypeHeaderCode
#include "qgsrectangle.h"
%End
  public:
    QgsRectangle( double xMin = 0, double yMin = 0, double xMax = 0, double yMax = 0 );
%Docstring
Constructor
%End
    QgsRectangle( const QgsPoint &p1, const QgsPoint &p2 );
%Docstring
Construct a rectangle from two points. The rectangle is normalized after construction.
%End
    QgsRectangle( const QRectF &qRectF );
%Docstring
Construct a rectangle from a QRectF. The rectangle is normalized after construction.
%End
    QgsRectangle( const QgsRectangle &other );
%Docstring
Copy constructor
%End

    void set( const QgsPoint &p1, const QgsPoint &p2 );
%Docstring
 Sets the rectangle from two QgsPoints. The rectangle is
 normalised after construction.
%End

    void set( double mXmin, double mYmin, double mXmax, double mYmax );
%Docstring
 Sets the rectangle from four points. The rectangle is
 normalised after construction.
%End

    void setXMinimum( double x );
%Docstring
 Set the minimum x value.
%End

    void setXMaximum( double x );
%Docstring
 Set the maximum x value.
%End

    void setYMinimum( double y );
%Docstring
 Set the minimum y value.
%End

    void setYMaximum( double y );
%Docstring
 Set the maximum y value.
%End

    void setMinimal();
%Docstring
 Set a rectangle so that min corner is at max
 and max corner is at min. It is NOT normalized.
%End

    double xMaximum() const;
%Docstring
 Returns the x maximum value (right side of rectangle).
 :rtype: float
%End

    double xMinimum() const;
%Docstring
 Returns the x minimum value (left side of rectangle).
 :rtype: float
%End

    double yMaximum() const;
%Docstring
 Returns the y maximum value (top side of rectangle).
 :rtype: float
%End

    double yMinimum() const;
%Docstring
 Returns the y minimum value (bottom side of rectangle).
 :rtype: float
%End

    void normalize();
%Docstring
 Normalize the rectangle so it has non-negative width/height.
%End

    double width() const;
%Docstring
 Returns the width of the rectangle.
 \see height()
 \see area()
 :rtype: float
%End

    double height() const;
%Docstring
 Returns the height of the rectangle.
 \see width()
 \see area()
 :rtype: float
%End

    double area() const;
%Docstring
 Returns the area of the rectangle.
.. versionadded:: 3.0
 \see width()
 \see height()
 \see perimeter()
 :rtype: float
%End

    double perimeter() const;
%Docstring
 Returns the perimeter of the rectangle.
.. versionadded:: 3.0
 \see area()
 :rtype: float
%End

    QgsPoint center() const;
%Docstring
 Returns the center point of the rectangle.
 :rtype: QgsPoint
%End

    void scale( double scaleFactor, const QgsPoint *c = 0 );
%Docstring
 Scale the rectangle around its center point.
%End

    void scale( double scaleFactor, double centerX, double centerY );
%Docstring
 Scale the rectangle around its center point.
%End

    void grow( double delta );
%Docstring
 Grows the rectangle by the specified amount.
%End

    void include( const QgsPoint &p );
%Docstring
 Updates the rectangle to include the specified point.
%End

    QgsRectangle buffer( double width );
%Docstring
 Get rectangle enlarged by buffer.
.. versionadded:: 2.1
 :rtype: QgsRectangle
%End

    QgsRectangle intersect( const QgsRectangle *rect ) const;
%Docstring
 Return the intersection with the given rectangle.
 :rtype: QgsRectangle
%End

    bool intersects( const QgsRectangle &rect ) const;
%Docstring
 Returns true when rectangle intersects with other rectangle.
 :rtype: bool
%End

    bool contains( const QgsRectangle &rect ) const;
%Docstring
 Return true when rectangle contains other rectangle.
 :rtype: bool
%End

    bool contains( const QgsPoint &p ) const;
%Docstring
 Return true when rectangle contains a point.
 :rtype: bool
%End

    void combineExtentWith( const QgsRectangle &rect );
%Docstring
 Expand the rectangle so that covers both the original rectangle and the given rectangle.
%End

    void combineExtentWith( double x, double y );
%Docstring
 Expand the rectangle so that covers both the original rectangle and the given point.
%End

    bool isEmpty() const;
%Docstring
 Returns true if the rectangle is empty.
 An empty rectangle may still be non-null if it contains valid information (e.g. bounding box of a point).
 :rtype: bool
%End

    bool isNull() const;
%Docstring
 Test if the rectangle is null (all coordinates zero or after call to setMinimal()).
 A null rectangle is also an empty rectangle.
.. versionadded:: 2.4
 :rtype: bool
%End

    QString asWktCoordinates() const;
%Docstring
 Returns a string representation of the rectangle in WKT format.
 :rtype: str
%End

    QString asWktPolygon() const;
%Docstring
 Returns a string representation of the rectangle as a WKT Polygon.
 :rtype: str
%End

    QRectF toRectF() const;
%Docstring
 Returns a QRectF with same coordinates as the rectangle.
 :rtype: QRectF
%End

    QString toString( int precision = 16 ) const;
%Docstring
 Returns a string representation of form xmin,ymin : xmax,ymax
 Coordinates will be truncated to the specified precision.
 If the specified precision is less than 0, a suitable minimum precision is used.
 :rtype: str
%End

    QString asPolygon() const;
%Docstring
 Returns the rectangle as a polygon.
 :rtype: str
%End

    bool operator==( const QgsRectangle &r1 ) const;
%Docstring
 Comparison operator
 :return: True if rectangles are equal
 :rtype: bool
%End

    bool operator!=( const QgsRectangle &r1 ) const;
%Docstring
 Comparison operator
 :return: False if rectangles are equal
 :rtype: bool
%End

//     QgsRectangle &operator=( const QgsRectangle &r1 );

    void unionRect( const QgsRectangle &rect );
%Docstring
 Updates the rectangle to include another rectangle.
%End

    bool isFinite() const;
%Docstring
 Returns true if the rectangle has finite boundaries. Will
 return false if any of the rectangle boundaries are NaN or Inf.
 :rtype: bool
%End

    void invert();
%Docstring
 Swap x/y coordinates in the rectangle.
%End

    QgsBox3d toBox3d( double zMin, double zMax ) const;
%Docstring
 Converts the rectangle to a 3D box, with the specified
 zMin and zMax z values.
.. versionadded:: 3.0
 :rtype: QgsBox3d
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgsrectangle.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/

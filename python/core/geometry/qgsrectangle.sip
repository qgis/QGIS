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

.. seealso:: :py:class:`QgsBox3d`
%End

%TypeHeaderCode
#include "qgsrectangle.h"
%End
  public:
    QgsRectangle( double xMin = 0, double yMin = 0, double xMax = 0, double yMax = 0 );
%Docstring
Constructor
%End
    QgsRectangle( const QgsPointXY &p1, const QgsPointXY &p2 );
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

    ~QgsRectangle();

    static QgsRectangle fromWkt( const QString &wkt );
%Docstring
Creates a new rectangle from a ``wkt`` string.
The WKT must contain only 5 vertices, representing a rectangle aligned with X and Y axes.

.. versionadded:: 3.0
%End

    static QgsRectangle fromCenterAndSize( QgsPointXY center, double width, double height );
%Docstring
Creates a new rectangle, given the specified ``center`` point
and ``width`` and ``height``.

.. versionadded:: 3.0
%End

    void set( const QgsPointXY &p1, const QgsPointXY &p2 );
%Docstring
Sets the rectangle from two :py:class:`QgsPoints`. The rectangle is
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
%End

    double xMinimum() const;
%Docstring
Returns the x minimum value (left side of rectangle).
%End

    double yMaximum() const;
%Docstring
Returns the y maximum value (top side of rectangle).
%End

    double yMinimum() const;
%Docstring
Returns the y minimum value (bottom side of rectangle).
%End

    void normalize();
%Docstring
Normalize the rectangle so it has non-negative width/height.
%End

    double width() const;
%Docstring
Returns the width of the rectangle.

.. seealso:: :py:func:`height`

.. seealso:: :py:func:`area`
%End

    double height() const;
%Docstring
Returns the height of the rectangle.

.. seealso:: :py:func:`width`

.. seealso:: :py:func:`area`
%End

    double area() const;
%Docstring
Returns the area of the rectangle.

.. versionadded:: 3.0

.. seealso:: :py:func:`width`

.. seealso:: :py:func:`height`

.. seealso:: :py:func:`perimeter`
%End

    double perimeter() const;
%Docstring
Returns the perimeter of the rectangle.

.. versionadded:: 3.0

.. seealso:: :py:func:`area`
%End

    QgsPointXY center() const;
%Docstring
Returns the center point of the rectangle.
%End

    void scale( double scaleFactor, const QgsPointXY *c = 0 );
%Docstring
Scale the rectangle around its center point.
%End

    void scale( double scaleFactor, double centerX, double centerY );
%Docstring
Scale the rectangle around its center point.
%End

    void grow( double delta );
%Docstring
Grows the rectangle in place by the specified amount.

.. seealso:: :py:func:`buffered`
%End

    void include( const QgsPointXY &p );
%Docstring
Updates the rectangle to include the specified point.
%End

    QgsRectangle buffered( double width ) const;
%Docstring
Get rectangle enlarged by buffer.

.. note::

   In earlier QGIS releases this method was named buffer().

.. versionadded:: 3.0

.. seealso:: :py:func:`grow`
%End

    QgsRectangle intersect( const QgsRectangle *rect ) const;
%Docstring
Return the intersection with the given rectangle.
%End

    bool intersects( const QgsRectangle &rect ) const;
%Docstring
Returns true when rectangle intersects with other rectangle.
%End

    bool contains( const QgsRectangle &rect ) const;
%Docstring
Return true when rectangle contains other rectangle.
%End

    bool contains( const QgsPointXY &p ) const;
%Docstring
Return true when rectangle contains a point.
%End

    void combineExtentWith( const QgsRectangle &rect );
%Docstring
Expand the rectangle so that covers both the original rectangle and the given rectangle.
%End

    void combineExtentWith( double x, double y );
%Docstring
Expand the rectangle so that covers both the original rectangle and the given point.
%End

    QgsRectangle operator-( const QgsVector v ) const;

    QgsRectangle operator+( const QgsVector v ) const;

    QgsRectangle &operator-=( const QgsVector v );

    QgsRectangle &operator+=( const QgsVector v );

    bool isEmpty() const;
%Docstring
Returns true if the rectangle is empty.
An empty rectangle may still be non-null if it contains valid information (e.g. bounding box of a point).
%End

    bool isNull() const;
%Docstring
Test if the rectangle is null (all coordinates zero or after call to setMinimal()).
A null rectangle is also an empty rectangle.

.. versionadded:: 2.4
%End

    QString asWktCoordinates() const;
%Docstring
Returns a string representation of the rectangle in WKT format.
%End

    QString asWktPolygon() const;
%Docstring
Returns a string representation of the rectangle as a WKT Polygon.
%End

    QRectF toRectF() const;
%Docstring
Returns a QRectF with same coordinates as the rectangle.
%End

    QString toString( int precision = 16 ) const;
%Docstring
Returns a string representation of form xmin,ymin : xmax,ymax
Coordinates will be truncated to the specified precision.
If the specified precision is less than 0, a suitable minimum precision is used.
%End

    QString asPolygon() const;
%Docstring
Returns the rectangle as a polygon.
%End

    bool operator==( const QgsRectangle &r1 ) const;

    bool operator!=( const QgsRectangle &r1 ) const;


    bool isFinite() const;
%Docstring
Returns true if the rectangle has finite boundaries. Will
return false if any of the rectangle boundaries are NaN or Inf.
%End

    void invert();
%Docstring
Swap x/y coordinates in the rectangle.
%End

    QgsBox3d toBox3d( double zMin, double zMax ) const;
%Docstring
Converts the rectangle to a 3D box, with the specified
``zMin`` and ``zMax`` z values.

.. versionadded:: 3.0
%End

    operator QVariant() const;

};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgsrectangle.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgsregularpolygon.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsRegularPolygon
{
%Docstring
 Regular Polygon geometry type.

A regular polygon is a polygon that is equiangular (all angles are equal in measure) and equilateral (all sides have the same length).
The regular polygon is defined by a center point with a number of sides/vertices, a radius and the first vertex.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsregularpolygon.h"
%End
  public:

    enum ConstructionOption
    {
      InscribedCircle,
      CircumscribedCircle
    };

    QgsRegularPolygon();
%Docstring
Constructor for QgsRegularPolygon.
%End

    QgsRegularPolygon( const QgsPoint &center, const double radius, const double azimuth, const unsigned int numberSides, const ConstructionOption circle );
%Docstring
Constructs a regular polygon by ``center`` and parameters for the first vertex. An empty regular polygon is returned if ``numberSides`` < 3 or ``ConstructionOption`` isn't valid.

:param center: The center of the regular polygon.
:param radius: Distance from the center and the first vertex or sides (see ``ConstructionOption``).
:param azimuth: Angle in degrees started from the North to the first vertex.
:param numberSides: Number of sides of the regular polygon.
:param circle: Option to create the polygon. see ConstructionOption
%End

    QgsRegularPolygon( const QgsPoint &center, const QgsPoint &pt1, const unsigned int numberSides, const ConstructionOption circle );
%Docstring
Constructs a regular polygon by ``center`` and another point.

:param center: The center of the regular polygon.
:param pt1: The first vertex if the polygon is inscribed in circle or the midpoint of a side if the polygon is circumscribed about circle.
:param numberSides: Number of sides of the regular polygon.
:param circle: Option to create the polygon inscribed in circle (the radius is the distance between the center and vertices) or circumscribed about circle (the radius is the distance from the center to the midpoints of the sides).
%End

    QgsRegularPolygon( const QgsPoint &pt1, const QgsPoint &pt2, const unsigned int numberSides );
%Docstring
Constructs a regular polygon by two points of the first side.

:param pt1: The first vertex of the first side, also first vertex of the regular polygon.
:param pt2: The second vertex of the first side.
:param numberSides: Number of sides of the regular polygon.
%End

    bool operator ==( const QgsRegularPolygon &rp ) const;
    bool operator !=( const QgsRegularPolygon &rp ) const;

    bool isEmpty() const;
%Docstring
A regular polygon is empty if radius equal to 0 or number of sides < 3
%End

    QgsPoint center() const;
%Docstring
Returns the center point of the regular polygon.

.. seealso:: :py:func:`setCenter`
%End

    double radius() const;
%Docstring
Returns the radius.
This is also the radius of the circumscribing circle.

.. seealso:: :py:func:`apothem`

.. seealso:: :py:func:`setRadius`
%End

    QgsPoint firstVertex() const;
%Docstring
Returns the first vertex (corner) of the regular polygon.

.. seealso:: :py:func:`setFirstVertex`
%End

    double apothem() const;
%Docstring
Returns the apothem of the regular polygon.
The apothem is the radius of the inscribed circle.

.. seealso:: :py:func:`radius`
%End

    unsigned int numberSides() const;
%Docstring
Returns the number of sides of the regular polygon.

.. seealso:: :py:func:`setNumberSides`
%End

    void setCenter( const QgsPoint &center );
%Docstring
Sets the center point.
Radius is unchanged. The first vertex is reprojected from the new center.

.. seealso:: :py:func:`center`
%End

    void setRadius( const double radius );
%Docstring
Sets the radius.
Center is unchanged. The first vertex is reprojected from the center with the new radius.

.. seealso:: :py:func:`radius`
%End

    void setFirstVertex( const QgsPoint &firstVertex );
%Docstring
Sets the first vertex.
Radius is unchanged. The center is reprojected from the new first vertex.

.. seealso:: :py:func:`firstVertex`
%End

    void setNumberSides( const unsigned int numberSides );
%Docstring
Sets the number of sides.
If numberSides < 3, the number of sides is unchanged.

.. seealso:: :py:func:`numberSides`
%End

    QgsPointSequence points() const;
%Docstring
Returns a list including the vertices of the regular polygon.
%End

    QgsPolygon *toPolygon() const /Factory/;
%Docstring
Returns as a polygon.
%End

    QgsLineString *toLineString() const /Factory/;
%Docstring
Returns as a linestring.
%End

    QgsTriangle toTriangle() const;
%Docstring
Returns as a triangle.
An empty triangle is returned if the regular polygon is empty or if the number of sides is different from 3.
%End

    QVector<QgsTriangle> triangulate() const;
%Docstring
Returns a triangulation (vertices from sides to the center) of the regular polygon.
An empty list is returned if the regular polygon is empty.
%End

    QgsCircle inscribedCircle() const;
%Docstring
Returns the inscribed circle
%End

    QgsCircle circumscribedCircle() const;
%Docstring
Returns the circumscribed circle
%End

    QString toString( int pointPrecision = 17, int radiusPrecision = 17, int anglePrecision = 2 ) const;
%Docstring
Returns a string representation of the regular polygon.
Members will be truncated to the specified precision.
%End

    double interiorAngle() const;
%Docstring
Returns the measure of the interior angles in degrees.
%End

    double centralAngle() const;
%Docstring
Returns the measure of the central angle (the angle subtended at the center of the polygon by one of its sides) in degrees.
%End

    double area() const;
%Docstring
Returns the area.
Returns 0 if the regular polygon is empty.
%End

    double perimeter() const;
%Docstring
Returns the perimeter.
Returns 0 if the regular polygon is empty.
%End

    double length() const;
%Docstring
Returns the length of a side.
Returns 0 if the regular polygon is empty.
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgsregularpolygon.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/

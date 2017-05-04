/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgsregularpolygon.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





enum ConstructionOption
{
  inscribedCircle,
  circumscribedCircle
};

class QgsRegularPolygon
{
%Docstring
 Regular Polygon geometry type.

 A regular polygon is defined by a center point with a number of sides/vertices, a radius and the first vertice.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsregularpolygon.h"
%End
  public:
    QgsRegularPolygon();

    QgsRegularPolygon( const QgsPointV2 center, const double radius, const double azimuth, const int numSides, const int circle );
%Docstring
 Constructs a regular polygon by center and parameters for the first vertice. An empty regular polygon is returned if numSides < 3 or circle option isn't valid.
 \param center The center of the regular polygon.
 \param radius Distance from the center and the first vertice or sides (see ``circle``).
 \param azimuth Angle in degrees started from the North to the first vertice.
 \param numSides Number of sides of the regular polygon.
 \param circle Option to create the polygon: inscribed in circle (the radius is the distance between the center and vertices) or circumscribed about circle (the radius is the distance from the center to the midpoints of the sides).
%End

    QgsRegularPolygon( const QgsPointV2 center, const QgsPointV2 pt1, const int  numSides, const int circle );
%Docstring
 Constructs a regular polygon by center and another point.
 \param center The center of the regular polygon.
 \param pt1 The first vertice if the polygon is inscribed in circle or the midpoint of a side if the polygon is circumscribed about circle.
 \param numSides Number of sides of the regular polygon.
 \param circle Option to create the polygon inscribed in circle (the radius is the distance between the center and vertices) or circumscribed about circle (the radius is the distance from the center to the midpoints of the sides).
%End

    QgsRegularPolygon( const QgsPointV2 pt1, const QgsPointV2 pt2, const int  numSides );
%Docstring
 Constructs a regular polygon by two points of the first side.
 \param pt1 The first vertice of the first side, also first vertice of the regular polygon.
 \param pt2 The second vertice of the first side.
 \param numSides Number of sides of the regular polygon.
%End

    virtual bool operator ==( const QgsRegularPolygon &rp ) const;
    virtual bool operator !=( const QgsRegularPolygon &rp ) const;

    virtual bool isEmpty() const;
%Docstring
A regular polygon is empty if radius equal to 0 or number of sides < 3
 :rtype: bool
%End

    QgsPointV2 center() const;
%Docstring
 Returns the center point of the regular polygon.
.. seealso:: setCenter()
 :rtype: QgsPointV2
%End

    double radius() const;
%Docstring
 Returns the radius.
 This is also the radius of the circumscribing circle.
.. seealso:: apothem()
.. seealso:: setRadius()
 :rtype: float
%End

    QgsPointV2 vertice() const;
%Docstring
 Returns the first vertice (corner) of the regular polygon.
.. seealso:: setVertice()
 :rtype: QgsPointV2
%End

    double apothem() const;
%Docstring
 Returns the apothem of the regular polygon.
 The apothem is the radius of the inscribed circle.
.. seealso:: radius()
 :rtype: float
%End

    int numSides() const;
%Docstring
 Returns the number of sides of the regular polygon.
.. seealso:: setNumSides()
 :rtype: int
%End

    void setCenter( const QgsPointV2 center );
%Docstring
 Sets the center point.
 Radius is unchanged. The first vertice is reprojected from the new center.
.. seealso:: center()
%End

    void setRadius( const double radius );
%Docstring
 Sets the radius.
 Center is unchanged. The first vertice is reprojected from the center with the new radius.
.. seealso:: radius()
%End

    void setVertice( const QgsPointV2 vertice );
%Docstring
 Sets the first vertice.
 Radius is unchanged. The center is reprojected from the new first vertice.
.. seealso:: vertice()
%End

    void setNumSides( const int numSides );
%Docstring
 Sets the number of sides.
 If numSides < 3, the number of sides is unchanged.
.. seealso:: numSides()
%End

    void points( QgsPointSequence &pts /Out/ ) const;
%Docstring
 Returns a list of points into ``pts``.
 \param pts List of points returned.
%End

    QgsPolygonV2 *toPolygon( ) const /Factory/;
%Docstring
 Returns as a polygon.
 :rtype: QgsPolygonV2
%End

    QgsLineString *toLineString( ) const /Factory/;
%Docstring
 Returns as a linestring.
 :rtype: QgsLineString
%End

    QgsTriangle toTriangle( ) const;
%Docstring
 Returns as a triangle.
 An empty triangle is returned if the regular polygon is empty.
 :rtype: QgsTriangle
%End

    QList<QgsTriangle> triangulate( ) const;
%Docstring
 Returns a triangulation (vertices from sides to the center) of the regular polygon.
 An empty list is returned if the regular polygon is empty.
 :rtype: list of QgsTriangle
%End

    QgsCircle inscribedCircle( ) const;
%Docstring
 Returns the inscribed circle
 :rtype: QgsCircle
%End

    QgsCircle circumscribedCircle( ) const;
%Docstring
 Returns the circumscribed circle
 :rtype: QgsCircle
%End

    QString toString( int pointPrecision = 17, int radiusPrecision = 17, int anglePrecision = 2 ) const;
%Docstring
 returns a string representation of the regular polygon.
 Members will be truncated to the specified precision.
 :rtype: str
%End

    double interiorAngle( ) const;
%Docstring
 Returns the measure of the interior angles in degrees.
 :rtype: float
%End

    double centralAngle( ) const;
%Docstring
 Returns the measure of the central angle (the angle subtended at the center of the polygon by one of its sides) in degrees.
 :rtype: float
%End

    double area( ) const;
%Docstring
 Returns the area.
 Returns 0 if the regular polygon is empty.
 :rtype: float
%End

    double perimeter( ) const;
%Docstring
 Returns the perimeter.
 Returns 0 if the regular polygon is empty.
 :rtype: float
%End

    double length( ) const;
%Docstring
 Returns the length of a side.
 Returns 0 if the regular polygon is empty.
 :rtype: float
%End

  protected:

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgsregularpolygon.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/

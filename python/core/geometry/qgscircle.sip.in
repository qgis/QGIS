/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgscircle.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/








class QgsCircle : QgsEllipse
{
%Docstring
 Circle geometry type.

A circle is defined by a center point with a radius and an azimuth.
The azimuth is the north angle to the semi-major axis, in degrees. By default, the semi-major axis is oriented to the north (0 degrees).

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgscircle.h"
%End
  public:
    QgsCircle();

    QgsCircle( const QgsPoint &center, double radius, double azimuth = 0 );
%Docstring
Constructs a circle by defining all the members.

:param center: The center of the circle.
:param radius: The radius of the circle.
:param azimuth: Angle in degrees started from the North to the first quadrant.
%End

    static QgsCircle from2Points( const QgsPoint &pt1, const QgsPoint &pt2 );
%Docstring
Constructs a circle by 2 points on the circle.
The center point can have z and m values which are the result from the midpoint operation between ``pt1`` and ``pt2``.
The radius is calculated from the 2D distance between ``pt1`` and ``pt2``.
The azimuth is the angle between ``pt1`` and ``pt2``.

:param pt1: First point.
:param pt2: Second point.
%End

    static QgsCircle from3Points( const QgsPoint &pt1, const QgsPoint &pt2, const QgsPoint &pt3, double epsilon = 1E-8 );
%Docstring
Constructs a circle by 3 points on the circle.
Z and m values are dropped for the center point.
The azimuth always takes the default value.
If the points are colinear an empty circle is returned.

:param pt1: First point.
:param pt2: Second point.
:param pt3: Third point.
:param epsilon: Value used to compare point.
%End

    static QgsCircle fromCenterDiameter( const QgsPoint &center, double diameter, double azimuth = 0 );
%Docstring
Constructs a circle by a center point and a diameter.
The center point keeps z and m values from ``center``.

:param center: Center point.
:param diameter: Diameter of the circle.
:param azimuth: Azimuth of the circle.
%End


    static QgsCircle fromCenterPoint( const QgsPoint &center, const QgsPoint &pt1 );
%Docstring
Constructs a circle by a center point and another point.
The center point keeps z and m values from ``center``.
Axes are calculated from the 2D distance between ``center`` and ``pt1``.
The azimuth is the angle between ``center`` and ``pt1``.

:param center: Center point.
:param pt1: A point on the circle.
%End


    static QgsCircle from3Tangents( const QgsPoint &pt1_tg1, const QgsPoint &pt2_tg1,
                                    const QgsPoint &pt1_tg2, const QgsPoint &pt2_tg2,
                                    const QgsPoint &pt1_tg3, const QgsPoint &pt2_tg3, double epsilon = 1E-8 );
%Docstring
Constructs a circle by 3 tangents on the circle (aka inscribed circle of a triangle).
Z and m values are dropped for the center point.
The azimuth always takes the default value.

:param pt1_tg1: First point of the first tangent.
:param pt2_tg1: Second point of the first tangent.
:param pt1_tg2: First point of the second tangent.
:param pt2_tg2: Second point of the second tangent.
:param pt1_tg3: First point of the third tangent.
:param pt2_tg3: Second point of the third tangent.
:param epsilon: Value used to compare point.
%End

    static QgsCircle fromExtent( const QgsPoint &pt1, const QgsPoint &pt2 );
%Docstring
Constructs a circle by an extent (aka bounding box / :py:class:`QgsRectangle`).
The center point can have z and m values which are the result from the midpoint operation between ``pt1`` and ``pt2``.
Axes are calculated from the 2D distance between ``pt1`` and ``pt2``.
The azimuth always takes the default value.

:param pt1: First corner.
:param pt2: Second corner.
%End

    static QgsCircle minimalCircleFrom3Points( const QgsPoint &pt1, const QgsPoint &pt2, const QgsPoint &pt3, double epsilon = 1E-8 );
%Docstring
Constructs the smallest circle from 3 points.
Z and m values are dropped for the center point.
The azimuth always takes the default value.
If the points are colinear an empty circle is returned.

:param pt1: First point.
:param pt2: Second point.
:param pt3: Third point.
:param epsilon: Value used to compare point.
%End

    virtual double area() const;

    virtual double perimeter() const;




    virtual void setSemiMajorAxis( const double semiMajorAxis );

%Docstring
Inherited method. Use setRadius instead.

.. seealso:: :py:func:`radius`

.. seealso:: :py:func:`setRadius`
%End

    virtual void setSemiMinorAxis( const double semiMinorAxis );

%Docstring
Inherited method. Use setRadius instead.

.. seealso:: :py:func:`radius`

.. seealso:: :py:func:`setRadius`
%End

    double radius() const;
%Docstring
Returns the radius of the circle
%End
    void setRadius( double radius );
%Docstring
Set the radius of the circle
%End

    QVector<QgsPoint> northQuadrant() const /Factory/;
%Docstring
The four quadrants of the ellipse.
They are oriented and started from North.

:return: quadrants defined by four points.

.. seealso:: :py:func:`quadrant`
%End

    QgsCircularString *toCircularString( bool oriented = false ) const;
%Docstring
Returns a circular string from the circle.

:param oriented: If oriented is true the start point is from azimuth instead from north.
%End

    bool contains( const QgsPoint &point, double epsilon = 1E-8 ) const;
%Docstring
Returns true if the circle contains the ``point``.
%End

    virtual QgsRectangle boundingBox() const;


    virtual QString toString( int pointPrecision = 17, int radiusPrecision = 17, int azimuthPrecision = 2 ) const;


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgscircle.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/

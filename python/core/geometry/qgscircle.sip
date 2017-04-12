/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgscircle.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/








class QgsCircle : QgsEllipse
{

%TypeHeaderCode
#include "qgscircle.h"
%End
  public:
    QgsCircle();

    QgsCircle( const QgsPointV2 &center, double radius, double azimuth = 0 );
%Docstring
 Constructs a circle by defining all the members.
 @param center The center of the circle.
 @param radius The radius of the circle.
 @param azimuth Angle in degrees started from the North to the first quadrant.
%End

    static QgsCircle by2Points( const QgsPointV2 &pt1, const QgsPointV2 &pt2 );
%Docstring
 Constructs a circle by 2 points on the circle.
 The center point can have z and m values which are the result from the midpoint operation between pt1 and pt2.
 The radius is calculated from the 2D distance between pt1 and pt2.
 The azimuth is the angle between pt1 and pt2.
 @param pt1 First point.
 @param pt2 Second point.
 :rtype: QgsCircle
%End

    static QgsCircle by3Points( const QgsPointV2 &pt1, const QgsPointV2 &pt2, const QgsPointV2 &pt3, double epsilon = 1E-8 );
%Docstring
 Constructs a circle by 3 points on the circle.
 Z and m values are dropped for the center point.
 The azimuth always takes the default value.
 If the points are colinear an empty circle is returned.
 @param pt1 First point.
 @param pt2 Second point.
 @param pt3 Third point.
 @param epsilon Value used to compare point.
 :rtype: QgsCircle
%End

    static QgsCircle byCenterDiameter( const QgsPointV2 &ptc, double diameter, double azimuth = 0 );
%Docstring
 Constructs a circle by a center point and a diameter.
 The center point keep z and m values from ptc.
 @param ptc Center point.
 @param diameter Diameter of the circle.
 @param azimuth Azimuth of the circle.
 :rtype: QgsCircle
%End


    static QgsCircle byCenterPoint( const QgsPointV2 &ptc, const QgsPointV2 &pt1 );
%Docstring
 Constructs a circle by a center point and a another point.
 The center point keep z and m values from ptc.
 Axes are calculated from the 2D distance between ptc and pt1.
 The azimuth is the angle between ptc and pt1.
 @param ptc Center point.
 @param pt1 A point on the circle.
 :rtype: QgsCircle
%End


    static QgsCircle by3Tangents( const QgsPointV2 &pt1_tg1, const QgsPointV2 &pt2_tg1,
                                  const QgsPointV2 &pt1_tg2, const QgsPointV2 &pt2_tg2,
                                  const QgsPointV2 &pt1_tg3, const QgsPointV2 &pt2_tg3, double epsilon = 1E-8 );
%Docstring
 Constructs a circle by 3 tangents on the circle (aka inscribed circle of a triangle).
 Z and m values are dropped for the center point.
 The azimuth always takes the default value.
 @param pt1_tg1 First point of the first tangent.
 @param pt2_tg1 Second point of the first tangent.
 @param pt1_tg2 First point of the second tangent.
 @param pt2_tg2 Second point of the second tangent.
 @param pt1_tg3 First point of the third tangent.
 @param pt2_tg3 Second point of the third tangent.
 @param epsilon Value used to compare point.
 :rtype: QgsCircle
%End

    static QgsCircle byExtent( const QgsPointV2 &pt1, const QgsPointV2 &pt2 );
%Docstring
 Constructs a circle by an extent (aka bounding box / QgsRectangle).
 The center point can have z and m values which are the result from the midpoint operation between pt1 and pt2.
 Axes are calculated from the 2D distance between pt1 and pt2.
 The azimuth always takes the default value.
 @param pt1 First corner.
 @param pt2 Second corner.
 :rtype: QgsCircle
%End

    double area() const;
%Docstring
 :rtype: float
%End
    double perimeter() const;
%Docstring
 :rtype: float
%End




    double radius() const;
%Docstring
Returns the radius of the circle
 :rtype: float
%End
    void setRadius( double radius );
%Docstring
Set the radius of the circle
%End

    QVector<QgsPointV2> northQuadrant() const;
%Docstring
 The four quadrant of the ellipse.
 The are oriented and started from North.
 @return quadrant defined by four point.
 @see quadrant()
 :rtype: list of QgsPointV2
%End

    QgsCircularString toCircularString( bool oriented = false ) const;
%Docstring
 Returns a circular string from the circle.
 @param oriented If oriented is true the start point are from azimuth instead from north.
 :rtype: QgsCircularString
%End


    QgsRectangle boundingBox() const;
%Docstring
 :rtype: QgsRectangle
%End

    QString toString( int pointPrecision = 17, int radiusPrecision = 17, int azimuthPrecision = 2 ) const;
%Docstring
 :rtype: str
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgscircle.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/

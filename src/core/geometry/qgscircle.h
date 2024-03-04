/***************************************************************************
                         qgscircle.h
                         --------------
    begin                : March 2017
    copyright            : (C) 2017 by Loîc Bartoletti
    email                : lbartoletti at tuxfamily dot org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCIRCLE_H
#define QGSCIRCLE_H

#include <QString>

#include "qgis_core.h"
#include "qgsellipse.h"
#include "qgspolygon.h"
#include "qgsrectangle.h"
#include "qgscircularstring.h"


class QgsPoint;

/**
 * \ingroup core
 * \class QgsCircle
 * \brief Circle geometry type.
 *
 * A circle is defined by a center point with a radius and an azimuth.
 * The azimuth is the north angle to the semi-major axis, in degrees. By default, the semi-major axis is oriented to the north (0 degrees).
 */


class CORE_EXPORT QgsCircle : public QgsEllipse
{
  public:
    QgsCircle();

    /**
     * Constructs a circle by defining all the members.
     * \param center The center of the circle.
     * \param radius The radius of the circle.
     * \param azimuth Angle in degrees started from the North to the first quadrant.
     */
    QgsCircle( const QgsPoint &center, double radius, double azimuth = 0 ) SIP_HOLDGIL;

    /**
     * Constructs a circle by 2 points on the circle.
     * The center point can have m value which is the result from the midpoint
     * operation between \a pt1 and \a pt2. Z dimension is also supported and
     * is retrieved from the first 3D point amongst \a pt1 and \a pt2.
     * The radius is calculated from the 2D distance between \a pt1 and \a pt2.
     * The azimuth is the angle between \a pt1 and \a pt2.
     * \param pt1 First point.
     * \param pt2 Second point.
     */
    static QgsCircle from2Points( const QgsPoint &pt1, const QgsPoint &pt2 ) SIP_HOLDGIL;

    /**
     * Constructs a circle by 3 points on the circle.
     * M value is dropped for the center point.
     * Z dimension is supported and is retrieved from the first 3D point
     * amongst \a pt1, \a pt2 and \a pt3.
     * The azimuth always takes the default value.
     * If the points are colinear an empty circle is returned.
     * \param pt1 First point.
     * \param pt2 Second point.
     * \param pt3 Third point.
     * \param epsilon Value used to compare point.
     */
    static QgsCircle from3Points( const QgsPoint &pt1, const QgsPoint &pt2, const QgsPoint &pt3, double epsilon = 1E-8 ) SIP_HOLDGIL;

    /**
     * Constructs a circle by a center point and a diameter.
     * The center point keeps z and m values from \a center.
     * \param center Center point.
     * \param diameter Diameter of the circle.
     * \param azimuth Azimuth of the circle.
     */
    static QgsCircle fromCenterDiameter( const QgsPoint &center, double diameter, double azimuth = 0 ) SIP_HOLDGIL;


    /**
     * Constructs a circle by a center point and another point.
     * The center point keeps z and m values from \a center.
     * Axes are calculated from the 2D distance between \a center and \a pt1.
     * The azimuth is the angle between \a center and \a pt1.
     * \param center Center point.
     * \param pt1 A point on the circle.
     */
    static QgsCircle fromCenterPoint( const QgsPoint &center, const QgsPoint &pt1 ) SIP_HOLDGIL;


    /**
     * Constructs a circle by 3 tangents on the circle (aka inscribed circle of a triangle).
     * Z and m values are dropped for the center point.
     * The azimuth always takes the default value.
     * \param pt1_tg1 First point of the first tangent.
     * \param pt2_tg1 Second point of the first tangent.
     * \param pt1_tg2 First point of the second tangent.
     * \param pt2_tg2 Second point of the second tangent.
     * \param pt1_tg3 First point of the third tangent.
     * \param pt2_tg3 Second point of the third tangent.
     * \param epsilon Value used to compare point.
     * \param pos Point to determine which circle use in case of multi return.
     * If the solution is not unique and pos is an empty point, an empty circle is returned. -- This case happens only when two tangents are parallels. (since QGIS 3.18)
     *
     * \see from3TangentsMulti()
     *
     * ### Example
     *
     * \code{.py}
     *  # [(0 0), (5 0)] and [(5 5), (10 5)] are parallels
     *  QgsCircle.from3Tangents(QgsPoint(0, 0), QgsPoint(5, 0), QgsPoint(5, 5), QgsPoint(10, 5), QgsPoint(2.5, 0), QgsPoint(7.5, 5))
     *  # <QgsCircle: Empty>
     *  QgsCircle.from3Tangents(QgsPoint(0, 0), QgsPoint(5, 0), QgsPoint(5, 5), QgsPoint(10, 5), QgsPoint(2.5, 0), QgsPoint(7.5, 5), pos=QgsPoint(2, 0))
     *  # <QgsCircle: Circle (Center: Point (1.46446609406726203 2.49999999999999911), Radius: 2.5, Azimuth: 0)>
     *  QgsCircle.from3Tangents(QgsPoint(0, 0), QgsPoint(5, 0), QgsPoint(5, 5), QgsPoint(10, 5), QgsPoint(2.5, 0), QgsPoint(7.5, 5), pos=QgsPoint(3, 0))
     *  # <QgsCircle: Circle (Center: Point (8.53553390593273775 2.5), Radius: 2.5, Azimuth: 0)>
     * \endcode
     */
    static QgsCircle from3Tangents( const QgsPoint &pt1_tg1, const QgsPoint &pt2_tg1,
                                    const QgsPoint &pt1_tg2, const QgsPoint &pt2_tg2,
                                    const QgsPoint &pt1_tg3, const QgsPoint &pt2_tg3,
                                    double epsilon = 1E-8,
                                    const QgsPoint &pos = QgsPoint() ) SIP_HOLDGIL;

    /**
     * Returns an array of circle constructed by 3 tangents on the circle (aka inscribed circle of a triangle).
     *
     * The vector can contain 0, 1 or 2 circles:
     *
     * - 0: Impossible to construct a circle from 3 tangents (three parallel tangents)
     * - 1: The three tangents make a triangle or when two tangents are parallel there are two possible circles (see examples).
     *   If pos is not an empty point, we use its coordinates to determine which circle will be returned.
     *   More precisely the circle that will be returned will be the one whose center is on the same side as pos relative to the third tangent.
     * - 2: Returns both solutions when two tangents are parallel (this implies that pos is an empty point).
     *
     * Z and m values are dropped for the center point.
     * The azimuth always takes the default value.
     * \param pt1_tg1 First point of the first tangent.
     * \param pt2_tg1 Second point of the first tangent.
     * \param pt1_tg2 First point of the second tangent.
     * \param pt2_tg2 Second point of the second tangent.
     * \param pt1_tg3 First point of the third tangent.
     * \param pt2_tg3 Second point of the third tangent.
     * \param epsilon Value used to compare point.
     * \param pos (optional) Point to determine which circle use in case of multi return.
     *
     * \see from3Tangents()
     *
     * ### Example
     *
     * \code{.py}
     *
     *  # [(0 0), (5 0)] and [(5 5), (10 5)] are parallels
     *  QgsCircle.from3TangentsMulti(QgsPoint(0, 0), QgsPoint(5, 0), QgsPoint(5, 5), QgsPoint(10, 5), QgsPoint(2.5, 0), QgsPoint(7.5, 5))
     *  # [<QgsCircle: Circle (Center: Point (8.53553390593273775 2.5), Radius: 2.5, Azimuth: 0)>, <QgsCircle: Circle (Center: Point (1.46446609406726203 2.49999999999999911), Radius: 2.5, Azimuth: 0)>]
     *  QgsCircle.from3TangentsMulti(QgsPoint(0, 0), QgsPoint(5, 0), QgsPoint(5, 5), QgsPoint(10, 5), QgsPoint(2.5, 0), QgsPoint(7.5, 5), pos=QgsPoint(2, 0))
     *  # [<QgsCircle: Circle (Center: Point (1.46446609406726203 2.49999999999999911), Radius: 2.5, Azimuth: 0)>]
     *  QgsCircle.from3TangentsMulti(QgsPoint(0, 0), QgsPoint(5, 0), QgsPoint(5, 5), QgsPoint(10, 5), QgsPoint(2.5, 0), QgsPoint(7.5, 5), pos=QgsPoint(3, 0))
     *  # [<QgsCircle: Circle (Center: Point (8.53553390593273775 2.5), Radius: 2.5, Azimuth: 0)>]
     *  # [(0 0), (5 0)], [(5 5), (10 5)] and [(15 5), (20 5)] are parallels
     *  QgsCircle.from3TangentsMulti(QgsPoint(0, 0), QgsPoint(5, 0), QgsPoint(5, 5), QgsPoint(10, 5), QgsPoint(15, 5), QgsPoint(20, 5))
     *  # []
     * \endcode
     */
    static QVector<QgsCircle> from3TangentsMulti( const QgsPoint &pt1_tg1, const QgsPoint &pt2_tg1,
        const QgsPoint &pt1_tg2, const QgsPoint &pt2_tg2,
        const QgsPoint &pt1_tg3, const QgsPoint &pt2_tg3,
        double epsilon = 1E-8,
        const QgsPoint &pos = QgsPoint() ) SIP_HOLDGIL;

    /**
     * Constructs a circle by an extent (aka bounding box / QgsRectangle).
     * The center point can have m value which is the result from the midpoint
     * operation between \a pt1 and \a pt2. Z dimension is also supported and
     * is retrieved from the first 3D point amongst \a pt1 and \a pt2.
     * Axes are calculated from the 2D distance between \a pt1 and \a pt2.
     * The azimuth always takes the default value.
     * \param pt1 First corner.
     * \param pt2 Second corner.
     */
    static QgsCircle fromExtent( const QgsPoint &pt1, const QgsPoint &pt2 ) SIP_HOLDGIL;

    /**
     * Constructs the smallest circle from 3 points.
     * Z and m values are dropped for the center point.
     * The azimuth always takes the default value.
     * If the points are colinear an empty circle is returned.
     * \param pt1 First point.
     * \param pt2 Second point.
     * \param pt3 Third point.
     * \param epsilon Value used to compare point.
     */
    static QgsCircle minimalCircleFrom3Points( const QgsPoint &pt1, const QgsPoint &pt2, const QgsPoint &pt3, double epsilon = 1E-8 ) SIP_HOLDGIL;

    /**
     * Calculates the intersections points between this circle and an \a other circle.
     *
     * If found, the intersection points will be stored in \a intersection1 and \a intersection2.
     *
     * By default this method does not consider any z values and instead treats the circles as 2-dimensional.
     * If \a useZ is set to TRUE, then an intersection will only occur if the z values of both circles are
     * equal. In this case the points returned for \a intersection1 and \a intersection2 will contain
     * the z value of the circle intersections.
     *
     * \returns number of intersection points found.
     *
     * \since QGIS 3.2
     */
    int intersections( const QgsCircle &other, QgsPoint &intersection1 SIP_OUT, QgsPoint &intersection2 SIP_OUT, bool useZ = false ) const;

    /**
     * Calculates the tangent points between this circle and the point \a p.
     *
     * If found, the tangent points will be stored in \a pt1 and \a pt2.
     *
     * Note that this method is 2D only and does not consider the z-value of the circle.
     *
     * \returns TRUE if tangent was found.
     *
     *
     * \see outerTangents() and innerTangents()
     * \since QGIS 3.2
     */
    bool tangentToPoint( const QgsPointXY &p, QgsPointXY &pt1 SIP_OUT, QgsPointXY &pt2 SIP_OUT ) const;

    /**
     * Calculates the outer tangent points between this circle
     * and an \a other circle.
     *
     * The outer tangent points correspond to the points at which the two lines
     * which are drawn so that they are tangential to both circles touch
     * the circles.
     *
     * The first tangent line is described by the points
     * stored in \a line1P1 and \a line1P2,
     * and the second line is described by the points stored in \a line2P1
     * and \a line2P2.
     *
     * Returns the number of tangents (either 0 or 2).
     *
     * Note that this method is 2D only and does not consider the z-value of the circle.
     *
     *
     * \see tangentToPoint() and innerTangents()
     * \since QGIS 3.2
     */
    int outerTangents( const QgsCircle &other,
                       QgsPointXY &line1P1 SIP_OUT, QgsPointXY &line1P2 SIP_OUT,
                       QgsPointXY &line2P1 SIP_OUT, QgsPointXY &line2P2 SIP_OUT ) const;

    /**
     * Calculates the inner tangent points between this circle
     * and an \a other circle.
     *
     * The inner tangent points correspond to the points at which the two lines
     * which are drawn so that they are tangential to both circles but on
     * different sides, touching the circles and crossing each other.
     *
     * The first tangent line is described by the points
     * stored in \a line1P1 and \a line1P2,
     * and the second line is described by the points stored in \a line2P1
     * and \a line2P2.
     *
     * Returns the number of tangents (either 0 or 2).
     *
     * Note that this method is 2D only and does not consider the z-value of the circle.
     *
     *
     * \see tangentToPoint() and outerTangents()
     * \since QGIS 3.6
     */
    int innerTangents( const QgsCircle &other,
                       QgsPointXY &line1P1 SIP_OUT, QgsPointXY &line1P2 SIP_OUT,
                       QgsPointXY &line2P1 SIP_OUT, QgsPointXY &line2P2 SIP_OUT ) const;

    double area() const override SIP_HOLDGIL;
    double perimeter() const override SIP_HOLDGIL;

    //inherited
    // void setAzimuth(const double azimuth);
    // double azimuth() const {return mAzimuth; }


    /**
     * Inherited method. Use setRadius instead.
     * \see radius()
     * \see setRadius()
     */
    void setSemiMajorAxis( double semiMajorAxis ) override SIP_HOLDGIL;

    /**
     * Inherited method. Use setRadius instead.
     * \see radius()
     * \see setRadius()
     */
    void setSemiMinorAxis( double semiMinorAxis ) override SIP_HOLDGIL;

    //! Returns the radius of the circle
    double radius() const SIP_HOLDGIL {return mSemiMajorAxis;}
    //! Sets the radius of the circle
    void setRadius( double radius ) SIP_HOLDGIL
    {
      mSemiMajorAxis = std::fabs( radius );
      mSemiMinorAxis = mSemiMajorAxis;
    }

    /**
     * The four quadrants of the ellipse.
     * They are oriented and started from North.
     * \return quadrants defined by four points.
     * \see quadrant()
     */
    QVector<QgsPoint> northQuadrant() const SIP_FACTORY;

    /**
     * Returns a circular string from the circle.
     * \param oriented If oriented is TRUE the start point is from azimuth instead from north.
     */
    QgsCircularString *toCircularString( bool oriented = false ) const;

    //! Returns TRUE if the circle contains the \a point.
    bool contains( const QgsPoint &point, double epsilon = 1E-8 ) const;

    QgsRectangle boundingBox() const override;

    QString toString( int pointPrecision = 17, int radiusPrecision = 17, int azimuthPrecision = 2 ) const override;

    /**
     * Returns a GML2 representation of the geometry.
     * Since GML2 does not supports curve, it will be converted to a LineString.
     * \param doc DOM document
     * \param precision number of decimal places for coordinates
     * \param ns XML namespace
     * \param axisOrder Axis order for generated GML
     * \see asGml3()
     */
    QDomElement asGml2( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const;

    /**
     * Returns a GML3 representation of the geometry.
     *
     * From the GML3 description:
     * A Circle is an arc whose ends coincide to form a simple closed loop.
     * The three control points shall be distinct non-co-linear points for
     * the circle to be unambiguously defined. The arc is simply extended
     * past the third control point until the first control point is encountered.
     *
     * Coordinates are taken from quadrant North, East and South.
     *
     * \param doc DOM document
     * \param precision number of decimal places for coordinates
     * \param ns XML namespace
     * \param axisOrder Axis order for generated GML
     * \see asGml2()
     */
    QDomElement asGml3( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsCircle: %1>" ).arg( sipCpp->toString() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif
};

#endif // QGSCIRCLE_H

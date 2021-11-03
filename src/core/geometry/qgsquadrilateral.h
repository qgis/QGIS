/***************************************************************************
                         qgsquadrilateral.h
                         -------------------
    begin                : November 2018
    copyright            : (C) 2018 by Loïc Bartoletti
    email                : loic dot bartoletti at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSQUADRILATERAL_H
#define QGSQUADRILATERAL_H


#include "qgis_core.h"
#include "qgspoint.h"
#include "qgspolygon.h"
#include "qgslinestring.h"

/**
 * \ingroup core
 * \class QgsQuadrilateral
 * \brief Quadrilateral geometry type.
 *
 * A quadrilateral is a polygon with four edges (or sides) and four vertices or corners.
 * This class allows the creation of simple quadrilateral (which does not self-intersect).
 * \since QGIS 3.6
 */
class CORE_EXPORT QgsQuadrilateral
{
  public:

    /**
     * Constructor for an empty quadrilateral geometry.
     */
    QgsQuadrilateral() SIP_HOLDGIL;

    /**
     * Construct a QgsQuadrilateral from four QgsPoint.
     * \param p1 first point
     * \param p2 second point
     * \param p3 third point
     * \param p4 fourth point
     * \see setPoints
     */
    QgsQuadrilateral( const QgsPoint &p1, const QgsPoint &p2, const QgsPoint &p3, const QgsPoint &p4 ) SIP_HOLDGIL;

    /**
     * Construct a QgsQuadrilateral from four QgsPointXY.
     * \param p1 first point
     * \param p2 second point
     * \param p3 third point
     * \param p4 fourth point
     * \see setPoints
     */
    explicit QgsQuadrilateral( const QgsPointXY &p1, const QgsPointXY &p2, const QgsPointXY &p3, const QgsPointXY &p4 ) SIP_HOLDGIL;


    /**
     * A quadrilateral can be constructed from 3 points where the second distance can be determined by the third point.
     *
     */
    enum ConstructionOption
    {
      Distance, //!< Second distance is equal to the distance between 2nd and 3rd point
      Projected, //!< Second distance is equal to the distance of the perpendicular projection of the 3rd point on the segment or its extension.
    };

    /**
     * Construct a QgsQuadrilateral as a Rectangle from 3 points.
     * In the case where one of the points is of type PointZ. The other points
     * will also be of type Z, even if they are of type Point. In addition,
     * the z used will be the one of the first point with a Z.
     * This ensures consistency in point types and the ability to export to a
     * Polygon or LineString.
     * M is taken from point \a p1.
     * \param p1 first point
     * \param p2 second point
     * \param p3 third point
     * \param mode Construction mode to construct the rectangle from 3 points
     * \see ConstructionOption
     */
    static QgsQuadrilateral rectangleFrom3Points( const QgsPoint &p1, const QgsPoint &p2, const QgsPoint &p3, ConstructionOption mode ) SIP_HOLDGIL;

    /**
     * Construct a QgsQuadrilateral as a rectangle from an extent, defined by
     * two opposite corner points.
     * Z and M are taken from point \a p1.
     * QgsQuadrilateral will have the same dimension as \a p1 dimension.
     * \param p1 first point
     * \param p2 second point
     */
    static QgsQuadrilateral rectangleFromExtent( const QgsPoint &p1, const QgsPoint &p2 ) SIP_HOLDGIL;

#ifndef SIP_RUN

    /**
     * Alias for rectangleFromDiagonal
     */
    static constexpr auto &rectangleFromDiagonal = rectangleFromExtent;
#endif

    /**
     * Construct a QgsQuadrilateral as a square from a diagonal.
     * Z and M are taken from point \a p1.
     * QgsQuadrilateral will have the same dimension as \a p1 dimension.
     * \param p1 first point
     * \param p2 second point
     */
    static QgsQuadrilateral squareFromDiagonal( const QgsPoint &p1, const QgsPoint &p2 ) SIP_HOLDGIL;

    /**
     * Construct a QgsQuadrilateral as a rectangle from center point \a center
     * and another point \a point.
     * Z and M are taken from point \a p1.
     * QgsQuadrilateral will have the same dimension as \a center dimension.
     * \param center center point
     * \param point corner point
     */
    static QgsQuadrilateral rectangleFromCenterPoint( const QgsPoint &center, const QgsPoint &point ) SIP_HOLDGIL;

    /**
     * Construct a QgsQuadrilateral as a rectangle from a QgsRectangle.
     * \param rectangle rectangle
     */
    static QgsQuadrilateral fromRectangle( const QgsRectangle &rectangle ) SIP_HOLDGIL;

    // TODO:
    // Rhombus

    /**
     * Compares two QgsQuadrilateral, allowing specification of the maximum allowable difference between points.
     * \param other the QgsQuadrilateral to compare
     * \param epsilon the maximum difference allowed / tolerance
     */
    bool equals( const QgsQuadrilateral &other, double epsilon = 4 * std::numeric_limits<double>::epsilon() ) const SIP_HOLDGIL;
    bool operator==( const QgsQuadrilateral &other ) const SIP_HOLDGIL;
    bool operator!=( const QgsQuadrilateral &other ) const SIP_HOLDGIL;

    /**
     * Convenient method to determine if a QgsQuadrilateral is valid.
     * A QgsQuadrilateral must be simple (not self-intersecting) and
     * cannot have collinear points.
     */
    bool isValid() const SIP_HOLDGIL;

    /**
     * Simple enumeration to ensure indices in setPoint
     */
    enum Point
    {
      Point1,
      Point2,
      Point3,
      Point4,
    };

    /**
     * Sets the point \a newPoint at the \a index.
     * Returns FALSE if the QgsQuadrilateral is not valid.
     * \see Point
     */
    bool setPoint( const QgsPoint &newPoint, Point index ) SIP_HOLDGIL;

    /**
     * Set all points
     * Returns FALSE if the QgsQuadrilateral is not valid:
     *
     * - The points do not have the same type
     * - The quadrilateral would have auto intersections
     * - The quadrilateral has double points
     * - The quadrilateral has collinear points
     */
    bool setPoints( const QgsPoint &p1, const QgsPoint &p2, const QgsPoint &p3, const QgsPoint &p4 ) SIP_HOLDGIL;

    /**
     * Returns a list including the vertices of the quadrilateral.
     */
    QgsPointSequence points() const;

    /**
     * Returns the quadrilateral as a new polygon. Ownership is transferred to the caller.
     */
    QgsPolygon *toPolygon( bool force2D = false ) const SIP_FACTORY;

    /**
     * Returns the quadrilateral as a new linestring. Ownership is transferred to the caller.
     */
    QgsLineString *toLineString( bool force2D = false ) const SIP_FACTORY;

    /**
     * Returns a string representation of the quadrilateral.
     * Members will be truncated to the specified precision.
     */
    QString toString( int pointPrecision = 17 ) const;

    /**
     * Returns the area of the quadrilateral, or 0 if the quadrilateral is empty.
     */
    double area() const SIP_HOLDGIL;

    /**
     * Returns the perimeter of the quadrilateral, or 0 if the quadrilateral is empty.
     */
    double perimeter() const SIP_HOLDGIL;
#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsQuadrilateral: %1>" ).arg( sipCpp->toString() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif
  private:
    QgsPoint mPoint1, mPoint2, mPoint3, mPoint4;
};

#endif // QGSQUADRILATERAL_H

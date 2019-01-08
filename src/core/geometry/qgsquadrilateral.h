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
 * A quadrilateral is a polygon with four edges (or sides) and four vertices or corners.
 * This class allows the creation of simple quadrilateral (which does not self-intersect).
 * \since QGIS 3.6
 */
class CORE_EXPORT QgsQuadrilateral
{
  public:
    QgsQuadrilateral();

    /**
     * Construct a QgsQuadrilateral from three QgsPoint.
     * \param p1 first point
     * \param p2 second point
     * \param p3 third point
     * \param p4 fourth point
     * \see setPoints
     */
    QgsQuadrilateral( const QgsPoint &p1, const QgsPoint &p2, const QgsPoint &p3, const QgsPoint &p4 );

    /**
     * Construct a QgsQuadrilateral from four QgsPoint.
     * \param p1 first point
     * \param p2 second point
     * \param p3 third point
     * \param p4 fourth point
     * \see setPoints
     */
    explicit QgsQuadrilateral( const QgsPointXY &p1, const QgsPointXY &p2, const QgsPointXY &p3, const QgsPointXY &p4 );


    /**
     * A quadrilateral can be constructed from 3 points where the second distance can be determined by the third point.
     *
     */
    enum ConstructionOption
    {
      Distance, //<! Second distance is equal to the distance between 2nd and 3rd point
      Projected, //<! Second distance is equal to the distance of the perpendicualr projection of the 3rd point on the segment or its extension.
    };

    /**
     * Construct a QgsQuadrilateral as a Rectangle from 3 points.
     * \param p1 first point
     * \param p2 second point
     * \param p3 third point
     * \param mode Construction mode to construct the rectangle from 3 points
     * \see ConstructionOption
     */
    static QgsQuadrilateral rectangleFrom3Points( const QgsPoint &p1, const QgsPoint &p2, const QgsPoint &p3, ConstructionOption mode );

    /**
     * Construct a QgsQuadrilateral as a Rectangle from is extent.
     * Z is taken from point \a p1.
     * \param p1 first point
     * \param p2 second point
     */
    static QgsQuadrilateral rectangleFromExtent( const QgsPoint &p1, const QgsPoint &p2 );

#ifndef SIP_RUN

    /**
     * Alias for rectangleFromDiagonal
     */
    static constexpr auto &rectangleFromDiagonal = rectangleFromExtent;
#endif

    /**
     * Construct a QgsQuadrilateral as a Square from a diagonal.
     * Z is taken from point \a p1.
     * \param p1 first point
     * \param p2 second point
     */
    static QgsQuadrilateral squareFromDiagonal( const QgsPoint &p1, const QgsPoint &p2 );

    /**
     * Construct a QgsQuadrilateral as a Rectangle from center point \a center
     * and another point \a point.
     * Z is taken from \a center point.
     * \param center center point
     * \param point corner point
     */
    static QgsQuadrilateral rectangleFromCenterPoint( const QgsPoint &center, const QgsPoint &point );

    /**
     * Construct a QgsQuadrilateral as a Rectangle from a QgsRectangle.
     * \param rectangle rectangle
     */
    static QgsQuadrilateral fromRectangle( const QgsRectangle &rectangle );

    // TODO:
    // Rhombus

    /**
     * Compare two QgsQuadrilateral but allow to specify the maximum difference
     * allowable between points.
     * \param other the QgsQuadrilateral to compare
     * \param epsilon the maximum difference allowed / tolerance
     */
    bool equals( const QgsQuadrilateral &other, double epsilon = 4 * std::numeric_limits<double>::epsilon() ) const;
    bool operator==( const QgsQuadrilateral &other ) const;
    bool operator!=( const QgsQuadrilateral &other ) const;

    /**
     * Convenient method to determine if a QgsQuadrilateral is valid.
     * A QgsQuadrilateral must be simple (not self-intersecting) and
     * cannot have collinear points.
     */
    bool isValid() const;

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
     * Returns false if the QgsQuadrilateral is not valid.
     * \see Point
     */
    bool setPoint( const QgsPoint &newPoint, Point index );

    /**
     * Set all points
     * Returns false if the QgsQuadrilateral is not valid:
     * - The points do not have the same type
     * - The quadrilateral would have auto intersections
     * - The quadrilateral has double points
     * - The quadrilateral has collinear points
     * \param p1 first point
     * \param p2 second point
     * \param p3 third point
     * \param p4 fourth point
     */
    bool setPoints( const QgsPoint &p1, const QgsPoint &p2, const QgsPoint &p3, const QgsPoint &p4 );

    /**
     * Returns a list including the vertices of the quadrilateral.
     */
    QgsPointSequence points() const;

    /**
     * Returnns as a new polygon. Ownership is transferred to the caller.
     */
    QgsPolygon *toPolygon( bool force2D = false ) const SIP_FACTORY;

    /**
     * Returnns as a new linestring. Ownership is transferred to the caller.
     */
    QgsLineString *toLineString( bool force2D = false ) const SIP_FACTORY;

    /**
     * Returns a string representation of the quadrilateral.
     * Members will be truncated to the specified precision.
     */
    QString toString( int pointPrecision = 17 ) const;

    /**
     * Returns the area.
     * Returns 0 if the quadrilateral is empty.
     */
    double area() const;

    /**
     * Returns the perimeter.
     * Returns 0 if the quadrilateral is empty.
     */
    double perimeter() const;
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

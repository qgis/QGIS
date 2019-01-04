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
 * \brief Quadrilateral (Quadrilateral) geometry type.
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
     */
    QgsQuadrilateral( const QgsPoint &p1, const QgsPoint &p2, const QgsPoint &p3, const QgsPoint &p4 );

    /**
     * Construct a QgsQuadrilateral from four QgsPoint.
     * \param p1 first point
     * \param p2 second point
     * \param p3 third point
     * \param p4 fourth point
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
     *
     * Construct a QgsQuadrilateral as a Rectangle from 3 points.
     * \param p1 first point
     * \param p2 second point
     * \param p3 third point
     * \param mode Construction mode to construct the rectangle from 3 points
     * \see ConstructionOption
     */
    static QgsQuadrilateral rectangleFrom3points( const QgsPoint &p1, const QgsPoint &p2, const QgsPoint &p3, ConstructionOption mode );
    // Extent
    static QgsQuadrilateral rectangleFromExtent( const QgsPoint &p1, const QgsPoint &p2 );

#ifndef SIP_RUN
    static constexpr auto &rectangleFromDiagonal = rectangleFromExtent;
#endif
    // Square by diagonal
    static QgsQuadrilateral squareFromDiagonal( const QgsPoint &p1, const QgsPoint &p2 );
    // center, point
    static QgsQuadrilateral rectangleFromCenterPoint( const QgsPoint &center, const QgsPoint &point );
    // rectangle
    static QgsQuadrilateral fromRectangle( const QgsRectangle &rectangle );
    // TODO:
    // Rhombus

    bool operator==( const QgsQuadrilateral &other ) const;
    bool operator!=( const QgsQuadrilateral &other ) const;

    bool isEmpty() const;
    enum Point
    {
      Point1,
      Point2,
      Point3,
      Point4,
    };

    void setPoint( const QgsPoint &newPoint, Point index );
    void setPoints( const QgsPoint &p1, const QgsPoint &p2, const QgsPoint &p3, const QgsPoint &p4 );

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

  private:
    QgsPoint mPoint1, mPoint2, mPoint3, mPoint4;
};

#endif // QGSQUADRILATERAL_H

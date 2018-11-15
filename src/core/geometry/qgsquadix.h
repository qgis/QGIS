/***************************************************************************
                         qgsquadix.h
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

#ifndef QGSQUADIX_H
#define QGSQUADIX_H


#include "qgis_core.h"
#include "qgspoint.h"
#include "qgspolygon.h"
#include "qgslinestring.h"

/**
 * \ingroup core
 * \class QgsQuadix
 * \brief Quadix (Quadrilateral) geometry type.
 * \since QGIS 3.6
 */
class CORE_EXPORT QgsQuadix
{
  public:
    QgsQuadix();

    /**
     * Construct a QgsQuadix from three QgsPointV2.
     * \param p1 first point
     * \param p2 second point
     * \param p3 third point
     * \param p4 fourth point
     */
    QgsQuadix( const QgsPoint &p1, const QgsPoint &p2, const QgsPoint &p3, const QgsPoint &p4 );

    /**
     * Construct a QgsQuadix from three QgsPoint.
     * \param p1 first point
     * \param p2 second point
     * \param p3 third point
     * \param p4 fourth point
     */
    explicit QgsQuadix( const QgsPointXY &p1, const QgsPointXY &p2, const QgsPointXY &p3, const QgsPointXY &p4 );


    /**
     * A quadrilateral can be constructed from 3 points where the second distance can be determined by the third point.
     *
     */
    enum ConstructionOption
    {
      Distance, //<! Second distance is equal to the distance between 2nd and 3rd point
      Projected, //<! Second distance is equal to the distance of the perpendicualr projection of the 3rd point on the segment or its extension.
    };
    // 3 points distance
    // 3 points projected
    static QgsQuadix rectangleFrom3points( const QgsPoint &p1, const QgsPoint &p2, const QgsPoint &p3, ConstructionOption mode );
    // Extent
    static QgsQuadix rectangleFromExtent( const QgsPoint &p1, const QgsPoint &p2 );
    static constexpr auto &rectangleFromDiagonal = rectangleFromExtent;
    // Square by diagonal
    static QgsQuadix squareFromDiagonal( const QgsPoint &p1, const QgsPoint &p2 );
    // center, point
    static QgsQuadix rectangleFromCenterPoint( const QgsPoint &center, const QgsPoint &point );
    // rectangle
    static QgsQuadix fromRectangle( const QgsRectangle &rectangle );
    // TODO:
    // Rhombus

    bool operator==( const QgsQuadix &other ) const;
    bool operator!=( const QgsQuadix &other ) const;

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
     * Returns as a polygon.
     */
    QgsPolygon *toPolygon( bool force2D = false ) const SIP_FACTORY;

    /**
     * Returns as a linestring.
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

#endif // QGSQUADIX_H

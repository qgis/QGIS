/***************************************************************************
                         qgsregularpolygon.h
                         --------------
    begin                : May 2017
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

#ifndef QGSREGULARPOLYGON_H
#define QGSREGULARPOLYGON_H

#include <QString>

#include "qgis_core.h"
#include "qgspoint.h"
#include "qgspolygon.h"
#include "qgslinestring.h"
#include "qgscircle.h"
#include "qgstriangle.h"


/**
 * \ingroup core
 * \class QgsRegularPolygon
 * \brief Regular Polygon geometry type.
 *
 * A regular polygon is a polygon that is equiangular (all angles are equal in measure) and equilateral (all sides have the same length).
 * The regular polygon is defined by a center point with a number of sides/vertices, a radius and the first vertex.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsRegularPolygon
{
  public:

    /**
     * A regular polygon can be constructed inscribed in a circle or circumscribed about a circle.
     *
     */
    enum ConstructionOption
    {
      InscribedCircle, //<! Inscribed in a circle (the radius is the distance between the center and vertices)
      CircumscribedCircle //<! Circumscribed about a circle (the radius is the distance from the center to the midpoints of the sides)
    };

    /**
     * Constructor for QgsRegularPolygon.
     */
    QgsRegularPolygon() = default;

    /**
     * Constructs a regular polygon by \a center and parameters for the first vertex. An empty regular polygon is returned if \a numberSides < 3 or \a ConstructionOption isn't valid.
     * \param center The center of the regular polygon.
     * \param radius Distance from the center and the first vertex or sides (see \a ConstructionOption).
     * \param azimuth Angle in degrees started from the North to the first vertex.
     * \param numberSides Number of sides of the regular polygon.
     * \param circle Option to create the polygon. \see ConstructionOption
     */
    QgsRegularPolygon( const QgsPoint &center, double radius, double azimuth, unsigned int numberSides, ConstructionOption circle );

    /**
     * Constructs a regular polygon by \a center and another point.
     * \param center The center of the regular polygon.
     * \param pt1 The first vertex if the polygon is inscribed in circle or the midpoint of a side if the polygon is circumscribed about circle.
     * \param numberSides Number of sides of the regular polygon.
     * \param circle Option to create the polygon inscribed in circle (the radius is the distance between the center and vertices) or circumscribed about circle (the radius is the distance from the center to the midpoints of the sides).
     */
    QgsRegularPolygon( const QgsPoint &center, const QgsPoint &pt1, unsigned int numberSides, ConstructionOption circle );

    /**
     * Constructs a regular polygon by two points of the first side.
     * \param pt1 The first vertex of the first side, also first vertex of the regular polygon.
     * \param pt2 The second vertex of the first side.
     * \param numberSides Number of sides of the regular polygon.
     */
    QgsRegularPolygon( const QgsPoint &pt1, const QgsPoint &pt2, unsigned int numberSides );

    bool operator ==( const QgsRegularPolygon &rp ) const;
    bool operator !=( const QgsRegularPolygon &rp ) const;

    //! A regular polygon is empty if radius equal to 0 or number of sides < 3
    bool isEmpty() const;

    /**
     * Returns the center point of the regular polygon.
     * \see setCenter()
     */
    QgsPoint center() const { return mCenter; }

    /**
     * Returns the radius.
     * This is also the radius of the circumscribing circle.
     * \see apothem()
     * \see setRadius()
     */
    double radius() const { return mRadius; }

    /**
     * Returns the first vertex (corner) of the regular polygon.
     * \see setFirstVertex()
     */
    QgsPoint firstVertex() const { return mFirstVertex; }

    /**
     * Returns the apothem of the regular polygon.
     * The apothem is the radius of the inscribed circle.
     * \see radius()
     */
    double apothem() const { return mRadius * std::cos( M_PI / mNumberSides ); }

    /**
     * Returns the number of sides of the regular polygon.
     * \see setNumberSides()
     */
    unsigned int numberSides() const { return mNumberSides; }

    /**
     * Sets the center point.
     * Radius is unchanged. The first vertex is reprojected from the new center.
     * \see center()
     */
    void setCenter( const QgsPoint &center );

    /**
     * Sets the radius.
     * Center is unchanged. The first vertex is reprojected from the center with the new radius.
     * \see radius()
     */
    void setRadius( double radius );

    /**
     * Sets the first vertex.
     * Radius is unchanged. The center is reprojected from the new first vertex.
     * \see firstVertex()
     */
    void setFirstVertex( const QgsPoint &firstVertex );

    /**
     * Sets the number of sides.
     * If numberSides < 3, the number of sides is unchanged.
     * \see numberSides()
     */
    void setNumberSides( unsigned int numberSides );

    /**
     * Returns a list including the vertices of the regular polygon.
     */
    QgsPointSequence points() const;

    /**
     * Returns as a polygon.
     */
    QgsPolygon *toPolygon() const SIP_FACTORY;

    /**
     * Returns as a linestring.
     */
    QgsLineString *toLineString() const SIP_FACTORY;

    /**
     * Returns as a triangle.
     * An empty triangle is returned if the regular polygon is empty or if the number of sides is different from 3.
     */
    QgsTriangle toTriangle() const;

    /**
     * Returns a triangulation (vertices from sides to the center) of the regular polygon.
     * An empty list is returned if the regular polygon is empty.
     */
    QVector<QgsTriangle> triangulate() const;

    /**
     * Returns the inscribed circle
     */
    QgsCircle inscribedCircle() const;

    /**
     * Returns the circumscribed circle
     */
    QgsCircle circumscribedCircle() const;

    /**
     * Returns a string representation of the regular polygon.
     * Members will be truncated to the specified precision.
     */
    QString toString( int pointPrecision = 17, int radiusPrecision = 17, int anglePrecision = 2 ) const;

    /**
     * Returns the measure of the interior angles in degrees.
     */
    double interiorAngle() const;

    /**
     * Returns the measure of the central angle (the angle subtended at the center of the polygon by one of its sides) in degrees.
     */
    double centralAngle() const;

    /**
     * Returns the area.
     * Returns 0 if the regular polygon is empty.
     */
    double area() const;

    /**
     * Returns the perimeter.
     * Returns 0 if the regular polygon is empty.
     */
    double perimeter() const;

    /**
     * Returns the length of a side.
     * Returns 0 if the regular polygon is empty.
     */
    double length() const;

  private:
    QgsPoint mCenter;
    QgsPoint mFirstVertex;
    unsigned int mNumberSides = 0;
    double mRadius = 0.0;

    /**
     * Convenient method to convert an apothem to a radius.
     */
    double apothemToRadius( double apothem, unsigned int numberSides ) const;

    /**
     * Convenient method for interiorAngle used by constructors.
     */
    double interiorAngle( unsigned int nbSides ) const;

    /**
     * Convenient method for centralAngle used by constructors.
     */
    double centralAngle( unsigned int nbSides ) const;

};

#endif // QGSREGULARPOLYGON_H

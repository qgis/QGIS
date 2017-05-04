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
#include "qgspointv2.h"
#include "qgspolygon.h"
#include "qgslinestring.h"
#include "qgscircle.h"
#include "qgstriangle.h"

enum ConstructionOption
{
  inscribedCircle,
  circumscribedCircle
};

/** \ingroup core
 * \class QgsRegularPolygon
 * \brief Regular Polygon geometry type.
 *
 * A regular polygon is defined by a center point with a number of sides/vertices, a radius and the first vertice.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsRegularPolygon
{
  public:
    QgsRegularPolygon();

    /** Constructs a regular polygon by center and parameters for the first vertice. An empty regular polygon is returned if numSides < 3 or circle option isn't valid.
     * \param center The center of the regular polygon.
     * \param radius Distance from the center and the first vertice or sides (see \a circle).
     * \param azimuth Angle in degrees started from the North to the first vertice.
     * \param numSides Number of sides of the regular polygon.
     * \param circle Option to create the polygon: inscribed in circle (the radius is the distance between the center and vertices) or circumscribed about circle (the radius is the distance from the center to the midpoints of the sides).
     */
    QgsRegularPolygon( const QgsPointV2 center, const double radius, const double azimuth, const int numSides, const int circle );

    /** Constructs a regular polygon by center and another point.
     * \param center The center of the regular polygon.
     * \param pt1 The first vertice if the polygon is inscribed in circle or the midpoint of a side if the polygon is circumscribed about circle.
     * \param numSides Number of sides of the regular polygon.
     * \param circle Option to create the polygon inscribed in circle (the radius is the distance between the center and vertices) or circumscribed about circle (the radius is the distance from the center to the midpoints of the sides).
     */
    QgsRegularPolygon( const QgsPointV2 center, const QgsPointV2 pt1, const int  numSides, const int circle );

    /** Constructs a regular polygon by two points of the first side.
     * \param pt1 The first vertice of the first side, also first vertice of the regular polygon.
     * \param pt2 The second vertice of the first side.
     * \param numSides Number of sides of the regular polygon.
     */
    QgsRegularPolygon( const QgsPointV2 pt1, const QgsPointV2 pt2, const int  numSides );

    virtual bool operator ==( const QgsRegularPolygon &rp ) const;
    virtual bool operator !=( const QgsRegularPolygon &rp ) const;

    //! A regular polygon is empty if radius equal to 0 or number of sides < 3
    virtual bool isEmpty() const;

    /** Returns the center point of the regular polygon.
     * \see setCenter()
     */
    QgsPointV2 center() const {return mCenter; }

    /** Returns the radius.
     * This is also the radius of the circumscribing circle.
     * \see apothem()
     * \see setRadius()
     */
    double radius() const {return mRadius; }

    /** Returns the first vertice (corner) of the regular polygon.
     * \see setVertice()
     */
    QgsPointV2 vertice() const {return mVertice; }

    /** Returns the apothem of the regular polygon.
     * The apothem is the radius of the inscribed circle.
     * \see radius()
     */
    double apothem() const {return mRadius * cos( M_PI / mNumSides ); }

    /** Returns the number of sides of the regular polygon.
     * \see setNumSides()
     */
    int numSides() const {return mNumSides; }

    /** Sets the center point.
     * Radius is unchanged. The first vertice is reprojected from the new center.
     * \see center()
     */
    void setCenter( const QgsPointV2 center );

    /** Sets the radius.
     * Center is unchanged. The first vertice is reprojected from the center with the new radius.
     * \see radius()
     */
    void setRadius( const double radius );

    /** Sets the first vertice.
     * Radius is unchanged. The center is reprojected from the new first vertice.
     * \see vertice()
     */
    void setVertice( const QgsPointV2 vertice );

    /** Sets the number of sides.
     * If numSides < 3, the number of sides is unchanged.
     * \see numSides()
     */
    void setNumSides( const int numSides );

    /** Returns a list of points into \a pts.
     * \param pts List of points returned.
     */
    void points( QgsPointSequence &pts SIP_OUT ) const;

    /** Returns as a polygon.
     */
    QgsPolygonV2 *toPolygon( ) const SIP_FACTORY;

    /** Returns as a linestring.
     */
    QgsLineString *toLineString( ) const SIP_FACTORY;

    /** Returns as a triangle.
     * An empty triangle is returned if the regular polygon is empty.
     */
    QgsTriangle toTriangle( ) const;

    /** Returns a triangulation (vertices from sides to the center) of the regular polygon.
     * An empty list is returned if the regular polygon is empty.
     */
    QList<QgsTriangle> triangulate( ) const;

    /** Returns the inscribed circle
     */
    QgsCircle inscribedCircle( ) const;

    /** Returns the circumscribed circle
     */
    QgsCircle circumscribedCircle( ) const;

    /**
     * returns a string representation of the regular polygon.
     * Members will be truncated to the specified precision.
     */
    QString toString( int pointPrecision = 17, int radiusPrecision = 17, int anglePrecision = 2 ) const;

    /** Returns the measure of the interior angles in degrees.
     */
    double interiorAngle( ) const;

    /** Returns the measure of the central angle (the angle subtended at the center of the polygon by one of its sides) in degrees.
     */
    double centralAngle( ) const;

    /** Returns the area.
     * Returns 0 if the regular polygon is empty.
     */
    double area( ) const;

    /** Returns the perimeter.
     * Returns 0 if the regular polygon is empty.
     */
    double perimeter( ) const;

    /** Returns the length of a side.
     * Returns 0 if the regular polygon is empty.
     */
    double length( ) const;

  protected:
    QgsPointV2 mCenter;
    QgsPointV2 mVertice;
    unsigned int mNumSides;
    double mRadius;

  private:

    /** Convenient method to convert an apothem to a radius.
     */
    double apothemToRadius( const double apothem, const unsigned int numSides ) const;

    /** Convenient method for interiorAngle used by constructors.
     */
    double interiorAngle( const unsigned int nbSides ) const;

    /** Convenient method for centralAngle used by constructors.
     */
    double centralAngle( const unsigned int nbSides ) const;

};

#endif // QGSREGULARPOLYGON_H

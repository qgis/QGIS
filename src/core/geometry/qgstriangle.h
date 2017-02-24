/***************************************************************************
                         qgstriangle.h
                         -------------------
    begin                : January 2017
    copyright            : (C) 2017 by Loïc Bartoletti
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

#ifndef QGSTRIANGLE_H
#define QGSTRIANGLE_H

#include "qgis_core.h"
#include "qgspolygon.h"
#include "qgslinestring.h"

/** \ingroup core
 * \class QgsTriangle
 * \brief Triangle geometry type.
 * \note added in QGIS 3.0
 */
class CORE_EXPORT QgsTriangle : public QgsPolygonV2
{
  public:
    QgsTriangle();
    QgsTriangle( const QgsPointV2 &p1, const QgsPointV2 &p2, const QgsPointV2 &p3 );

    // inherited: bool operator==( const QgsTriangle& other ) const;
    // inherited: bool operator!=( const QgsTriangle& other ) const;

    virtual QString geometryType() const override { return QStringLiteral( "Triangle" ); }
    virtual QgsTriangle* clone() const override;
    void clear() override;

    virtual bool fromWkb( QgsConstWkbPtr& wkbPtr ) override;

    bool fromWkt( const QString &wkt ) override;

    // inherited: QString asWkt( int precision = 17 ) const;
    // inherited: QDomElement asGML2( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const;
    // inherited: QDomElement asGML3( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const;
    // inherited: QString asJSON( int precision = 17 ) const;

    // inherited: QgsPolygonV2* surfaceToPolygon() const override;

    QgsAbstractGeometry* toCurveType() const override;

    void addInteriorRing( QgsCurve* ring ) override; // NOTE: no interior ring for triangle.
    //overridden to handle LineString25D rings
    virtual void setExteriorRing( QgsCurve* ring ) override;

    virtual QgsAbstractGeometry* boundary() const override;

    // inherited: double pointDistanceToBoundary( double x, double y ) const;

    /**
     *  Returns coordinates of a vertex.
     *  @param atVertex index of the vertex
     *  @return Coordinates of the vertex or QgsPointV2(0,0) on error
     */
    QgsPointV2 vertexAt( int atVertex ) const;

    // TODO:

    /**
     * Returns the three lengths of the triangle.
     * @return Lenghts of triangle ABC where [AB] is at 0, [BC] is at 1, [CA] is at 2
     */
    QVector<double> lengths() const;

    /**
     * Returns the three angles of the triangle.
     * @return Angles in radians of triangle ABC where angle BAC is at 0, angle ABC is at 1, angle BCA is at 2
     */
    QVector<double> angles() const;

    /**
     * Is the triangle isocele (two sides with the same length)?
     * @param lengthTolerance Tolerance to apply for comparison
     * @return True or False
     */
    bool isIsocele( double lengthTolerance = 0.0001 ) const;

    /**
     * Is the triangle equilateral (three sides with the same length)?
     * @param lengthTolerance Tolerance to apply for comparison
     * @return True or False
     */
    bool isEquilateral( double lengthTolerance = 0.0001 ) const;

    /**
     * Is the triangle right-angled?
     * @param angleTolerance Tolerance to apply for comparison
     * @return True or False
     */
    bool isRight( double angleTolerance = 0.0001 ) const;

    /**
     * Is the triangle scalene (all sides have differen lengths)?
     * @param lengthTolerance Tolerance to apply for comparison
     * @return True or False
     */
    bool isScalene( double lengthTolerance = 0.0001 ) const;

    /**
     * Altitudes
     * @return Three altitudes from this triangle
     */
    QVector<QgsLineString *> altitudes( ) const;
    // orthocenter
    // bisectors
    // medians
    // medial
    // circumcenter
    // circumradius
    // circumscribedCircle -> need QgsCircle (from CADDigitize.CADCircle)
    // incenter
    // inradius
    // inscribedCircle -> need QgsCircle (from CADDigitize.CADCircle)

};
#endif // QGSTRIANGLE_H

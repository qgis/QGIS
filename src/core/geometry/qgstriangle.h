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

    // TODO:
    // length
    // altitudes (not confuse with Z)
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
    // isIsocele
    // isEquilateral
    // isRight
    // isScalene


};
#endif // QGSTRIANGLE_H

/***************************************************************************
                         qgspolygonv2.h
                         -------------------
    begin                : September 2014
    copyright            : (C) 2014 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOLYGONV2_H
#define QGSPOLYGONV2_H

#include "qgscurvepolygonv2.h"

/** \ingroup core
 * \class QgsPolygonV2
 * \brief Polygon geometry type.
 * \note added in QGIS 2.10
 * \note this API is not considered stable and may change for 2.12
 */
class CORE_EXPORT QgsPolygonV2: public QgsCurvePolygonV2
{
  public:
    QgsPolygonV2();

    bool operator==( const QgsPolygonV2& other ) const;
    bool operator!=( const QgsPolygonV2& other ) const;

    virtual QString geometryType() const override { return "Polygon"; }
    virtual QgsPolygonV2* clone() const override;
    void clear() override;

    virtual bool fromWkb( QgsConstWkbPtr wkb ) override;

    // inherited: bool fromWkt( const QString &wkt );

    int wkbSize() const override;
    unsigned char* asWkb( int& binarySize ) const override;
    // inherited: QString asWkt( int precision = 17 ) const;
    // inherited: QDomElement asGML2( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const;
    // inherited: QDomElement asGML3( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const;
    // inherited: QString asJSON( int precision = 17 ) const;

    QgsPolygonV2* surfaceToPolygon() const override;

    /** Returns the geometry converted to the more generic curve type QgsCurvePolygonV2
     @return the converted geometry. Caller takes ownership*/
    QgsAbstractGeometryV2* toCurveType() const override;

    void addInteriorRing( QgsCurveV2* ring ) override;
    //overridden to handle LineString25D rings
    virtual void setExteriorRing( QgsCurveV2* ring ) override;

};
#endif // QGSPOLYGONV2_H

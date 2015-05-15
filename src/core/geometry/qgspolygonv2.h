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

class CORE_EXPORT QgsPolygonV2: public QgsCurvePolygonV2
{
  public:
    virtual QString geometryType() const { return "Polygon"; }
    virtual QgsAbstractGeometryV2* clone() const;

    virtual bool fromWkb( const unsigned char* wkb );
    // inherited: bool fromWkt( const QString &wkt );

    int wkbSize() const;
    unsigned char* asWkb( int& binarySize ) const;
    // inherited: QString asWkt( int precision = 17 ) const;
    // inherited: QDomElement asGML2( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const;
    // inherited: QDomElement asGML3( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const;
    // inherited: QString asJSON( int precision = 17 ) const;

    QgsPolygonV2* surfaceToPolygon() const;
};
#endif // QGSPOLYGONV2_H

/***************************************************************************
                        qgsmultipolygonv2.h
  -------------------------------------------------------------------
Date                 : 28 Oct 2014
Copyright            : (C) 2014 by Marco Hugentobler
email                : marco.hugentobler at sourcepole dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMULTIPOLYGONV2_H
#define QGSMULTIPOLYGONV2_H

#include "qgsgeometrycollectionv2.h"

class CORE_EXPORT QgsMultiPolygonV2: public QgsGeometryCollectionV2
{
  public:
    virtual QString geometryType() const { return "MultiPolygon"; }
    QgsAbstractGeometryV2* clone() const;

    bool fromWkt( const QString& wkt );

    // inherited: int wkbSize() const;
    // inherited: unsigned char* asWkb( int& binarySize ) const;
    // inherited: QString asWkt( int precision = 17 ) const;
    QDomElement asGML2( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const;
    QDomElement asGML3( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const;
    QString asJSON( int precision = 17 ) const;


    /**Adds a geometry and takes ownership. Returns true in case of success*/
    virtual bool addGeometry( QgsAbstractGeometryV2* g );
};

#endif // QGSMULTIPOLYGONV2_H

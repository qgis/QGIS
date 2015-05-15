/***************************************************************************
                           qgsgeometryimport.h
                         -----------------------
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

#ifndef QGSGEOMETRYIMPORT_H
#define QGSGEOMETRYIMPORT_H

#include "qgsrectangle.h"
#include "qgswkbtypes.h"
#include <QString>

class QgsAbstractGeometryV2;
class QgsLineStringV2;

//compatibility with old classes
#include "qgspoint.h"
typedef QVector<QgsPoint> QgsPolyline;
typedef QVector<QgsPolyline> QgsPolygon;
typedef QVector<QgsPoint> QgsMultiPoint;
typedef QVector<QgsPolyline> QgsMultiPolyline;
typedef QVector<QgsPolygon> QgsMultiPolygon;

class CORE_EXPORT QgsGeometryImport
{
  public:
    static QgsAbstractGeometryV2* geomFromWkb( const unsigned char* wkb );
    static QgsAbstractGeometryV2* geomFromWkt( const QString& text );
    /** construct geometry from a point */
    static QgsAbstractGeometryV2* fromPoint( const QgsPoint& point );
    /** construct geometry from a multipoint */
    static QgsAbstractGeometryV2* fromMultiPoint( const QgsMultiPoint& multipoint );
    /** construct geometry from a polyline */
    static QgsAbstractGeometryV2* fromPolyline( const QgsPolyline& polyline );
    /** construct geometry from a multipolyline*/
    static QgsAbstractGeometryV2* fromMultiPolyline( const QgsMultiPolyline& multiline );
    /** construct geometry from a polygon */
    static QgsAbstractGeometryV2* fromPolygon( const QgsPolygon& polygon );
    /** construct geometry from a multipolygon */
    static QgsAbstractGeometryV2* fromMultiPolygon( const QgsMultiPolygon& multipoly );
    /** construct geometry from a rectangle */
    static QgsAbstractGeometryV2* fromRect( const QgsRectangle& rect );
    /** return empty geometry from wkb type*/
    static QgsAbstractGeometryV2* geomFromWkbType( QgsWKBTypes::Type t );

  private:
    static QgsLineStringV2* linestringFromPolyline( const QgsPolyline& polyline );
};

#endif // QGSGEOMETRYIMPORT_H

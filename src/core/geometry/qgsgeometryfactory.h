/***************************************************************************
                           qgsgeometryfactory.h
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

#ifndef QGSGEOMETRYFACTORY_H
#define QGSGEOMETRYFACTORY_H

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

/** \ingroup core
 * \class QgsGeometryFactory
 * \brief Contains geometry creation routines.
 * \note added in QGIS 2.10
 * \note this API is not considered stable and may change for 2.12
 */
class CORE_EXPORT QgsGeometryFactory
{
  public:
    /** Construct geometry from a WKB string.
     */
    static QgsAbstractGeometryV2* geomFromWkb( const unsigned char* wkb );

    /** Construct geometry from a WKT string.
     */
    static QgsAbstractGeometryV2* geomFromWkt( const QString& text );

    /** Construct geometry from a point */
    static QgsAbstractGeometryV2* fromPoint( const QgsPoint& point );
    /** Construct geometry from a multipoint */
    static QgsAbstractGeometryV2* fromMultiPoint( const QgsMultiPoint& multipoint );
    /** Construct geometry from a polyline */
    static QgsAbstractGeometryV2* fromPolyline( const QgsPolyline& polyline );
    /** Construct geometry from a multipolyline*/
    static QgsAbstractGeometryV2* fromMultiPolyline( const QgsMultiPolyline& multiline );
    /** Construct geometry from a polygon */
    static QgsAbstractGeometryV2* fromPolygon( const QgsPolygon& polygon );
    /** Construct geometry from a multipolygon */
    static QgsAbstractGeometryV2* fromMultiPolygon( const QgsMultiPolygon& multipoly );
    /** Construct geometry from a rectangle */
    static QgsAbstractGeometryV2* fromRect( const QgsRectangle& rect );
    /** Return empty geometry from wkb type*/
    static QgsAbstractGeometryV2* geomFromWkbType( QgsWKBTypes::Type t );

  private:
    static QgsLineStringV2* linestringFromPolyline( const QgsPolyline& polyline );
};

#endif // QGSGEOMETRYFACTORY_H

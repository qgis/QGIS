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

#include <QString>

class QgsAbstractGeometry;
class QgsLineString;
class QgsConstWkbPtr;
class QgsRectangle;

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
 * \note not available in Python bindings
 */
class CORE_EXPORT QgsGeometryFactory
{
  public:
    /** Construct geometry from a WKB string.
     */
    static QgsAbstractGeometry* geomFromWkb( QgsConstWkbPtr wkb );

    /** Construct geometry from a WKT string.
     */
    static QgsAbstractGeometry* geomFromWkt( const QString& text );

    /** Construct geometry from a point */
    static QgsAbstractGeometry* fromPoint( const QgsPoint& point );
    /** Construct geometry from a multipoint */
    static QgsAbstractGeometry* fromMultiPoint( const QgsMultiPoint& multipoint );
    /** Construct geometry from a polyline */
    static QgsAbstractGeometry* fromPolyline( const QgsPolyline& polyline );
    /** Construct geometry from a multipolyline*/
    static QgsAbstractGeometry* fromMultiPolyline( const QgsMultiPolyline& multiline );
    /** Construct geometry from a polygon */
    static QgsAbstractGeometry* fromPolygon( const QgsPolygon& polygon );
    /** Construct geometry from a multipolygon */
    static QgsAbstractGeometry* fromMultiPolygon( const QgsMultiPolygon& multipoly );
    /** Construct geometry from a rectangle */
    static QgsAbstractGeometry* fromRect( const QgsRectangle& rect );
    /** Return empty geometry from wkb type*/
    static QgsAbstractGeometry* geomFromWkbType( QgsWkbTypes::Type t );

  private:
    static QgsLineString* linestringFromPolyline( const QgsPolyline& polyline );
};

#endif // QGSGEOMETRYFACTORY_H

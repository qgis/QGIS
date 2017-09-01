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

#define SIP_NO_FILE

#include "qgis_core.h"
#include <QString>
#include <memory>

class QgsAbstractGeometry;
class QgsLineString;
class QgsConstWkbPtr;
class QgsRectangle;
class QgsGeometryCollection;
class QgsMultiPointV2;
class QgsMultiLineString;
class QgsPolygonV2;
class QgsMultiPolygonV2;

//compatibility with old classes
#include "qgspointxy.h"
typedef QVector<QgsPointXY> QgsPolyline;
typedef QVector<QgsPolyline> QgsPolygon;
typedef QVector<QgsPointXY> QgsMultiPoint;
typedef QVector<QgsPolyline> QgsMultiPolyline;
typedef QVector<QgsPolygon> QgsMultiPolygon;

/** \ingroup core
 * \class QgsGeometryFactory
 * \brief Contains geometry creation routines.
 * \since QGIS 2.10
 * \note not available in Python bindings
 */
class CORE_EXPORT QgsGeometryFactory
{
  public:

    /** Construct geometry from a WKB string.
     * Updates position of the passed WKB pointer.
     */
    static std::unique_ptr< QgsAbstractGeometry > geomFromWkb( QgsConstWkbPtr &wkb );

    /** Construct geometry from a WKT string.
     */
    static std::unique_ptr< QgsAbstractGeometry > geomFromWkt( const QString &text );

    //! Construct geometry from a point
    static std::unique_ptr< QgsAbstractGeometry > fromPoint( const QgsPointXY &point );
    //! Construct geometry from a multipoint
    static std::unique_ptr<QgsMultiPointV2> fromMultiPoint( const QgsMultiPoint &multipoint );
    //! Construct geometry from a polyline
    static std::unique_ptr< QgsAbstractGeometry > fromPolyline( const QgsPolyline &polyline );
    //! Construct geometry from a multipolyline
    static std::unique_ptr<QgsMultiLineString> fromMultiPolyline( const QgsMultiPolyline &multiline );
    //! Construct geometry from a polygon
    static std::unique_ptr<QgsPolygonV2> fromPolygon( const QgsPolygon &polygon );
    //! Construct geometry from a multipolygon
    static std::unique_ptr<QgsMultiPolygonV2> fromMultiPolygon( const QgsMultiPolygon &multipoly );
    //! Construct geometry from a rectangle
    static std::unique_ptr<QgsPolygonV2> fromRect( const QgsRectangle &rect );
    //! Return empty geometry from wkb type
    static std::unique_ptr< QgsAbstractGeometry > geomFromWkbType( QgsWkbTypes::Type t );

    /**
     * Returns a new geometry collection matching a specified WKB \a type. For instance, if
     * type is PolygonM the returned geometry will be a QgsMultiPolygonV2 with M values.
     */
    static std::unique_ptr< QgsGeometryCollection > createCollectionOfType( QgsWkbTypes::Type type );

  private:
    static std::unique_ptr< QgsLineString > linestringFromPolyline( const QgsPolyline &polyline );
};

#endif // QGSGEOMETRYFACTORY_H

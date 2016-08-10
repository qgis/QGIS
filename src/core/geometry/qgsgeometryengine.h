/***************************************************************************
                        qgsgeometryengine.h
  -------------------------------------------------------------------
Date                 : 22 Sept 2014
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

#ifndef QGSGEOMETRYENGINE_H
#define QGSGEOMETRYENGINE_H

#include "qgslinestring.h"

#include <QList>

class QgsAbstractGeometry;

/** \ingroup core
 * \class QgsGeometryEngine
 * \brief Contains geometry relation and modification algorithms.
 * \note added in QGIS 2.10
 * \note this API is not considered stable and may change for 2.12
 */
class CORE_EXPORT QgsGeometryEngine
{
  public:
    QgsGeometryEngine( const QgsAbstractGeometry* geometry ): mGeometry( geometry ) {}
    virtual ~QgsGeometryEngine() {}

    virtual void geometryChanged() = 0;
    virtual void prepareGeometry() = 0;

    virtual QgsAbstractGeometry* intersection( const QgsAbstractGeometry& geom, QString* errorMsg = nullptr ) const = 0;
    virtual QgsAbstractGeometry* difference( const QgsAbstractGeometry& geom, QString* errorMsg = nullptr ) const = 0;
    virtual QgsAbstractGeometry* combine( const QgsAbstractGeometry& geom, QString* errorMsg = nullptr ) const = 0;
    virtual QgsAbstractGeometry* combine( const QList< QgsAbstractGeometry* >&, QString* errorMsg = nullptr ) const = 0;
    virtual QgsAbstractGeometry* symDifference( const QgsAbstractGeometry& geom, QString* errorMsg = nullptr ) const = 0;
    virtual QgsAbstractGeometry* buffer( double distance, int segments, QString* errorMsg = nullptr ) const = 0;
    virtual QgsAbstractGeometry* buffer( double distance, int segments, int endCapStyle, int joinStyle, double mitreLimit, QString* errorMsg = nullptr ) const = 0;
    virtual QgsAbstractGeometry* simplify( double tolerance, QString* errorMsg = nullptr ) const = 0;
    virtual QgsAbstractGeometry* interpolate( double distance, QString* errorMsg = nullptr ) const = 0;
    virtual QgsAbstractGeometry* envelope( QString* errorMsg = nullptr ) const = 0;
    virtual bool centroid( QgsPointV2& pt, QString* errorMsg = nullptr ) const = 0;
    virtual bool pointOnSurface( QgsPointV2& pt, QString* errorMsg = nullptr ) const = 0;
    virtual QgsAbstractGeometry* convexHull( QString* errorMsg = nullptr ) const = 0;
    virtual double distance( const QgsAbstractGeometry& geom, QString* errorMsg = nullptr ) const = 0;
    virtual bool intersects( const QgsAbstractGeometry& geom, QString* errorMsg = nullptr ) const = 0;
    virtual bool touches( const QgsAbstractGeometry& geom, QString* errorMsg = nullptr ) const = 0;
    virtual bool crosses( const QgsAbstractGeometry& geom, QString* errorMsg = nullptr ) const = 0;
    virtual bool within( const QgsAbstractGeometry& geom, QString* errorMsg = nullptr ) const = 0;
    virtual bool overlaps( const QgsAbstractGeometry& geom, QString* errorMsg = nullptr ) const = 0;
    virtual bool contains( const QgsAbstractGeometry& geom, QString* errorMsg = nullptr ) const = 0;
    virtual bool disjoint( const QgsAbstractGeometry& geom, QString* errorMsg = nullptr ) const = 0;

    /** Returns the Dimensional Extended 9 Intersection Model (DE-9IM) representation of the
     * relationship between the geometries.
     * @param geom geometry to relate to
     * @param errorMsg destination storage for any error message
     * @returns DE-9IM string for relationship, or an empty string if an error occurred
     * @note added in QGIS 2.12
     */
    virtual QString relate( const QgsAbstractGeometry& geom, QString* errorMsg = nullptr ) const = 0;

    /** Tests whether two geometries are related by a specified Dimensional Extended 9 Intersection Model (DE-9IM)
     * pattern.
     * @param geom geometry to relate to
     * @param pattern DE-9IM pattern for match
     * @param errorMsg destination storage for any error message
     * @returns true if geometry relationship matches with pattern
     * @note added in QGIS 2.14
     */
    virtual bool relatePattern( const QgsAbstractGeometry& geom, const QString& pattern, QString* errorMsg = nullptr ) const = 0;

    virtual double area( QString* errorMsg = nullptr ) const = 0;
    virtual double length( QString* errorMsg = nullptr ) const = 0;
    virtual bool isValid( QString* errorMsg = nullptr ) const = 0;
    virtual bool isEqual( const QgsAbstractGeometry& geom, QString* errorMsg = nullptr ) const = 0;
    virtual bool isEmpty( QString* errorMsg ) const = 0;

    virtual int splitGeometry( const QgsLineString& splitLine,
                               QList<QgsAbstractGeometry*>& newGeometries,
                               bool topological,
                               QgsPointSequence &topologyTestPoints, QString* errorMsg = nullptr ) const
    {
      Q_UNUSED( splitLine );
      Q_UNUSED( newGeometries );
      Q_UNUSED( topological );
      Q_UNUSED( topologyTestPoints );
      Q_UNUSED( errorMsg );
      return 2;
    }

    virtual QgsAbstractGeometry* offsetCurve( double distance, int segments, int joinStyle, double mitreLimit, QString* errorMsg = nullptr ) const = 0;

  protected:
    const QgsAbstractGeometry* mGeometry;

    QgsGeometryEngine();
};

#endif // QGSGEOMETRYENGINE_H

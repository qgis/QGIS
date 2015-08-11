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

#ifndef QGSVECTORTOPOLOGY_H
#define QGSVECTORTOPOLOGY_H

#include "qgspointv2.h"
#include "qgslinestringv2.h"

#include <QList>

class QgsAbstractGeometryV2;


/** \ingroup core
 * \class QgsGeometryEngine
 * \brief Contains geometry relation and modification algorithms.
 * \note added in QGIS 2.10
 * \note this API is not considered stable and may change for 2.12
 */
class CORE_EXPORT QgsGeometryEngine
{
  public:
    QgsGeometryEngine( const QgsAbstractGeometryV2* geometry ): mGeometry( geometry ) {}
    virtual ~QgsGeometryEngine() {}

    virtual void geometryChanged() = 0;
    virtual void prepareGeometry() = 0;

    virtual QgsAbstractGeometryV2* intersection( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual QgsAbstractGeometryV2* difference( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual QgsAbstractGeometryV2* combine( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual QgsAbstractGeometryV2* combine( const QList< const QgsAbstractGeometryV2* > ) const = 0;
    virtual QgsAbstractGeometryV2* symDifference( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual QgsAbstractGeometryV2* buffer( double distance, int segments ) const = 0;
    virtual QgsAbstractGeometryV2* buffer( double distance, int segments, int endCapStyle, int joinStyle, double mitreLimit ) const = 0;
    virtual QgsAbstractGeometryV2* simplify( double tolerance ) const = 0;
    virtual QgsAbstractGeometryV2* interpolate( double distance ) const = 0;
    virtual bool centroid( QgsPointV2& pt ) const = 0;
    virtual bool pointOnSurface( QgsPointV2& pt ) const = 0;
    virtual QgsAbstractGeometryV2* convexHull() const = 0;
    virtual double distance( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual bool intersects( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual bool touches( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual bool crosses( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual bool within( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual bool overlaps( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual bool contains( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual bool disjoint( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual double area() const = 0;
    virtual double length() const = 0;
    virtual bool isValid() const = 0;
    virtual bool isEqual( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual bool isEmpty() const = 0;

    virtual int splitGeometry( const QgsLineStringV2& splitLine,
                               QList<QgsAbstractGeometryV2*>& newGeometries,
                               bool topological,
                               QList<QgsPointV2> &topologyTestPoints ) const
    {
      Q_UNUSED( splitLine );
      Q_UNUSED( newGeometries );
      Q_UNUSED( topological );
      Q_UNUSED( topologyTestPoints );
      return 2;
    } //= 0;

    virtual QgsAbstractGeometryV2* offsetCurve( double distance, int segments, int joinStyle, double mitreLimit ) const = 0;

  protected:
    const QgsAbstractGeometryV2* mGeometry;

    QgsGeometryEngine();
};

#endif // QGSVECTORTOPOLOGY_H

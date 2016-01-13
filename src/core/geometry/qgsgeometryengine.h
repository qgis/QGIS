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

#include "qgspointv2.h"
#include "qgslinestringv2.h"
#include "qgsgeos.h"

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
    QgsGeometryEngine( const QgsAbstractGeometryV2* geometry, int precision = 0 );
    ~QgsGeometryEngine();

    /**
     * Will clear any cached geometry data.
     *
     * TODO QGIS3: remove this and use implicitly shared QgsGeometry instead.
     */
    void geometryChanged();

    /**
     * Prepare the geomtetry for efficient processing.
     *
     * TODO: What is the optimal strategy for this in regards to different backends?
     * Should the caller have to deal with this kind of lowlevel functionality?
     * Do we need prepareGeosGeometry() instead?
     */
    void prepareGeometry();

    QgsAbstractGeometryV2* intersection( const QgsAbstractGeometryV2& geom, QString* errorMsg = nullptr ) const;
    QgsAbstractGeometryV2* difference( const QgsAbstractGeometryV2& geom, QString* errorMsg = nullptr ) const;
    QgsAbstractGeometryV2* combine( const QgsAbstractGeometryV2& geom, QString* errorMsg = nullptr ) const;
    QgsAbstractGeometryV2* combine( const QList< QgsAbstractGeometryV2* >&geomList, QString* errorMsg = nullptr ) const;
    QgsAbstractGeometryV2* symDifference( const QgsAbstractGeometryV2& geom, QString* errorMsg = nullptr ) const;
    QgsAbstractGeometryV2* buffer( double distance, int segments, QString* errorMsg = nullptr ) const;
    QgsAbstractGeometryV2* buffer( double distance, int segments, int endCapStyle, int joinStyle, double mitreLimit, QString* errorMsg = nullptr ) const;
    QgsAbstractGeometryV2* simplify( double tolerance, QString* errorMsg = nullptr ) const;
    QgsAbstractGeometryV2* interpolate( double distance, QString* errorMsg = nullptr ) const;
    QgsAbstractGeometryV2* envelope( QString* errorMsg = nullptr ) const;
    bool centroid( QgsPointV2& pt, QString* errorMsg = nullptr ) const;
    bool pointOnSurface( QgsPointV2& pt, QString* errorMsg = nullptr ) const;
    QgsAbstractGeometryV2* convexHull( QString* errorMsg = nullptr ) const;
    double distance( const QgsAbstractGeometryV2& geom, QString* errorMsg = nullptr ) const;
    bool intersects( const QgsAbstractGeometryV2& geom, QString* errorMsg = nullptr ) const;
    bool touches( const QgsAbstractGeometryV2& geom, QString* errorMsg = nullptr ) const;
    bool crosses( const QgsAbstractGeometryV2& geom, QString* errorMsg = nullptr ) const;
    bool within( const QgsAbstractGeometryV2& geom, QString* errorMsg = nullptr ) const;
    bool overlaps( const QgsAbstractGeometryV2& geom, QString* errorMsg = nullptr ) const;
    bool contains( const QgsAbstractGeometryV2& geom, QString* errorMsg = nullptr ) const;
    bool disjoint( const QgsAbstractGeometryV2& geom, QString* errorMsg = nullptr ) const;

    /** Returns the Dimensional Extended 9 Intersection Model (DE-9IM) representation of the
     * relationship between the geometries.
     * @param geom geometry to relate to
     * @param errorMsg destination storage for any error message
     * @returns DE-9IM string for relationship, or an empty string if an error occurred
     * @note added in QGIS 2.12
     */
    QString relate( const QgsAbstractGeometryV2& geom, QString* errorMsg = nullptr ) const;

    /** Tests whether two geometries are related by a specified Dimensional Extended 9 Intersection Model (DE-9IM)
     * pattern.
     * @param geom geometry to relate to
     * @param pattern DE-9IM pattern for match
     * @param errorMsg destination storage for any error message
     * @returns true if geometry relationship matches with pattern
     * @note added in QGIS 2.14
     */
    bool relatePattern( const QgsAbstractGeometryV2& geom, const QString& pattern, QString* errorMsg = nullptr ) const;

    double area( QString* errorMsg = nullptr ) const;
    double length( QString* errorMsg = nullptr ) const;
    bool isValid( QString* errorMsg = nullptr ) const;
    bool isEqual( const QgsAbstractGeometryV2& geom, QString* errorMsg = nullptr ) const;
    bool isEmpty( QString* errorMsg ) const;

    int splitGeometry( const QgsLineStringV2& splitLine,
                       QList<QgsAbstractGeometryV2*>& newGeometries,
                       bool topological,
                       QList<QgsPointV2> &topologyTestPoints, QString* errorMsg = nullptr ) const;

    QgsAbstractGeometryV2* offsetCurve( double distance, int segments, int joinStyle, double mitreLimit, QString* errorMsg = nullptr ) const;

  private:
    /**
     * Private default constructor. Use QgsGeometry::createEngine() instead.
     */
    QgsGeometryEngine();
    /**
     * Creates a geos engine if not present or returns an existing one.
     */
    QgsGeos* geosEngine() const;

    const QgsAbstractGeometryV2* mGeometry;
    int mPrecision;

    mutable QgsGeos* mGeosEngine;
};

#endif // QGSGEOMETRYENGINE_H

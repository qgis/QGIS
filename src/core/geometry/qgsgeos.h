/***************************************************************************
                        qgsgeos.h
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

#ifndef QGSGEOS_H
#define QGSGEOS_H

#include "qgsgeometryengine.h"
#include "qgspointv2.h"
#include <geos_c.h>

class QgsLineStringV2;
class QgsPolygonV2;

/** Does vector analysis using the geos library and handles import, export, exception handling*
 * \note this API is not considered stable and may change for 2.12
 */
class CORE_EXPORT QgsGeos: public QgsGeometryEngine
{
  public:
    QgsGeos( const QgsAbstractGeometryV2* geometry );
    ~QgsGeos();

    /** Removes caches*/
    void geometryChanged() override;
    void prepareGeometry() override;

    QgsAbstractGeometryV2* intersection( const QgsAbstractGeometryV2& geom ) const override;
    QgsAbstractGeometryV2* difference( const QgsAbstractGeometryV2& geom ) const override;
    QgsAbstractGeometryV2* combine( const QgsAbstractGeometryV2& geom ) const override;
    QgsAbstractGeometryV2* combine( const QList< const QgsAbstractGeometryV2* > ) const override;
    QgsAbstractGeometryV2* symDifference( const QgsAbstractGeometryV2& geom ) const override;
    QgsAbstractGeometryV2* buffer( double distance, int segments ) const override;
    QgsAbstractGeometryV2* buffer( double distance, int segments, int endCapStyle, int joinStyle, double mitreLimit ) const override;
    QgsAbstractGeometryV2* simplify( double tolerance ) const override;
    QgsAbstractGeometryV2* interpolate( double distance ) const override;
    bool centroid( QgsPointV2& pt ) const override;
    bool pointOnSurface( QgsPointV2& pt ) const override;
    QgsAbstractGeometryV2* convexHull() const override;
    double distance( const QgsAbstractGeometryV2& geom ) const override;
    bool intersects( const QgsAbstractGeometryV2& geom ) const override;
    bool touches( const QgsAbstractGeometryV2& geom ) const override;
    bool crosses( const QgsAbstractGeometryV2& geom ) const override;
    bool within( const QgsAbstractGeometryV2& geom ) const override;
    bool overlaps( const QgsAbstractGeometryV2& geom ) const override;
    bool contains( const QgsAbstractGeometryV2& geom ) const override;
    bool disjoint( const QgsAbstractGeometryV2& geom ) const override;
    double area() const override;
    double length() const override;
    bool isValid() const override;
    bool isEqual( const QgsAbstractGeometryV2& geom ) const override;
    bool isEmpty() const override;

    /** Splits this geometry according to a given line.
    @param splitLine the line that splits the geometry
    @param[out] newGeometries list of new geometries that have been created with the split
    @param topological true if topological editing is enabled
    @param[out] topologyTestPoints points that need to be tested for topological completeness in the dataset
    @return 0 in case of success, 1 if geometry has not been split, error else*/
    int splitGeometry( const QgsLineStringV2& splitLine,
                       QList<QgsAbstractGeometryV2*>& newGeometries,
                       bool topological,
                       QList<QgsPointV2> &topologyTestPoints ) const override;

    QgsAbstractGeometryV2* offsetCurve( double distance, int segments, int joinStyle, double mitreLimit ) const override;
    QgsAbstractGeometryV2* reshapeGeometry( const QgsLineStringV2& reshapeWithLine, int* errorCode ) const;

    static QgsAbstractGeometryV2* fromGeos( const GEOSGeometry* geos );
    static QgsPolygonV2* fromGeosPolygon( const GEOSGeometry* geos );
    static GEOSGeometry* asGeos( const QgsAbstractGeometryV2* geom );
    static QgsPointV2 coordSeqPoint( const GEOSCoordSequence* cs, int i, bool hasZ, bool hasM );

    static GEOSContextHandle_t getGEOSHandler();

  private:
    mutable GEOSGeometry* mGeos;
    const GEOSPreparedGeometry* mGeosPrepared;

    enum Overlay
    {
      INTERSECTION,
      DIFFERENCE,
      UNION,
      SYMDIFFERENCE
    };

    enum Relation
    {
      INTERSECTS,
      TOUCHES,
      CROSSES,
      WITHIN,
      OVERLAPS,
      CONTAINS,
      DISJOINT
    };

    //geos util functions
    void cacheGeos() const;
    QgsAbstractGeometryV2* overlay( const QgsAbstractGeometryV2& geom, Overlay op ) const;
    bool relation( const QgsAbstractGeometryV2& geom, Relation r ) const;
    static GEOSCoordSequence* createCoordinateSequence( const QgsCurveV2* curve );
    static QgsLineStringV2* sequenceToLinestring( const GEOSGeometry* geos, bool hasZ, bool hasM );
    static int numberOfGeometries( GEOSGeometry* g );
    static GEOSGeometry* nodeGeometries( const GEOSGeometry *splitLine, const GEOSGeometry *geom );
    int mergeGeometriesMultiTypeSplit( QVector<GEOSGeometry*>& splitResult ) const;
    static GEOSGeometry* createGeosCollection( int typeId, const QVector<GEOSGeometry*>& geoms );

    static GEOSGeometry* createGeosPoint( const QgsAbstractGeometryV2* point, int coordDims );
    static GEOSGeometry* createGeosLinestring( const QgsAbstractGeometryV2* curve );
    static GEOSGeometry* createGeosPolygon( const QgsAbstractGeometryV2* poly );

    //utils for geometry split
    int topologicalTestPointsSplit( const GEOSGeometry* splitLine, QList<QgsPointV2>& testPoints ) const;
    GEOSGeometry* linePointDifference( GEOSGeometry* GEOSsplitPoint ) const;
    int splitLinearGeometry( GEOSGeometry* splitLine, QList<QgsAbstractGeometryV2*>& newGeometries ) const;
    int splitPolygonGeometry( GEOSGeometry* splitLine, QList<QgsAbstractGeometryV2*>& newGeometries ) const;

    //utils for reshape
    static GEOSGeometry* reshapeLine( const GEOSGeometry* line, const GEOSGeometry* reshapeLineGeos );
    static GEOSGeometry* reshapePolygon( const GEOSGeometry* polygon, const GEOSGeometry* reshapeLineGeos );
    static int lineContainedInLine( const GEOSGeometry* line1, const GEOSGeometry* line2 );
    static int pointContainedInLine( const GEOSGeometry* point, const GEOSGeometry* line );
    static int geomDigits( const GEOSGeometry* geom );
};

/// @cond

class GEOSException
{
  public:
    GEOSException( QString theMsg )
    {
      if ( theMsg == "Unknown exception thrown"  && lastMsg.isNull() )
      {
        msg = theMsg;
      }
      else
      {
        msg = theMsg;
        lastMsg = msg;
      }
    }

    // copy constructor
    GEOSException( const GEOSException &rhs )
    {
      *this = rhs;
    }

    ~GEOSException()
    {
      if ( lastMsg == msg )
        lastMsg = QString::null;
    }

    QString what()
    {
      return msg;
    }

  private:
    QString msg;
    static QString lastMsg;
};

/// @endcond

#endif // QGSGEOS_H

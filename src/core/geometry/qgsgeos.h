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
#include "qgsgeometry.h"
#include <geos_c.h>

class QgsLineStringV2;
class QgsPolygonV2;

/** \ingroup core
 * Does vector analysis using the geos library and handles import, export, exception handling*
 * \note this API is not considered stable and may change for 2.12
 * \note not available in Python bindings
 */
class CORE_EXPORT QgsGeos: public QgsGeometryEngine
{
  public:
    /** GEOS geometry engine constructor
     * @param geometry The geometry
     * @param precision The precision of the grid to which to snap the geometry vertices. If 0, no snapping is performed.
     */
    QgsGeos( const QgsAbstractGeometryV2* geometry, double precision = 0 );
    ~QgsGeos();

    /** Removes caches*/
    void geometryChanged() override;
    void prepareGeometry() override;

    QgsAbstractGeometryV2* intersection( const QgsAbstractGeometryV2& geom, QString* errorMsg = nullptr ) const override;
    QgsAbstractGeometryV2* difference( const QgsAbstractGeometryV2& geom, QString* errorMsg = nullptr ) const override;
    QgsAbstractGeometryV2* combine( const QgsAbstractGeometryV2& geom, QString* errorMsg = nullptr ) const override;
    QgsAbstractGeometryV2* combine( const QList< QgsAbstractGeometryV2*>&, QString* errorMsg = nullptr ) const override;
    QgsAbstractGeometryV2* symDifference( const QgsAbstractGeometryV2& geom, QString* errorMsg = nullptr ) const override;
    QgsAbstractGeometryV2* buffer( double distance, int segments, QString* errorMsg = nullptr ) const override;
    QgsAbstractGeometryV2* buffer( double distance, int segments, int endCapStyle, int joinStyle, double mitreLimit, QString* errorMsg = nullptr ) const override;
    QgsAbstractGeometryV2* simplify( double tolerance, QString* errorMsg = nullptr ) const override;
    QgsAbstractGeometryV2* interpolate( double distance, QString* errorMsg = nullptr ) const override;
    QgsAbstractGeometryV2* envelope( QString* errorMsg = nullptr ) const override;
    bool centroid( QgsPointV2& pt, QString* errorMsg = nullptr ) const override;
    bool pointOnSurface( QgsPointV2& pt, QString* errorMsg = nullptr ) const override;
    QgsAbstractGeometryV2* convexHull( QString* errorMsg = nullptr ) const override;
    double distance( const QgsAbstractGeometryV2& geom, QString* errorMsg = nullptr ) const override;
    bool intersects( const QgsAbstractGeometryV2& geom, QString* errorMsg = nullptr ) const override;
    bool touches( const QgsAbstractGeometryV2& geom, QString* errorMsg = nullptr ) const override;
    bool crosses( const QgsAbstractGeometryV2& geom, QString* errorMsg = nullptr ) const override;
    bool within( const QgsAbstractGeometryV2& geom, QString* errorMsg = nullptr ) const override;
    bool overlaps( const QgsAbstractGeometryV2& geom, QString* errorMsg = nullptr ) const override;
    bool contains( const QgsAbstractGeometryV2& geom, QString* errorMsg = nullptr ) const override;
    bool disjoint( const QgsAbstractGeometryV2& geom, QString* errorMsg = nullptr ) const override;
    QString relate( const QgsAbstractGeometryV2& geom, QString* errorMsg = nullptr ) const override;
    bool relatePattern( const QgsAbstractGeometryV2& geom, const QString& pattern, QString* errorMsg = nullptr ) const override;
    double area( QString* errorMsg = nullptr ) const override;
    double length( QString* errorMsg = nullptr ) const override;
    bool isValid( QString* errorMsg = nullptr ) const override;
    bool isEqual( const QgsAbstractGeometryV2& geom, QString* errorMsg = nullptr ) const override;
    bool isEmpty( QString* errorMsg = nullptr ) const override;

    /** Splits this geometry according to a given line.
    @param splitLine the line that splits the geometry
    @param[out] newGeometries list of new geometries that have been created with the split
    @param topological true if topological editing is enabled
    @param[out] topologyTestPoints points that need to be tested for topological completeness in the dataset
    @param[out] errorMsg error messages emitted, if any
    @return 0 in case of success, 1 if geometry has not been split, error else*/
    int splitGeometry( const QgsLineStringV2& splitLine,
                       QList<QgsAbstractGeometryV2*>& newGeometries,
                       bool topological,
                       QgsPointSequenceV2 &topologyTestPoints,
                       QString* errorMsg = nullptr ) const override;

    QgsAbstractGeometryV2* offsetCurve( double distance, int segments, int joinStyle, double mitreLimit, QString* errorMsg = nullptr ) const override;
    QgsAbstractGeometryV2* reshapeGeometry( const QgsLineStringV2& reshapeWithLine, int* errorCode, QString* errorMsg = nullptr ) const;

    /** Returns the closest point on the geometry to the other geometry.
     * @note added in QGIS 2.14
     * @see shortestLine()
     */
    QgsGeometry closestPoint( const QgsGeometry& other, QString* errorMsg = nullptr ) const;

    /** Returns the shortest line joining this geometry to the other geometry.
     * @note added in QGIS 2.14
     * @see closestPoint()
     */
    QgsGeometry shortestLine( const QgsGeometry& other, QString* errorMsg = nullptr ) const;

    /** Create a geometry from a GEOSGeometry
     * @param geos GEOSGeometry. Ownership is NOT transferred.
     */
    static QgsAbstractGeometryV2* fromGeos( const GEOSGeometry* geos );
    static QgsPolygonV2* fromGeosPolygon( const GEOSGeometry* geos );
    static GEOSGeometry* asGeos( const QgsAbstractGeometryV2* geom , double precision = 0 );
    static QgsPointV2 coordSeqPoint( const GEOSCoordSequence* cs, int i, bool hasZ, bool hasM );

    static GEOSContextHandle_t getGEOSHandler();

  private:
    mutable GEOSGeometry* mGeos;
    const GEOSPreparedGeometry* mGeosPrepared;
    double mPrecision;

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
    QgsAbstractGeometryV2* overlay( const QgsAbstractGeometryV2& geom, Overlay op, QString* errorMsg = nullptr ) const;
    bool relation( const QgsAbstractGeometryV2& geom, Relation r, QString* errorMsg = nullptr ) const;
    static GEOSCoordSequence* createCoordinateSequence( const QgsCurveV2* curve , double precision, bool forceClose = false );
    static QgsLineStringV2* sequenceToLinestring( const GEOSGeometry* geos, bool hasZ, bool hasM );
    static int numberOfGeometries( GEOSGeometry* g );
    static GEOSGeometry* nodeGeometries( const GEOSGeometry *splitLine, const GEOSGeometry *geom );
    int mergeGeometriesMultiTypeSplit( QVector<GEOSGeometry*>& splitResult ) const;

    /** Ownership of geoms is transferred
     */
    static GEOSGeometry* createGeosCollection( int typeId, const QVector<GEOSGeometry*>& geoms );

    static GEOSGeometry* createGeosPoint( const QgsAbstractGeometryV2* point, int coordDims , double precision );
    static GEOSGeometry* createGeosLinestring( const QgsAbstractGeometryV2* curve, double precision );
    static GEOSGeometry* createGeosPolygon( const QgsAbstractGeometryV2* poly, double precision );

    //utils for geometry split
    int topologicalTestPointsSplit( const GEOSGeometry* splitLine, QgsPointSequenceV2 &testPoints, QString* errorMsg = nullptr ) const;
    GEOSGeometry* linePointDifference( GEOSGeometry* GEOSsplitPoint ) const;
    int splitLinearGeometry( GEOSGeometry* splitLine, QList<QgsAbstractGeometryV2*>& newGeometries ) const;
    int splitPolygonGeometry( GEOSGeometry* splitLine, QList<QgsAbstractGeometryV2*>& newGeometries ) const;

    //utils for reshape
    static GEOSGeometry* reshapeLine( const GEOSGeometry* line, const GEOSGeometry* reshapeLineGeos, double precision );
    static GEOSGeometry* reshapePolygon( const GEOSGeometry* polygon, const GEOSGeometry* reshapeLineGeos , double precision );
    static int lineContainedInLine( const GEOSGeometry* line1, const GEOSGeometry* line2 );
    static int pointContainedInLine( const GEOSGeometry* point, const GEOSGeometry* line );
    static int geomDigits( const GEOSGeometry* geom );
};

/// @cond PRIVATE

class GEOSException
{
  public:
    explicit GEOSException( const QString& theMsg )
    {
      if ( theMsg == "Unknown exception thrown" && lastMsg().isNull() )
      {
        msg = theMsg;
      }
      else
      {
        msg = theMsg;
        lastMsg() = msg;
      }
    }

    // copy constructor
    GEOSException( const GEOSException &rhs )
    {
      *this = rhs;
    }

    ~GEOSException()
    {
      if ( lastMsg() == msg )
        lastMsg() = QString::null;
    }

    QString what()
    {
      return msg;
    }

  private:
    QString msg;
    static QString& lastMsg() { static QString _lastMsg; return _lastMsg; }
};

/// @endcond

#endif // QGSGEOS_H

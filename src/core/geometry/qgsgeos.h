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
#include <geos_c.h>

class QgsLineString;
class QgsPolygonV2;
class QgsGeometry;

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
    QgsGeos( const QgsAbstractGeometry* geometry, double precision = 0 );
    ~QgsGeos();

    /** Removes caches*/
    void geometryChanged() override;
    void prepareGeometry() override;

    QgsAbstractGeometry* intersection( const QgsAbstractGeometry& geom, QString* errorMsg = nullptr ) const override;
    QgsAbstractGeometry* difference( const QgsAbstractGeometry& geom, QString* errorMsg = nullptr ) const override;
    QgsAbstractGeometry* combine( const QgsAbstractGeometry& geom, QString* errorMsg = nullptr ) const override;
    QgsAbstractGeometry* combine( const QList< QgsAbstractGeometry*>&, QString* errorMsg = nullptr ) const override;
    QgsAbstractGeometry* symDifference( const QgsAbstractGeometry& geom, QString* errorMsg = nullptr ) const override;
    QgsAbstractGeometry* buffer( double distance, int segments, QString* errorMsg = nullptr ) const override;
    QgsAbstractGeometry* buffer( double distance, int segments, int endCapStyle, int joinStyle, double mitreLimit, QString* errorMsg = nullptr ) const override;
    QgsAbstractGeometry* simplify( double tolerance, QString* errorMsg = nullptr ) const override;
    QgsAbstractGeometry* interpolate( double distance, QString* errorMsg = nullptr ) const override;
    QgsAbstractGeometry* envelope( QString* errorMsg = nullptr ) const override;
    bool centroid( QgsPointV2& pt, QString* errorMsg = nullptr ) const override;
    bool pointOnSurface( QgsPointV2& pt, QString* errorMsg = nullptr ) const override;
    QgsAbstractGeometry* convexHull( QString* errorMsg = nullptr ) const override;
    double distance( const QgsAbstractGeometry& geom, QString* errorMsg = nullptr ) const override;
    bool intersects( const QgsAbstractGeometry& geom, QString* errorMsg = nullptr ) const override;
    bool touches( const QgsAbstractGeometry& geom, QString* errorMsg = nullptr ) const override;
    bool crosses( const QgsAbstractGeometry& geom, QString* errorMsg = nullptr ) const override;
    bool within( const QgsAbstractGeometry& geom, QString* errorMsg = nullptr ) const override;
    bool overlaps( const QgsAbstractGeometry& geom, QString* errorMsg = nullptr ) const override;
    bool contains( const QgsAbstractGeometry& geom, QString* errorMsg = nullptr ) const override;
    bool disjoint( const QgsAbstractGeometry& geom, QString* errorMsg = nullptr ) const override;
    QString relate( const QgsAbstractGeometry& geom, QString* errorMsg = nullptr ) const override;
    bool relatePattern( const QgsAbstractGeometry& geom, const QString& pattern, QString* errorMsg = nullptr ) const override;
    double area( QString* errorMsg = nullptr ) const override;
    double length( QString* errorMsg = nullptr ) const override;
    bool isValid( QString* errorMsg = nullptr ) const override;
    bool isEqual( const QgsAbstractGeometry& geom, QString* errorMsg = nullptr ) const override;
    bool isEmpty( QString* errorMsg = nullptr ) const override;

    /** Splits this geometry according to a given line.
    @param splitLine the line that splits the geometry
    @param[out] newGeometries list of new geometries that have been created with the split
    @param topological true if topological editing is enabled
    @param[out] topologyTestPoints points that need to be tested for topological completeness in the dataset
    @param[out] errorMsg error messages emitted, if any
    @return 0 in case of success, 1 if geometry has not been split, error else*/
    int splitGeometry( const QgsLineString& splitLine,
                       QList<QgsAbstractGeometry*>& newGeometries,
                       bool topological,
                       QgsPointSequence &topologyTestPoints,
                       QString* errorMsg = nullptr ) const override;

    QgsAbstractGeometry* offsetCurve( double distance, int segments, int joinStyle, double mitreLimit, QString* errorMsg = nullptr ) const override;

    /**
     * Returns a single sided buffer for a geometry. The buffer is only
     * applied to one side of the geometry.
     * @param distance buffer distance
     * @param segments for round joins, number of segments to approximate quarter-circle
     * @param side side of geometry to buffer (0 = left, 1 = right)
     * @param joinStyle join style for corners ( Round (1) / Mitre (2) / Bevel (3) )
     * @param mitreLimit limit on the mitre ratio used for very sharp corners
     * @param errorMsg error messages emitted, if any
     * @return buffered geometry, or an nullptr if buffer could not be
     * calculated
     * @note added in QGIS 3.0
     */
    QgsAbstractGeometry* singleSidedBuffer( double distance, int segments, int side,
                                            int joinStyle, double mitreLimit,
                                            QString* errorMsg = nullptr ) const;


    QgsAbstractGeometry* reshapeGeometry( const QgsLineString& reshapeWithLine, int* errorCode, QString* errorMsg = nullptr ) const;

    /** Merges any connected lines in a LineString/MultiLineString geometry and
     * converts them to single line strings.
     * @param errorMsg if specified, will be set to any reported GEOS errors
     * @returns a LineString or MultiLineString geometry, with any connected lines
     * joined. An empty geometry will be returned if the input geometry was not a
     * LineString/MultiLineString geometry.
     * @note added in QGIS 3.0
     */
    QgsGeometry mergeLines( QString* errorMsg = nullptr ) const;

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
    static QgsAbstractGeometry* fromGeos( const GEOSGeometry* geos );
    static QgsPolygonV2* fromGeosPolygon( const GEOSGeometry* geos );
    static GEOSGeometry* asGeos( const QgsAbstractGeometry* geom , double precision = 0 );
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
    QgsAbstractGeometry* overlay( const QgsAbstractGeometry& geom, Overlay op, QString* errorMsg = nullptr ) const;
    bool relation( const QgsAbstractGeometry& geom, Relation r, QString* errorMsg = nullptr ) const;
    static GEOSCoordSequence* createCoordinateSequence( const QgsCurve* curve , double precision, bool forceClose = false );
    static QgsLineString* sequenceToLinestring( const GEOSGeometry* geos, bool hasZ, bool hasM );
    static int numberOfGeometries( GEOSGeometry* g );
    static GEOSGeometry* nodeGeometries( const GEOSGeometry *splitLine, const GEOSGeometry *geom );
    int mergeGeometriesMultiTypeSplit( QVector<GEOSGeometry*>& splitResult ) const;

    /** Ownership of geoms is transferred
     */
    static GEOSGeometry* createGeosCollection( int typeId, const QVector<GEOSGeometry*>& geoms );

    static GEOSGeometry* createGeosPoint( const QgsAbstractGeometry* point, int coordDims , double precision );
    static GEOSGeometry* createGeosLinestring( const QgsAbstractGeometry* curve, double precision );
    static GEOSGeometry* createGeosPolygon( const QgsAbstractGeometry* poly, double precision );

    //utils for geometry split
    int topologicalTestPointsSplit( const GEOSGeometry* splitLine, QgsPointSequence &testPoints, QString* errorMsg = nullptr ) const;
    GEOSGeometry* linePointDifference( GEOSGeometry* GEOSsplitPoint ) const;
    int splitLinearGeometry( GEOSGeometry* splitLine, QList<QgsAbstractGeometry*>& newGeometries ) const;
    int splitPolygonGeometry( GEOSGeometry* splitLine, QList<QgsAbstractGeometry*>& newGeometries ) const;

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

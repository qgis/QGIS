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

#define SIP_NO_FILE

#include "qgis_core.h"
#include "qgsgeometryengine.h"
#include "qgsgeometry.h"
#include <geos_c.h>

class QgsLineString;
class QgsPolygonV2;
class QgsGeometry;
class QgsGeometryCollection;

/** \ingroup core
 * Does vector analysis using the geos library and handles import, export, exception handling*
 * \note not available in Python bindings
 */
class CORE_EXPORT QgsGeos: public QgsGeometryEngine
{
  public:

    /** GEOS geometry engine constructor
     * \param geometry The geometry
     * \param precision The precision of the grid to which to snap the geometry vertices. If 0, no snapping is performed.
     */
    QgsGeos( const QgsAbstractGeometry *geometry, double precision = 0 );
    ~QgsGeos();

    //! Removes caches
    void geometryChanged() override;
    void prepareGeometry() override;

    QgsAbstractGeometry *intersection( const QgsAbstractGeometry *geom, QString *errorMsg = nullptr ) const override;
    QgsAbstractGeometry *difference( const QgsAbstractGeometry *geom, QString *errorMsg = nullptr ) const override;

    /**
     * Performs a fast, non-robust intersection between the geometry and
     * a \a rectangle. The returned geometry may be invalid.
     */
    QgsAbstractGeometry *clip( const QgsRectangle &rectangle, QString *errorMsg = nullptr ) const;

    /**
     * Subdivides the geometry. The returned geometry will be a collection containing subdivided parts
     * from the original geometry, where no part has more then the specified maximum number of nodes (\a maxNodes).
     *
     * This is useful for dividing a complex geometry into less complex parts, which are better able to be spatially
     * indexed and faster to perform further operations such as intersects on. The returned geometry parts may
     * not be valid and may contain self-intersections.
     *
     * The minimum allowed value for \a maxNodes is 8.
     *
     * Curved geometries are not supported.
     *
     * \since QGIS 3.0
     */
    QgsAbstractGeometry *subdivide( int maxNodes, QString *errorMsg = nullptr ) const;

    QgsAbstractGeometry *combine( const QgsAbstractGeometry *geom, QString *errorMsg = nullptr ) const override;
    QgsAbstractGeometry *combine( const QList< QgsAbstractGeometry *> &, QString *errorMsg = nullptr ) const override;
    QgsAbstractGeometry *symDifference( const QgsAbstractGeometry *geom, QString *errorMsg = nullptr ) const override;
    QgsAbstractGeometry *buffer( double distance, int segments, QString *errorMsg = nullptr ) const override;
    QgsAbstractGeometry *buffer( double distance, int segments, int endCapStyle, int joinStyle, double miterLimit, QString *errorMsg = nullptr ) const override;
    QgsAbstractGeometry *simplify( double tolerance, QString *errorMsg = nullptr ) const override;
    QgsAbstractGeometry *interpolate( double distance, QString *errorMsg = nullptr ) const override;
    QgsAbstractGeometry *envelope( QString *errorMsg = nullptr ) const override;
    QgsPoint *centroid( QString *errorMsg = nullptr ) const override;
    QgsPoint *pointOnSurface( QString *errorMsg = nullptr ) const override;
    QgsAbstractGeometry *convexHull( QString *errorMsg = nullptr ) const override;
    double distance( const QgsAbstractGeometry *geom, QString *errorMsg = nullptr ) const override;
    bool intersects( const QgsAbstractGeometry *geom, QString *errorMsg = nullptr ) const override;
    bool touches( const QgsAbstractGeometry *geom, QString *errorMsg = nullptr ) const override;
    bool crosses( const QgsAbstractGeometry *geom, QString *errorMsg = nullptr ) const override;
    bool within( const QgsAbstractGeometry *geom, QString *errorMsg = nullptr ) const override;
    bool overlaps( const QgsAbstractGeometry *geom, QString *errorMsg = nullptr ) const override;
    bool contains( const QgsAbstractGeometry *geom, QString *errorMsg = nullptr ) const override;
    bool disjoint( const QgsAbstractGeometry *geom, QString *errorMsg = nullptr ) const override;
    QString relate( const QgsAbstractGeometry *geom, QString *errorMsg = nullptr ) const override;
    bool relatePattern( const QgsAbstractGeometry *geom, const QString &pattern, QString *errorMsg = nullptr ) const override;
    double area( QString *errorMsg = nullptr ) const override;
    double length( QString *errorMsg = nullptr ) const override;
    bool isValid( QString *errorMsg = nullptr ) const override;
    bool isEqual( const QgsAbstractGeometry *geom, QString *errorMsg = nullptr ) const override;
    bool isEmpty( QString *errorMsg = nullptr ) const override;
    bool isSimple( QString *errorMsg = nullptr ) const override;

    /** Splits this geometry according to a given line.
    \param splitLine the line that splits the geometry
    \param[out] newGeometries list of new geometries that have been created with the split
    \param topological true if topological editing is enabled
    \param[out] topologyTestPoints points that need to be tested for topological completeness in the dataset
    \param[out] errorMsg error messages emitted, if any
    \returns 0 in case of success, 1 if geometry has not been split, error else*/
    EngineOperationResult splitGeometry( const QgsLineString &splitLine,
                                         QList<QgsAbstractGeometry *> &newGeometries,
                                         bool topological,
                                         QgsPointSequence &topologyTestPoints,
                                         QString *errorMsg = nullptr ) const override;

    QgsAbstractGeometry *offsetCurve( double distance, int segments, int joinStyle, double miterLimit, QString *errorMsg = nullptr ) const override;

    /**
     * Returns a single sided buffer for a geometry. The buffer is only
     * applied to one side of the geometry.
     * \param distance buffer distance
     * \param segments for round joins, number of segments to approximate quarter-circle
     * \param side side of geometry to buffer (0 = left, 1 = right)
     * \param joinStyle join style for corners ( Round (1) / Miter (2) / Bevel (3) )
     * \param miterLimit limit on the miter ratio used for very sharp corners
     * \param errorMsg error messages emitted, if any
     * \returns buffered geometry, or an nullptr if buffer could not be
     * calculated
     * \since QGIS 3.0
     */
    QgsAbstractGeometry *singleSidedBuffer( double distance, int segments, int side,
                                            int joinStyle, double miterLimit,
                                            QString *errorMsg = nullptr ) const;

    /**
     * Reshapes the geometry using a line
     * @param reshapeWithLine the line used to reshape lines or polygons
     * @param errorCode if specified, provides result of operation (success or reason of failure)
     * @param errorMsg if specified, provides more details about failure
     * @return the reshaped geometry
     */
    QgsAbstractGeometry *reshapeGeometry( const QgsLineString &reshapeWithLine, EngineOperationResult *errorCode, QString *errorMsg = nullptr ) const;

    /** Merges any connected lines in a LineString/MultiLineString geometry and
     * converts them to single line strings.
     * \param errorMsg if specified, will be set to any reported GEOS errors
     * \returns a LineString or MultiLineString geometry, with any connected lines
     * joined. An empty geometry will be returned if the input geometry was not a
     * LineString/MultiLineString geometry.
     * \since QGIS 3.0
     */
    QgsGeometry mergeLines( QString *errorMsg = nullptr ) const;

    /** Returns the closest point on the geometry to the other geometry.
     * \since QGIS 2.14
     * \see shortestLine()
     */
    QgsGeometry closestPoint( const QgsGeometry &other, QString *errorMsg = nullptr ) const;

    /** Returns the shortest line joining this geometry to the other geometry.
     * \since QGIS 2.14
     * \see closestPoint()
     */
    QgsGeometry shortestLine( const QgsGeometry &other, QString *errorMsg = nullptr ) const;

    /** Returns a distance representing the location along this linestring of the closest point
     * on this linestring geometry to the specified point. Ie, the returned value indicates
     * how far along this linestring you need to traverse to get to the closest location
     * where this linestring comes to the specified point.
     * \param point point to seek proximity to
     * \param errorMsg error messages emitted, if any
     * \note only valid for linestring geometries
     * \returns distance along line, or -1 on error
     */
    double lineLocatePoint( const QgsPoint &point, QString *errorMsg = nullptr ) const;

    /**
     * Creates a GeometryCollection geometry containing possible polygons formed from the constituent
     * linework of a set of \a geometries. The input geometries must be fully noded (i.e. nodes exist
     * at every common intersection of the geometries). The easiest way to ensure this is to first
     * unary union these geometries by calling combine() on the set of input geometries and then
     * pass the result to polygonize().
     * An empty geometry will be returned in the case of errors.
     * \since QGIS 3.0
     */
    static QgsGeometry polygonize( const QList<QgsAbstractGeometry *> &geometries, QString *errorMsg = nullptr );

    /**
     * Creates a Voronoi diagram for the nodes contained within the geometry.
     *
     * Returns the Voronoi polygons for the nodes contained within the geometry.
     * If \a extent is specified then it will be used as a clipping envelope for the diagram.
     * If no extent is set then the clipping envelope will be automatically calculated.
     * In either case the diagram will be clipped to the larger of the provided envelope
     * OR the envelope surrounding all input nodes.
     * The \a tolerance parameter specifies an optional snapping tolerance which can
     * be used to improve the robustness of the diagram calculation.
     * If \a edgesOnly is true than line string boundary geometries will be returned
     * instead of polygons.
     * An empty geometry will be returned if the diagram could not be calculated.
     * \since QGIS 3.0
     */
    QgsGeometry voronoiDiagram( const QgsAbstractGeometry *extent = nullptr, double tolerance = 0.0, bool edgesOnly = false, QString *errorMsg = nullptr ) const;

    /**
     * Returns the Delaunay triangulation for the vertices of the geometry.
     * The \a tolerance parameter specifies an optional snapping tolerance which can
     * be used to improve the robustness of the triangulation.
     * If \a edgesOnly is true than line string boundary geometries will be returned
     * instead of polygons.
     * An empty geometry will be returned if the diagram could not be calculated.
     * \since QGIS 3.0
     */
    QgsGeometry delaunayTriangulation( double tolerance = 0.0, bool edgesOnly = false, QString *errorMsg = nullptr ) const;

    /** Create a geometry from a GEOSGeometry
     * \param geos GEOSGeometry. Ownership is NOT transferred.
     */
    static QgsAbstractGeometry *fromGeos( const GEOSGeometry *geos );
    static QgsPolygonV2 *fromGeosPolygon( const GEOSGeometry *geos );
    static GEOSGeometry *asGeos( const QgsAbstractGeometry *geom, double precision = 0 );
    static QgsPoint coordSeqPoint( const GEOSCoordSequence *cs, int i, bool hasZ, bool hasM );

    static GEOSContextHandle_t getGEOSHandler();


  private:
    mutable GEOSGeometry *mGeos;
    const GEOSPreparedGeometry *mGeosPrepared = nullptr;
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
    QgsAbstractGeometry *overlay( const QgsAbstractGeometry *geom, Overlay op, QString *errorMsg = nullptr ) const;
    bool relation( const QgsAbstractGeometry *geom, Relation r, QString *errorMsg = nullptr ) const;
    static GEOSCoordSequence *createCoordinateSequence( const QgsCurve *curve, double precision, bool forceClose = false );
    static QgsLineString *sequenceToLinestring( const GEOSGeometry *geos, bool hasZ, bool hasM );
    static int numberOfGeometries( GEOSGeometry *g );
    static GEOSGeometry *nodeGeometries( const GEOSGeometry *splitLine, const GEOSGeometry *geom );
    int mergeGeometriesMultiTypeSplit( QVector<GEOSGeometry *> &splitResult ) const;

    /** Ownership of geoms is transferred
     */
    static GEOSGeometry *createGeosCollection( int typeId, const QVector<GEOSGeometry *> &geoms );

    static GEOSGeometry *createGeosPointXY( double x, double y, bool hasZ, double z, bool hasM, double m, int coordDims, double precision );
    static GEOSGeometry *createGeosPoint( const QgsAbstractGeometry *point, int coordDims, double precision );
    static GEOSGeometry *createGeosLinestring( const QgsAbstractGeometry *curve, double precision );
    static GEOSGeometry *createGeosPolygon( const QgsAbstractGeometry *poly, double precision );

    //utils for geometry split
    bool topologicalTestPointsSplit( const GEOSGeometry *splitLine, QgsPointSequence &testPoints, QString *errorMsg = nullptr ) const;
    GEOSGeometry *linePointDifference( GEOSGeometry *GEOSsplitPoint ) const;
    EngineOperationResult splitLinearGeometry( GEOSGeometry *splitLine, QList<QgsAbstractGeometry *> &newGeometries ) const;
    EngineOperationResult splitPolygonGeometry( GEOSGeometry *splitLine, QList<QgsAbstractGeometry *> &newGeometries ) const;

    //utils for reshape
    static GEOSGeometry *reshapeLine( const GEOSGeometry *line, const GEOSGeometry *reshapeLineGeos, double precision );
    static GEOSGeometry *reshapePolygon( const GEOSGeometry *polygon, const GEOSGeometry *reshapeLineGeos, double precision );
    static int lineContainedInLine( const GEOSGeometry *line1, const GEOSGeometry *line2 );
    static int pointContainedInLine( const GEOSGeometry *point, const GEOSGeometry *line );
    static int geomDigits( const GEOSGeometry *geom );
    void subdivideRecursive( const GEOSGeometry *currentPart, int maxNodes, int depth, QgsGeometryCollection *parts, const QgsRectangle &clipRect ) const;
};

/// @cond PRIVATE


class GEOSException : public std::runtime_error
{
  public:
    explicit GEOSException( const QString &message )
      : std::runtime_error( message.toUtf8().constData() )
    {
    }
};

/// @endcond

#endif // QGSGEOS_H

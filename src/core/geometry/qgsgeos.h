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

#if defined(GEOS_VERSION_MAJOR) && (GEOS_VERSION_MAJOR<3)
#define GEOSGeometry struct GEOSGeom_t
#define GEOSCoordSequence struct GEOSCoordSeq_t
#endif

class QgsLineString;
class QgsPolygon;
class QgsGeometry;
class QgsGeometryCollection;

/**
 * Contains geos related utilities and functions.
 * \note not available in Python bindings.
 * \since QGIS 3.0
 */
namespace geos
{

  /**
   * Destroys the GEOS geometry \a geom, using the static QGIS
   * geos context.
   */
  struct GeosDeleter
  {

    /**
     * Destroys the GEOS geometry \a geom, using the static QGIS
     * geos context.
     */
    void CORE_EXPORT operator()( GEOSGeometry *geom );

    /**
     * Destroys the GEOS prepared geometry \a geom, using the static QGIS
     * geos context.
     */
    void CORE_EXPORT operator()( const GEOSPreparedGeometry *geom );

    /**
     * Destroys the GEOS buffer params \a params, using the static QGIS
     * geos context.
     */
    void CORE_EXPORT operator()( GEOSBufferParams *params );

    /**
     * Destroys the GEOS coordinate sequence \a sequence, using the static QGIS
     * geos context.
     */
    void CORE_EXPORT operator()( GEOSCoordSequence *sequence );
  };

  /**
   * Scoped GEOS pointer.
   */
  using unique_ptr = std::unique_ptr< GEOSGeometry, GeosDeleter>;

  /**
   * Scoped GEOS prepared geometry pointer.
   */
  using prepared_unique_ptr = std::unique_ptr< const GEOSPreparedGeometry, GeosDeleter>;

  /**
   * Scoped GEOS buffer params pointer.
   */
  using buffer_params_unique_ptr = std::unique_ptr< GEOSBufferParams, GeosDeleter>;

  /**
   * Scoped GEOS coordinate sequence pointer.
   */
  using coord_sequence_unique_ptr = std::unique_ptr< GEOSCoordSequence, GeosDeleter>;

}

/**
 * \ingroup core
 * Does vector analysis using the geos library and handles import, export, exception handling*
 * \note not available in Python bindings
 */
class CORE_EXPORT QgsGeos: public QgsGeometryEngine
{
  public:

    /**
     * GEOS geometry engine constructor
     * \param geometry The geometry
     * \param precision The precision of the grid to which to snap the geometry vertices. If 0, no snapping is performed.
     */
    QgsGeos( const QgsAbstractGeometry *geometry, double precision = 0 );

    /**
     * Creates a new QgsGeometry object, feeding in a geometry in GEOS format.
     * This class will take ownership of the buffer.
     */
    static QgsGeometry geometryFromGeos( GEOSGeometry *geos );

    /**
     * Creates a new QgsGeometry object, feeding in a geometry in GEOS format.
     */
    static QgsGeometry geometryFromGeos( const geos::unique_ptr &geos );

    /**
     * Adds a new island polygon to a multipolygon feature
     * \param geometry geometry to add part to
     * \param newPart part to add. Ownership is NOT transferred.
     * \returns OperationResult a result code: success or reason of failure
     */
    static QgsGeometry::OperationResult addPart( QgsGeometry &geometry, GEOSGeometry *newPart );

    void geometryChanged() override;
    void prepareGeometry() override;

    QgsAbstractGeometry *intersection( const QgsAbstractGeometry *geom, QString *errorMsg = nullptr ) const override;
    QgsAbstractGeometry *difference( const QgsAbstractGeometry *geom, QString *errorMsg = nullptr ) const override;

    /**
     * Performs a fast, non-robust intersection between the geometry and
     * a \a rectangle. The returned geometry may be invalid.
     */
    std::unique_ptr< QgsAbstractGeometry > clip( const QgsRectangle &rectangle, QString *errorMsg = nullptr ) const;

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
    std::unique_ptr< QgsAbstractGeometry > subdivide( int maxNodes, QString *errorMsg = nullptr ) const;

    QgsAbstractGeometry *combine( const QgsAbstractGeometry *geom, QString *errorMsg = nullptr ) const override;
    QgsAbstractGeometry *combine( const QVector<QgsAbstractGeometry *> &geomList, QString *errorMsg ) const override;
    QgsAbstractGeometry *combine( const QVector< QgsGeometry > &, QString *errorMsg = nullptr ) const override;
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

    /**
     * Returns the Hausdorff distance between this geometry and \a geom. This is basically a measure of how similar or dissimilar 2 geometries are.
     *
     * This algorithm is an approximation to the standard Hausdorff distance. This approximation is exact or close enough for a large
     * subset of useful cases. Examples of these are:
     *
     * - computing distance between Linestrings that are roughly parallel to each other,
     * and roughly equal in length. This occurs in matching linear networks.
     * - Testing similarity of geometries.
     *
     * If the default approximate provided by this method is insufficient, use hausdorffDistanceDensify() instead.
     *
     * \see hausdorffDistanceDensify()
     * \since QGIS 3.0
     */
    double hausdorffDistance( const QgsAbstractGeometry *geom, QString *errorMsg = nullptr ) const;

    /**
     * Returns the Hausdorff distance between this geometry and \a geom. This is basically a measure of how similar or dissimilar 2 geometries are.
     *
     * This function accepts a \a densifyFraction argument. The function performs a segment
     * densification before computing the discrete Hausdorff distance. The \a densifyFraction parameter
     * sets the fraction by which to densify each segment. Each segment will be split into a
     * number of equal-length subsegments, whose fraction of the total length is
     * closest to the given fraction.
     *
     * This method can be used when the default approximation provided by hausdorffDistance()
     * is not sufficient. Decreasing the \a densifyFraction parameter will make the
     * distance returned approach the true Hausdorff distance for the geometries.
     *
     * \see hausdorffDistance()
     * \since QGIS 3.0
     */
    double hausdorffDistanceDensify( const QgsAbstractGeometry *geom, double densifyFraction, QString *errorMsg = nullptr ) const;

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
    bool isValid( QString *errorMsg = nullptr, bool allowSelfTouchingHoles = false, QgsGeometry *errorLoc = nullptr ) const override;
    bool isEqual( const QgsAbstractGeometry *geom, QString *errorMsg = nullptr ) const override;
    bool isEmpty( QString *errorMsg = nullptr ) const override;
    bool isSimple( QString *errorMsg = nullptr ) const override;

    EngineOperationResult splitGeometry( const QgsLineString &splitLine,
                                         QVector<QgsGeometry> &newGeometries,
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
     * \returns buffered geometry, or an NULLPTR if buffer could not be
     * calculated
     * \since QGIS 3.0
     */
    std::unique_ptr< QgsAbstractGeometry > singleSidedBuffer( double distance, int segments, int side,
        int joinStyle, double miterLimit,
        QString *errorMsg = nullptr ) const;

    /**
     * Reshapes the geometry using a line
     * \param reshapeWithLine the line used to reshape lines or polygons
     * \param errorCode if specified, provides result of operation (success or reason of failure)
     * \param errorMsg if specified, provides more details about failure
     * \return the reshaped geometry
     */
    std::unique_ptr< QgsAbstractGeometry > reshapeGeometry( const QgsLineString &reshapeWithLine, EngineOperationResult *errorCode, QString *errorMsg = nullptr ) const;

    /**
     * Merges any connected lines in a LineString/MultiLineString geometry and
     * converts them to single line strings.
     * \param errorMsg if specified, will be set to any reported GEOS errors
     * \returns a LineString or MultiLineString geometry, with any connected lines
     * joined. An empty geometry will be returned if the input geometry was not a
     * LineString/MultiLineString geometry.
     * \since QGIS 3.0
     */
    QgsGeometry mergeLines( QString *errorMsg = nullptr ) const;

    /**
     * Returns the closest point on the geometry to the other geometry.
     * \see shortestLine()
     * \since QGIS 2.14
     */
    QgsGeometry closestPoint( const QgsGeometry &other, QString *errorMsg = nullptr ) const;

    /**
     * Returns the shortest line joining this geometry to the other geometry.
     * \see closestPoint()
     * \since QGIS 2.14
     */
    QgsGeometry shortestLine( const QgsGeometry &other, QString *errorMsg = nullptr ) const;

    /**
     * Returns a distance representing the location along this linestring of the closest point
     * on this linestring geometry to the specified point. Ie, the returned value indicates
     * how far along this linestring you need to traverse to get to the closest location
     * where this linestring comes to the specified point.
     * \param point point to seek proximity to
     * \param errorMsg error messages emitted, if any
     * \returns distance along line, or -1 on error
     * \note only valid for linestring geometries
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
    static QgsGeometry polygonize( const QVector<const QgsAbstractGeometry *> &geometries, QString *errorMsg = nullptr );

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
     * If \a edgesOnly is TRUE than line string boundary geometries will be returned
     * instead of polygons.
     * An empty geometry will be returned if the diagram could not be calculated.
     * \since QGIS 3.0
     */
    QgsGeometry voronoiDiagram( const QgsAbstractGeometry *extent = nullptr, double tolerance = 0.0, bool edgesOnly = false, QString *errorMsg = nullptr ) const;

    /**
     * Returns the Delaunay triangulation for the vertices of the geometry.
     * The \a tolerance parameter specifies an optional snapping tolerance which can
     * be used to improve the robustness of the triangulation.
     * If \a edgesOnly is TRUE than line string boundary geometries will be returned
     * instead of polygons.
     * An empty geometry will be returned if the diagram could not be calculated.
     * \since QGIS 3.0
     */
    QgsGeometry delaunayTriangulation( double tolerance = 0.0, bool edgesOnly = false, QString *errorMsg = nullptr ) const;

    /**
     * Create a geometry from a GEOSGeometry
     * \param geos GEOSGeometry. Ownership is NOT transferred.
     */
    static std::unique_ptr< QgsAbstractGeometry > fromGeos( const GEOSGeometry *geos );
    static std::unique_ptr< QgsPolygon > fromGeosPolygon( const GEOSGeometry *geos );


    /**
     * Returns a geos geometry - caller takes ownership of the object (should be deleted with GEOSGeom_destroy_r)
     * \param geometry geometry to convert to GEOS representation
     * \param precision The precision of the grid to which to snap the geometry vertices. If 0, no snapping is performed.
     */
    static geos::unique_ptr asGeos( const QgsGeometry &geometry, double precision = 0 );

    /**
     * Returns a geos geometry - caller takes ownership of the object (should be deleted with GEOSGeom_destroy_r)
     * \param geometry geometry to convert to GEOS representation
     * \param precision The precision of the grid to which to snap the geometry vertices. If 0, no snapping is performed.
     */
    static geos::unique_ptr asGeos( const QgsAbstractGeometry *geometry, double precision = 0 );
    static QgsPoint coordSeqPoint( const GEOSCoordSequence *cs, int i, bool hasZ, bool hasM );

    static GEOSContextHandle_t getGEOSHandler();


  private:
    mutable geos::unique_ptr mGeos;
    geos::prepared_unique_ptr mGeosPrepared;
    double mPrecision = 0.0;

    enum Overlay
    {
      OverlayIntersection,
      OverlayDifference,
      OverlayUnion,
      OverlaySymDifference
    };

    enum Relation
    {
      RelationIntersects,
      RelationTouches,
      RelationCrosses,
      RelationWithin,
      RelationOverlaps,
      RelationContains,
      RelationDisjoint
    };

    //geos util functions
    void cacheGeos() const;
    std::unique_ptr< QgsAbstractGeometry > overlay( const QgsAbstractGeometry *geom, Overlay op, QString *errorMsg = nullptr ) const;
    bool relation( const QgsAbstractGeometry *geom, Relation r, QString *errorMsg = nullptr ) const;
    static GEOSCoordSequence *createCoordinateSequence( const QgsCurve *curve, double precision, bool forceClose = false );
    static std::unique_ptr< QgsLineString > sequenceToLinestring( const GEOSGeometry *geos, bool hasZ, bool hasM );
    static int numberOfGeometries( GEOSGeometry *g );
    static geos::unique_ptr nodeGeometries( const GEOSGeometry *splitLine, const GEOSGeometry *geom );
    int mergeGeometriesMultiTypeSplit( QVector<GEOSGeometry *> &splitResult ) const;

    /**
     * Ownership of geoms is transferred
     */
    static geos::unique_ptr createGeosCollection( int typeId, const QVector<GEOSGeometry *> &geoms );

    static geos::unique_ptr createGeosPointXY( double x, double y, bool hasZ, double z, bool hasM, double m, int coordDims, double precision );
    static geos::unique_ptr createGeosPoint( const QgsAbstractGeometry *point, int coordDims, double precision );
    static geos::unique_ptr createGeosLinestring( const QgsAbstractGeometry *curve, double precision );
    static geos::unique_ptr createGeosPolygon( const QgsAbstractGeometry *poly, double precision );

    //utils for geometry split
    bool topologicalTestPointsSplit( const GEOSGeometry *splitLine, QgsPointSequence &testPoints, QString *errorMsg = nullptr ) const;
    geos::unique_ptr linePointDifference( GEOSGeometry *GEOSsplitPoint ) const;
    EngineOperationResult splitLinearGeometry( GEOSGeometry *splitLine, QVector<QgsGeometry > &newGeometries ) const;
    EngineOperationResult splitPolygonGeometry( GEOSGeometry *splitLine, QVector<QgsGeometry > &newGeometries ) const;

    //utils for reshape
    static geos::unique_ptr reshapeLine( const GEOSGeometry *line, const GEOSGeometry *reshapeLineGeos, double precision );
    static geos::unique_ptr reshapePolygon( const GEOSGeometry *polygon, const GEOSGeometry *reshapeLineGeos, double precision );
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

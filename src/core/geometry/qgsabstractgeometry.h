/***************************************************************************
                        qgsabstractgeometry.h
  -------------------------------------------------------------------
Date                 : 04 Sept 2014
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

#ifndef QGSABSTRACTGEOMETRYV2
#define QGSABSTRACTGEOMETRYV2

#include <array>
#include <functional>
#include <type_traits>
#include <QString>

#include "qgis_core.h"
#include "qgis.h"
#include "qgswkbtypes.h"
#include "qgswkbptr.h"

#ifndef SIP_RUN
#include "json_fwd.hpp"
using namespace nlohmann;
#endif

class QgsMapToPixel;
class QgsCurve;
class QgsMultiCurve;
class QgsMultiPoint;

struct QgsVertexId;
class QgsVertexIterator;
class QPainter;
class QDomDocument;
class QDomElement;
class QgsGeometryPartIterator;
class QgsGeometryConstPartIterator;
class QgsConstWkbPtr;
class QPainterPath;
class QgsAbstractGeometryTransformer;
class QgsFeedback;
class QgsCoordinateTransform;
class QgsPoint;
class QgsRectangle;
class QgsBox3D;

typedef QVector< QgsPoint > QgsPointSequence;
#ifndef SIP_RUN
typedef QVector< QgsPointSequence > QgsRingSequence;
typedef QVector< QgsRingSequence > QgsCoordinateSequence;
#else
typedef QVector< QVector< QgsPoint > > QgsRingSequence;
typedef QVector< QVector< QVector< QgsPoint > > > QgsCoordinateSequence;
#endif


/**
 * \ingroup core
 * \class QgsAbstractGeometry
 * \brief Abstract base class for all geometries
 *
 * \note QgsAbstractGeometry objects are inherently Cartesian/planar geometries. They have no concept of geodesy, and none
 * of the methods or properties exposed from the QgsAbstractGeometry API (or QgsGeometry API) utilize
 * geodesic calculations. Accordingly, properties like length() and area() and spatial operations like centroid()
 * are always calculated using strictly Cartesian mathematics. In contrast, the QgsDistanceArea class exposes
 * methods for working with geodesic calculations and spatial operations on geometries,
 * and should be used whenever calculations which account for the curvature of the Earth (or any other celestial body)
 * are required.
 *
 */
class CORE_EXPORT QgsAbstractGeometry
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( qgsgeometry_cast<QgsPoint *>( sipCpp ) != nullptr )
      sipType = sipType_QgsPoint;
    else if ( qgsgeometry_cast<QgsLineString *>( sipCpp ) != nullptr )
      sipType = sipType_QgsLineString;
    else if ( qgsgeometry_cast<QgsCircularString *>( sipCpp ) != nullptr )
      sipType = sipType_QgsCircularString;
    else if ( qgsgeometry_cast<QgsCompoundCurve *>( sipCpp ) != nullptr )
      sipType = sipType_QgsCompoundCurve;
    else if ( qgsgeometry_cast<QgsTriangle *>( sipCpp ) != nullptr )
      sipType = sipType_QgsTriangle;
    else if ( qgsgeometry_cast<QgsPolygon *>( sipCpp ) != nullptr )
      sipType = sipType_QgsPolygon;
    else if ( qgsgeometry_cast<QgsCurvePolygon *>( sipCpp ) != nullptr )
      sipType = sipType_QgsCurvePolygon;
    else if ( qgsgeometry_cast<QgsMultiPoint *>( sipCpp ) != nullptr )
      sipType = sipType_QgsMultiPoint;
    else if ( qgsgeometry_cast<QgsMultiLineString *>( sipCpp ) != nullptr )
      sipType = sipType_QgsMultiLineString;
    else if ( qgsgeometry_cast<QgsMultiPolygon *>( sipCpp ) != nullptr )
      sipType = sipType_QgsMultiPolygon;
    else if ( qgsgeometry_cast<QgsMultiSurface *>( sipCpp ) != nullptr )
      sipType = sipType_QgsMultiSurface;
    else if ( qgsgeometry_cast<QgsMultiCurve *>( sipCpp ) != nullptr )
      sipType = sipType_QgsMultiCurve;
    else if ( qgsgeometry_cast<QgsGeometryCollection *>( sipCpp ) != nullptr )
      sipType = sipType_QgsGeometryCollection;
    else
      sipType = 0;
    SIP_END
#endif

    Q_GADGET

  public:

    //! Segmentation tolerance as maximum angle or maximum difference between approximation and circle
    enum SegmentationToleranceType
    {

      /**
       * Maximum angle between generating radii (lines from arc center
       * to output vertices)
      */
      MaximumAngle = 0,

      /**
       * Maximum distance between an arbitrary point on the original
       * curve and closest point on its approximation.
      */
      MaximumDifference
    };
    Q_ENUM( SegmentationToleranceType )

    //! Axis order for GML generation
    enum AxisOrder
    {

      /**
       * X comes before Y (or lon before lat)
       */
      XY = 0,

      /**
       * Y comes before X (or lat before lon)
       */
      YX
    };
    Q_ENUM( QgsAbstractGeometry::AxisOrder )

    /**
     * Constructor for QgsAbstractGeometry.
     */
    QgsAbstractGeometry() = default;
    virtual ~QgsAbstractGeometry() = default;
    QgsAbstractGeometry( const QgsAbstractGeometry &geom );
    QgsAbstractGeometry &operator=( const QgsAbstractGeometry &geom );

    virtual bool operator==( const QgsAbstractGeometry &other ) const = 0;
    virtual bool operator!=( const QgsAbstractGeometry &other ) const = 0;

    /**
     * Performs fuzzy comparison between this geometry and \a other using an \a epsilon.
     *
     * The comparison is done by examining the specific values (such as x and y) that define the location of vertices in the geometry.
     *
     * \see fuzzyDistanceEqual
     * \see QgsGeometryUtilsBase::fuzzyDistanceEqual
     *
     * \since QGIS 3.36
     */
    virtual bool fuzzyEqual( const QgsAbstractGeometry &other, double epsilon = 1e-8 ) const = 0;

    /**
     * Performs fuzzy distance comparison between this geometry and \a other using an \a epsilon.
     *
     * Traditionally, the comparison is done by examining the specific values (such as x and y) that define the location of vertices in the geometry.
     * It focuses on the numerical differences or relationships between these values.
     * On the other hand, comparing distances between points considers the actual spatial separation or length between the points, regardless of their coordinate values.
     * This comparison involves measuring the distance between two points using formulas like the distance formula. Here, it's the "distance comparison" (fuzzyDistanceEqual).
     *
     * \see fuzzyEqual
     * \see QgsGeometryUtilsBase::fuzzyEqual
     *
     * \since QGIS 3.36
     */
    virtual bool fuzzyDistanceEqual( const QgsAbstractGeometry &other, double epsilon = 1e-8 ) const = 0;

    /**
     * Clones the geometry by performing a deep copy
     */
    virtual QgsAbstractGeometry *clone() const = 0 SIP_FACTORY;

    /**
     * Comparator for sorting of geometry.
     *
     * \since QGIS 3.20
     */
    virtual int compareTo( const QgsAbstractGeometry *other ) const;

    /**
     * Clears the geometry, ie reset it to a null geometry
     */
    virtual void clear() = 0;

    /**
     * Returns the minimal bounding box for the geometry
     */
    virtual QgsRectangle boundingBox() const;

    /**
     * Returns the 3D bounding box for the geometry.
     *
     * \since QGIS 3.34
     */
    virtual QgsBox3D boundingBox3D() const = 0;

    //mm-sql interface

    /**
     * Returns the inherent dimension of the geometry. For example, this is 0 for a point geometry,
     * 1 for a linestring and 2 for a polygon.
     */
    virtual int dimension() const = 0;

    /**
     * Returns a unique string representing the geometry type.
     * \see wkbType
     * \see wktTypeStr
     */
    virtual QString geometryType() const = 0;

    /**
     * Returns the WKB type of the geometry.
     * \see geometryType
     * \see wktTypeStr
     */
    inline Qgis::WkbType wkbType() const SIP_HOLDGIL { return mWkbType; }

    /**
     * Returns the WKT type string of the geometry.
     * \see geometryType
     * \see wkbType
     */
    QString wktTypeStr() const;

    /**
     * Returns TRUE if the geometry is 3D and contains a z-value.
     * \see isMeasure
     */
    bool is3D() const SIP_HOLDGIL
    {
      return QgsWkbTypes::hasZ( mWkbType );
    }

    /**
     * Returns TRUE if the geometry contains m values.
     * \see is3D
     */
    bool isMeasure() const SIP_HOLDGIL
    {
      return QgsWkbTypes::hasM( mWkbType );
    }

    /**
     * Returns the closure of the combinatorial boundary of the geometry (ie the topological boundary of the geometry).
     * For instance, a polygon geometry will have a boundary consisting of the linestrings for each ring in the polygon.
     * \returns boundary for geometry. May be NULLPTR for some geometry types.
     */
    virtual QgsAbstractGeometry *boundary() const = 0 SIP_FACTORY;

    /**
     * Reorganizes the geometry into a normalized form (or "canonical" form).
     *
     * Polygon rings will be rearranged so that their starting vertex is the lower left and ring orientation follows the
     * right hand rule, collections are ordered by geometry type, and other normalization techniques are applied. The
     * resultant geometry will be geometrically equivalent to the original geometry.
     *
     * \since QGIS 3.20
     */
    virtual void normalize() = 0;

    //import

    /**
     * Sets the geometry from a WKB string.
     * After successful read the wkb argument will be at the position where the reading has stopped.
     * \see fromWkt
     */
    virtual bool fromWkb( QgsConstWkbPtr &wkb ) = 0;

    /**
     * Sets the geometry from a WKT string.
     * \see fromWkb
     */
    virtual bool fromWkt( const QString &wkt ) = 0;

    //export

    /**
     * WKB export flags.
     * \since QGIS 3.14
     */
    enum WkbFlag SIP_ENUM_BASETYPE( IntFlag )
    {
      FlagExportTrianglesAsPolygons = 1 << 0, //!< Triangles should be exported as polygon geometries
      FlagExportNanAsDoubleMin = 1 << 1, //!< Use -DOUBLE_MAX to represent NaN (since QGIS 3.30)
    };
    Q_DECLARE_FLAGS( WkbFlags, WkbFlag )

    /**
     * Returns the length of the QByteArray returned by asWkb()
     *
     * The optional \a flags argument specifies flags controlling WKB export behavior
     *
     * \since QGIS 3.16
     */
    virtual int wkbSize( QgsAbstractGeometry::WkbFlags flags = QgsAbstractGeometry::WkbFlags() ) const = 0;

    /**
     * Returns a WKB representation of the geometry.
     *
     * The optional \a flags argument specifies flags controlling WKB export behavior (since QGIS 3.14).
     *
     * \see asWkt
     * \see asGml2
     * \see asGml3
     * \see asJson()
     */
    virtual QByteArray asWkb( WkbFlags flags = QgsAbstractGeometry::WkbFlags() ) const = 0;

    /**
     * Returns a WKT representation of the geometry.
     * \param precision number of decimal places for coordinates
     * \see asWkb()
     * \see asGml2()
     * \see asGml3()
     * \see asJson()
     */
    virtual QString asWkt( int precision = 17 ) const = 0;

    /**
     * Returns a GML2 representation of the geometry.
     * \param doc DOM document
     * \param precision number of decimal places for coordinates
     * \param ns XML namespace
     * \param axisOrder Axis order for generated GML
     * \see asWkb()
     * \see asWkt()
     * \see asGml3()
     * \see asJson()
     */
    virtual QDomElement asGml2( QDomDocument &doc, int precision = 17, const QString &ns = "gml", AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const = 0;

    /**
     * Returns a GML3 representation of the geometry.
     * \param doc DOM document
     * \param precision number of decimal places for coordinates
     * \param ns XML namespace
     * \param axisOrder Axis order for generated GML
     * \see asWkb()
     * \see asWkt()
     * \see asGml2()
     * \see asJson()
     */
    virtual QDomElement asGml3( QDomDocument &doc, int precision = 17, const QString &ns = "gml", AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const = 0;

    /**
     * Returns a GeoJSON representation of the geometry as a QString.
     * \param precision number of decimal places for coordinates
     * \see asWkb()
     * \see asWkt()
     * \see asGml2()
     * \see asGml3()
     * \see asJsonObject()
     */
    QString asJson( int precision = 17 );

    /**
     * Returns a json object representation of the geometry.
     * \see asWkb()
     * \see asWkt()
     * \see asGml2()
     * \see asGml3()
     * \see asJson()
     * \note not available in Python bindings
     * \since QGIS 3.10
     */
    virtual json asJsonObject( int precision = 17 ) SIP_SKIP const;

    /**
     * Returns a KML representation of the geometry.
     * \since QGIS 3.12
     */
    virtual QString asKml( int precision = 17 ) const = 0;


    //render pipeline

    /**
     * Transforms the geometry using a coordinate transform
     * \param ct coordinate transform
     * \param d transformation direction
     * \param transformZ set to TRUE to also transform z coordinates. This requires that
     * the z coordinates in the geometry represent height relative to the vertical datum
     * of the source CRS (generally ellipsoidal heights) and are expressed in its vertical
     * units (generally meters). If FALSE, then z coordinates will not be changed by the
     * transform.
     */
    virtual void transform( const QgsCoordinateTransform &ct, Qgis::TransformDirection d = Qgis::TransformDirection::Forward, bool transformZ = false ) SIP_THROW( QgsCsException ) = 0;

    /**
     * Transforms the x and y components of the geometry using a QTransform object \a t.
     *
     * Optionally, the geometry's z values can be scaled via \a zScale and translated via \a zTranslate.
     * Similarly, m-values can be scaled via \a mScale and translated via \a mTranslate.
     */
    virtual void transform( const QTransform &t, double zTranslate = 0.0, double zScale = 1.0,
                            double mTranslate = 0.0, double mScale = 1.0 ) = 0;

    /**
     * Draws the geometry using the specified QPainter.
     * \param p destination QPainter
     */
    virtual void draw( QPainter &p ) const = 0;

    /**
     * Returns the geometry represented as a QPainterPath.
     *
     * \warning not all geometry subclasses can be represented by a QPainterPath, e.g.
     * points and multipoint geometries will return an empty path.
     *
     * \since QGIS 3.16
     */
    virtual QPainterPath asQPainterPath() const = 0;

    /**
     * Returns the vertex number corresponding to a vertex \a id.
     *
     * The vertex numbers start at 0, so a return value of 0 corresponds
     * to the first vertex.
     *
     * Returns -1 if a corresponding vertex could not be found.
     *
     */
    virtual int vertexNumberFromVertexId( QgsVertexId id ) const = 0;

    /**
     * Returns next vertex id and coordinates
     * \param id initial value should be the starting vertex id. The next vertex id will be stored
     * in this variable if found.
     * \param vertex container for found node
     * \returns FALSE if at end
     */
    virtual bool nextVertex( QgsVertexId &id, QgsPoint &vertex SIP_OUT ) const = 0;

    /**
     * Returns the vertices adjacent to a specified \a vertex within a geometry.
     */
    virtual void adjacentVertices( QgsVertexId vertex, QgsVertexId &previousVertex SIP_OUT, QgsVertexId &nextVertex SIP_OUT ) const = 0;

    /**
     * Retrieves the sequence of geometries, rings and nodes.
     * \returns coordinate sequence
     */
    virtual QgsCoordinateSequence coordinateSequence() const = 0;

    /**
     * Returns the number of nodes contained in the geometry
     */
    virtual int nCoordinates() const;

    /**
     * Returns the point corresponding to a specified vertex id
     */
    virtual QgsPoint vertexAt( QgsVertexId id ) const = 0;

    /**
     * Searches for the closest segment of the geometry to a given point.
     * \param pt specifies the point to find closest segment to
     * \param segmentPt storage for the closest point within the geometry
     * \param vertexAfter storage for the ID of the vertex at the end of the closest segment
     * \param leftOf indicates whether the point lies on the left side of the geometry (-1 if point is to the left
     * of the geometry, +1 if the point is to the right of the geometry, or 0 for cases where left/right could not
     * be determined, e.g. point exactly on a line)
     * FALSE if point is to right of segment)
     * \param epsilon epsilon for segment snapping
     * \returns squared distance to closest segment or negative value on error
     */
    virtual double closestSegment( const QgsPoint &pt, QgsPoint &segmentPt SIP_OUT,
                                   QgsVertexId &vertexAfter SIP_OUT,
                                   int *leftOf SIP_OUT = nullptr, double epsilon = 4 * std::numeric_limits<double>::epsilon() ) const = 0;

    //low-level editing

    /**
     * Inserts a vertex into the geometry
     * \param position vertex id for position of inserted vertex
     * \param vertex vertex to insert
     * \returns TRUE if insert was successful
     * \see moveVertex
     * \see deleteVertex
     */
    virtual bool insertVertex( QgsVertexId position, const QgsPoint &vertex ) = 0;

    /**
     * Moves a vertex within the geometry
     * \param position vertex id for vertex to move
     * \param newPos new position of vertex
     * \returns TRUE if move was successful
     * \see insertVertex
     * \see deleteVertex
     */
    virtual bool moveVertex( QgsVertexId position, const QgsPoint &newPos ) = 0;

    /**
     * Deletes a vertex within the geometry
     * \param position vertex id for vertex to delete
     * \returns TRUE if delete was successful
     * \see insertVertex
     * \see moveVertex
     */
    virtual bool deleteVertex( QgsVertexId position ) = 0;

    /**
     * Returns the planar, 2-dimensional length of the geometry.
     *
     * \warning QgsAbstractGeometry objects are inherently Cartesian/planar geometries, and the length
     * returned by this method is calculated using strictly Cartesian mathematics. In contrast,
     * the QgsDistanceArea class exposes methods for calculating the lengths of geometries using
     * geodesic calculations which account for the curvature of the Earth (or any other
     * celestial body).
     *
     * \see area()
     * \see perimeter()
     */
    virtual double length() const;

    /**
     * Returns the planar, 2-dimensional perimeter of the geometry.
     *
     * \warning QgsAbstractGeometry objects are inherently Cartesian/planar geometries, and the perimeter
     * returned by this method is calculated using strictly Cartesian mathematics. In contrast,
     * the QgsDistanceArea class exposes methods for calculating the perimeters of geometries using
     * geodesic calculations which account for the curvature of the Earth (or any other
     * celestial body).
     *
     * \see area()
     * \see length()
     */
    virtual double perimeter() const;

    /**
     * Returns the planar, 2-dimensional area of the geometry.
     *
     * \warning QgsAbstractGeometry objects are inherently Cartesian/planar geometries, and the area
     * returned by this method is calculated using strictly Cartesian mathematics. In contrast,
     * the QgsDistanceArea class exposes methods for calculating the areas of geometries using
     * geodesic calculations which account for the curvature of the Earth (or any other
     * celestial body).
     *
     * \see length()
     * \see perimeter()
     */
    virtual double area() const;

    /**
     * Returns the length of the segment of the geometry which begins at \a startVertex.
     *
     * \warning QgsAbstractGeometry objects are inherently Cartesian/planar geometries, and the lengths
     * returned by this method are calculated using strictly Cartesian mathematics.
     *
     */
    virtual double segmentLength( QgsVertexId startVertex ) const = 0;

    //! Returns the centroid of the geometry
    virtual QgsPoint centroid() const;

    /**
     * Returns TRUE if the geometry is empty
     */
    virtual bool isEmpty() const;

    /**
     * Returns TRUE if the geometry contains curved segments
     */
    virtual bool hasCurvedSegments() const;

    /**
     * Returns TRUE if the bounding box of this geometry intersects with a \a rectangle.
     *
     * Since this test only considers the bounding box of the geometry, is is very fast to
     * calculate and handles invalid geometries.
     *
     * \since QGIS 3.20
     */
    virtual bool boundingBoxIntersects( const QgsRectangle &rectangle ) const SIP_HOLDGIL;

    /**
     * Returns TRUE if the bounding box of this geometry intersects with a \a box3d.
     *
     * Since this test only considers the bounding box of the geometry, is is very fast to
     * calculate and handles invalid geometries.
     *
     * \since QGIS 3.34
     */
    virtual bool boundingBoxIntersects( const QgsBox3D &box3d ) const SIP_HOLDGIL;

    /**
     * Returns a version of the geometry without curves. Caller takes ownership of
     * the returned geometry.
     * \param tolerance segmentation tolerance
     * \param toleranceType maximum segmentation angle or maximum difference between approximation and curve
     */
    virtual QgsAbstractGeometry *segmentize( double tolerance = M_PI / 180., SegmentationToleranceType toleranceType = MaximumAngle ) const SIP_FACTORY;

    /**
     * Returns the geometry converted to the more generic curve type.
     * E.g. QgsLineString -> QgsCompoundCurve, QgsPolygon -> QgsCurvePolygon,
     * QgsMultiLineString -> QgsMultiCurve, QgsMultiPolygon -> QgsMultiSurface
     * \returns the converted geometry. Caller takes ownership
    */
    virtual QgsAbstractGeometry *toCurveType() const = 0 SIP_FACTORY;

    /**
     * Makes a new geometry with all the points or vertices snapped to the closest point of the grid.
     * Ownership is transferred to the caller.
     *
     * If the gridified geometry could not be calculated NULLPTR will be returned.
     * It may generate an invalid geometry (in some corner cases).
     * It can also be thought as rounding the edges and it may be useful for removing errors.
     *
     * Example:
     *
     * \code{.py}
     *   geometry.snappedToGrid(1, 1)
     * \endcode
     *
     * In this case we use a 2D grid of 1x1 to gridify.
     * In this case, it can be thought like rounding the x and y of all the points/vertices to full units (remove all decimals).
     * \param hSpacing Horizontal spacing of the grid (x axis). 0 to disable.
     * \param vSpacing Vertical spacing of the grid (y axis). 0 to disable.
     * \param dSpacing Depth spacing of the grid (z axis). 0 (default) to disable.
     * \param mSpacing Custom dimension spacing of the grid (m axis). 0 (default) to disable.
     * \param removeRedundantPoints if TRUE, then points which are redundant (e.g. they represent mid points on a straight line segment) will be skipped (since QGIS 3.38)
     */
    virtual QgsAbstractGeometry *snappedToGrid( double hSpacing, double vSpacing, double dSpacing = 0, double mSpacing = 0, bool removeRedundantPoints = false ) const = 0 SIP_FACTORY;

    /**
     * Removes duplicate nodes from the geometry, wherever removing the nodes does not result in a
     * degenerate geometry.
     *
     * The \a epsilon parameter specifies the tolerance for coordinates when determining that
     * vertices are identical.
     *
     * By default, z values are not considered when detecting duplicate nodes. E.g. two nodes
     * with the same x and y coordinate but different z values will still be considered
     * duplicate and one will be removed. If \a useZValues is TRUE, then the z values are
     * also tested and nodes with the same x and y but different z will be maintained.
     *
     * Note that duplicate nodes are not tested between different parts of a multipart geometry. E.g.
     * a multipoint geometry with overlapping points will not be changed by this method.
     *
     * The function will return TRUE if nodes were removed, or FALSE if no duplicate nodes
     * were found.
     *
     */
    virtual bool removeDuplicateNodes( double epsilon = 4 * std::numeric_limits<double>::epsilon(), bool useZValues = false ) = 0;

    /**
     * Returns approximate angle at a vertex. This is usually the average angle between adjacent
     * segments, and can be pictured as the orientation of a line following the curvature of the
     * geometry at the specified vertex.
     * \param vertex the vertex id
     * \returns rotation in radians, clockwise from north
     */
    virtual double vertexAngle( QgsVertexId vertex ) const = 0;

    /**
     * Returns the number of vertices of which this geometry is built.
     */
    virtual int vertexCount( int part = 0, int ring = 0 ) const = 0;

    /**
     * Returns the number of rings of which this geometry is built.
     */
    virtual int ringCount( int part = 0 ) const = 0;

    /**
     * Returns count of parts contained in the geometry.
     * \see vertexCount
     * \see ringCount
     */
    virtual int partCount() const = 0;

    /**
     * Adds a z-dimension to the geometry, initialized to a preset value.
     * \param zValue initial z-value for all nodes
     * \returns TRUE on success
     * \see dropZValue()
     * \see addMValue()
     */
    virtual bool addZValue( double zValue = 0 ) = 0;

    /**
     * Adds a measure to the geometry, initialized to a preset value.
     * \param mValue initial m-value for all nodes
     * \returns TRUE on success
     * \see dropMValue()
     * \see addZValue()
     */
    virtual bool addMValue( double mValue = 0 ) = 0;

    /**
     * Drops any z-dimensions which exist in the geometry.
     * \returns TRUE if Z values were present and have been removed
     * \see addZValue()
     * \see dropMValue()
     */
    virtual bool dropZValue() = 0;

    /**
     * Drops any measure values which exist in the geometry.
     * \returns TRUE if m-values were present and have been removed
     * \see addMValue()
     * \see dropZValue()
     */
    virtual bool dropMValue() = 0;

    /**
     * Swaps the x and y coordinates from the geometry. This can be used
     * to repair geometries which have accidentally had their latitude and longitude
     * coordinates reversed.
     * \since QGIS 3.2
     */
    virtual void swapXy() = 0;

    /**
     * Converts the geometry to a specified type.
     * \returns TRUE if conversion was successful
     */
    virtual bool convertTo( Qgis::WkbType type );

    /**
     * Returns a reference to the simplest lossless representation of this geometry,
     * e.g. if the geometry is a multipart geometry type with a single member geometry,
     * a reference to that part will be returned.
     *
     * This method employs the following logic:
     *
     * - For multipart geometries containing a single part only a direct reference to that part will be returned.
     * - For compound curve geometries containing a single curve only a direct reference to that curve will be returned.
     *
     * This method returns a reference only, and does not involve any geometry cloning.
     *
     * \note Ownership of the returned geometry is NOT transferred, and remains with the original
     * geometry object. Callers must take care to ensure that the original geometry object
     * exists for the lifespan of the returned object.
     *
     * \since QGIS 3.20
     */
    virtual const QgsAbstractGeometry *simplifiedTypeRef() const SIP_HOLDGIL;

    /**
     * Checks validity of the geometry, and returns TRUE if the geometry is valid.
     *
     * \param error will be set to the validity error message
     * \param flags indicates optional flags which control the type of validity checking performed
     * (corresponding to Qgis::GeometryValidityFlags).
     *
     * \returns TRUE if geometry is valid
     *
     * \since QGIS 3.8
     */
    virtual bool isValid( QString &error SIP_OUT, Qgis::GeometryValidityFlags flags = Qgis::GeometryValidityFlags() ) const = 0;

    /**
     * Transforms the vertices from the geometry in place, using the specified geometry \a transformer
     * object.
     *
     * Depending on the \a transformer used, this may result in an invalid geometry.
     *
     * The optional \a feedback argument can be used to cancel the transformation before it completes.
     * If this is done, the geometry will be left in a semi-transformed state.
     *
     * \returns TRUE if the geometry was successfully transformed.
     *
     * \since QGIS 3.18
     */
    virtual bool transform( QgsAbstractGeometryTransformer *transformer, QgsFeedback *feedback = nullptr ) = 0;

#ifndef SIP_RUN

    /**
     * Filters the vertices from the geometry in place, removing any which do not return TRUE for the \a filter function
     * check. Has no meaning when called on a single point geometry.
     *
     * Depending on the \a filter used, this may result in an invalid geometry.
     *
     * \note Not available in Python bindings
     * \since QGIS 3.2
     */
    virtual void filterVertices( const std::function< bool( const QgsPoint & ) > &filter );

    /**
     * Transforms the vertices from the geometry in place, applying the \a transform function
     * to every vertex.
     *
     * Depending on the \a transform used, this may result in an invalid geometry.
     *
     * Transform functions are not permitted to alter the dimensionality of vertices. If
     * a transform which adds (or removes) z/m values is desired, first call the corresponding
     * addZValue() or addMValue() function to change the geometry's dimensionality and then
     * transform.
     *
     * \note Not available in Python bindings
     * \since QGIS 3.4
     */
    virtual void transformVertices( const std::function< QgsPoint( const QgsPoint & ) > &transform );

    /**
     * \ingroup core
     * \brief The part_iterator class provides STL-style iterator for geometry parts.
     * \since QGIS 3.6
     */
    class CORE_EXPORT part_iterator
    {
      private:

        int mIndex = 0; //!< Current part in the geometry
        QgsAbstractGeometry *mGeometry = nullptr;

      public:
        //! Create invalid iterator
        part_iterator() = default;

        //! Create part iterator for a geometry
        part_iterator( QgsAbstractGeometry *g, int index );

        /**
         * The prefix ++ operator (++it) advances the iterator to the next part and returns an iterator to the new current part.
         * Calling this function on iterator that is already past the last item leads to undefined results.
         */
        part_iterator &operator++();

        //! The postfix ++ operator (it++) advances the iterator to the next part and returns an iterator to the previously current part.
        part_iterator operator++( int );

        //! Returns the current item.
        QgsAbstractGeometry *operator*() const;

        //! Returns the part number of the current item.
        int partNumber() const;

        bool operator==( part_iterator other ) const;
        bool operator!=( part_iterator other ) const { return !( *this == other ); }
    };

    /**
     * Returns STL-style iterator pointing to the first part of the geometry.
     *
     * \see parts_end()
     * \see parts()
     *
     * \since QGIS 3.6
     */
    part_iterator parts_begin()
    {
      return part_iterator( this, 0 );
    }

    /**
     * Returns STL-style iterator pointing to the imaginary part after the last part of the geometry.
     *
     * \see parts_begin()
     * \see parts()
     *
     * \since QGIS 3.6
     */
    part_iterator parts_end();

    /**
     * Returns Java-style iterator for traversal of parts of the geometry. This iterator
     * returns read-only references to parts and cannot be used to modify the parts.
     *
     * \note Not available in Python bindings
     * \since QGIS 3.6
     */
    QgsGeometryConstPartIterator parts() const;

    /**
     * \ingroup core
     * \brief The part_iterator class provides STL-style iterator for const references to geometry parts.
     * \since QGIS 3.6
     */
    class CORE_EXPORT const_part_iterator
    {
      private:

        int mIndex = 0; //!< Current part in the geometry
        const QgsAbstractGeometry *mGeometry = nullptr;

      public:
        //! Create invalid iterator
        const_part_iterator() = default;

        //! Create part iterator for a geometry
        const_part_iterator( const QgsAbstractGeometry *g, int index );

        /**
         * The prefix ++ operator (++it) advances the iterator to the next part and returns an iterator to the new current part.
         * Calling this function on iterator that is already past the last item leads to undefined results.
         */
        const_part_iterator &operator++();

        //! The postfix ++ operator (it++) advances the iterator to the next part and returns an iterator to the previously current part.
        const_part_iterator operator++( int );

        //! Returns the current item.
        const QgsAbstractGeometry *operator*() const;

        //! Returns the part number of the current item.
        int partNumber() const;

        bool operator==( const_part_iterator other ) const;
        bool operator!=( const_part_iterator other ) const { return !( *this == other ); }
    };

    /**
     * Returns STL-style iterator pointing to the const first part of the geometry.
     *
     * \see const_parts_end()
     *
     * \since QGIS 3.6
     */
    const_part_iterator const_parts_begin() const
    {
      return const_part_iterator( this, 0 );
    }

    /**
     * Returns STL-style iterator pointing to the imaginary const part after the last part of the geometry.
     *
     * \see const_parts_begin()
     *
     * \since QGIS 3.6
     */
    const_part_iterator const_parts_end() const;


    /**
     * \ingroup core
     * \brief The vertex_iterator class provides STL-style iterator for vertices.
     */
    class CORE_EXPORT vertex_iterator
    {
      private:

        /**
         * A helper structure to keep track of vertex traversal within one level within a geometry.
         * For example, linestring geometry will have just one level, while multi-polygon has three levels
         * (part index, ring index, vertex index).
         */
        struct Level
        {
          const QgsAbstractGeometry *g = nullptr;  //!< Current geometry
          int index = 0;               //!< Ptr in the current geometry

          bool operator==( const Level &other ) const;
        };

        std::array<Level, 3> levels;  //!< Stack of levels - three levels should be sufficient (e.g. part index, ring index, vertex index)
        int depth = -1;               //!< At what depth level are we right now

        void digDown();   //!< Prepare the stack of levels so that it points to a leaf child geometry

      public:
        //! Create invalid iterator
        vertex_iterator() = default;

        //! Create vertex iterator for a geometry
        vertex_iterator( const QgsAbstractGeometry *g, int index );

        /**
         * The prefix ++ operator (++it) advances the iterator to the next vertex and returns an iterator to the new current vertex.
         * Calling this function on iterator that is already past the last item leads to undefined results.
         */
        vertex_iterator &operator++();

        //! The postfix ++ operator (it++) advances the iterator to the next vertex and returns an iterator to the previously current vertex.
        vertex_iterator operator++( int );

        //! Returns the current item.
        QgsPoint operator*() const;

        //! Returns vertex ID of the current item.
        QgsVertexId vertexId() const;

        bool operator==( const vertex_iterator &other ) const;
        bool operator!=( const vertex_iterator &other ) const { return !( *this == other ); }
    };

    /**
     * Returns STL-style iterator pointing to the first vertex of the geometry.
     *
     * \see vertices_end()
     * \see vertices()
     *
     */
    vertex_iterator vertices_begin() const
    {
      return vertex_iterator( this, 0 );
    }

    /**
     * Returns STL-style iterator pointing to the imaginary vertex after the last vertex of the geometry.
     *
     * \see vertices_begin()
     * \see vertices()
     *
     */
    vertex_iterator vertices_end() const
    {
      return vertex_iterator( this, childCount() );
    }
#endif

    /**
     * Returns Java-style iterator for traversal of parts of the geometry. This iterator
     * can safely be used to modify parts of the geometry.
     *
     * Example
     *
     * \code{.py}
     *   # print the WKT representation of each part in a multi-point geometry
     *   geometry = QgsMultiPoint.fromWkt( 'MultiPoint( 0 0, 1 1, 2 2)' )
     *   for part in geometry.parts():
     *       print(part.asWkt())
     *
     *   # single part geometries only have one part - this loop will iterate once only
     *   geometry = QgsLineString.fromWkt( 'LineString( 0 0, 10 10 )' )
     *   for part in geometry.parts():
     *       print(part.asWkt())
     *
     *   # parts can be modified during the iteration
     *   geometry = QgsMultiPoint.fromWkt( 'MultiPoint( 0 0, 1 1, 2 2)' )
     *   for part in geometry.parts():
     *       part.transform(ct)
     *
     *   # part iteration can also be combined with vertex iteration
     *   geometry = QgsMultiPolygon.fromWkt( 'MultiPolygon((( 0 0, 0 10, 10 10, 10 0, 0 0 ),( 5 5, 5 6, 6 6, 6 5, 5 5)),((20 2, 22 2, 22 4, 20 4, 20 2)))' )
     *   for part in geometry.parts():
     *       for v in part.vertices():
     *           print(v.x(), v.y())
     *
     * \endcode
     *
     * \see vertices()
     * \since QGIS 3.6
     */
    QgsGeometryPartIterator parts();


    /**
     * Returns a read-only, Java-style iterator for traversal of vertices of all the geometry, including all geometry parts and rings.
     *
     * \warning The iterator returns a copy of individual vertices, and accordingly geometries cannot be
     * modified using the iterator. See transformVertices() for a safe method to modify vertices "in-place".
     *
     * Example
     *
     * \code{.py}
     *   # print the x and y coordinate for each vertex in a LineString
     *   geometry = QgsLineString.fromWkt( 'LineString( 0 0, 1 1, 2 2)' )
     *   for v in geometry.vertices():
     *       print(v.x(), v.y())
     *
     *   # vertex iteration includes all parts and rings
     *   geometry = QgsMultiPolygon.fromWkt( 'MultiPolygon((( 0 0, 0 10, 10 10, 10 0, 0 0 ),( 5 5, 5 6, 6 6, 6 5, 5 5)),((20 2, 22 2, 22 4, 20 4, 20 2)))' )
     *   for v in geometry.vertices():
     *       print(v.x(), v.y())
     * \endcode
     *
     * \see parts()
     */
    QgsVertexIterator vertices() const;

    /**
     * Creates a new geometry with the same class and same WKB type as the original and transfers ownership.
     * To create it, the geometry is default constructed and then the WKB is changed.
     * \see clone()
     */
    virtual QgsAbstractGeometry *createEmptyWithSameType() const = 0 SIP_FACTORY;

  protected:

    /**
     * Returns the sort index for the geometry, used in the compareTo() method to compare
     * geometries of different types.
     *
     * \since QGIS 3.20
     */
    int sortIndex() const;

    /**
     * Compares to an \a other geometry of the same class, and returns a integer
     * for sorting of the two geometries.
     *
     * \note The actual logic for the sorting is an internal detail only and is subject to change
     * between QGIS versions. The result should only be used for direct comparison of geometries
     * and not stored for later use.
     *
     * \since QGIS 3.20
     */
    virtual int compareToSameClass( const QgsAbstractGeometry *other ) const = 0;

    /**
     * Returns whether the geometry has any child geometries (FALSE for point / curve, TRUE otherwise)
     * \note used for vertex_iterator implementation
     */
    virtual bool hasChildGeometries() const;

    /**
     * Returns number of child geometries (for geometries with child geometries) or child points (for geometries without child geometries - i.e. curve / point)
     * \note used for vertex_iterator implementation
     */
    virtual int childCount() const { return 0; }

    /**
     * Returns pointer to child geometry (for geometries with child geometries - i.e. geom. collection / polygon)
     * \note used for vertex_iterator implementation
     */
    virtual QgsAbstractGeometry *childGeometry( int index ) const { Q_UNUSED( index ) return nullptr; }

    /**
     * Returns point at index (for geometries without child geometries - i.e. curve / point)
     * \note used for vertex_iterator implementation
     */
    virtual QgsPoint childPoint( int index ) const;

  protected:
    Qgis::WkbType mWkbType = Qgis::WkbType::Unknown;

    /**
     * Updates the geometry type based on whether sub geometries contain z or m values.
     */
    void setZMTypeFromSubGeometry( const QgsAbstractGeometry *subggeom, Qgis::WkbType baseGeomType );

    /**
     * Default calculator for the minimal bounding box for the geometry. Derived classes should override this method
     * if a more efficient bounding box calculation is available.
     */
    virtual QgsRectangle calculateBoundingBox() const;

    /**
     * Calculates the minimal 3D bounding box for the geometry.
     * \see calculateBoundingBox()
     *
     * \since QGIS 3.34
     */
    virtual QgsBox3D calculateBoundingBox3D() const;

    /**
     * Clears any cached parameters associated with the geometry, e.g., bounding boxes
     */
    virtual void clearCache() const;

    friend class TestQgsGeometry;
};


#ifndef SIP_RUN

template <class T>
inline T qgsgeometry_cast( const QgsAbstractGeometry *geom )
{
  return const_cast<T>( std::remove_pointer<T>::type::cast( geom ) );
}

#endif

// clazy:excludeall=qstring-allocations

/**
 * \ingroup core
 * \brief Java-style iterator for traversal of vertices of a geometry
 */
class CORE_EXPORT QgsVertexIterator
{
  public:
    //! Constructor for QgsVertexIterator
    QgsVertexIterator() = default;

    //! Constructs iterator for the given geometry
    QgsVertexIterator( const QgsAbstractGeometry *geometry )
      : g( geometry )
      , i( g->vertices_begin() )
      , n( g->vertices_end() )
    {
    }

    //! Find out whether there are more vertices
    bool hasNext() const
    {
      return g && g->vertices_end() != i;
    }

    //! Returns next vertex of the geometry (undefined behavior if hasNext() returns FALSE before calling next())
    QgsPoint next();

#ifdef SIP_RUN
    QgsVertexIterator *__iter__();
    % MethodCode
    sipRes = sipCpp;
    % End

    SIP_PYOBJECT __next__() SIP_TYPEHINT( QgsPoint );
    % MethodCode
    if ( sipCpp->hasNext() )
      sipRes = sipConvertFromType( new QgsPoint( sipCpp->next() ), sipType_QgsPoint, Py_None );
    else
      PyErr_SetString( PyExc_StopIteration, "" );
    % End
#endif

  private:
    const QgsAbstractGeometry *g = nullptr;
    QgsAbstractGeometry::vertex_iterator i, n;

};

/**
 * \ingroup core
 * \brief Java-style iterator for traversal of parts of a geometry
 * \since QGIS 3.6
 */
class CORE_EXPORT QgsGeometryPartIterator
{
  public:
    //! Constructor for QgsGeometryPartIterator
    QgsGeometryPartIterator() = default;

    //! Constructs iterator for the given geometry
    QgsGeometryPartIterator( QgsAbstractGeometry *geometry )
      : g( geometry )
      , i( g->parts_begin() )
      , n( g->parts_end() )
    {
    }

    //! Find out whether there are more parts
    bool hasNext() const SIP_HOLDGIL
    {
      return g && g->parts_end() != i;
    }

    //! Returns next part of the geometry (undefined behavior if hasNext() returns FALSE before calling next())
    QgsAbstractGeometry *next();

#ifdef SIP_RUN
    QgsGeometryPartIterator *__iter__();
    % MethodCode
    sipRes = sipCpp;
    % End

    SIP_PYOBJECT __next__() SIP_TYPEHINT( QgsAbstractGeometry );
    % MethodCode
    if ( sipCpp->hasNext() )
      sipRes = sipConvertFromType( sipCpp->next(), sipType_QgsAbstractGeometry, NULL );
    else
      PyErr_SetString( PyExc_StopIteration, "" );
    % End
#endif

  private:
    QgsAbstractGeometry *g = nullptr;
    QgsAbstractGeometry::part_iterator i, n;

};


/**
 * \ingroup core
 * \brief Java-style iterator for const traversal of parts of a geometry
 * \since QGIS 3.6
 */
class CORE_EXPORT QgsGeometryConstPartIterator
{
  public:
    //! Constructor for QgsGeometryConstPartIterator
    QgsGeometryConstPartIterator() = default;

    //! Constructs iterator for the given geometry
    QgsGeometryConstPartIterator( const QgsAbstractGeometry *geometry )
      : g( geometry )
      , i( g->const_parts_begin() )
      , n( g->const_parts_end() )
    {
    }

    //! Find out whether there are more parts
    bool hasNext() const SIP_HOLDGIL
    {
      return g && g->const_parts_end() != i;
    }

    //! Returns next part of the geometry (undefined behavior if hasNext() returns FALSE before calling next())
    const QgsAbstractGeometry *next();

#ifdef SIP_RUN
    QgsGeometryConstPartIterator *__iter__();
    % MethodCode
    sipRes = sipCpp;
    % End

    SIP_PYOBJECT __next__() SIP_TYPEHINT( QgsAbstractGeometry );
    % MethodCode
    if ( sipCpp->hasNext() )
      sipRes = sipConvertFromType( const_cast< QgsAbstractGeometry * >( sipCpp->next() ), sipType_QgsAbstractGeometry, NULL );
    else
      PyErr_SetString( PyExc_StopIteration, "" );
    % End
#endif

  private:
    const QgsAbstractGeometry *g = nullptr;
    QgsAbstractGeometry::const_part_iterator i, n;

};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsAbstractGeometry::WkbFlags )

#endif //QGSABSTRACTGEOMETRYV2

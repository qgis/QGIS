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

#include <functional>

#include <QString>

#include "qgis_core.h"
#include "qgis.h"
#include "qgscoordinatetransform.h"
#include "qgswkbtypes.h"
#include "qgswkbptr.h"

class QgsMapToPixel;
class QgsCurve;
class QgsMultiCurve;
class QgsMultiPoint;
class QgsPoint;
struct QgsVertexId;
class QgsVertexIterator;
class QPainter;
class QDomDocument;
class QDomElement;

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
 * \since QGIS 2.10
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
       * to output vertices) */
      MaximumAngle = 0,

      /**
       * Maximum distance between an arbitrary point on the original
       * curve and closest point on its approximation. */
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
     * Clones the geometry by performing a deep copy
     */
    virtual QgsAbstractGeometry *clone() const = 0 SIP_FACTORY;

    /**
     * Clears the geometry, ie reset it to a null geometry
     */
    virtual void clear() = 0;

    /**
     * Returns the minimal bounding box for the geometry
     */
    virtual QgsRectangle boundingBox() const = 0;

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
    inline QgsWkbTypes::Type wkbType() const { return mWkbType; }

    /**
     * Returns the WKT type string of the geometry.
     * \see geometryType
     * \see wkbType
     */
    QString wktTypeStr() const;

    /**
     * Returns true if the geometry is 3D and contains a z-value.
     * \see isMeasure
     */
    bool is3D() const
    {
      return QgsWkbTypes::hasZ( mWkbType );
    }

    /**
     * Returns true if the geometry contains m values.
     * \see is3D
     */
    bool isMeasure() const
    {
      return QgsWkbTypes::hasM( mWkbType );
    }

    /**
     * Returns the closure of the combinatorial boundary of the geometry (ie the topological boundary of the geometry).
     * For instance, a polygon geometry will have a boundary consisting of the linestrings for each ring in the polygon.
     * \returns boundary for geometry. May be null for some geometry types.
     * \since QGIS 3.0
     */
    virtual QgsAbstractGeometry *boundary() const = 0 SIP_FACTORY;

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
     * Returns a WKB representation of the geometry.
     * \see asWkt
     * \see asGml2
     * \see asGml3
     * \see asJson()
     * \since QGIS 3.0
     */
    virtual QByteArray asWkb() const = 0;

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
     * Returns a GeoJSON representation of the geometry.
     * \param precision number of decimal places for coordinates
     * \see asWkb()
     * \see asWkt()
     * \see asGml2()
     * \see asGml3()
     */
    virtual QString asJson( int precision = 17 ) const = 0;

    //render pipeline

    /**
     * Transforms the geometry using a coordinate transform
     * \param ct coordinate transform
     * \param d transformation direction
     * \param transformZ set to true to also transform z coordinates. This requires that
     * the z coordinates in the geometry represent height relative to the vertical datum
     * of the source CRS (generally ellipsoidal heights) and are expressed in its vertical
     * units (generally meters). If false, then z coordinates will not be changed by the
     * transform.
     */
    virtual void transform( const QgsCoordinateTransform &ct, QgsCoordinateTransform::TransformDirection d = QgsCoordinateTransform::ForwardTransform, bool transformZ = false ) SIP_THROW( QgsCsException ) = 0;

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
     * Returns the vertex number corresponding to a vertex \a id.
     *
     * The vertex numbers start at 0, so a return value of 0 corresponds
     * to the first vertex.
     *
     * Returns -1 if a corresponding vertex could not be found.
     *
     * \since QGIS 3.0
     */
    virtual int vertexNumberFromVertexId( QgsVertexId id ) const = 0;

    /**
     * Returns next vertex id and coordinates
     * \param id initial value should be the starting vertex id. The next vertex id will be stored
     * in this variable if found.
     * \param vertex container for found node
     * \returns false if at end
     */
    virtual bool nextVertex( QgsVertexId &id, QgsPoint &vertex SIP_OUT ) const = 0;

    /**
     * Returns the vertices adjacent to a specified \a vertex within a geometry.
     * \since QGIS 3.0
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
     * false if point is to right of segment)
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
     * \returns true if insert was successful
     * \see moveVertex
     * \see deleteVertex
     */
    virtual bool insertVertex( QgsVertexId position, const QgsPoint &vertex ) = 0;

    /**
     * Moves a vertex within the geometry
     * \param position vertex id for vertex to move
     * \param newPos new position of vertex
     * \returns true if move was successful
     * \see insertVertex
     * \see deleteVertex
     */
    virtual bool moveVertex( QgsVertexId position, const QgsPoint &newPos ) = 0;

    /**
     * Deletes a vertex within the geometry
     * \param position vertex id for vertex to delete
     * \returns true if delete was successful
     * \see insertVertex
     * \see moveVertex
     */
    virtual bool deleteVertex( QgsVertexId position ) = 0;

    /**
     * Returns the length of the geometry.
     * \see area()
     * \see perimeter()
     */
    virtual double length() const;

    /**
     * Returns the perimeter of the geometry.
     * \see area()
     * \see length()
     */
    virtual double perimeter() const;

    /**
     * Returns the area of the geometry.
     * \see length()
     * \see perimeter()
     */
    virtual double area() const;

    /**
     * Returns the length of the segment of the geometry which begins at \a startVertex.
     * \since QGIS 3.0
     */
    virtual double segmentLength( QgsVertexId startVertex ) const = 0;

    //! Returns the centroid of the geometry
    virtual QgsPoint centroid() const;

    /**
     * Returns true if the geometry is empty
     */
    virtual bool isEmpty() const;

    /**
     * Returns true if the geometry contains curved segments
     */
    virtual bool hasCurvedSegments() const;

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
     * If the gridified geometry could not be calculated a nullptr will be returned.
     * It may generate an invalid geometry (in some corner cases).
     * It can also be thought as rounding the edges and it may be useful for removing errors.
     * Example:
     * \code{.cpp}
     * geometry->snappedToGrid(1, 1);
     * \endcode
     * In this case we use a 2D grid of 1x1 to gridify.
     * In this case, it can be thought like rounding the x and y of all the points/vertices to full units (remove all decimals).
     * \param hSpacing Horizontal spacing of the grid (x axis). 0 to disable.
     * \param vSpacing Vertical spacing of the grid (y axis). 0 to disable.
     * \param dSpacing Depth spacing of the grid (z axis). 0 (default) to disable.
     * \param mSpacing Custom dimension spacing of the grid (m axis). 0 (default) to disable.
     * \since 3.0
     */
    virtual QgsAbstractGeometry *snappedToGrid( double hSpacing, double vSpacing, double dSpacing = 0, double mSpacing = 0 ) const = 0 SIP_FACTORY;

    /**
     * Removes duplicate nodes from the geometry, wherever removing the nodes does not result in a
     * degenerate geometry.
     *
     * The \a epsilon parameter specifies the tolerance for coordinates when determining that
     * vertices are identical.
     *
     * By default, z values are not considered when detecting duplicate nodes. E.g. two nodes
     * with the same x and y coordinate but different z values will still be considered
     * duplicate and one will be removed. If \a useZValues is true, then the z values are
     * also tested and nodes with the same x and y but different z will be maintained.
     *
     * Note that duplicate nodes are not tested between different parts of a multipart geometry. E.g.
     * a multipoint geometry with overlapping points will not be changed by this method.
     *
     * The function will return true if nodes were removed, or false if no duplicate nodes
     * were found.
     *
     * \since QGIS 3.0
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
     * \returns true on success
     * \see dropZValue()
     * \see addMValue()
     * \since QGIS 2.12
     */
    virtual bool addZValue( double zValue = 0 ) = 0;

    /**
     * Adds a measure to the geometry, initialized to a preset value.
     * \param mValue initial m-value for all nodes
     * \returns true on success
     * \see dropMValue()
     * \see addZValue()
     * \since QGIS 2.12
     */
    virtual bool addMValue( double mValue = 0 ) = 0;

    /**
     * Drops any z-dimensions which exist in the geometry.
     * \returns true if Z values were present and have been removed
     * \see addZValue()
     * \see dropMValue()
     * \since QGIS 2.14
     */
    virtual bool dropZValue() = 0;

    /**
     * Drops any measure values which exist in the geometry.
     * \returns true if m-values were present and have been removed
     * \see addMValue()
     * \see dropZValue()
     * \since QGIS 2.14
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
     * \returns true if conversion was successful
     * \since QGIS 2.14
     */
    virtual bool convertTo( QgsWkbTypes::Type type );

#ifndef SIP_RUN

    /**
     * Filters the vertices from the geometry in place, removing any which do not return true for the \a filter function
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
     * The vertex_iterator class provides STL-style iterator for vertices.
     * \since QGIS 3.0
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
        };

        Level levels[3];  //!< Stack of levels - three levels should be sufficient (e.g. part index, ring index, vertex index)
        int depth = -1;        //!< At what depth level are we right now

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
     * Returns STL-style iterator pointing to the first vertex of the geometry
     * \since QGIS 3.0
     */
    vertex_iterator vertices_begin() const
    {
      return vertex_iterator( this, 0 );
    }

    /**
     * Returns STL-style iterator pointing to the imaginary vertex after the last vertex of the geometry
     * \since QGIS 3.0
     */
    vertex_iterator vertices_end() const
    {
      return vertex_iterator( this, childCount() );
    }
#endif

    /**
     * Returns Java-style iterator for traversal of vertices of the geometry
     * \since QGIS 3.0
     */
    QgsVertexIterator vertices() const;

    /**
     * Creates a new geometry with the same class and same WKB type as the original and transfers ownership.
     * To create it, the geometry is default constructed and then the WKB is changed.
     * \see clone()
     * \since 3.0
     */
    virtual QgsAbstractGeometry *createEmptyWithSameType() const = 0 SIP_FACTORY;

  protected:

    /**
     * Returns whether the geometry has any child geometries (false for point / curve, true otherwise)
     * \note used for vertex_iterator implementation
     * \since QGIS 3.0
     */
    virtual bool hasChildGeometries() const;

    /**
     * Returns number of child geometries (for geometries with child geometries) or child points (for geometries without child geometries - i.e. curve / point)
     * \note used for vertex_iterator implementation
     * \since QGIS 3.0
     */
    virtual int childCount() const { return 0; }

    /**
     * Returns pointer to child geometry (for geometries with child geometries - i.e. geom. collection / polygon)
     * \note used for vertex_iterator implementation
     * \since QGIS 3.0
     */
    virtual QgsAbstractGeometry *childGeometry( int index ) const { Q_UNUSED( index ); return nullptr; }

    /**
     * Returns point at index (for geometries without child geometries - i.e. curve / point)
     * \note used for vertex_iterator implementation
     * \since QGIS 3.0
     */
    virtual QgsPoint childPoint( int index ) const;

  protected:
    QgsWkbTypes::Type mWkbType = QgsWkbTypes::Unknown;

    /**
     * Updates the geometry type based on whether sub geometries contain z or m values.
     */
    void setZMTypeFromSubGeometry( const QgsAbstractGeometry *subggeom, QgsWkbTypes::Type baseGeomType );

    /**
     * Default calculator for the minimal bounding box for the geometry. Derived classes should override this method
     * if a more efficient bounding box calculation is available.
     */
    virtual QgsRectangle calculateBoundingBox() const;

    /**
     * Clears any cached parameters associated with the geometry, e.g., bounding boxes
     */
    virtual void clearCache() const;

    friend class TestQgsGeometry;
};


/**
 * \ingroup core
 * \class QgsVertexId
 * \brief Utility class for identifying a unique vertex within a geometry.
 * \since QGIS 2.10
 */
struct CORE_EXPORT QgsVertexId
{
  enum VertexType
  {
    SegmentVertex = 1, //start / endpoint of a segment
    CurveVertex
  };

  explicit QgsVertexId( int _part = -1, int _ring = -1, int _vertex = -1, VertexType _type = SegmentVertex )
    : part( _part )
    , ring( _ring )
    , vertex( _vertex )
    , type( _type )
  {}

  /**
   * Returns true if the vertex id is valid
   */
  bool isValid() const { return part >= 0 && ring >= 0 && vertex >= 0; }

  bool operator==( QgsVertexId other ) const
  {
    return part == other.part && ring == other.ring && vertex == other.vertex;
  }
  bool operator!=( QgsVertexId other ) const
  {
    return part != other.part || ring != other.ring || vertex != other.vertex;
  }
  bool partEqual( QgsVertexId o ) const
  {
    return part >= 0 && o.part == part;
  }
  bool ringEqual( QgsVertexId o ) const
  {
    return partEqual( o ) && ( ring >= 0 && o.ring == ring );
  }
  bool vertexEqual( QgsVertexId o ) const
  {
    return ringEqual( o ) && ( vertex >= 0 && o.ring == ring );
  }
  bool isValid( const QgsAbstractGeometry *geom ) const
  {
    return ( part >= 0 && part < geom->partCount() ) &&
           ( ring < geom->ringCount( part ) ) &&
           ( vertex < 0 || vertex < geom->vertexCount( part, ring ) );
  }

  int part;
  int ring;
  int vertex;
  VertexType type;
};

#ifndef SIP_RUN

template <class T>
inline T qgsgeometry_cast( const QgsAbstractGeometry *geom )
{
  return const_cast<T>( reinterpret_cast<T>( 0 )->cast( geom ) );
}

#endif

// clazy:excludeall=qstring-allocations

/**
 * \ingroup core
 * \brief Java-style iterator for traversal of vertices of a geometry
 * \since QGIS 3.0
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

    //! Returns next vertex of the geometry (undefined behavior if hasNext() returns false before calling next())
    QgsPoint next();

#ifdef SIP_RUN
    QgsVertexIterator *__iter__();
    % MethodCode
    sipRes = sipCpp;
    % End

    SIP_PYOBJECT __next__();
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

#endif //QGSABSTRACTGEOMETRYV2

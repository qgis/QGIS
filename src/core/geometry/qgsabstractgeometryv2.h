/***************************************************************************
                        qgsabstractgeometryv2.h
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

#include "qgscoordinatetransform.h"
#include "qgsrectangle.h"
#include "qgswkbtypes.h"
#include "qgswkbptr.h"

#include <QString>

class QgsMapToPixel;
class QgsCurveV2;
class QgsMultiCurveV2;
class QgsMultiPointV2;
class QgsPointV2;
struct QgsVertexId;
class QPainter;

typedef QList< QgsPointV2 > QgsPointSequenceV2;
typedef QList< QgsPointSequenceV2 > QgsRingSequenceV2;
typedef QList< QgsRingSequenceV2 > QgsCoordinateSequenceV2;

/** \ingroup core
 * \class QgsAbstractGeometryV2
 * \brief Abstract base class for all geometries
 * \note added in QGIS 2.10
 */
class CORE_EXPORT QgsAbstractGeometryV2
{
  public:

    /** Segmentation tolerance as maximum angle or maximum difference between approximation and circle*/
    enum SegmentationToleranceType
    {
      MaximumAngle = 0,
      MaximumDifference
    };

    QgsAbstractGeometryV2();
    virtual ~QgsAbstractGeometryV2();
    QgsAbstractGeometryV2( const QgsAbstractGeometryV2& geom );
    virtual QgsAbstractGeometryV2& operator=( const QgsAbstractGeometryV2& geom );

    /** Clones the geometry by performing a deep copy
     */
    virtual QgsAbstractGeometryV2* clone() const = 0;

    /** Clears the geometry, ie reset it to a null geometry
     */
    virtual void clear() = 0;

    /** Returns the minimal bounding box for the geometry
     */
    virtual QgsRectangle boundingBox() const = 0;

    //mm-sql interface
    /** Returns the inherent dimension of the geometry. For example, this is 0 for a point geometry,
     * 1 for a linestring and 2 for a polygon.
     */
    virtual int dimension() const = 0;
    //virtual int coordDim() const { return mCoordDimension; }

    /** Returns a unique string representing the geometry type.
     * @see wkbType
     * @see wktTypeStr
     */
    virtual QString geometryType() const = 0;

    /** Returns the WKB type of the geometry.
     * @see geometryType
     * @see wktTypeStr
     */
    QgsWKBTypes::Type wkbType() const { return mWkbType; }

    /** Returns the WKT type string of the geometry.
     * @see geometryType
     * @see wkbType
     */
    QString wktTypeStr() const;

    /** Returns true if the geometry is 3D and contains a z-value.
     * @see isMeasure
     */
    bool is3D() const;

    /** Returns true if the geometry contains m values.
     * @see is3D
     */
    bool isMeasure() const;

#if 0
    virtual bool transform( const QgsCoordinateTransform& ct ) =  0;
    virtual bool isEmpty() const = 0;
    virtual bool isSimple() const = 0;
    virtual bool isValid() const = 0;
    virtual QgsMultiPointV2* locateAlong() const = 0;
    virtual QgsMultiCurveV2* locateBetween() const = 0;
    virtual QgsCurveV2* boundary() const = 0;
    virtual QgsRectangle envelope() const = 0;
#endif

    //import

    /** Sets the geometry from a WKB string.
     * @see fromWkt
     */
    virtual bool fromWkb( QgsConstWkbPtr wkb ) = 0;

    /** Sets the geometry from a WKT string.
     * @see fromWkb
     */
    virtual bool fromWkt( const QString& wkt ) = 0;

    //export

    /** Returns the size of the WKB representation of the geometry.
     * @see asWkb
     */
    virtual int wkbSize() const = 0;

    /** Returns a WKB representation of the geometry.
     * @param binarySize will be set to the size of the returned WKB string
     * @see wkbSize
     * @see asWkt
     * @see asGML2
     * @see asGML3
     * @see asJSON
     */
    virtual unsigned char* asWkb( int& binarySize ) const = 0;

    /** Returns a WKT representation of the geometry.
     * @param precision number of decimal places for coordinates
     * @see asWkb
     * @see asGML2
     * @see asGML3
     * @see asJSON
     */
    virtual QString asWkt( int precision = 17 ) const = 0;

    /** Returns a GML2 representation of the geometry.
     * @param doc DOM document
     * @param precision number of decimal places for coordinates
     * @param ns XML namespace
     * @see asWkb
     * @see asWkt
     * @see asGML3
     * @see asJSON
     */
    virtual QDomElement asGML2( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const = 0;

    /** Returns a GML3 representation of the geometry.
     * @param doc DOM document
     * @param precision number of decimal places for coordinates
     * @param ns XML namespace
     * @see asWkb
     * @see asWkt
     * @see asGML2
     * @see asJSON
     */
    virtual QDomElement asGML3( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const = 0;

    /** Returns a GeoJSON representation of the geometry.
     * @param precision number of decimal places for coordinates
     * @see asWkb
     * @see asWkt
     * @see asGML2
     * @see asGML3
     */
    virtual QString asJSON( int precision = 17 ) const = 0;

    //render pipeline

    /** Transforms the geometry using a coordinate transform
     * @param ct coordinate transform
     * @param d transformation direction
     * @param transformZ set to true to also transform z coordinates. This requires that
     * the z coordinates in the geometry represent height relative to the vertical datum
     * of the source CRS (generally ellipsoidal heights) and are expressed in its vertical
     * units (generally meters). If false, then z coordinates will not be changed by the
     * transform.
     */
    virtual void transform( const QgsCoordinateTransform& ct, QgsCoordinateTransform::TransformDirection d = QgsCoordinateTransform::ForwardTransform,
                            bool transformZ = false ) = 0;

    /** Transforms the geometry using a QTransform object
     * @param t QTransform transformation
     */
    virtual void transform( const QTransform& t ) = 0;

#if 0
    virtual void clip( const QgsRectangle& rect ); //todo
#endif

    /** Draws the geometry using the specified QPainter.
     * @param p destination QPainter
     */
    virtual void draw( QPainter& p ) const = 0;

    /** Returns next vertex id and coordinates
     * @param id initial value should be the starting vertex id. The next vertex id will be stored
     * in this variable if found.
     * @param vertex container for found node
     * @return false if at end
     */
    virtual bool nextVertex( QgsVertexId& id, QgsPointV2& vertex ) const = 0;

    /** Retrieves the sequence of geometries, rings and nodes.
     * @return coordinate sequence
     */
    virtual QgsCoordinateSequenceV2 coordinateSequence() const = 0;

    /** Returns the number of nodes contained in the geometry
     */
    int nCoordinates() const;

    /** Returns the point corresponding to a specified vertex id
     */
    virtual QgsPointV2 vertexAt( QgsVertexId id ) const = 0;

    /** Searches for the closest segment of the geometry to a given point.
     * @param pt specifies the point to find closest segment to
     * @param segmentPt storage for the closest point within the geometry
     * @param vertexAfter storage for the ID of the vertex at the end of the closest segment
     * @param leftOf returns whether the point lies on the left side of the nearest segment (true if point is to left of segment,
     * false if point is to right of segment)
     * @param epsilon epsilon for segment snapping
     * @returns squared distance to closest segment
     */
    virtual double closestSegment( const QgsPointV2& pt, QgsPointV2& segmentPt, QgsVertexId& vertexAfter, bool* leftOf, double epsilon ) const = 0;

    //low-level editing

    /** Inserts a vertex into the geometry
     * @param position vertex id for position of inserted vertex
     * @param vertex vertex to insert
     * @returns true if insert was successful
     * @see moveVertex
     * @see deleteVertex
     */
    virtual bool insertVertex( QgsVertexId position, const QgsPointV2& vertex ) = 0;

    /** Moves a vertex within the geometry
     * @param position vertex id for vertex to move
     * @param newPos new position of vertex
     * @returns true if move was successful
     * @see insertVertex
     * @see deleteVertex
     */
    virtual bool moveVertex( QgsVertexId position, const QgsPointV2& newPos ) = 0;

    /** Deletes a vertex within the geometry
     * @param position vertex id for vertex to delete
     * @returns true if delete was successful
     * @see insertVertex
     * @see moveVertex
     */
    virtual bool deleteVertex( QgsVertexId position ) = 0;

    /** Returns the length of the geometry.
     * @see area()
     * @see perimeter()
     */
    virtual double length() const { return 0.0; }

    /** Returns the perimeter of the geometry.
     * @see area()
     * @see length()
     */
    virtual double perimeter() const { return 0.0; }

    /** Returns the area of the geometry.
     * @see length()
     * @see perimeter()
     */
    virtual double area() const { return 0.0; }

    /** Returns the centroid of the geometry */
    virtual QgsPointV2 centroid() const;

    /** Returns true if the geometry is empty
     */
    bool isEmpty() const;

    /** Returns true if the geometry contains curved segments
     */
    virtual bool hasCurvedSegments() const { return false; }

    /** Returns a version of the geometry without curves. Caller takes ownership of
     * the returned geometry.
     * @param tolerance segmentation tolerance
     * @param toleranceType maximum segmentation angle or maximum difference between approximation and curve
     */
    virtual QgsAbstractGeometryV2* segmentize( double tolerance = M_PI / 180., SegmentationToleranceType toleranceType = MaximumAngle ) const;

    /** Returns the geometry converted to the more generic curve type.
        E.g. QgsLineStringV2 -> QgsCompoundCurveV2, QgsPolygonV2 -> QgsCurvePolygonV2,
        QgsMultiLineStringV2 -> QgsMultiCurveV2, QgsMultiPolygonV2 -> QgsMultiSurfaceV2
        @return the converted geometry. Caller takes ownership*/
    virtual QgsAbstractGeometryV2* toCurveType() const { return 0; }

    /** Returns approximate angle at a vertex. This is usually the average angle between adjacent
     * segments, and can be pictured as the orientation of a line following the curvature of the
     * geometry at the specified vertex.
     * @param vertex the vertex id
     * @return rotation in radians, clockwise from north
     */
    virtual double vertexAngle( QgsVertexId vertex ) const = 0;

    virtual int vertexCount( int part = 0, int ring = 0 ) const = 0;
    virtual int ringCount( int part = 0 ) const = 0;

    /** Returns count of parts contained in the geometry.
     * @see vertexCount
     * @see ringCount
     */
    virtual int partCount() const = 0;

    /** Adds a z-dimension to the geometry, initialized to a preset value.
     * @param zValue initial z-value for all nodes
     * @returns true on success
     * @note added in QGIS 2.12
     * @see dropZValue()
     * @see addMValue()
     */
    virtual bool addZValue( double zValue = 0 ) = 0;

    /** Adds a measure to the geometry, initialized to a preset value.
     * @param mValue initial m-value for all nodes
     * @returns true on success
     * @note added in QGIS 2.12
     * @see dropMValue()
     * @see addZValue()
     */
    virtual bool addMValue( double mValue = 0 ) = 0;

    /** Drops any z-dimensions which exist in the geometry.
     * @returns true if Z values were present and have been removed
     * @see addZValue()
     * @see dropMValue()
     * @note added in QGIS 2.14
     */
    virtual bool dropZValue() = 0;

    /** Drops any measure values which exist in the geometry.
     * @returns true if m-values were present and have been removed
     * @see addMValue()
     * @see dropZValue()
     * @note added in QGIS 2.14
     */
    virtual bool dropMValue() = 0;

    /** Converts the geometry to a specified type.
     * @returns true if conversion was successful
     * @note added in QGIS 2.14
     */
    virtual bool convertTo( QgsWKBTypes::Type type );

  protected:
    QgsWKBTypes::Type mWkbType;

    /** Updates the geometry type based on whether sub geometries contain z or m values.
     */
    void setZMTypeFromSubGeometry( const QgsAbstractGeometryV2* subggeom, QgsWKBTypes::Type baseGeomType );

    /** Default calculator for the minimal bounding box for the geometry. Derived classes should override this method
     * if a more efficient bounding box calculation is available.
     */
    virtual QgsRectangle calculateBoundingBox() const;

    /** Clears any cached parameters associated with the geometry, eg bounding boxes
     */
    virtual void clearCache() const {}

};


/** \ingroup core
 * \class QgsVertexId
 * \brief Utility class for identifying a unique vertex within a geometry.
 * \note added in QGIS 2.10
 */
struct CORE_EXPORT QgsVertexId
{
  enum VertexType
  {
    SegmentVertex = 1, //start / endpoint of a segment
    CurveVertex
  };

  QgsVertexId( int _part = -1, int _ring = -1, int _vertex = -1, VertexType _type = SegmentVertex )
      : part( _part )
      , ring( _ring )
      , vertex( _vertex )
      , type( _type )
  {}

  /** Returns true if the vertex id is valid
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
  bool isValid( const QgsAbstractGeometryV2* geom ) const
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

#endif //QGSABSTRACTGEOMETRYV2

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
#include <QString>

class QgsCoordinateTransform;
class QgsMapToPixel;
class QgsCurveV2;
class QgsMultiCurveV2;
class QgsMultiPointV2;
class QgsPointV2;
class QgsConstWkbPtr;
class QgsWkbPtr;
class QPainter;

/** \ingroup core
 * \class QgsVertexId
 * \brief Utility class for identifying a unique vertex within a geometry.
 * \note added in QGIS 2.10
 * \note this API is not considered stable and may change for 2.12
 */
struct CORE_EXPORT QgsVertexId
{
  enum VertexType
  {
    SegmentVertex = 1, //start / endpoint of a segment
    CurveVertex
  };

  QgsVertexId(): part( - 1 ), ring( -1 ), vertex( -1 ), type( SegmentVertex ) {}
  QgsVertexId( int _part, int _ring, int _vertex, VertexType _type = SegmentVertex )
      : part( _part ), ring( _ring ), vertex( _vertex ), type( _type ) {}

  /** Returns true if the vertex id is valid
   */
  bool isValid() const { return part >= 0 && ring >= 0 && vertex >= 0; }

  bool operator==( const QgsVertexId& other )
  {
    return part == other.part && ring == other.ring && vertex == other.vertex;
  }
  bool operator!=( const QgsVertexId& other )
  {
    return part != other.part || ring != other.ring || vertex != other.vertex;
  }

  int part;
  int ring;
  int vertex;
  VertexType type;
};

/** \ingroup core
 * \class QgsAbstractGeometryV2
 * \brief Abstract base class for all geometries
 * \note added in QGIS 2.10
 */
class CORE_EXPORT QgsAbstractGeometryV2
{
  public:
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
    QgsRectangle boundingBox() const;

    /** Calculates the minimal bounding box for the geometry. Derived classes should override this method
     * to return the correct bounding box.
     */
    virtual QgsRectangle calculateBoundingBox() const;

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
    virtual bool fromWkb( const unsigned char * wkb ) = 0;

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
       @param d transformation direction
     */
    virtual void transform( const QgsCoordinateTransform& ct, QgsCoordinateTransform::TransformDirection d = QgsCoordinateTransform::ForwardTransform ) = 0;

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
     * @param coord destination for coordinate sequence.
     */
    virtual void coordinateSequence( QList< QList< QList< QgsPointV2 > > >& coord ) const = 0;

    /** Returns the number of nodes contained in the geometry
     */
    int nCoordinates() const;

    /** Returns the point corresponding to a specified vertex id
     */
    QgsPointV2 vertexAt( const QgsVertexId& id ) const;

    /** Searches for the closest segment of the geometry to a given point.
     * @param pt Specifies the point for search
     * @param segmentPt storage for the closest point within the geometry
     * @param vertexAfter storage for the id of the vertex after the closest segment
     * @param leftOf returns if the point lies on the left of right side of the segment ( < 0 means left, > 0 means right )
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
    virtual bool insertVertex( const QgsVertexId& position, const QgsPointV2& vertex ) = 0;

    /** Moves a vertex within the geometry
     * @param position vertex id for vertex to move
     * @param newPos new position of vertex
     * @returns true if move was successful
     * @see insertVertex
     * @see deleteVertex
     */
    virtual bool moveVertex( const QgsVertexId& position, const QgsPointV2& newPos ) = 0;

    /** Deletes a vertex within the geometry
     * @param position vertex id for vertex to delete
     * @returns true if delete was successful
     * @see insertVertex
     * @see moveVertex
     */
    virtual bool deleteVertex( const QgsVertexId& position ) = 0;

    /** Returns the length (or perimeter for area geometries) of the geometry.
     * @see area
     */
    virtual double length() const { return 0.0; }

    /** Returns the area of the geometry.
     * @see length
     */
    virtual double area() const { return 0.0; }

    /** Returns true if the geometry is empty
     */
    bool isEmpty() const;

    /** Returns true if the geometry contains curved segments
     */
    virtual bool hasCurvedSegments() const { return false; }

    /** Returns a version of the geometry without curves. Caller takes ownership of
     * the returned geometry.
     */
    virtual QgsAbstractGeometryV2* segmentize() const { return clone(); }

    /** Returns approximate rotation angle for a vertex. Usually average angle between adjacent segments.
        @param vertex the vertex id
        @return rotation in radians, clockwise from north*/
    virtual double vertexAngle( const QgsVertexId& vertex ) const = 0;

  protected:
    QgsWKBTypes::Type mWkbType;
    mutable QgsRectangle mBoundingBox;

    /** Updates the geometry type based on whether sub geometries contain z or m values.
     */
    void setZMTypeFromSubGeometry( const QgsAbstractGeometryV2* subggeom, QgsWKBTypes::Type baseGeomType );

    /** Reads a WKB header and tests its validity.
     * @param wkbPtr
     * @param wkbType destination for WKB type from header
     * @param endianSwap will be set to true if endian from WKB must be swapped to match QGIS platform endianness
     * @param expectedType expected WKB type
     * @returns true if header is valid and matches expected type
     */
    static bool readWkbHeader( QgsConstWkbPtr& wkbPtr, QgsWKBTypes::Type& wkbType, bool& endianSwap, QgsWKBTypes::Type expectedType );

};

#endif //QGSABSTRACTGEOMETRYV2

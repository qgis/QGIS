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

/**Abstract base class for all geometries*/
class CORE_EXPORT QgsAbstractGeometryV2
{
  public:
    QgsAbstractGeometryV2();
    virtual ~QgsAbstractGeometryV2();
    QgsAbstractGeometryV2( const QgsAbstractGeometryV2& geom );
    virtual QgsAbstractGeometryV2& operator=( const QgsAbstractGeometryV2& geom );

    virtual QgsAbstractGeometryV2* clone() const = 0;
    virtual void clear() = 0;

    QgsRectangle boundingBox() const;

    //mm-sql interface
    virtual int dimension() const = 0;
    //virtual int coordDim() const { return mCoordDimension; }
    virtual QString geometryType() const = 0;
    QgsWKBTypes::Type wkbType() const { return mWkbType; }
    QString wktTypeStr() const;
    bool is3D() const;
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
    virtual bool fromWkb( const unsigned char * wkb ) = 0;
    virtual bool fromWkt( const QString& wkt ) = 0;

    //export
    virtual int wkbSize() const = 0;
    virtual unsigned char* asWkb( int& binarySize ) const = 0;
    virtual QString asWkt( int precision = 17 ) const = 0;
    virtual QDomElement asGML2( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const = 0;
    virtual QDomElement asGML3( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const = 0;
    virtual QString asJSON( int precision = 17 ) const = 0;

    virtual QgsRectangle calculateBoundingBox() const;

    //render pipeline
    virtual void transform( const QgsCoordinateTransform& ct ) = 0;
    virtual void transform( const QTransform& t ) = 0;
    virtual void clip( const QgsRectangle& rect ) { Q_UNUSED( rect ); } //todo
    virtual void draw( QPainter& p ) const = 0;

    /**Returns next vertex id and coordinates
    @return false if at end*/
    virtual bool nextVertex( QgsVertexId& id, QgsPointV2& vertex ) const = 0;

    virtual void coordinateSequence( QList< QList< QList< QgsPointV2 > > >& coord ) const = 0;
    int nCoordinates() const;
    QgsPointV2 vertexAt( const QgsVertexId& id ) const;
    virtual double closestSegment( const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon ) const = 0;

    //low-level editing
    virtual bool insertVertex( const QgsVertexId& position, const QgsPointV2& vertex ) = 0;
    virtual bool moveVertex( const QgsVertexId& position, const QgsPointV2& newPos ) = 0;
    virtual bool deleteVertex( const QgsVertexId& position ) = 0;

    /**Length for linear geometries,perimeter for area geometries*/
    virtual double length() const { return 0.0; }
    virtual double area() const { return 0.0; }

    bool isEmpty() const;

    virtual bool hasCurvedSegments() const { return false; }
    /**Returns a geometry without curves. Caller takes ownership*/
    virtual QgsAbstractGeometryV2* segmentize() const { return clone(); }

  protected:
    QgsWKBTypes::Type mWkbType;
    mutable QgsRectangle mBoundingBox;

    void setZMTypeFromSubGeometry( const QgsAbstractGeometryV2* subggeom, QgsWKBTypes::Type baseGeomType );

    static bool readWkbHeader( QgsConstWkbPtr& wkbPtr, QgsWKBTypes::Type& wkbType, bool& endianSwap, QgsWKBTypes::Type expectedType );
};

#endif //QGSABSTRACTGEOMETRYV2

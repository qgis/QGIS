struct QgsVertexId
{
%TypeHeaderCode
#include <qgsabstractgeometryv2.h>
%End

    enum VertexType
    {
      SegmentVertex, //start / endpoint of a segment
      CurveVertex
    };

  QgsVertexId();
  QgsVertexId( int _part, int _ring, int _vertex, VertexType _type );

  bool isValid() const;
  int part;
  int ring;
  int vertex;
  VertexType type;
};

class QgsAbstractGeometryV2
{
%TypeHeaderCode
#include <qgsabstractgeometryv2.h>
#include <qgslinestringv2.h>
%End

%ConvertToSubClassCode
      if (dynamic_cast<QgsPointV2*>(sipCpp) != NULL)
        sipClass = sipClass_QgsPointV2;
      else if (dynamic_cast<QgsLineStringV2*>(sipCpp) != NULL )
        sipClass = sipClass_QgsLineStringV2;
      else if (dynamic_cast<QgsCircularStringV2*>(sipCpp) != NULL )
        sipClass = sipClass_QgsCircularStringV2;
      else if (dynamic_cast<QgsCompoundCurveV2*>(sipCpp) != NULL )
        sipClass = sipClass_QgsCompoundCurveV2;
      else if (dynamic_cast<QgsPolygonV2*>(sipCpp) != NULL )
        sipClass = sipClass_QgsPolygonV2;
      else if (dynamic_cast<QgsCurvePolygonV2*>(sipCpp) != NULL )
        sipClass = sipClass_QgsCurvePolygonV2;
      else if (dynamic_cast<QgsMultiPointV2*>(sipCpp) != NULL )
        sipClass = sipClass_QgsMultiPointV2;
      else if (dynamic_cast<QgsLineStringV2*>(sipCpp) != NULL )
        sipClass = sipClass_QgsLineStringV2;
      else if (dynamic_cast<QgsMultiPolygonV2*>(sipCpp) != NULL )
        sipClass = sipClass_QgsMultiPolygonV2;
      else if (dynamic_cast<QgsMultiSurfaceV2*>(sipCpp) != NULL )
        sipClass = sipClass_QgsMultiSurfaceV2;
      else if (dynamic_cast<QgsMultiCurveV2*>(sipCpp) != NULL )
        sipClass = sipClass_QgsMultiCurveV2;
      else
        sipClass = 0;
%End

    public:

    QgsAbstractGeometryV2();
    virtual ~QgsAbstractGeometryV2();
    QgsAbstractGeometryV2( const QgsAbstractGeometryV2& geom );
    //virtual QgsAbstractGeometryV2& operator=( const QgsAbstractGeometryV2& geom );

    virtual QgsAbstractGeometryV2* clone() const = 0;
    virtual void clear() = 0;

    QgsRectangle boundingBox() const;

    //mm-sql interface
    virtual int dimension() const = 0;
    //virtual int coordDim() const { return mCoordDimension; }
    virtual QString geometryType() const = 0;
    QgsWKBTypes::Type wkbType() const;
    QString wktTypeStr() const;
    bool is3D() const;
    bool isMeasure() const;

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
    virtual void transform( const QgsCoordinateTransform& ct, QgsCoordinateTransform::TransformDirection d = QgsCoordinateTransform::ForwardTransform ) = 0;
    virtual void transform( const QTransform& t ) = 0;
    //virtual void clip( const QgsRectangle& rect );
    virtual void draw( QPainter& p ) const = 0;

    /** Returns next vertex id and coordinates
    @return false if at end*/
    virtual bool nextVertex( QgsVertexId& id, QgsPointV2& vertex ) const = 0;

    virtual void coordinateSequence( QList< QList< QList< QgsPointV2 > > >& coord /Out/ ) const = 0;
    int nCoordinates() const;
    QgsPointV2 vertexAt( const QgsVertexId& id ) const;
    virtual double closestSegment( const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon ) const = 0;

    //low-level editing
    virtual bool insertVertex( const QgsVertexId& position, const QgsPointV2& vertex ) = 0;
    virtual bool moveVertex( const QgsVertexId& position, const QgsPointV2& newPos ) = 0;
    virtual bool deleteVertex( const QgsVertexId& position ) = 0;

    /** Length for linear geometries,perimeter for area geometries*/
    virtual double length() const;
    virtual double area() const;

    virtual bool hasCurvedSegments() const;
    /** Returns a geometry without curves. Caller takes ownership*/
    virtual QgsAbstractGeometryV2* segmentize() const /Factory/;
};

class QgsPointV2: public QgsAbstractGeometryV2
{
%TypeHeaderCode
#include <qgspointv2.h>
%End

    public:
      QgsPointV2( double x = 0.0, double y = 0.0 );
      QgsPointV2( QgsWKBTypes::Type type, double x = 0.0, double y = 0.0, double z = 0.0, double m = 0.0 );

      bool operator==( const QgsPointV2& pt ) const;
      bool operator!=( const QgsPointV2& pt ) const;

      virtual QgsAbstractGeometryV2* clone() const;
      void clear();

      double x() const;
      double y() const;
      double z() const;
      double m() const;

      void setX( double x );
      void setY( double y );
      void setZ( double z );
      void setM( double m );

      virtual QString geometryType() const;

      //implementation of inherited methods
      virtual int dimension() const;


      virtual bool fromWkb( const unsigned char* wkb );
      virtual bool fromWkt( const QString& wkt );

      int wkbSize() const;
      unsigned char* asWkb( int& binarySize ) const;
      QString asWkt( int precision = 17 ) const;
      QDomElement asGML2( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const;
      QDomElement asGML3( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const;
      QString asJSON( int precision = 17 ) const;

      virtual QgsRectangle calculateBoundingBox() const;

      void draw( QPainter& p ) const;
      void transform( const QgsCoordinateTransform& ct );
      void transform( const QTransform& t );

      virtual void coordinateSequence( QList< QList< QList< QgsPointV2 > > >& coord /Out/ ) const;

      //low-level editing
      virtual bool insertVertex( const QgsVertexId& position, const QgsPointV2& vertex );
      virtual bool moveVertex( const QgsVertexId& position, const QgsPointV2& newPos );
      virtual bool deleteVertex( const QgsVertexId& position );

      double closestSegment( const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon ) const;
      bool nextVertex( QgsVertexId& id, QgsPointV2& vertex ) const;

      /** Angle undefined. Always returns 0.0
          @param vertex the vertex id
          @return 0.0*/
      double vertexAngle( const QgsVertexId& vertex ) const;
};

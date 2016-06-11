class QgsPolygonV2: public QgsCurvePolygonV2
{
%TypeHeaderCode
#include <qgspolygonv2.h>
%End

  public:
    QgsPolygonV2();

    bool operator==( const QgsPolygonV2& other ) const;
    bool operator!=( const QgsPolygonV2& other ) const;

    virtual QString geometryType() const;
    virtual QgsPolygonV2* clone() const;
    void clear();

    virtual bool fromWkb( QgsConstWkbPtr wkb );

    // inherited: bool fromWkt( const QString &wkt );

    int wkbSize() const;
    unsigned char* asWkb( int& binarySize ) const;
    // inherited: QString asWkt( int precision = 17 ) const;
    // inherited: QDomElement asGML2( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const;
    // inherited: QDomElement asGML3( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const;
    // inherited: QString asJSON( int precision = 17 ) const;

    QgsPolygonV2* surfaceToPolygon() const;

    /** Returns the geometry converted to the more generic curve type QgsCurvePolygonV2
     @return the converted geometry. Caller takes ownership*/
    QgsAbstractGeometryV2* toCurveType() const /Factory/;

    void addInteriorRing( QgsCurveV2* ring /Transfer/ );
    //overridden to handle LineString25D rings
    virtual void setExteriorRing( QgsCurveV2* ring /Transfer/ );

};

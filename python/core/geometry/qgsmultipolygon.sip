class QgsMultiPolygonV2: public QgsMultiSurface
{
%TypeHeaderCode
#include <qgsmultipolygon.h>
%End
  public:
    virtual QString geometryType() const;
    virtual QgsMultiPolygonV2* clone() const;

    bool fromWkt( const QString& wkt );

    // inherited: int wkbSize() const;
    // inherited: unsigned char* asWkb( int& binarySize ) const;
    // inherited: QString asWkt( int precision = 17 ) const;
    QDomElement asGML2( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const;
    QDomElement asGML3( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const;
    QString asJSON( int precision = 17 ) const;


    /** Adds a geometry and takes ownership. Returns true in case of success*/
    virtual bool addGeometry( QgsAbstractGeometry* g );

    /** Returns the geometry converted to the more generic curve type QgsMultiSurface
    @return the converted geometry. Caller takes ownership*/
    QgsAbstractGeometry* toCurveType() const /Factory/;

    virtual QgsAbstractGeometry* boundary() const /Factory/;

  protected:

    virtual bool wktOmitChildType() const;
};

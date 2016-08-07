class QgsMultiCurve: public QgsGeometryCollection
{
%TypeHeaderCode
#include <qgsmulticurve.h>
%End

  public:
    virtual QString geometryType() const;
    virtual QgsMultiCurve* clone() const;

    bool fromWkt( const QString& wkt );

    // inherited: int wkbSize() const;
    // inherited: unsigned char* asWkb( int& binarySize ) const;
    // inherited: QString asWkt( int precision = 17 ) const;
    QDomElement asGML2( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const;
    QDomElement asGML3( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const;
    QString asJSON( int precision = 17 ) const;

    /** Adds a geometry and takes ownership. Returns true in case of success*/
    virtual bool addGeometry( QgsAbstractGeometry* g );
    /** Returns a geometry without curves. Caller takes ownership*/
    QgsAbstractGeometry* segmentize() const /Factory/;

    /** Returns a copy of the multi curve, where each component curve has had its line direction reversed.
     * @note added in QGIS 2.14
     */
    QgsMultiCurve* reversed() const /Factory/;

    virtual QgsAbstractGeometry* boundary() const /Factory/;
};

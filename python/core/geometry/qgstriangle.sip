class QgsTriangle : public QgsPolygonV2
{
%TypeHeaderCode
#include <qgstriangle.h>
%End

  public:
    QgsTriangle();
    QgsTriangle( const QgsPointV2 &p1, const QgsPointV2 &p2, const QgsPointV2 &p3 );

    // inherited: bool operator==( const QgsTriangle& other ) const;
    // inherited: bool operator!=( const QgsTriangle& other ) const;

    virtual QString geometryType() const;
    virtual QgsTriangle* clone() const /Factory/;
    void clear();

    virtual bool fromWkb( QgsConstWkbPtr& wkbPtr );
    bool fromWkt( const QString &wkt );
    // inherited: QString asWkt( int precision = 17 ) const;
    // inherited: QDomElement asGML2( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const;
    // inherited: QDomElement asGML3( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const;
    // inherited: QString asJSON( int precision = 17 ) const;

    // inherited: QgsPolygonV2* surfaceToPolygon() const;
    QgsAbstractGeometry* toCurveType() const /Factory/;
    void addInteriorRing( QgsCurve* ring /Transfer/ ); // NOTE: no interior ring for triangle.
    //overridden to handle LineString25D rings
    virtual void setExteriorRing( QgsCurve* ring /Transfer/ );
    virtual QgsAbstractGeometry* boundary() const /Factory/;
    // inherited: double pointDistanceToBoundary( double x, double y ) const;
    QgsPointV2 vertexAt( int atVertex ) const;

    QVector<double> lengths() const;
    QVector<double> angles() const;

    bool isIsocele( double lengthTolerance = 0.0001 ) const;
    bool isEquilateral( double lengthTolerance = 0.0001 ) const;
    bool isRight( double angleTolerance = 0.0001 ) const;
    bool isScalene( double lengthTolerance = 0.0001 ) const;

    QVector<QgsLineString> altitudes( ) const;
    QVector<QgsLineString> medians( ) const;
    QVector<QgsLineString> bisectors( double lengthTolerance = 0.0001 ) const;

    QgsTriangle medial( ) const;
    QgsPointV2 orthocenter( double lengthTolerance = 0.0001 ) const;
    QgsPointV2 circumscribedCenter( ) const;
    double circumscribedRadius( ) const;
    // TODO:
    // QgsCircle circumscribedCircle ( ) const; // need QgsCircle (from CADDigitize.CADCircle)
    QgsPointV2 inscribedCenter( ) const;
    double inscribedRadius( ) const;
    // TODO:
    // QgsCircle inscribedCircle ( ) const; // need QgsCircle (from CADDigitize.CADCircle)
};

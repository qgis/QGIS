/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgsmultipoint.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsMultiPoint: QgsGeometryCollection
{
%Docstring
Multi point geometry collection.

.. versionadded:: 2.10
%End

%TypeHeaderCode
#include "qgsmultipoint.h"
%End
  public:
    QgsMultiPoint();

    virtual QString geometryType() const;

    virtual QgsMultiPoint *clone() const /Factory/;

    virtual QgsMultiPoint *toCurveType() const /Factory/;

    virtual bool fromWkt( const QString &wkt );

    virtual void clear();

    virtual QDomElement asGml2( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const;

    virtual QDomElement asGml3( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const;

    virtual QString asJson( int precision = 17 ) const;

    virtual int nCoordinates() const;

    virtual bool addGeometry( QgsAbstractGeometry *g /Transfer/ );

    virtual bool insertGeometry( QgsAbstractGeometry *g /Transfer/, int index );

    virtual QgsAbstractGeometry *boundary() const /Factory/;

    virtual int vertexNumberFromVertexId( QgsVertexId id ) const;

    virtual double segmentLength( QgsVertexId startVertex ) const;

    virtual bool isValid( QString &error /Out/, int flags = 0 ) const;



    virtual QgsMultiPoint *createEmptyWithSameType() const /Factory/;


    SIP_PYOBJECT __repr__();
%MethodCode
    QString wkt = sipCpp->asWkt();
    if ( wkt.length() > 1000 )
      wkt = wkt.left( 1000 ) + QStringLiteral( "..." );
    QString str = QStringLiteral( "<QgsMultiPoint: %1>" ).arg( wkt );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
%End

  protected:

    virtual bool wktOmitChildType() const;


};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgsmultipoint.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/

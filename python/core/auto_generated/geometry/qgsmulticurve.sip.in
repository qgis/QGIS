/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgsmulticurve.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsMultiCurve: QgsGeometryCollection
{
%Docstring
Multi curve geometry collection.

.. versionadded:: 2.10
%End

%TypeHeaderCode
#include "qgsmulticurve.h"
%End
  public:
    QgsMultiCurve();
    virtual QString geometryType() const;

    virtual QgsMultiCurve *clone() const /Factory/;

    virtual void clear();

    virtual QgsMultiCurve *toCurveType() const /Factory/;

    virtual bool fromWkt( const QString &wkt );

    virtual QDomElement asGml2( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const;

    virtual QDomElement asGml3( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const;

    virtual QString asJson( int precision = 17 ) const;

    virtual bool addGeometry( QgsAbstractGeometry *g /Transfer/ );

    virtual bool insertGeometry( QgsAbstractGeometry *g /Transfer/, int index );


    QgsMultiCurve *reversed() const /Factory/;
%Docstring
Returns a copy of the multi curve, where each component curve has had its line direction reversed.

.. versionadded:: 2.14
%End

    virtual QgsAbstractGeometry *boundary() const /Factory/;



    virtual QgsMultiCurve *createEmptyWithSameType() const /Factory/;


    SIP_PYOBJECT __repr__();
%MethodCode
    QString wkt = sipCpp->asWkt();
    if ( wkt.length() > 1000 )
      wkt = wkt.left( 1000 ) + QStringLiteral( "..." );
    QString str = QStringLiteral( "<QgsMultiCurve: %1>" ).arg( wkt );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgsmulticurve.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/

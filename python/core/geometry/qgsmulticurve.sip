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


    virtual bool fromWkt( const QString &wkt );


    virtual QDomElement asGML2( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const;

    virtual QDomElement asGML3( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const;

    virtual QString asJSON( int precision = 17 ) const;


    virtual bool addGeometry( QgsAbstractGeometry *g /Transfer/ );
%Docstring
Adds a geometry and takes ownership. Returns true in case of success
 :rtype: bool
%End

    QgsMultiCurve *reversed() const /Factory/;
%Docstring
 Returns a copy of the multi curve, where each component curve has had its line direction reversed.
.. versionadded:: 2.14
 :rtype: QgsMultiCurve
%End

    virtual QgsAbstractGeometry *boundary() const /Factory/;


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgsmulticurve.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgsmultipolygon.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsMultiPolygonV2: QgsMultiSurface
{
%Docstring
 Multi polygon geometry collection.
.. versionadded:: 2.10
%End

%TypeHeaderCode
#include "qgsmultipolygon.h"
%End
  public:
    QgsMultiPolygonV2();
    virtual QString geometryType() const;
    virtual QgsMultiPolygonV2 *clone() const /Factory/;


    virtual bool fromWkt( const QString &wkt );


    virtual QDomElement asGML2( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const;

%Docstring
inherited: QString asWkt( int precision = 17 ) const;
 :rtype: QDomElement
%End
    virtual QDomElement asGML3( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const;

    virtual QString asJSON( int precision = 17 ) const;


    virtual bool addGeometry( QgsAbstractGeometry *g /Transfer/ );
%Docstring
Adds a geometry and takes ownership. Returns true in case of success
 :rtype: bool
%End

    virtual QgsAbstractGeometry *toCurveType() const /Factory/;

%Docstring
 Returns the geometry converted to the more generic curve type QgsMultiSurface
:return: the converted geometry. Caller takes ownership*
 :rtype: QgsAbstractGeometry
%End

    virtual QgsAbstractGeometry *boundary() const /Factory/;

  protected:

    virtual bool wktOmitChildType() const;
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgsmultipolygon.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/

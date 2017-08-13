/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgsmultisurface.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsMultiSurface: QgsGeometryCollection
{
%Docstring
 Multi surface geometry collection.
.. versionadded:: 2.10
%End

%TypeHeaderCode
#include "qgsmultisurface.h"
%End
  public:
    QgsMultiSurface();
    virtual QString geometryType() const;
    virtual QgsMultiSurface *clone() const /Factory/;


    virtual bool fromWkt( const QString &wkt );


    virtual QDomElement asGML2( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const;

    virtual QDomElement asGML3( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const;

    virtual QString asJSON( int precision = 17 ) const;



    virtual bool addGeometry( QgsAbstractGeometry *g /Transfer/ );
%Docstring
Adds a geometry and takes ownership. Returns true in case of success
 :rtype: bool
%End

    virtual QgsAbstractGeometry *boundary() const /Factory/;


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgsmultisurface.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/

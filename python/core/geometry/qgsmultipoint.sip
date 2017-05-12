/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgsmultipoint.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsMultiPointV2: QgsGeometryCollection
{
%Docstring
 Multi point geometry collection.
.. versionadded:: 2.10
%End

%TypeHeaderCode
#include "qgsmultipoint.h"
%End
  public:
    QgsMultiPointV2();
    virtual QString geometryType() const;
    virtual QgsMultiPointV2 *clone() const /Factory/;


    virtual bool fromWkt( const QString &wkt );


    virtual QDomElement asGML2( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const;

    virtual QDomElement asGML3( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const;

    virtual QString asJSON( int precision = 17 ) const;


    virtual int nCoordinates() const;

    virtual bool addGeometry( QgsAbstractGeometry *g /Transfer/ );
%Docstring
Adds a geometry and takes ownership. Returns true in case of success
 :rtype: bool
%End

    virtual QgsAbstractGeometry *boundary() const /Factory/;

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

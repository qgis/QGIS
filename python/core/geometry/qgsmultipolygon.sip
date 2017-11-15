/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgsmultipolygon.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsMultiPolygon: QgsMultiSurface
{
%Docstring
 Multi polygon geometry collection.
.. versionadded:: 2.10
%End

%TypeHeaderCode
#include "qgsmultipolygon.h"
%End
  public:
    QgsMultiPolygon();
    virtual QString geometryType() const;

    virtual void clear();

    virtual QgsMultiPolygon *clone() const /Factory/;

    virtual bool fromWkt( const QString &wkt );

    virtual QDomElement asGml2( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const;

    virtual QDomElement asGml3( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const;

    virtual QString asJson( int precision = 17 ) const;

    virtual bool addGeometry( QgsAbstractGeometry *g /Transfer/ );

    virtual bool insertGeometry( QgsAbstractGeometry *g /Transfer/, int index );


    virtual QgsMultiSurface *toCurveType() const /Factory/;

%Docstring
 Returns the geometry converted to the more generic curve type QgsMultiSurface
:return: the converted geometry. Caller takes ownership*
 :rtype: QgsMultiSurface
%End

    virtual QgsAbstractGeometry *boundary() const /Factory/;


  protected:
    virtual QgsMultiPolygon *createEmptyWithSameType() const /Factory/;

    virtual bool wktOmitChildType() const;

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgsmultipolygon.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/

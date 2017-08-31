/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgsmultilinestring.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsMultiLineString: QgsMultiCurve
{
%Docstring
 Multi line string geometry collection.
.. versionadded:: 2.10
%End

%TypeHeaderCode
#include "qgsmultilinestring.h"
%End
  public:
    QgsMultiLineString();

    virtual QString geometryType() const;
    virtual QgsMultiLineString *createEmptyWithSameType() const /Factory/;
    virtual QgsMultiLineString *clone() const /Factory/;

    virtual void clear();

    virtual bool fromWkt( const QString &wkt );

    virtual QDomElement asGML2( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const;

    virtual QDomElement asGML3( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const;

    virtual QString asJSON( int precision = 17 ) const;

    virtual bool addGeometry( QgsAbstractGeometry *g /Transfer/ );

    virtual bool insertGeometry( QgsAbstractGeometry *g /Transfer/, int index );


    virtual QgsMultiCurve *toCurveType() const /Factory/;

%Docstring
 Returns the geometry converted to the more generic curve type QgsMultiCurve
:return: the converted geometry. Caller takes ownership*
 :rtype: QgsMultiCurve
%End

  protected:

    virtual bool wktOmitChildType() const;

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgsmultilinestring.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/

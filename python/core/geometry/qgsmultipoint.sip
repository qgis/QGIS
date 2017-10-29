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
    QgsMultiPoint();

    virtual QString geometryType() const;

    virtual QgsMultiPoint *clone() const /Factory/;

    virtual QgsMultiPoint *toCurveType() const /Factory/;

    virtual bool fromWkt( const QString &wkt );

    virtual void clear();

    virtual QDomElement asGML2( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const;

    virtual QDomElement asGML3( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const;

    virtual QString asJSON( int precision = 17 ) const;

    virtual int nCoordinates() const;

    virtual bool addGeometry( QgsAbstractGeometry *g /Transfer/ );

    virtual bool insertGeometry( QgsAbstractGeometry *g /Transfer/, int index );

    virtual QgsAbstractGeometry *boundary() const /Factory/;

    virtual int vertexNumberFromVertexId( QgsVertexId id ) const;



  protected:
    virtual QgsMultiPoint *createEmptyWithSameType() const /Factory/;

    virtual bool wktOmitChildType() const;


};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgsmultipoint.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/

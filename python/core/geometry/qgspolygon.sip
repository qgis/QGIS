/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgspolygon.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsPolygon: QgsCurvePolygon
{
%Docstring
 Polygon geometry type.
.. versionadded:: 2.10
%End

%TypeHeaderCode
#include "qgspolygon.h"
%End
  public:
    QgsPolygon();

    virtual QString geometryType() const;

    virtual QgsPolygon *clone() const /Factory/;

    virtual void clear();

    virtual bool fromWkb( QgsConstWkbPtr &wkb );

    virtual QByteArray asWkb() const;

    virtual QgsPolygon *surfaceToPolygon() const /Factory/;


    virtual QgsCurvePolygon *toCurveType() const /Factory/;

%Docstring
 Returns the geometry converted to the more generic curve type QgsCurvePolygon
:return: the converted geometry. Caller takes ownership*
 :rtype: QgsCurvePolygon
%End

    virtual void addInteriorRing( QgsCurve *ring /Transfer/ );

    virtual void setExteriorRing( QgsCurve *ring /Transfer/ );


    virtual QgsAbstractGeometry *boundary() const /Factory/;


    double pointDistanceToBoundary( double x, double y ) const;
%Docstring
 Returns the distance from a point to the boundary of the polygon (either the
 exterior ring or any closer interior rings). The returned distance will be
 negative if the point lies outside the polygon.
.. versionadded:: 3.0
 :rtype: float
%End


  protected:

    virtual QgsPolygon *createEmptyWithSameType() const /Factory/;



};
/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgspolygon.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/

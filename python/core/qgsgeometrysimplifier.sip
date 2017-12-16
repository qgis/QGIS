/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsgeometrysimplifier.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsAbstractGeometrySimplifier
{
%Docstring
Abstract base class for simplify geometries using a specific algorithm
%End

%TypeHeaderCode
#include "qgsgeometrysimplifier.h"
%End
  public:
    virtual ~QgsAbstractGeometrySimplifier();

    virtual QgsGeometry simplify( const QgsGeometry &geometry ) const = 0;
%Docstring
Returns a simplified version the specified geometry
%End

  public:
    static bool isGeneralizableByDeviceBoundingBox( const QgsRectangle &envelope, float mapToPixelTol = 1.0f );
%Docstring
Returns whether the device-envelope can be replaced by its BBOX when is applied the specified tolerance
%End
    static bool isGeneralizableByDeviceBoundingBox( const QVector<QPointF> &points, float mapToPixelTol = 1.0f );
%Docstring
Returns whether the device-geometry can be replaced by its BBOX when is applied the specified tolerance
%End
};


class QgsTopologyPreservingSimplifier : QgsAbstractGeometrySimplifier
{
%Docstring
Implementation of GeometrySimplifier using the Douglas-Peucker algorithm

Simplifies a geometry, ensuring that the result is a valid geometry having the same dimension and number of components as the input.
The simplification uses a maximum distance difference algorithm similar to the one used in the Douglas-Peucker algorithm.
%End

%TypeHeaderCode
#include "qgsgeometrysimplifier.h"
%End
  public:

    QgsTopologyPreservingSimplifier( double tolerance );
%Docstring
Constructor for QgsTopologyPreservingSimplifier. The tolerance parameter
is specified in layer units.
%End

    virtual QgsGeometry simplify( const QgsGeometry &geometry ) const;


  protected:

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsgeometrysimplifier.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/

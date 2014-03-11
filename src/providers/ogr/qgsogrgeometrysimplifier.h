/***************************************************************************
    qgsogrgeometrysimplifier.h
    ---------------------
    begin                : December 2013
    copyright            : (C) 2013 by Alvaro Huarte
    email                : http://wiki.osgeo.org/wiki/Alvaro_Huarte

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOGRGEOMETRYSIMPLIFIER_H
#define QGSOGRGEOMETRYSIMPLIFIER_H

#include "qgsmaptopixelgeometrysimplifier.h"
#include "qgspoint.h"

#include <ogr_api.h>

/**
 * Abstract base class for simplify OGR-geometries using a specific algorithm
 */
class QgsOgrAbstractGeometrySimplifier
{
  public:
    virtual ~QgsOgrAbstractGeometrySimplifier();

    //! Simplifies the specified geometry
    virtual bool simplifyGeometry( OGRGeometryH geometry ) = 0;
};

#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 1900
/**
 * OGR Implementation of GeometrySimplifier using the Douglas-Peucker algorithm
 *
 * Simplifies a geometry, ensuring that the result is a valid geometry having the same dimension and number of components as the input.
 * The simplification uses a maximum distance difference algorithm similar to the one used in the Douglas-Peucker algorithm.
 */
class QgsOgrTopologyPreservingSimplifier : public QgsOgrAbstractGeometrySimplifier, QgsTopologyPreservingSimplifier
{
  public:
    QgsOgrTopologyPreservingSimplifier( double tolerance );
    virtual ~QgsOgrTopologyPreservingSimplifier();

    //! Simplifies the specified geometry
    virtual bool simplifyGeometry( OGRGeometryH geometry );
};
#endif

#if defined(GDAL_VERSION_NUM) && defined(GDAL_COMPUTE_VERSION)
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(1,11,0)
/**
 * OGR implementation of GeometrySimplifier using the "MapToPixel" algorithm
 *
 * Simplifies a geometry removing points within of the maximum distance difference that defines the MapToPixel info of a RenderContext request.
 * This class enables simplify the geometries to be rendered in a MapCanvas target to speed up the vector drawing.
 */
class QgsOgrMapToPixelSimplifier : public QgsOgrAbstractGeometrySimplifier, QgsMapToPixelSimplifier
{
  public:
    QgsOgrMapToPixelSimplifier( int simplifyFlags, double tolerance );
    virtual ~QgsOgrMapToPixelSimplifier();

  private:
    //! Point memory buffer for optimize the simplification process
    QgsPoint* mPointBufferPtr;
    //! Current Point memory buffer size
    int mPointBufferCount;

    //! Simplifies the OGR-geometry (Removing duplicated points) when is applied the specified map2pixel context
    bool simplifyOgrGeometry( QGis::GeometryType geometryType, double* xptr, int xStride, double* yptr, int yStride, int pointCount, int& pointSimplifiedCount );
    //! Simplifies the OGR-geometry (Removing duplicated points) when is applied the specified map2pixel context
    bool simplifyOgrGeometry( OGRGeometryH geometry, bool isaLinearRing );

    //! Returns a point buffer of the specified size
    QgsPoint* mallocPoints( int numPoints );
    //! Returns a point buffer of the specified envelope
    QgsPoint* getEnvelopePoints( const QgsRectangle& envelope, int& numPoints, bool isaLinearRing );

    //! Load a point array to the specified LineString geometry
    static void setGeometryPoints( OGRGeometryH geometry, QgsPoint* points, int numPoints );

  public:
    //! Simplifies the specified geometry
    virtual bool simplifyGeometry( OGRGeometryH geometry );
};
#endif // GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(1,11,0)
#endif // defined(GDAL_VERSION_NUM) && defined(GDAL_COMPUTE_VERSION)

#endif // QGSOGRGEOMETRYSIMPLIFIER_H

/***************************************************************************
    qgsogrmaptopixelgeometrysimplifier.h
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

#ifndef QGSOGRMAPTOPIXELGEOMETRYSIMPLIFIER_H
#define QGSOGRMAPTOPIXELGEOMETRYSIMPLIFIER_H

#include "qgsmaptopixelgeometrysimplifier.h"
#include <ogr_geometry.h>

/**
 * OGR implementation of GeometrySimplifier using the "MapToPixel" algorithm
 *
 * Simplifies a geometry removing points within of the maximum distance difference that defines the MapToPixel info of a RenderContext request.
 * This class enables simplify the geometries to be rendered in a MapCanvas target to speed up the vector drawing.
 */
class QgsOgrMapToPixelSimplifier : public QgsMapToPixelSimplifier
{
  public:
    QgsOgrMapToPixelSimplifier( int simplifyFlags, const QgsCoordinateTransform* coordinateTransform, const QgsMapToPixel* mapToPixel, float mapToPixelTol );
    virtual ~QgsOgrMapToPixelSimplifier();

  private:
    //! Point memory buffer for optimize the simplification process
    OGRRawPoint* mPointBufferPtr;
    //! Current Point memory buffer size
    int mPointBufferCount;

    //! Simplifies the OGR-geometry (Removing duplicated points) when is applied the specified map2pixel context
    bool simplifyOgrGeometry( QGis::GeometryType geometryType, const QgsRectangle& envelope, double* xptr, int xStride, double* yptr, int yStride, int pointCount, int& pointSimplifiedCount );
    //! Simplifies the OGR-geometry (Removing duplicated points) when is applied the specified map2pixel context
    bool simplifyOgrGeometry( OGRGeometry* geometry, bool isaLinearRing );

    //! Returns a point buffer of the specified size
    OGRRawPoint* mallocPoints( int numPoints );

  public:
    //! Simplifies the specified geometry
    bool simplifyGeometry( OGRGeometry* geometry );
};

#endif // QGSOGRMAPTOPIXELGEOMETRYSIMPLIFIER_H

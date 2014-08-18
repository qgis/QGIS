/***************************************************************************
    qgsgeometrysimplifier.h
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

#ifndef QGSGEOMETRYSIMPLIFIER_H
#define QGSGEOMETRYSIMPLIFIER_H

#include "qgsgeometry.h"

/**
 * Abstract base class for simplify geometries using a specific algorithm
 */
class CORE_EXPORT QgsAbstractGeometrySimplifier
{
  public:
    virtual ~QgsAbstractGeometrySimplifier();

    //! Returns a simplified version the specified geometry
    virtual QgsGeometry* simplify( QgsGeometry* geometry ) const = 0;
    //! Simplifies the specified geometry
    virtual bool simplifyGeometry( QgsGeometry* geometry ) const = 0;

    // MapToPixel simplification helper methods
  public:
    //! Returns whether the device-envelope can be replaced by its BBOX when is applied the specified tolerance
    static bool isGeneralizableByDeviceBoundingBox( const QgsRectangle&   envelope, float mapToPixelTol = 1.0f );
    //! Returns whether the device-geometry can be replaced by its BBOX when is applied the specified tolerance
    static bool isGeneralizableByDeviceBoundingBox( const QVector<QPointF>& points, float mapToPixelTol = 1.0f );
};

/***************************************************************************/
/**
 * Implementation of GeometrySimplifier using the Douglas-Peucker algorithm
 *
 * Simplifies a geometry, ensuring that the result is a valid geometry having the same dimension and number of components as the input.
 * The simplification uses a maximum distance difference algorithm similar to the one used in the Douglas-Peucker algorithm.
 */
class CORE_EXPORT QgsTopologyPreservingSimplifier : public QgsAbstractGeometrySimplifier
{
  public:
    QgsTopologyPreservingSimplifier( double tolerance );
    virtual ~QgsTopologyPreservingSimplifier();

  protected:
    //! Distance tolerance for the simplification
    double mTolerance;

  public:
    //! Returns a simplified version the specified geometry
    virtual QgsGeometry* simplify( QgsGeometry* geometry ) const;
    //! Simplifies the specified geometry
    virtual bool simplifyGeometry( QgsGeometry* geometry ) const;
};

#endif // QGSGEOMETRYSIMPLIFIER_H

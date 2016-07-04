/***************************************************************************
    qgsmaptopixelgeometrysimplifier.h
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

#ifndef QGSMAPTOPIXELGEOMETRYSIMPLIFIER_H
#define QGSMAPTOPIXELGEOMETRYSIMPLIFIER_H

#include "qgsgeometry.h"
#include "qgscoordinatetransform.h"
#include "qgsmaptopixel.h"

#include "qgsgeometrysimplifier.h"

/** \ingroup core
 * Implementation of GeometrySimplifier using the "MapToPixel" algorithm
 *
 * Simplifies a geometry removing points within of the maximum distance difference that defines the MapToPixel info of a RenderContext request.
 * This class enables simplify the geometries to be rendered in a MapCanvas target to speed up the vector drawing.
 */
class CORE_EXPORT QgsMapToPixelSimplifier : public QgsAbstractGeometrySimplifier
{
  public:
    //! Types of simplification algorithms that can be used
    enum SimplifyAlgorithm
    {
      Distance    = 0, //!< The simplification uses the distance between points to remove duplicate points
      SnapToGrid  = 1, //!< The simplification uses a grid (similar to ST_SnapToGrid) to remove duplicate points
      Visvalingam = 2, //!< The simplification gives each point in a line an importance weighting, so that least important points are removed first
    };

    //! Constructor
    QgsMapToPixelSimplifier( int simplifyFlags, double tolerance, SimplifyAlgorithm simplifyAlgorithm = Distance );
    virtual ~QgsMapToPixelSimplifier();

    //! Applicable simplification flags
    enum SimplifyFlag
    {
      NoFlags          = 0, //!< No simplification can be applied
      SimplifyGeometry = 1, //!< The geometries can be simplified using the current map2pixel context state
      SimplifyEnvelope = 2, //!< The geometries can be fully simplified by its BoundingBox
    };

  private:
    //! Simplify the WKB-geometry using the specified tolerance
    static bool simplifyWkbGeometry( int simplifyFlags, SimplifyAlgorithm simplifyAlgorithm, QGis::WkbType wkbType, QgsConstWkbPtr sourceWkbPtr, QgsWkbPtr targetWkbPtr, int &targetWkbSize, const QgsRectangle& envelope, double map2pixelTol, bool writeHeader = true, bool isaLinearRing = false );

  protected:
    //! Current simplification flags
    int mSimplifyFlags;

    //! Current algorithm
    SimplifyAlgorithm mSimplifyAlgorithm;

    //! Distance tolerance for the simplification
    double mTolerance;

    //! Returns the squared 2D-distance of the vector defined by the two points specified
    static float calculateLengthSquared2D( double x1, double y1, double x2, double y2 );

    //! Returns whether the points belong to the same grid
    static bool equalSnapToGrid( double x1, double y1, double x2, double y2, double gridOriginX, double gridOriginY, float gridInverseSizeXY );

  public:
    //! Gets the simplification hints of the vector layer managed
    int simplifyFlags() const { return mSimplifyFlags; }
    //! Sets the simplification hints of the vector layer managed
    void setSimplifyFlags( int simplifyFlags ) { mSimplifyFlags = simplifyFlags; }

    //! Gets the local simplification algorithm of the vector layer managed
    SimplifyAlgorithm simplifyAlgorithm() const { return mSimplifyAlgorithm; }
    //! Sets the local simplification algorithm of the vector layer managed
    void setSimplifyAlgorithm( SimplifyAlgorithm simplifyAlgorithm ) { mSimplifyAlgorithm = simplifyAlgorithm; }

    //! Returns a simplified version the specified geometry
    virtual QgsGeometry* simplify( QgsGeometry* geometry ) const override;
    //! Simplifies the specified geometry
    virtual bool simplifyGeometry( QgsGeometry* geometry ) const override;

    //! Simplifies the specified WKB-point array
    virtual bool simplifyPoints( QgsWKBTypes::Type wkbType, QgsConstWkbPtr& sourceWkbPtr, QPolygonF& targetPoints ) const;

    // MapToPixel simplification helper methods
  public:

    //! Returns whether the envelope can be replaced by its BBOX when is applied the specified map2pixel context
    static bool isGeneralizableByMapBoundingBox( const QgsRectangle& envelope, double map2pixelTol );

    //! Returns whether the envelope can be replaced by its BBOX when is applied the specified map2pixel context
    inline bool isGeneralizableByMapBoundingBox( const QgsRectangle& envelope ) const
    {
      return isGeneralizableByMapBoundingBox( envelope, mTolerance );
    }

    //! Simplifies the geometry when is applied the specified map2pixel context
    static bool simplifyGeometry( QgsGeometry* geometry, int simplifyFlags, double tolerance, SimplifyAlgorithm simplifyAlgorithm = Distance );

    //! Simplifies the WKB-point array when is applied the specified map2pixel context
    static bool simplifyPoints( QgsWKBTypes::Type wkbType, QgsConstWkbPtr& sourceWkbPtr, QPolygonF& targetPoints, int simplifyFlags, double tolerance, SimplifyAlgorithm simplifyAlgorithm = Distance );
};

#endif // QGSMAPTOPIXELGEOMETRYSIMPLIFIER_H

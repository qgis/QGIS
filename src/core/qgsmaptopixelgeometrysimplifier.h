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

/**
 * Implementation of GeometrySimplifier using the "MapToPixel" algorithm
 *
 * Simplifies a geometry removing points within of the maximum distance difference that defines the MapToPixel info of a RenderContext request.
 * This class enables simplify the geometries to be rendered in a MapCanvas target to speed up the vector drawing.
 */
class CORE_EXPORT QgsMapToPixelSimplifier : public QgsAbstractGeometrySimplifier
{
  public:
    QgsMapToPixelSimplifier( int simplifyFlags, const QgsCoordinateTransform* coordinateTransform, const QgsMapToPixel* mapToPixel, float mapToPixelTol );
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
    static bool simplifyWkbGeometry( int simplifyFlags, QGis::WkbType wkbType, unsigned char* sourceWkb, size_t sourceWkbSize, unsigned char* targetWkb, size_t& targetWkbSize, const QgsRectangle& envelope, float map2pixelTol, bool writeHeader = true, bool isaLinearRing = false );

  protected:
    //! Current simplification flags
    int mSimplifyFlags;

    //! For transformation between coordinate systems from current layer to map target. Can be 0 if on-the-fly reprojection is not used
    const QgsCoordinateTransform* mMapCoordTransform;
    //! For transformation between map coordinates and device coordinates
    const QgsMapToPixel* mMapToPixel;
    //! Factor tolterance to apply in transformation between map coordinates and device coordinates
    float mMapToPixelTol;

    //! Returns the squared 2D-distance of the vector defined by the two points specified
    static float calculateLengthSquared2D( double x1, double y1, double x2, double y2 );
    //! Returns the MapTolerance for transform between map coordinates and device coordinates
    static float calculateViewPixelTolerance( const QgsRectangle& boundingRect, const QgsCoordinateTransform* ct, const QgsMapToPixel* mapToPixel );

  public:
    int simplifyFlags() const { return mSimplifyFlags; }
    void setSimplifyFlags( int simplifyFlags ) { mSimplifyFlags = simplifyFlags; }

    const QgsCoordinateTransform* coordinateTransform() const { return mMapCoordTransform; }
    void setCoordinateTransform( const QgsCoordinateTransform* ct ) { mMapCoordTransform = ct; }

    const QgsMapToPixel* mapToPixel() const { return mMapToPixel; }
    void setMapToPixel( const QgsMapToPixel* mtp ) { mMapToPixel = mtp; }

    float mapToPixelTol() const { return mMapToPixelTol; }
    void setMapToPixelTol( float map2pixelTol ) { mMapToPixelTol = map2pixelTol; }

    //! Returns a simplified version the specified geometry
    virtual QgsGeometry* simplify( QgsGeometry* geometry );
    //! Simplifies the specified geometry
    virtual bool simplifyGeometry( QgsGeometry* geometry );

    // MapToPixel simplification helper methods
  public:

    //! Returns whether the envelope can be replaced by its BBOX when is applied the specified map2pixel context
    static bool canbeGeneralizedByMapBoundingBox( const QgsRectangle& envelope,
        const QgsCoordinateTransform* coordinateTransform, const QgsMapToPixel* mapToPixel, float mapToPixelTol = 1.0f );

    //! Returns whether the envelope can be replaced by its BBOX when is applied the specified map2pixel context
    inline bool canbeGeneralizedByMapBoundingBox( const QgsRectangle& envelope ) const { return canbeGeneralizedByMapBoundingBox( envelope, mMapCoordTransform, mMapToPixel, mMapToPixelTol ); }

    //! Simplifies the geometry when is applied the specified map2pixel context
    static bool simplifyGeometry( QgsGeometry* geometry,
                                  int simplifyFlags, const QgsCoordinateTransform* coordinateTransform, const QgsMapToPixel* mapToPixel, float mapToPixelTol = 1.0f );

};

#endif // QGSMAPTOPIXELGEOMETRYSIMPLIFIER_H

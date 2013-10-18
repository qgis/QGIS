/***************************************************************************
    qgsmaprequest.h
    ----------------------
    begin                : October 2013
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

#ifndef QGSMAPREQUEST_H
#define QGSMAPREQUEST_H

#include "qgscoordinatetransform.h"
#include "qgsmaptopixel.h"

class QgsRectangle;
class QgsGeometry;

/**
 * This class wraps a generic request for a map layer (or directly its data provider).
 * The request may apply a simplification using the map2pixel render state to fetch 
 * only a particular subset of information.
 */
class CORE_EXPORT QgsMapRequest
{
  public:
    //! construct a default request
    QgsMapRequest();
    //! copy constructor
    QgsMapRequest( const QgsMapRequest& rh );

    QgsMapRequest& operator=( const QgsMapRequest& rh );

   ~QgsMapRequest();

  public:
    const QgsCoordinateTransform* coordinateTransform() const { return mMapCoordTransform; }
    QgsMapRequest& setCoordinateTransform( const QgsCoordinateTransform* ct );
	
    const QgsMapToPixel* mapToPixel() const { return mMapToPixel; }
    QgsMapRequest& setMapToPixel( const QgsMapToPixel* mtp );

    float mapToPixelTol() const { return mMapToPixelTol; }
    QgsMapRequest& setMapToPixelTol( float map2pixelTol );

  protected:
    //! For transformation between coordinate systems from current layer to map target. Can be 0 if on-the-fly reprojection is not used
    const QgsCoordinateTransform* mMapCoordTransform;    
    //! For transformation between map coordinates and device coordinates
    const QgsMapToPixel* mMapToPixel;
    //! Factor tolterance to apply in transformation between map coordinates and device coordinates
    float mMapToPixelTol;

  public:
    //! Returns whether the devided-geometry can be replaced by its BBOX when is applied the specified the map2pixel context
    static bool canbeGeneralizedByWndBoundingBox( const QgsRectangle&   envelope, float mapToPixelTol = 1.0f );
    //! Returns whether the devided-geometry can be replaced by its BBOX when is applied the specified the map2pixel context
    static bool canbeGeneralizedByWndBoundingBox( const QVector<QPointF>& points, float mapToPixelTol = 1.0f );

    //! Simplify the specified geometry (Removing duplicated points) when is applied the map2pixel context
    static bool simplifyGeometry( QgsGeometry* geometry, 
                                  const QgsCoordinateTransform* coordinateTransform, const QgsMapToPixel* mtp, float mapToPixelTol = 1.0f );

	//! Simplify the specified geometry (Removing duplicated points) when is applied the map2pixel context
    inline bool simplifyGeometry( QgsGeometry* geometry ) { return simplifyGeometry( geometry, mMapCoordTransform, mMapToPixel, mMapToPixelTol ); }
};

#endif // QGSMAPREQUEST_H

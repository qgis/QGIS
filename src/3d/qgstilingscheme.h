/***************************************************************************
  qgstilingscheme.h
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTILINGSCHEME_H
#define QGSTILINGSCHEME_H

#include "qgis_3d.h"

#include <qgscoordinatereferencesystem.h>
#include <qgspointxy.h>

class QgsRectangle;

/**
 * \ingroup 3d
 * The class encapsulates tiling scheme (just like with WMTS / TMS / XYZ layers).
 * The origin (tile [0,0]) is in bottom-left corner.
 * \since QGIS 3.0
 */
class _3D_EXPORT QgsTilingScheme
{
  public:
    //! Creates invalid tiling scheme
    QgsTilingScheme() = default;

    //! Creates tiling scheme where level 0 tile is centered at the full extent and the full extent completely fits into the level 0 tile
    QgsTilingScheme( const QgsRectangle &fullExtent, const QgsCoordinateReferenceSystem &crs );

    //! Returns map coordinates at tile coordinates (for lower-left corner of the tile)
    QgsPointXY tileToMap( int x, int y, int z ) const;
    //! Returns tile coordinates for given map coordinates and Z level
    void mapToTile( const QgsPointXY &pt, int z, float &x, float &y ) const;

    //! Returns map coordinates of the extent of a tile
    QgsRectangle tileToExtent( int x, int y, int z ) const;

    //! Returns coordinates of a tile that most tightly fits the whole extent
    void extentToTile( const QgsRectangle &extent, int &x, int &y, int &z ) const;

    //! Returns CRS of the tiling scheme
    QgsCoordinateReferenceSystem crs() const { return mCrs; }

  private:
    QgsPointXY mMapOrigin; //!< Origin point in map coordinates: (0,0) in the tiling scheme
    double mBaseTileSide = 0;  //!< Length of tile side at zoom level 0 in map coordinates
    QgsCoordinateReferenceSystem mCrs;  //!< CRS of the coordinates

};

#endif // QGSTILINGSCHEME_H

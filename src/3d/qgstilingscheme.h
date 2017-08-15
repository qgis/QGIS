#ifndef QGSTILINGSCHEME_H
#define QGSTILINGSCHEME_H

#include <qgscoordinatereferencesystem.h>
#include <qgspointxy.h>

class QgsRectangle;

//! The class encapsulates tiling scheme (just like with WMTS / TMS / XYZ layers).
//! The origin (tile [0,0]) is in bottom-left corner.
class QgsTilingScheme
{
  public:
    //! Creates invalid tiling scheme
    QgsTilingScheme();

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

    QgsPointXY mapOrigin; //!< Origin point in map coordinates: (0,0) in the tiling scheme
    double baseTileSide;  //!< Length of tile side at zoom level 0 in map coordinates
    QgsCoordinateReferenceSystem crs;  //!< CRS of the coordinates

};

#endif // QGSTILINGSCHEME_H

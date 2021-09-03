/***************************************************************************
  qgstilingscheme.cpp
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

#include "qgstilingscheme.h"

#include "qgschunknode_p.h"
#include "qgsrectangle.h"

QgsTilingScheme::QgsTilingScheme( const QgsRectangle &fullExtent, const QgsCoordinateReferenceSystem &crs )
  : mCrs( crs )
{
  mMapOrigin = QgsPointXY( fullExtent.xMinimum(), fullExtent.yMinimum() );
  mBaseTileSide = std::max( fullExtent.width(), fullExtent.height() );
}

QgsPointXY QgsTilingScheme::tileToMap( int x, int y, int z ) const
{
  const double tileSide = mBaseTileSide / pow( 2, z );
  const double mx = mMapOrigin.x() + x * tileSide;
  const double my = mMapOrigin.y() + y * tileSide;
  return QgsPointXY( mx, my );
}

void QgsTilingScheme::mapToTile( const QgsPointXY &pt, int z, float &x, float &y ) const
{
  const double tileSide = mBaseTileSide / pow( 2, z );
  x = ( pt.x() - mMapOrigin.x() ) / tileSide;
  y = ( pt.y() - mMapOrigin.y() ) / tileSide;
}

QgsRectangle QgsTilingScheme::tileToExtent( int x, int y, int z ) const
{
  const QgsPointXY pt0 = tileToMap( x, y, z );
  const QgsPointXY pt1 = tileToMap( x + 1, y + 1, z );
  return QgsRectangle( pt0, pt1 );
}

QgsRectangle QgsTilingScheme::tileToExtent( const QgsChunkNodeId &nodeId ) const
{
  return tileToExtent( nodeId.x, nodeId.y, nodeId.d );
}

void QgsTilingScheme::extentToTile( const QgsRectangle &extent, int &x, int &y, int &z ) const
{
  x = y = z = 0;  // start with root tile
  while ( true )
  {
    // try to see if any child tile fully contains our extent - if so, go deeper
    if ( tileToExtent( x * 2, y * 2, z + 1 ).contains( extent ) )
    {
      x = x * 2;
      y = y * 2;
    }
    else if ( tileToExtent( x * 2 + 1, y * 2, z + 1 ).contains( extent ) )
    {
      x = x * 2 + 1;
      y = y * 2;
    }
    else if ( tileToExtent( x * 2, y * 2 + 1, z + 1 ).contains( extent ) )
    {
      x = x * 2;
      y = y * 2 + 1;
    }
    else if ( tileToExtent( x * 2 + 1, y * 2 + 1, z + 1 ).contains( extent ) )
    {
      x = x * 2 + 1;
      y = y * 2 + 1;
    }
    else
    {
      return;  // cannot go deeper
    }
    z++;
  }
}

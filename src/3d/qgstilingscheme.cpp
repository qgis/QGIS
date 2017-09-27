#include "qgstilingscheme.h"

#include "qgsrectangle.h"


QgsTilingScheme::QgsTilingScheme()
  : mMapOrigin()
  , mBaseTileSide( 0 )
{
}

QgsTilingScheme::QgsTilingScheme( const QgsRectangle &fullExtent, const QgsCoordinateReferenceSystem &crs )
  : mCrs( crs )
{
  mMapOrigin = QgsPointXY( fullExtent.xMinimum(), fullExtent.yMinimum() );
  mBaseTileSide = qMax( fullExtent.width(), fullExtent.height() );
}

QgsPointXY QgsTilingScheme::tileToMap( int x, int y, int z ) const
{
  double tileSide = mBaseTileSide / pow( 2, z );
  double mx = mMapOrigin.x() + x * tileSide;
  double my = mMapOrigin.y() + y * tileSide;
  return QgsPointXY( mx, my );
}

void QgsTilingScheme::mapToTile( const QgsPointXY &pt, int z, float &x, float &y ) const
{
  double tileSide = mBaseTileSide / pow( 2, z );
  x = ( pt.x() - mMapOrigin.x() ) / tileSide;
  y = ( pt.y() - mMapOrigin.y() ) / tileSide;
}

QgsRectangle QgsTilingScheme::tileToExtent( int x, int y, int z ) const
{
  QgsPointXY pt0 = tileToMap( x, y, z );
  QgsPointXY pt1 = tileToMap( x + 1, y + 1, z );
  return QgsRectangle( pt0, pt1 );
}

void QgsTilingScheme::extentToTile( const QgsRectangle &extent, int &x, int &y, int &z ) const
{
  x = y = z = 0;  // start with root tile
  while ( 1 )
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

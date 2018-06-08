/***************************************************************************
  qgsrasteranalysisutils.cpp
  ---------------------
  Date                 : June 2018
  Copyright            : (C) 2018 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrasteranalysisutils.h"

#include "qgsfeedback.h"
#include "qgsrasterblock.h"
#include "qgsrasteriterator.h"
#include "qgsgeos.h"

///@cond PRIVATE

void QgsRasterAnalysisUtils::cellInfoForBBox( const QgsRectangle &rasterBBox, const QgsRectangle &featureBBox, double cellSizeX, double cellSizeY,
    int &nCellsX, int &nCellsY, int rasterWidth, int rasterHeight, QgsRectangle &rasterBlockExtent )
{
  //get intersecting bbox
  QgsRectangle intersectBox = rasterBBox.intersect( &featureBBox );
  if ( intersectBox.isEmpty() )
  {
    nCellsX = 0;
    nCellsY = 0;
    rasterBlockExtent = QgsRectangle();
    return;
  }

  //get offset in pixels in x- and y- direction
  int offsetX = static_cast< int >( std::floor( ( intersectBox.xMinimum() - rasterBBox.xMinimum() ) / cellSizeX ) );
  int offsetY = static_cast< int >( std::floor( ( rasterBBox.yMaximum() - intersectBox.yMaximum() ) / cellSizeY ) );

  int maxColumn = static_cast< int >( std::floor( ( intersectBox.xMaximum() - rasterBBox.xMinimum() ) / cellSizeX ) ) + 1;
  int maxRow = static_cast< int >( std::floor( ( rasterBBox.yMaximum() - intersectBox.yMinimum() ) / cellSizeY ) ) + 1;

  nCellsX = maxColumn - offsetX;
  nCellsY = maxRow - offsetY;

  //avoid access to cells outside of the raster (may occur because of rounding)
  nCellsX = std::min( offsetX + nCellsX, rasterWidth ) - offsetX;
  nCellsY = std::min( offsetY + nCellsY, rasterHeight ) - offsetY;

  rasterBlockExtent = QgsRectangle( rasterBBox.xMinimum() + offsetX * cellSizeX,
                                    rasterBBox.yMaximum() - offsetY * cellSizeY,
                                    rasterBBox.xMinimum() + ( nCellsX + offsetX ) * cellSizeX,
                                    rasterBBox.yMaximum() - ( nCellsY + offsetY ) * cellSizeY );
}

void QgsRasterAnalysisUtils::statisticsFromMiddlePointTest( QgsRasterInterface *rasterInterface, int rasterBand, const QgsGeometry &poly, int nCellsX, int nCellsY, double cellSizeX, double cellSizeY, const QgsRectangle &rasterBBox,  const std::function<void( double )> &addValue, bool skipNodata )
{
  std::unique_ptr< QgsGeometryEngine > polyEngine( QgsGeometry::createGeometryEngine( poly.constGet( ) ) );
  if ( !polyEngine )
  {
    return;
  }
  polyEngine->prepareGeometry();

  const int maxWidth = 4000;
  const int maxHeight = 4000;

  QgsRasterIterator iter( rasterInterface );
  iter.setMaximumTileWidth( maxWidth );
  iter.setMaximumTileHeight( maxHeight );
  iter.startRasterRead( rasterBand, nCellsX, nCellsY, rasterBBox );

  QgsRasterBlock *block = nullptr;
  int iterLeft = 0;
  int iterTop = 0;
  int iterCols = 0;
  int iterRows = 0;
  while ( iter.readNextRasterPart( rasterBand, iterCols, iterRows, &block, iterLeft, iterTop ) )
  {
    double cellCenterY = rasterBBox.yMinimum() + ( iterTop + iterRows - 0.5 ) * cellSizeY;
    for ( int row = 0; row < iterRows; ++row )
    {
      double cellCenterX = rasterBBox.xMinimum() + ( iterLeft + 0.5 ) * cellSizeX;
      for ( int col = 0; col < iterCols; ++col )
      {
        double pixelValue = block->value( row, col );
        if ( validPixel( pixelValue ) && ( !skipNodata || !block->isNoData( row, col ) ) )
        {
          QgsPoint cellCenter( cellCenterX, cellCenterY );
          if ( polyEngine->contains( &cellCenter ) )
          {
            addValue( pixelValue );
          }
        }
        cellCenterX += cellSizeX;
      }
      cellCenterY -= cellSizeY;
    }
    delete block;
  }
}

void QgsRasterAnalysisUtils::statisticsFromPreciseIntersection( QgsRasterInterface *rasterInterface, int rasterBand, const QgsGeometry &poly, int nCellsX, int nCellsY, double cellSizeX, double cellSizeY, const QgsRectangle &rasterBBox,  const std::function<void( double, double )> &addValue, bool skipNodata )
{
  QgsGeometry pixelRectGeometry;

  double hCellSizeX = cellSizeX / 2.0;
  double hCellSizeY = cellSizeY / 2.0;
  double pixelArea = cellSizeX * cellSizeY;
  double weight = 0;

  std::unique_ptr< QgsGeometryEngine > polyEngine( QgsGeometry::createGeometryEngine( poly.constGet( ) ) );
  if ( !polyEngine )
  {
    return;
  }
  polyEngine->prepareGeometry();

  const int maxWidth = 4000;
  const int maxHeight = 4000;

  QgsRasterIterator iter( rasterInterface );
  iter.setMaximumTileWidth( maxWidth );
  iter.setMaximumTileHeight( maxHeight );
  iter.startRasterRead( rasterBand, nCellsX, nCellsY, rasterBBox );

  QgsRasterBlock *block = nullptr;
  int iterLeft = 0;
  int iterTop = 0;
  int iterCols = 0;
  int iterRows = 0;
  while ( iter.readNextRasterPart( rasterBand, iterCols, iterRows, &block, iterLeft, iterTop ) )
  {
    double currentY = rasterBBox.yMinimum() + ( iterTop + iterRows - 0.5 ) * cellSizeY;
    for ( int row = 0; row < iterRows; ++row )
    {
      double currentX = rasterBBox.xMinimum() + ( iterLeft + 0.5 ) * cellSizeX;
      for ( int col = 0; col < iterCols; ++col )
      {
        double pixelValue = block->value( row, col );
        if ( validPixel( pixelValue ) && ( !skipNodata || !block->isNoData( row, col ) ) )
        {
          pixelRectGeometry = QgsGeometry::fromRect( QgsRectangle( currentX - hCellSizeX, currentY - hCellSizeY, currentX + hCellSizeX, currentY + hCellSizeY ) );
          // GEOS intersects tests on prepared geometry is MAGNITUDES faster than calculating the intersection itself,
          // so we first test to see if there IS an intersection before doing the actual calculation
          if ( !pixelRectGeometry.isNull() && polyEngine->intersects( pixelRectGeometry.constGet() ) )
          {
            //intersection
            QgsGeometry intersectGeometry = pixelRectGeometry.intersection( poly );
            if ( !intersectGeometry.isEmpty() )
            {
              double intersectionArea = intersectGeometry.area();
              if ( intersectionArea > 0.0 )
              {
                weight = intersectionArea / pixelArea;
                addValue( pixelValue, weight );
              }
            }
          }
        }
        currentX += cellSizeX;
      }
      currentY -= cellSizeY;
    }
  }
}

bool QgsRasterAnalysisUtils::validPixel( double value )
{
  return !std::isnan( value );
}

///@endcond PRIVATE

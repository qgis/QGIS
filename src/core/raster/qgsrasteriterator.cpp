/***************************************************************************
    qgsrasteriterator.cpp
    ---------------------
    begin                : July 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsrasteriterator.h"
#include "qgsrasterinterface.h"
#include "qgsrasterprojector.h"
#include "qgsrasterviewport.h"
#include "qgsrasterdataprovider.h"

QgsRasterIterator::QgsRasterIterator( QgsRasterInterface *input )
  : mInput( input )
  , mMaximumTileWidth( DEFAULT_MAXIMUM_TILE_WIDTH )
  , mMaximumTileHeight( DEFAULT_MAXIMUM_TILE_HEIGHT )
{
  for ( QgsRasterInterface *ri = input; ri; ri = ri->input() )
  {
    QgsRasterDataProvider *rdp = dynamic_cast<QgsRasterDataProvider *>( ri );
    if ( rdp )
    {
      mMaximumTileWidth = rdp->stepWidth();
      mMaximumTileHeight = rdp->stepHeight();
      break;
    }
  }
}

QgsRectangle QgsRasterIterator::subRegion( const QgsRectangle &rasterExtent, int rasterWidth, int rasterHeight, const QgsRectangle &subRegion, int &subRegionWidth, int &subRegionHeight, int &subRegionLeft, int &subRegionTop )
{
  const double xRes = rasterExtent.width() / rasterWidth;
  const double yRes = rasterExtent.height() / rasterHeight;

  int top = 0;
  int bottom = rasterHeight - 1;
  int left = 0;
  int right = rasterWidth - 1;

  if ( subRegion.yMaximum() < rasterExtent.yMaximum() )
  {
    top = static_cast< int >( std::floor( ( rasterExtent.yMaximum() - subRegion.yMaximum() ) / yRes ) );
  }
  if ( subRegion.yMinimum() > rasterExtent.yMinimum() )
  {
    bottom = static_cast< int >( std::ceil( ( rasterExtent.yMaximum() - subRegion.yMinimum() ) / yRes ) - 1 );
  }

  if ( subRegion.xMinimum() > rasterExtent.xMinimum() )
  {
    left = static_cast< int >( std::floor( ( subRegion.xMinimum() - rasterExtent.xMinimum() ) / xRes ) );
  }
  if ( subRegion.xMaximum() < rasterExtent.xMaximum() )
  {
    right = static_cast< int >( std::ceil( ( subRegion.xMaximum() - rasterExtent.xMinimum() ) / xRes ) - 1 );
  }

  subRegionWidth = right - left + 1;
  subRegionHeight = bottom - top + 1;
  subRegionLeft = left;
  subRegionTop = top;

  return QgsRectangle( rasterExtent.xMinimum() + ( left * xRes ),
                       rasterExtent.yMaximum() - ( ( top + subRegionHeight ) * yRes ),
                       rasterExtent.xMinimum() + ( ( left + subRegionWidth ) * xRes ),
                       rasterExtent.yMaximum() - ( top * yRes ) );
}

void QgsRasterIterator::startRasterRead( int bandNumber, qgssize nCols, qgssize nRows, const QgsRectangle &extent, QgsRasterBlockFeedback *feedback )
{
  if ( !mInput )
  {
    return;
  }

  mExtent = extent;
  mFeedback = feedback;

  //remove any previous part on that band
  removePartInfo( bandNumber );

  //split raster into small portions if necessary
  RasterPartInfo pInfo;
  pInfo.nCols = nCols;
  pInfo.nRows = nRows;
  pInfo.currentCol = 0;
  pInfo.currentRow = 0;
  mRasterPartInfos.insert( bandNumber, pInfo );
}

bool QgsRasterIterator::next( int bandNumber, int &columns, int &rows, int &topLeftColumn, int &topLeftRow, QgsRectangle &blockExtent )
{
  return readNextRasterPartInternal( bandNumber, columns, rows, nullptr, topLeftColumn, topLeftRow, &blockExtent );
}

bool QgsRasterIterator::readNextRasterPart( int bandNumber,
    int &nCols, int &nRows,
    QgsRasterBlock **block,
    int &topLeftCol, int &topLeftRow )
{
  *block = nullptr;
  std::unique_ptr< QgsRasterBlock > nextBlock;
  const bool result = readNextRasterPart( bandNumber, nCols, nRows, nextBlock, topLeftCol, topLeftRow );
  if ( result )
    *block = nextBlock.release();
  return result;
}

bool QgsRasterIterator::readNextRasterPart( int bandNumber, int &nCols, int &nRows, std::unique_ptr<QgsRasterBlock> &block, int &topLeftCol, int &topLeftRow, QgsRectangle *blockExtent )
{
  return readNextRasterPartInternal( bandNumber, nCols, nRows, &block, topLeftCol, topLeftRow, blockExtent );
}

bool QgsRasterIterator::readNextRasterPartInternal( int bandNumber, int &nCols, int &nRows, std::unique_ptr<QgsRasterBlock> *block, int &topLeftCol, int &topLeftRow, QgsRectangle *blockExtent )
{
  QgsDebugMsgLevel( QStringLiteral( "Entered" ), 4 );
  if ( block )
    block->reset();
  //get partinfo
  const QMap<int, RasterPartInfo>::iterator partIt = mRasterPartInfos.find( bandNumber );
  if ( partIt == mRasterPartInfos.end() )
  {
    return false;
  }

  RasterPartInfo &pInfo = partIt.value();

  // If we started with zero cols or zero rows, just return (avoids divide by zero below)
  if ( 0 == pInfo.nCols || 0 == pInfo.nRows )
  {
    return false;
  }

  //remove last data block

  //already at end
  if ( pInfo.currentCol == pInfo.nCols && pInfo.currentRow == pInfo.nRows )
  {
    return false;
  }

  //read data block
  nCols = static_cast< int >( std::min( static_cast< qgssize >( mMaximumTileWidth ), pInfo.nCols - pInfo.currentCol ) );
  nRows = static_cast< int >( std::min( static_cast< qgssize >( mMaximumTileHeight ), pInfo.nRows - pInfo.currentRow ) );
  QgsDebugMsgLevel( QStringLiteral( "nCols = %1 nRows = %2" ).arg( nCols ).arg( nRows ), 4 );

  //get subrectangle
  const QgsRectangle viewPortExtent = mExtent;
  const double xmin = viewPortExtent.xMinimum() + pInfo.currentCol / static_cast< double >( pInfo.nCols ) * viewPortExtent.width();
  const double xmax = pInfo.currentCol + nCols == pInfo.nCols ? viewPortExtent.xMaximum() :  // avoid extra FP math if not necessary
                      viewPortExtent.xMinimum() + ( pInfo.currentCol + nCols ) / static_cast< double >( pInfo.nCols ) * viewPortExtent.width();
  const double ymin = pInfo.currentRow + nRows == pInfo.nRows ? viewPortExtent.yMinimum() :  // avoid extra FP math if not necessary
                      viewPortExtent.yMaximum() - ( pInfo.currentRow + nRows ) / static_cast< double >( pInfo.nRows ) * viewPortExtent.height();
  const double ymax = viewPortExtent.yMaximum() - pInfo.currentRow / static_cast< double >( pInfo.nRows ) * viewPortExtent.height();
  const QgsRectangle blockRect( xmin, ymin, xmax, ymax );

  if ( blockExtent )
    *blockExtent = blockRect;

  if ( block )
    block->reset( mInput->block( bandNumber, blockRect, nCols, nRows, mFeedback ) );
  topLeftCol = pInfo.currentCol;
  topLeftRow = pInfo.currentRow;

  pInfo.currentCol += nCols;
  if ( pInfo.currentCol == pInfo.nCols && pInfo.currentRow + nRows == pInfo.nRows ) //end of raster
  {
    pInfo.currentRow = pInfo.nRows;
  }
  else if ( pInfo.currentCol == pInfo.nCols ) //start new row
  {
    pInfo.currentCol = 0;
    pInfo.currentRow += nRows;
  }

  return true;
}

void QgsRasterIterator::stopRasterRead( int bandNumber )
{
  removePartInfo( bandNumber );
}

void QgsRasterIterator::removePartInfo( int bandNumber )
{
  const auto partIt = mRasterPartInfos.constFind( bandNumber );
  if ( partIt != mRasterPartInfos.constEnd() )
  {
    mRasterPartInfos.remove( bandNumber );
  }
}

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

void QgsRasterIterator::startRasterRead( int bandNumber, int nCols, int nRows, const QgsRectangle &extent, QgsRasterBlockFeedback *feedback )
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
  bool result = readNextRasterPart( bandNumber, nCols, nRows, nextBlock, topLeftCol, topLeftRow );
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
  QMap<int, RasterPartInfo>::iterator partIt = mRasterPartInfos.find( bandNumber );
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
  nCols = std::min( mMaximumTileWidth, pInfo.nCols - pInfo.currentCol );
  nRows = std::min( mMaximumTileHeight, pInfo.nRows - pInfo.currentRow );
  QgsDebugMsgLevel( QStringLiteral( "nCols = %1 nRows = %2" ).arg( nCols ).arg( nRows ), 4 );

  //get subrectangle
  QgsRectangle viewPortExtent = mExtent;
  double xmin = viewPortExtent.xMinimum() + pInfo.currentCol / static_cast< double >( pInfo.nCols ) * viewPortExtent.width();
  double xmax = pInfo.currentCol + nCols == pInfo.nCols ? viewPortExtent.xMaximum() :  // avoid extra FP math if not necessary
                viewPortExtent.xMinimum() + ( pInfo.currentCol + nCols ) / static_cast< double >( pInfo.nCols ) * viewPortExtent.width();
  double ymin = pInfo.currentRow + nRows == pInfo.nRows ? viewPortExtent.yMinimum() :  // avoid extra FP math if not necessary
                viewPortExtent.yMaximum() - ( pInfo.currentRow + nRows ) / static_cast< double >( pInfo.nRows ) * viewPortExtent.height();
  double ymax = viewPortExtent.yMaximum() - pInfo.currentRow / static_cast< double >( pInfo.nRows ) * viewPortExtent.height();
  QgsRectangle blockRect( xmin, ymin, xmax, ymax );

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
  auto partIt = mRasterPartInfos.constFind( bandNumber );
  if ( partIt != mRasterPartInfos.constEnd() )
  {
    mRasterPartInfos.remove( bandNumber );
  }
}

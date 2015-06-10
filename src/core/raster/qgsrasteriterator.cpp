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

QgsRasterIterator::QgsRasterIterator( QgsRasterInterface* input ): mInput( input ),
    mMaximumTileWidth( 2000 ), mMaximumTileHeight( 2000 )
{
}

QgsRasterIterator::~QgsRasterIterator()
{
}

void QgsRasterIterator::startRasterRead( int bandNumber, int nCols, int nRows, const QgsRectangle& extent )
{
  if ( !mInput )
  {
    return;
  }

  mExtent = extent;

  //remove any previous part on that band
  removePartInfo( bandNumber );

  //split raster into small portions if necessary
  RasterPartInfo pInfo;
  pInfo.nCols = nCols;
  pInfo.nRows = nRows;
  pInfo.currentCol = 0;
  pInfo.currentRow = 0;
  pInfo.prj = 0;
  mRasterPartInfos.insert( bandNumber, pInfo );
}

bool QgsRasterIterator::readNextRasterPart( int bandNumber,
    int& nCols, int& nRows,
    QgsRasterBlock **block,
    int& topLeftCol, int& topLeftRow )
{
  QgsDebugMsg( "Entered" );
  *block = 0;
  //get partinfo
  QMap<int, RasterPartInfo>::iterator partIt = mRasterPartInfos.find( bandNumber );
  if ( partIt == mRasterPartInfos.end() )
  {
    return false;
  }

  RasterPartInfo& pInfo = partIt.value();

  // If we started with zero cols or zero rows, just return (avoids divide by zero below)
  if ( 0 ==  pInfo.nCols || 0 == pInfo.nRows )
  {
    return false;
  }

  //remove last data block
  delete pInfo.prj;
  pInfo.prj = 0;

  //already at end
  if ( pInfo.currentCol == pInfo.nCols && pInfo.currentRow == pInfo.nRows )
  {
    return false;
  }

  //read data block
  nCols = qMin( mMaximumTileWidth, pInfo.nCols - pInfo.currentCol );
  nRows = qMin( mMaximumTileHeight, pInfo.nRows - pInfo.currentRow );
  QgsDebugMsg( QString( "nCols = %1 nRows = %2" ).arg( nCols ).arg( nRows ) );

  //get subrectangle
  QgsRectangle viewPortExtent = mExtent;
  double xmin = viewPortExtent.xMinimum() + pInfo.currentCol / ( double )pInfo.nCols * viewPortExtent.width();
  double xmax = viewPortExtent.xMinimum() + ( pInfo.currentCol + nCols ) / ( double )pInfo.nCols * viewPortExtent.width();
  double ymin = viewPortExtent.yMaximum() - ( pInfo.currentRow + nRows ) / ( double )pInfo.nRows * viewPortExtent.height();
  double ymax = viewPortExtent.yMaximum() - pInfo.currentRow / ( double )pInfo.nRows * viewPortExtent.height();
  QgsRectangle blockRect( xmin, ymin, xmax, ymax );

  *block = mInput->block( bandNumber, blockRect, nCols, nRows );
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
  QMap<int, RasterPartInfo>::iterator partIt = mRasterPartInfos.find( bandNumber );
  if ( partIt != mRasterPartInfos.end() )
  {
    RasterPartInfo& pInfo = partIt.value();
    delete pInfo.prj;
    mRasterPartInfos.remove( bandNumber );
  }
}

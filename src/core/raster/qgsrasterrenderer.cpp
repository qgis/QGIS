/***************************************************************************
                         qgsrasterrenderer.cpp
                         ---------------------
    begin                : December 2011
    copyright            : (C) 2011 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrasterrenderer.h"
#include "qgsrasterviewport.h"
#include "qgsmaptopixel.h"

QgsRasterRenderer::QgsRasterRenderer( QgsRasterDataProvider* provider, QgsRasterResampler* resampler ): mProvider( provider ), mResampler( resampler )
{
}

QgsRasterRenderer::~QgsRasterRenderer()
{
  //remove remaining memory in partinfos
  QMap<int, RasterPartInfo>::iterator partIt = mRasterPartInfos.begin();
  for ( ; partIt != mRasterPartInfos.end(); ++partIt )
  {
    CPLFree( partIt.value().data );
  }
}

void QgsRasterRenderer::startRasterRead( int bandNumber, QgsRasterViewPort* viewPort, const QgsMapToPixel* mapToPixel, double& oversampling )
{
  oversampling = 1.0; //default (e.g. for nearest neighbour)
  if ( !viewPort || !mapToPixel || !mProvider )
  {
    return;
  }

  //remove any previous part on that band
  removePartInfo( bandNumber );

  //calculate oversampling factor
  if ( mResampler )
  {
    QgsRectangle providerExtent = mProvider->extent();
    if ( viewPort->mSrcCRS.isValid() && viewPort->mDestCRS.isValid() && viewPort->mSrcCRS != viewPort->mDestCRS )
    {
      QgsCoordinateTransform t( viewPort->mSrcCRS, viewPort->mDestCRS );
      providerExtent = t.transformBoundingBox( providerExtent );
    }
    double pixelRatio = mapToPixel->mapUnitsPerPixel() / ( providerExtent.width() / mProvider->xSize() );
    oversampling = pixelRatio > 1.0 ? 2.5 : pixelRatio;
  }

  //split raster into small portions if necessary
  RasterPartInfo pInfo;
  pInfo.nCols = viewPort->drawableAreaXDim * oversampling;
  pInfo.nRows = viewPort->drawableAreaYDim * oversampling;
  int totalMemoryUsage = pInfo.nCols * pInfo.nRows * mProvider->dataTypeSize( bandNumber );
  int parts = totalMemoryUsage / /*100000000*/ 100000 + 1;
  pInfo.nPartsPerDimension = sqrt( parts );
  pInfo.nColsPerPart = pInfo.nCols / pInfo.nPartsPerDimension;
  pInfo.nRowsPerPart = pInfo.nRows / pInfo.nPartsPerDimension;
  pInfo.currentCol = 0;
  pInfo.currentRow = 0;
  pInfo.data = 0;
  mRasterPartInfos.insert( bandNumber, pInfo );
}

bool QgsRasterRenderer::readNextRasterPart( int bandNumber, QgsRasterViewPort* viewPort, int& nCols, int& nRows, void** rasterData, int& topLeftCol, int& topLeftRow )
{
  if ( !viewPort )
  {
    return false;
  }

  //get partinfo
  QMap<int, RasterPartInfo>::iterator partIt = mRasterPartInfos.find( bandNumber );
  if ( partIt == mRasterPartInfos.end() )
  {
    return false;
  }

  RasterPartInfo& pInfo = partIt.value();

  //remove last data block
  CPLFree( pInfo.data );
  pInfo.data = 0;

  //already at end
  if ( pInfo.currentCol == pInfo.nCols && pInfo.currentRow == pInfo.nRows )
  {
    return false;
  }

  //read data block
  nCols = qMin( pInfo.nColsPerPart, pInfo.nCols - pInfo.currentCol );
  nRows = qMin( pInfo.nRowsPerPart, pInfo.nRows - pInfo.currentRow );
  int typeSize = mProvider->dataTypeSize( bandNumber ) / 8;
  pInfo.data = VSIMalloc( typeSize * nCols *  nRows );

  //get subrectangle
  QgsRectangle viewPortExtent = viewPort->mDrawnExtent;
  double xmin = viewPortExtent.xMinimum() + pInfo.currentCol / ( double )pInfo.nCols * viewPortExtent.width();
  double xmax = viewPortExtent.xMinimum() + ( pInfo.currentCol + nCols ) / ( double )pInfo.nCols * viewPortExtent.width();
  double ymin = viewPortExtent.yMaximum() - ( pInfo.currentRow + nRows ) / ( double )pInfo.nRows * viewPortExtent.height();
  double ymax = viewPortExtent.yMaximum() - pInfo.currentRow / ( double )pInfo.nRows * viewPortExtent.height();
  QgsRectangle blockRect( xmin, ymin, xmax, ymax );

  mProvider->readBlock( bandNumber, blockRect, nCols, nRows, viewPort->mSrcCRS, viewPort->mDestCRS, pInfo.data );
  *rasterData = pInfo.data;
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
    pInfo.currentRow += pInfo.nRowsPerPart;
  }

  return true;
}

void QgsRasterRenderer::stopRasterRead( int bandNumber )
{
  removePartInfo( bandNumber );
}

void QgsRasterRenderer::removePartInfo( int bandNumber )
{
  QMap<int, RasterPartInfo>::iterator partIt = mRasterPartInfos.find( bandNumber );
  if ( partIt != mRasterPartInfos.end() )
  {
    RasterPartInfo& pInfo = partIt.value();
    CPLFree( pInfo.data );
    mRasterPartInfos.remove( bandNumber );
  }
}

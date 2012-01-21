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
#include "qgsrasterresampler.h"
#include "qgsrastertransparency.h"
#include "qgsrasterviewport.h"
#include "qgsmaptopixel.h"
#include <QImage>
#include <QPainter>

QgsRasterRenderer::QgsRasterRenderer( QgsRasterDataProvider* provider ): mProvider( provider ), mZoomedInResampler( 0 ), mZoomedOutResampler( 0 ),
    mOpacity( 1.0 ), mRasterTransparency( 0 ), mAlphaBand( -1 ), mInvertColor( false )
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

  delete mZoomedInResampler;
  delete mZoomedOutResampler;
}

void QgsRasterRenderer::setZoomedInResampler( QgsRasterResampler* r )
{
  delete mZoomedInResampler;
  mZoomedInResampler = r;
}

void QgsRasterRenderer::setZoomedOutResampler( QgsRasterResampler* r )
{
  delete mZoomedOutResampler;
  mZoomedOutResampler = r;
}

void QgsRasterRenderer::startRasterRead( int bandNumber, QgsRasterViewPort* viewPort, const QgsMapToPixel* mapToPixel, double& oversamplingX, double& oversamplingY )
{
  if ( !viewPort || !mapToPixel || !mProvider )
  {
    return;
  }

  //remove any previous part on that band
  removePartInfo( bandNumber );

  //calculate oversampling factor
  double oversampling = 1.0; //approximate global oversampling factor

  if ( mZoomedInResampler || mZoomedOutResampler )
  {
    QgsRectangle providerExtent = mProvider->extent();
    if ( viewPort->mSrcCRS.isValid() && viewPort->mDestCRS.isValid() && viewPort->mSrcCRS != viewPort->mDestCRS )
    {
      QgsCoordinateTransform t( viewPort->mSrcCRS, viewPort->mDestCRS );
      providerExtent = t.transformBoundingBox( providerExtent );
    }
    double pixelRatio = mapToPixel->mapUnitsPerPixel() / ( providerExtent.width() / mProvider->xSize() );
    oversampling = ( pixelRatio > 4.0 ) ? 4.0 : pixelRatio;
  }

  //set oversampling back to 1.0 if no resampler for zoomed in / zoomed out (nearest neighbour)
  if (( oversampling < 1.0 && !mZoomedInResampler ) || ( oversampling > 1.0 && !mZoomedOutResampler ) )
  {
    oversampling = 1.0;
  }

  //split raster into small portions if necessary
  RasterPartInfo pInfo;
  pInfo.nCols = viewPort->drawableAreaXDim;
  pInfo.nRows = viewPort->drawableAreaYDim;

  //effective oversampling factors are different to global one because of rounding
  oversamplingX = (( double )pInfo.nCols * oversampling ) / viewPort->drawableAreaXDim;
  oversamplingY = (( double )pInfo.nRows * oversampling ) / viewPort->drawableAreaYDim;

  int totalMemoryUsage = pInfo.nCols * oversamplingX * pInfo.nRows * oversamplingY * mProvider->dataTypeSize( bandNumber );
  int parts = totalMemoryUsage / 100000000 + 1;
  int nPartsPerDimension = sqrt( parts );
  pInfo.nColsPerPart = pInfo.nCols / nPartsPerDimension;
  pInfo.nRowsPerPart = pInfo.nRows / nPartsPerDimension;
  pInfo.currentCol = 0;
  pInfo.currentRow = 0;
  pInfo.data = 0;
  mRasterPartInfos.insert( bandNumber, pInfo );
}

bool QgsRasterRenderer::readNextRasterPart( int bandNumber, double oversamplingX, double oversamplingY, QgsRasterViewPort* viewPort,
    int& nCols, int& nRows, int& nColsRaster, int& nRowsRaster, void** rasterData, int& topLeftCol, int& topLeftRow )
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

  //get subrectangle
  QgsRectangle viewPortExtent = viewPort->mDrawnExtent;
  double xmin = viewPortExtent.xMinimum() + pInfo.currentCol / ( double )pInfo.nCols * viewPortExtent.width();
  double xmax = viewPortExtent.xMinimum() + ( pInfo.currentCol + nCols ) / ( double )pInfo.nCols * viewPortExtent.width();
  double ymin = viewPortExtent.yMaximum() - ( pInfo.currentRow + nRows ) / ( double )pInfo.nRows * viewPortExtent.height();
  double ymax = viewPortExtent.yMaximum() - pInfo.currentRow / ( double )pInfo.nRows * viewPortExtent.height();
  QgsRectangle blockRect( xmin, ymin, xmax, ymax );

  nColsRaster = nCols * oversamplingX;
  nRowsRaster = nRows * oversamplingY;
  pInfo.data = VSIMalloc( typeSize * nColsRaster *  nRowsRaster );
  mProvider->readBlock( bandNumber, blockRect, nColsRaster, nRowsRaster, viewPort->mSrcCRS, viewPort->mDestCRS, pInfo.data );
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

bool QgsRasterRenderer::usesTransparency( QgsCoordinateReferenceSystem& srcSRS, QgsCoordinateReferenceSystem& dstSRS ) const
{
  //transparency is always used if on-the-fly reprojection is enabled
  bool reprojectionEnabled = ( srcSRS.isValid() && dstSRS.isValid() && srcSRS != dstSRS );
  if ( !mProvider || reprojectionEnabled )
  {
    return true;
  }
  return ( mAlphaBand > 0 || ( mRasterTransparency && !mRasterTransparency->isEmpty( mProvider->noDataValue() ) ) || !doubleNear( mOpacity, 1.0 ) );
}

void QgsRasterRenderer::drawImage( QPainter* p, QgsRasterViewPort* viewPort, const QImage& img, int topLeftCol, int topLeftRow,
                                   int nCols, int nRows, double oversamplingX, double oversamplingY ) const
{
  if ( !p || !viewPort )
  {
    return;
  }

  //top left position in device coords
  QPoint tlPoint = QPoint( viewPort->topLeftPoint.x() + topLeftCol, viewPort->topLeftPoint.y() + topLeftRow );

  //resample and draw image
  if (( mZoomedInResampler || mZoomedOutResampler ) && !doubleNear( oversamplingX, 1.0 ) && !doubleNear( oversamplingY, 1.0 ) )
  {
    QImage dstImg( nCols, nRows, QImage::Format_ARGB32_Premultiplied );
    if ( mZoomedInResampler && oversamplingX < 1.0 )
    {
      mZoomedInResampler->resample( img, dstImg );
    }
    else if ( mZoomedOutResampler && oversamplingX > 1.0 )
    {
      mZoomedOutResampler->resample( img, dstImg );
    }

    p->drawImage( tlPoint, dstImg );
  }
  else //use original image
  {
    p->drawImage( tlPoint, img );
  }
}

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
#include "qgsrasterprojector.h"
#include "qgsrastertransparency.h"
#include "qgsrasterviewport.h"
#include "qgsmaptopixel.h"

//resamplers
#include "qgsbilinearrasterresampler.h"
#include "qgscubicrasterresampler.h"

#include <QDomDocument>
#include <QDomElement>
#include <QImage>
#include <QPainter>

//QgsRasterRenderer::QgsRasterRenderer( QgsRasterFace* input, const QString& type ): mInput( input ),
QgsRasterRenderer::QgsRasterRenderer( QgsRasterFace* input, const QString& type ): QgsRasterFace( input ),
    mType( type ), mZoomedInResampler( 0 ), mZoomedOutResampler( 0 ), mOpacity( 1.0 ), mRasterTransparency( 0 ),
    mAlphaBand( -1 ), mInvertColor( false ), mMaxOversampling( 2.0 )
{
}

QgsRasterRenderer::~QgsRasterRenderer()
{
  //remove remaining memory in partinfos
  //QMap<int, RasterPartInfo>::iterator partIt = mRasterPartInfos.begin();
  //for ( ; partIt != mRasterPartInfos.end(); ++partIt )
  //{
  //CPLFree( partIt.value().data );
  //}

  delete mZoomedInResampler;
  delete mZoomedOutResampler;
  delete mRasterTransparency;
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

/*
void QgsRasterRenderer::startRasterRead( int bandNumber, QgsRasterViewPort* viewPort, const QgsMapToPixel* mapToPixel, double& oversamplingX, double& oversamplingY )
{
  if ( !viewPort || !mapToPixel || !mInput )
  {
    return;
  }

  //remove any previous part on that band
  removePartInfo( bandNumber );

  //calculate oversampling factor
  double oversampling = 1.0; //approximate global oversampling factor

  if ( mZoomedInResampler || mZoomedOutResampler )
  {
    QgsRectangle providerExtent = mInput->extent();
    if ( viewPort->mSrcCRS.isValid() && viewPort->mDestCRS.isValid() && viewPort->mSrcCRS != viewPort->mDestCRS )
    {
      QgsCoordinateTransform t( viewPort->mSrcCRS, viewPort->mDestCRS );
      providerExtent = t.transformBoundingBox( providerExtent );
    }
    double pixelRatio = mapToPixel->mapUnitsPerPixel() / ( providerExtent.width() / mInput->xSize() );
    oversampling = ( pixelRatio > mMaxOversampling ) ? mMaxOversampling : pixelRatio;
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

  int totalMemoryUsage = pInfo.nCols * oversamplingX * pInfo.nRows * oversamplingY * mInput->dataTypeSize( bandNumber );
  int parts = totalMemoryUsage / 100000000 + 1;
  int nPartsPerDimension = sqrt(( double ) parts );
  pInfo.nColsPerPart = pInfo.nCols / nPartsPerDimension;
  pInfo.nRowsPerPart = pInfo.nRows / nPartsPerDimension;
  pInfo.currentCol = 0;
  pInfo.currentRow = 0;
  pInfo.data = 0;
  pInfo.prj = 0;
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
  delete pInfo.prj;
  pInfo.prj = 0;

  //already at end
  if ( pInfo.currentCol == pInfo.nCols && pInfo.currentRow == pInfo.nRows )
  {
    return false;
  }

  //read data block
  nCols = qMin( pInfo.nColsPerPart, pInfo.nCols - pInfo.currentCol );
  nRows = qMin( pInfo.nRowsPerPart, pInfo.nRows - pInfo.currentRow );
  int typeSize = mInput->dataTypeSize( bandNumber ) / 8;

  //get subrectangle
  QgsRectangle viewPortExtent = viewPort->mDrawnExtent;
  double xmin = viewPortExtent.xMinimum() + pInfo.currentCol / ( double )pInfo.nCols * viewPortExtent.width();
  double xmax = viewPortExtent.xMinimum() + ( pInfo.currentCol + nCols ) / ( double )pInfo.nCols * viewPortExtent.width();
  double ymin = viewPortExtent.yMaximum() - ( pInfo.currentRow + nRows ) / ( double )pInfo.nRows * viewPortExtent.height();
  double ymax = viewPortExtent.yMaximum() - pInfo.currentRow / ( double )pInfo.nRows * viewPortExtent.height();
  QgsRectangle blockRect( xmin, ymin, xmax, ymax );

  if ( viewPort->mSrcCRS.isValid() && viewPort->mDestCRS.isValid() && viewPort->mSrcCRS != viewPort->mDestCRS )
  {
    pInfo.prj = new QgsRasterProjector( viewPort->mSrcCRS,
                                        viewPort->mDestCRS, blockRect, nRows, nCols, 0, 0, mInput->extent() );

    // If we zoom out too much, projector srcRows / srcCols maybe 0, which can cause problems in providers
    if ( pInfo.prj->srcRows() <= 0 || pInfo.prj->srcCols() <= 0 )
    {
      delete pInfo.prj;
      pInfo.prj = 0;
      return false;
    }

    blockRect = pInfo.prj->srcExtent();
  }

  if ( pInfo.prj )
  {
    nColsRaster = pInfo.prj->srcCols() * oversamplingX;
    nRowsRaster = pInfo.prj->srcRows() * oversamplingY;
  }
  else
  {
    nColsRaster = nCols * oversamplingX;
    nRowsRaster = nRows * oversamplingY;
  }
  //pInfo.data = VSIMalloc( typeSize * nColsRaster *  nRowsRaster );
  //mInput->readBlock( bandNumber, blockRect, nColsRaster, nRowsRaster, pInfo.data );
  pInfo.data = mInput->readBlock( bandNumber, blockRect, nColsRaster, nRowsRaster );

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
    delete pInfo.prj;
    mRasterPartInfos.remove( bandNumber );
  }
}
*/

bool QgsRasterRenderer::usesTransparency( QgsCoordinateReferenceSystem& srcSRS, QgsCoordinateReferenceSystem& dstSRS ) const
{
  //transparency is always used if on-the-fly reprojection is enabled
  bool reprojectionEnabled = ( srcSRS.isValid() && dstSRS.isValid() && srcSRS != dstSRS );
  if ( !mInput || reprojectionEnabled )
  {
    return true;
  }
  return ( mAlphaBand > 0 || ( mRasterTransparency && !mRasterTransparency->isEmpty( mInput->noDataValue() ) ) || !doubleNear( mOpacity, 1.0 ) );
}

void QgsRasterRenderer::setRasterTransparency( QgsRasterTransparency* t )
{
  delete mRasterTransparency;
  mRasterTransparency = t;
}

/*
void QgsRasterRenderer::drawImage( QPainter* p, QgsRasterViewPort* viewPort, const QImage& img, int topLeftCol, int topLeftRow,
                                   int nCols, int nRows, double oversamplingX, double oversamplingY ) const
{
  if ( !p || !viewPort )
  {
    return;
  }

  //get QgsRasterProjector
  QgsRasterProjector* prj = 0;
  QMap<int, RasterPartInfo>::const_iterator partInfoIt = mRasterPartInfos.constBegin();
  if ( partInfoIt != mRasterPartInfos.constEnd() )
  {
    prj = partInfoIt->prj;
  }

  //top left position in device coords
  QPoint tlPoint = QPoint( viewPort->topLeftPoint.x() + topLeftCol, viewPort->topLeftPoint.y() + topLeftRow );

  //resample and draw image
  if (( mZoomedInResampler || mZoomedOutResampler ) && !doubleNear( oversamplingX, 1.0 ) && !doubleNear( oversamplingY, 1.0 ) )
  {
    QImage dstImg;
    if ( prj )
    {
      dstImg = QImage( prj->srcCols(), prj->srcRows(), QImage::Format_ARGB32_Premultiplied );
    }
    else
    {
      dstImg = QImage( nCols, nRows, QImage::Format_ARGB32_Premultiplied );
    }
    if ( mZoomedInResampler && oversamplingX < 1.0 )
    {
      mZoomedInResampler->resample( img, dstImg );
    }
    else if ( mZoomedOutResampler && oversamplingX > 1.0 )
    {
      mZoomedOutResampler->resample( img, dstImg );
    }

    if ( prj )
    {
      QImage projectedImg( nCols, nRows, QImage::Format_ARGB32_Premultiplied );
      projectImage( dstImg, projectedImg, prj );
      p->drawImage( tlPoint, projectedImg );
    }
    else
    {
      p->drawImage( tlPoint, dstImg );
    }
  }
  else //use original image
  {
    if ( prj )
    {
      QImage projectedImg( nCols, nRows, QImage::Format_ARGB32_Premultiplied );
      projectImage( img, projectedImg, prj );
      p->drawImage( tlPoint, projectedImg );
    }
    else
    {
      p->drawImage( tlPoint, img );
    }
  }
}

void QgsRasterRenderer::projectImage( const QImage& srcImg, QImage& dstImage, QgsRasterProjector* prj ) const
{
  int nRows = dstImage.height();
  int nCols = dstImage.width();
  int srcRow, srcCol;
  for ( int i = 0; i < nRows; ++i )
  {
    for ( int j = 0; j < nCols; ++j )
    {
      prj->srcRowCol( i, j, &srcRow, &srcCol );
      dstImage.setPixel( j, i, srcImg.pixel( srcCol, srcRow ) );
    }
  }
}
*/

void QgsRasterRenderer::_writeXML( QDomDocument& doc, QDomElement& rasterRendererElem ) const
{
  if ( rasterRendererElem.isNull() )
  {
    return;
  }

  rasterRendererElem.setAttribute( "type", mType );
  rasterRendererElem.setAttribute( "opacity", QString::number( mOpacity ) );
  rasterRendererElem.setAttribute( "alphaBand", mAlphaBand );
  rasterRendererElem.setAttribute( "maxOversampling", QString::number( mMaxOversampling ) );
  rasterRendererElem.setAttribute( "invertColor", mInvertColor );
  if ( mZoomedInResampler )
  {
    rasterRendererElem.setAttribute( "zoomedInResampler", mZoomedInResampler->type() );
  }
  if ( mZoomedOutResampler )
  {
    rasterRendererElem.setAttribute( "zoomedOutResampler", mZoomedOutResampler->type() );
  }

  if ( mRasterTransparency )
  {
    mRasterTransparency->writeXML( doc, rasterRendererElem );
  }
}

void QgsRasterRenderer::readXML( const QDomElement& rendererElem )
{
  if ( rendererElem.isNull() )
  {
    return;
  }

  mType = rendererElem.attribute( "type" );
  mOpacity = rendererElem.attribute( "opacity", "1.0" ).toDouble();
  mAlphaBand = rendererElem.attribute( "alphaBand", "-1" ).toInt();
  mMaxOversampling = rendererElem.attribute( "maxOversampling", "2.0" ).toDouble();
  mInvertColor = rendererElem.attribute( "invertColor", "0" ).toInt();

  QString zoomedInResamplerType = rendererElem.attribute( "zoomedInResampler" );
  if ( zoomedInResamplerType == "bilinear" )
  {
    mZoomedInResampler = new QgsBilinearRasterResampler();
  }
  else if ( zoomedInResamplerType == "cubic" )
  {
    mZoomedInResampler = new QgsCubicRasterResampler();
  }

  QString zoomedOutResamplerType = rendererElem.attribute( "zoomedOutResampler" );
  if ( zoomedOutResamplerType == "bilinear" )
  {
    mZoomedOutResampler = new QgsBilinearRasterResampler();
  }

  //todo: read mRasterTransparency
  QDomElement rasterTransparencyElem = rendererElem.firstChildElement( "rasterTransparency" );
  if ( !rasterTransparencyElem.isNull() )
  {
    delete mRasterTransparency;
    mRasterTransparency = new QgsRasterTransparency();
    mRasterTransparency->readXML( rasterTransparencyElem );
  }
}

/***************************************************************************
                         qgsmultibandcolorrenderer.cpp
                         -----------------------------
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

#include "qgsmultibandcolorrenderer.h"
#include "qgsmaptopixel.h"
#include "qgsrasterresampler.h"
#include "qgsrasterviewport.h"
#include <QImage>
#include <QPainter>

QgsMultiBandColorRenderer::QgsMultiBandColorRenderer( QgsRasterDataProvider* provider, int redBand, int greenBand, int blueBand, QgsRasterResampler* resampler ):
    QgsRasterRenderer( provider, resampler ), mRedBand( redBand ), mGreenBand( greenBand ), mBlueBand( blueBand )
{
}

QgsMultiBandColorRenderer::~QgsMultiBandColorRenderer()
{
}

void QgsMultiBandColorRenderer::draw( QPainter* p, QgsRasterViewPort* viewPort, const QgsMapToPixel* theQgsMapToPixel )
{
  if ( !p || !mProvider || !viewPort || !theQgsMapToPixel )
  {
    return;
  }

  //read data from provider
  int redTypeSize = mProvider->dataTypeSize( mRedBand ) / 8;
  int greenTypeSize = mProvider->dataTypeSize( mGreenBand ) / 8;
  int blueTypeSize = mProvider->dataTypeSize( mBlueBand ) / 8;

  QgsRasterDataProvider::DataType redType = ( QgsRasterDataProvider::DataType )mProvider->dataType( mRedBand );
  QgsRasterDataProvider::DataType greenType = ( QgsRasterDataProvider::DataType )mProvider->dataType( mGreenBand );
  QgsRasterDataProvider::DataType blueType = ( QgsRasterDataProvider::DataType )mProvider->dataType( mBlueBand );

  int nCols, nRows;

  if ( mResampler )
  {
    //read data at source resolution if zoomed in, else do oversampling with factor 2.5
    QgsRectangle providerExtent = mProvider->extent();
    if ( viewPort->mSrcCRS.isValid() && viewPort->mDestCRS.isValid() && viewPort->mSrcCRS != viewPort->mDestCRS )
    {
      QgsCoordinateTransform t( viewPort->mSrcCRS, viewPort->mDestCRS );
      providerExtent = t.transformBoundingBox( providerExtent );
    }
    double pixelRatio = theQgsMapToPixel->mapUnitsPerPixel() / ( providerExtent.width() / mProvider->xSize() );
    double oversampling = pixelRatio > 1.0 ? 2.5 : pixelRatio;
    nCols = viewPort->drawableAreaXDim * oversampling;
    nRows = viewPort->drawableAreaYDim * oversampling;
  }
  else
  {
    nCols = viewPort->drawableAreaXDim;
    nRows = viewPort->drawableAreaYDim;
  }

  void* redData = VSIMalloc( redTypeSize * nCols *  nRows );
  void* greenData = VSIMalloc( greenTypeSize * nCols *  nRows );
  void* blueData = VSIMalloc( blueTypeSize * nCols *  nRows );

  mProvider->readBlock( mRedBand, viewPort->mDrawnExtent, nCols, nRows,
                        viewPort->mSrcCRS, viewPort->mDestCRS, redData );
  mProvider->readBlock( mGreenBand, viewPort->mDrawnExtent, nCols, nRows,
                        viewPort->mSrcCRS, viewPort->mDestCRS, greenData );
  mProvider->readBlock( mBlueBand, viewPort->mDrawnExtent, nCols, nRows,
                        viewPort->mSrcCRS, viewPort->mDestCRS, blueData );

  QImage img( nCols, nRows, QImage::Format_ARGB32_Premultiplied );
  QRgb* imageScanLine = 0;
  int currentRasterPos = 0;
  int redVal, greenVal, blueVal;

  for ( int i = 0; i < nRows; ++i )
  {
    imageScanLine = ( QRgb* )( img.scanLine( i ) );
    for ( int j = 0; j < nCols; ++j )
    {
      redVal = readValue( redData, redType, currentRasterPos );
      greenVal = readValue( greenData, greenType, currentRasterPos );
      blueVal = readValue( blueData, blueType, currentRasterPos );
      imageScanLine[j] = qRgba( redVal, greenVal, blueVal, 255 );
      ++currentRasterPos;
    }
  }

  CPLFree( redData );
  CPLFree( greenData );
  CPLFree( blueData );

  if ( mResampler )
  {
    QImage dstImg( viewPort->drawableAreaXDim, viewPort->drawableAreaYDim, QImage::Format_ARGB32_Premultiplied );
    mResampler->resample( img, dstImg );
    p->drawImage( QPointF( viewPort->topLeftPoint.x(), viewPort->topLeftPoint.y() ), dstImg );
  }
  else
  {
    p->drawImage( QPointF( viewPort->topLeftPoint.x(), viewPort->topLeftPoint.y() ), img );
  }
}

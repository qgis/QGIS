/***************************************************************************
                         qgspalettedrasterrenderer.cpp
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

#include "qgspalettedrasterrenderer.h"
#include "qgscoordinatetransform.h"
#include "qgsmaptopixel.h"
#include "qgsrasterviewport.h"
#include <QColor>
#include <QImage>
#include <QPainter>

#include "qgsbilinearrasterresampler.h"

QgsPalettedRasterRenderer::QgsPalettedRasterRenderer( QgsRasterDataProvider* provider, int bandNumber,
    QColor* colorArray, int nColors, QgsRasterResampler* resampler ):
    QgsRasterRenderer( provider, resampler ), mBandNumber( bandNumber ), mColors( colorArray ), mNColors( nColors )
{
}

QgsPalettedRasterRenderer::~QgsPalettedRasterRenderer()
{
  delete[] mColors;
}

void QgsPalettedRasterRenderer::draw( QPainter* p, QgsRasterViewPort* viewPort, const QgsMapToPixel* theQgsMapToPixel )
{
  if ( !p || !mProvider || !viewPort || !theQgsMapToPixel )
  {
    return;
  }

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

  //read data from provider
  int typeSize = mProvider->dataTypeSize( mBandNumber ) / 8;
  QgsRasterDataProvider::DataType rasterType = ( QgsRasterDataProvider::DataType )mProvider->dataType( mBandNumber );
  void* rasterData = VSIMalloc( typeSize * nCols *  nRows );
  mProvider->readBlock( mBandNumber, viewPort->mDrawnExtent, nCols, nRows,
                        viewPort->mSrcCRS, viewPort->mDestCRS, rasterData );
  int currentRasterPos = 0;

  QImage img( nCols, nRows, QImage::Format_ARGB32_Premultiplied );
  QRgb* imageScanLine = 0;

  for ( int i = 0; i < nRows; ++i )
  {
    imageScanLine = ( QRgb* )( img.scanLine( i ) );
    for ( int j = 0; j < nCols; ++j )
    {
      int val = readValue( rasterData, rasterType, currentRasterPos );
      imageScanLine[j] = mColors[ val ].rgba();
      ++currentRasterPos;
    }
  }
  CPLFree( rasterData );

  if ( mResampler ) //resample to output resolution
  {
    QImage dstImg( viewPort->drawableAreaXDim, viewPort->drawableAreaYDim, QImage::Format_ARGB32_Premultiplied );
    QgsBilinearRasterResampler rs;
    rs.resample( img, dstImg );
    p->drawImage( QPointF( viewPort->topLeftPoint.x(), viewPort->topLeftPoint.y() ), dstImg );
  }
  else //use original image
  {
    p->drawImage( QPointF( viewPort->topLeftPoint.x(), viewPort->topLeftPoint.y() ), img );
  }
}

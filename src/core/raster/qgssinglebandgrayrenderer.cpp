/***************************************************************************
                         qgssinglebandgrayrenderer.cpp
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

#include "qgssinglebandgrayrenderer.h"
#include "qgscontrastenhancement.h"
#include "qgsrastertransparency.h"
#include <QImage>

QgsSingleBandGrayRenderer::QgsSingleBandGrayRenderer( QgsRasterDataProvider* provider, int grayBand, QgsRasterResampler* resampler ):
    QgsRasterRenderer( provider, resampler ), mGrayBand( grayBand ), mContrastEnhancement( 0 )
{
}

QgsSingleBandGrayRenderer::~QgsSingleBandGrayRenderer()
{
}

void QgsSingleBandGrayRenderer::draw( QPainter* p, QgsRasterViewPort* viewPort, const QgsMapToPixel* theQgsMapToPixel )
{
  if ( !p || !mProvider || !viewPort || !theQgsMapToPixel )
  {
    return;
  }

  double oversamplingX, oversamplingY;
  startRasterRead( mGrayBand, viewPort, theQgsMapToPixel, oversamplingX, oversamplingY );
  if ( mAlphaBand > 0 && mGrayBand != mAlphaBand )
  {
    startRasterRead( mAlphaBand, viewPort, theQgsMapToPixel, oversamplingX, oversamplingY );
  }

  int nCols = 0;
  int nRows = 0;
  int topLeftCol = 0;
  int topLeftRow = 0;
  QgsRasterDataProvider::DataType rasterType = ( QgsRasterDataProvider::DataType )mProvider->dataType( mGrayBand );
  QgsRasterDataProvider::DataType alphaType = QgsRasterDataProvider::UnknownDataType;
  if ( mAlphaBand > 0 )
  {
    alphaType = ( QgsRasterDataProvider::DataType )mProvider->dataType( mAlphaBand );
  }
  void* rasterData;
  void* alphaData;
  double currentAlpha = mOpacity;
  int grayVal;
  QRgb myDefaultColor = qRgba( 0, 0, 0, 0 );


  while ( readNextRasterPart( mGrayBand, viewPort, nCols, nRows, &rasterData, topLeftCol, topLeftRow ) )
  {
    if ( mAlphaBand > 0 && mGrayBand != mAlphaBand )
    {
      readNextRasterPart( mAlphaBand, viewPort, nCols, nRows, &alphaData, topLeftCol, topLeftRow );
    }
    else if ( mAlphaBand > 0 )
    {
      alphaData = rasterData;
    }


    //create image
    QImage img( nCols, nRows, QImage::Format_ARGB32_Premultiplied );
    QRgb* imageScanLine = 0;
    int currentRasterPos = 0;

    for ( int i = 0; i < nRows; ++i )
    {
      imageScanLine = ( QRgb* )( img.scanLine( i ) );
      for ( int j = 0; j < nCols; ++j )
      {
        grayVal = readValue( rasterData, rasterType, currentRasterPos );

        if ( mContrastEnhancement )
        {
          if ( !mContrastEnhancement->isValueInDisplayableRange( grayVal ) )
          {
            imageScanLine[ j ] = myDefaultColor;
            ++currentRasterPos;
            continue;
          }
          grayVal = mContrastEnhancement->enhanceContrast( grayVal );
        }

        if ( mInvertColor )
        {
          grayVal = 255 - grayVal;
        }

        //alpha
        currentAlpha = mOpacity;
        if ( mRasterTransparency )
        {
          currentAlpha = mRasterTransparency->alphaValue( grayVal, mOpacity * 255 ) / 255.0;
        }
        if ( mAlphaBand > 0 )
        {
          currentAlpha *= ( readValue( alphaData, alphaType, currentRasterPos ) / 255.0 );
        }

        if ( doubleNear( currentAlpha, 255 ) )
        {
          imageScanLine[j] = qRgba( grayVal, grayVal, grayVal, 255 );
        }
        else
        {
          imageScanLine[j] = qRgba( currentAlpha * grayVal, currentAlpha * grayVal, currentAlpha * grayVal, currentAlpha * 255 );
        }
        ++currentRasterPos;
      }
    }

    drawImage( p, viewPort, img, topLeftCol, topLeftRow, nCols, nRows, oversamplingX, oversamplingY );
  }

  stopRasterRead( mGrayBand );
  if ( mAlphaBand > 0 && mGrayBand != mAlphaBand )
  {
    stopRasterRead( mAlphaBand );
  }

}

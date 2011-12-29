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
#include "qgsrasterresampler.h"
#include "qgsrastertransparency.h"
#include "qgsrasterviewport.h"
#include <QColor>
#include <QImage>
#include <QPainter>

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

  double oversamplingX, oversamplingY;
  QgsRasterDataProvider::DataType transparencyType;
  if ( mAlphaBand > 0 )
  {
    transparencyType = ( QgsRasterDataProvider::DataType )mProvider->dataType( mAlphaBand );
  }
  startRasterRead( mBandNumber, viewPort, theQgsMapToPixel, oversamplingX, oversamplingY );

  //Read alpha band if necessary
  if ( mAlphaBand > 0 && mAlphaBand != mBandNumber )
  {
    startRasterRead( mAlphaBand, viewPort, theQgsMapToPixel, oversamplingX, oversamplingY );
  }

  int nCols = 0;
  int nRows = 0;
  int topLeftCol = 0;
  int topLeftRow = 0;
  int currentRasterPos = 0;
  QgsRasterDataProvider::DataType rasterType = ( QgsRasterDataProvider::DataType )mProvider->dataType( mBandNumber );
  void* rasterData;
  double currentOpacity = mOpacity;

  bool hasTransparency = usesTransparency();
  void* transparencyData;

  while ( readNextRasterPart( mBandNumber, viewPort, nCols, nRows, &rasterData, topLeftCol, topLeftRow ) )
  {
    if ( mAlphaBand > 0 && mAlphaBand != mBandNumber )
    {
      readNextRasterPart( mAlphaBand, viewPort, nCols, nRows, &transparencyData, topLeftCol, topLeftRow );
    }
    else if ( mAlphaBand == mBandNumber )
    {
      transparencyData = rasterData;
    }

    //create image
    QImage img( nCols, nRows, QImage::Format_ARGB32_Premultiplied );
    QRgb* imageScanLine = 0;
    int val = 0;
    currentRasterPos = 0;

    for ( int i = 0; i < nRows; ++i )
    {
      imageScanLine = ( QRgb* )( img.scanLine( i ) );
      for ( int j = 0; j < nCols; ++j )
      {
        val = readValue( rasterData, rasterType, currentRasterPos );
        if ( !hasTransparency )
        {
          imageScanLine[j] = mColors[ val ].rgba();
        }
        else
        {
          currentOpacity = mOpacity;
          if ( mRasterTransparency )
          {
            currentOpacity = mRasterTransparency->alphaValue( val, mOpacity * 255 ) / 255.0;
          }
          if ( mAlphaBand > 0 )
          {
            currentOpacity *= ( readValue( transparencyData, transparencyType, currentRasterPos ) / 255.0 );
          }
          QColor& currentColor = mColors[val];
          imageScanLine[j] = qRgba( currentOpacity * currentColor.red(), currentOpacity * currentColor.green(), currentOpacity * currentColor.blue(), currentOpacity * 255 );
        }
        ++currentRasterPos;
      }
    }

    //top left position in device coords
    QPointF tlPoint = QPointF( viewPort->topLeftPoint.x(), viewPort->topLeftPoint.y() );
    tlPoint += QPointF( topLeftCol / oversamplingX, topLeftRow / oversamplingY );

    //draw image
    if ( mResampler ) //resample to output resolution
    {
      QImage dstImg( nCols / oversamplingX, nRows / oversamplingY, QImage::Format_ARGB32_Premultiplied );
      mResampler->resample( img, dstImg );
      p->drawImage( tlPoint, dstImg );
    }
    else //use original image
    {
      p->drawImage( tlPoint, img );
    }
  }

  //stop raster reading
  stopRasterRead( mBandNumber );
  if ( mAlphaBand > 0 && mAlphaBand != mBandNumber )
  {
    stopRasterRead( mAlphaBand );
  }
}

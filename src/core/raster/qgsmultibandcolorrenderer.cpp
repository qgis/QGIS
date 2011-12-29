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
#include "qgsrastertransparency.h"
#include "qgsrasterviewport.h"
#include <QImage>
#include <QPainter>
#include <QSet>

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

  QgsRasterDataProvider::DataType redType = ( QgsRasterDataProvider::DataType )mProvider->dataType( mRedBand );
  QgsRasterDataProvider::DataType greenType = ( QgsRasterDataProvider::DataType )mProvider->dataType( mGreenBand );
  QgsRasterDataProvider::DataType blueType = ( QgsRasterDataProvider::DataType )mProvider->dataType( mBlueBand );
  QgsRasterDataProvider::DataType transparencyType;
  if ( mAlphaBand > 0 )
  {
    transparencyType = ( QgsRasterDataProvider::DataType )mProvider->dataType( mAlphaBand );
  }

  double oversamplingX, oversamplingY;
  QSet<int> bands;
  bands << mRedBand << mGreenBand << mBlueBand;
  if ( mAlphaBand > 0 )
  {
    bands << mAlphaBand;
  }

  QMap<int, void*> bandData;
  void* defaultPointer;
  QSet<int>::const_iterator bandIt = bands.constBegin();
  for ( ; bandIt != bands.constEnd(); ++bandIt )
  {
    bandData.insert( *bandIt, defaultPointer );
    startRasterRead( *bandIt, viewPort, theQgsMapToPixel, oversamplingX, oversamplingY );
  }

  void* redData;
  void* greenData;
  void* blueData;
  void* alphaData;
  int nCols, nRows, topLeftCol, topLeftRow;

  bool readSuccess;
  while ( true )
  {
    QSet<int>::const_iterator bandIt = bands.constBegin();
    for ( ; bandIt != bands.constEnd(); ++bandIt )
    {
      readSuccess = readNextRasterPart( *bandIt, viewPort, nCols, nRows, &bandData[*bandIt], topLeftCol, topLeftRow );
    }

    if ( !readSuccess )
    {
      break;
    }

    redData = bandData[mRedBand];
    greenData = bandData[mGreenBand];
    blueData = bandData[mBlueBand];
    if ( mAlphaBand > 0 )
    {
      alphaData = bandData[mAlphaBand];
    }

    QImage img( nCols, nRows, QImage::Format_ARGB32_Premultiplied );
    QRgb* imageScanLine = 0;
    int currentRasterPos = 0;
    int redVal, greenVal, blueVal;
    double currentOpacity = mOpacity; //opacity (between 0 and 1)

    for ( int i = 0; i < nRows; ++i )
    {
      imageScanLine = ( QRgb* )( img.scanLine( i ) );
      for ( int j = 0; j < nCols; ++j )
      {
        redVal = readValue( redData, redType, currentRasterPos );
        greenVal = readValue( greenData, greenType, currentRasterPos );
        blueVal = readValue( blueData, blueType, currentRasterPos );

        //opacity
        currentOpacity = mOpacity;
        if ( mRasterTransparency )
        {
          currentOpacity = mRasterTransparency->alphaValue( redVal, greenVal, blueVal, mOpacity * 255 ) / 255.0;
        }
        if ( mAlphaBand > 0 )
        {
          currentOpacity *= ( readValue( alphaData, transparencyType, currentRasterPos ) / 255.0 );
        }

        if ( doubleNear( currentOpacity, 255 ) )
        {
          imageScanLine[j] = qRgba( redVal, greenVal, blueVal, 255 );
        }
        else
        {
          imageScanLine[j] = qRgba( currentOpacity * redVal, currentOpacity * greenVal, currentOpacity * blueVal, currentOpacity * 255 );
        }
        ++currentRasterPos;
      }
    }

    //draw image
    //top left position in device coords
    QPointF tlPoint = QPointF( viewPort->topLeftPoint.x(), viewPort->topLeftPoint.y() );
    tlPoint += QPointF( topLeftCol / oversamplingX, topLeftRow / oversamplingY );

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

  bandIt = bands.constBegin();
  for ( ; bandIt != bands.constEnd(); ++bandIt )
  {
    stopRasterRead( *bandIt );
  }
}

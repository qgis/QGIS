/***************************************************************************
                         qgssinglebandpseudocolorrenderer.cpp
                         ------------------------------------
    begin                : January 2012
    copyright            : (C) 2012 by Marco Hugentobler
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

#include "qgssinglebandpseudocolorrenderer.h"
#include "qgsrastershader.h"
#include "qgsrastertransparency.h"
#include "qgsrasterviewport.h"
#include <QImage>

QgsSingleBandPseudoColorRenderer::QgsSingleBandPseudoColorRenderer( QgsRasterDataProvider* provider, int band, QgsRasterShader* shader ):
    QgsRasterRenderer( provider ), mShader( shader ), mBand( band )
{
}

QgsSingleBandPseudoColorRenderer::~QgsSingleBandPseudoColorRenderer()
{
}

void QgsSingleBandPseudoColorRenderer::draw( QPainter* p, QgsRasterViewPort* viewPort, const QgsMapToPixel* theQgsMapToPixel )
{
  if ( !p || !mProvider || !viewPort || !theQgsMapToPixel || !mShader )
  {
    return;
  }

  double oversamplingX, oversamplingY;
  QgsRasterDataProvider::DataType transparencyType = QgsRasterDataProvider::UnknownDataType;
  if ( mAlphaBand > 0 )
  {
    transparencyType = ( QgsRasterDataProvider::DataType )mProvider->dataType( mAlphaBand );
  }
  startRasterRead( mBand, viewPort, theQgsMapToPixel, oversamplingX, oversamplingY );
  //Read alpha band if necessary
  if ( mAlphaBand > 0 && mAlphaBand != mBand )
  {
    startRasterRead( mAlphaBand, viewPort, theQgsMapToPixel, oversamplingX, oversamplingY );
  }

  //number of cols/rows in output pixels
  int nCols = 0;
  int nRows = 0;
  //number of raster cols/rows with oversampling
  int nRasterCols = 0;
  int nRasterRows = 0;
  //shift to top left point for the raster part
  int topLeftCol = 0;
  int topLeftRow = 0;
  void* rasterData;
  void* transparencyData;
  double currentOpacity = mOpacity;
  QgsRasterDataProvider::DataType rasterType = ( QgsRasterDataProvider::DataType )mProvider->dataType( mBand );
  int red, green, blue;
  QRgb myDefaultColor = qRgba( 255, 255, 255, 0 );

  //rendering is faster without considering user-defined transparency
  bool hasTransparency = usesTransparency( viewPort->mSrcCRS, viewPort->mDestCRS );

  while ( readNextRasterPart( mBand, oversamplingX, oversamplingY, viewPort, nCols, nRows, nRasterCols, nRasterRows,
                              &rasterData, topLeftCol, topLeftRow ) )
  {
    if ( mAlphaBand > 0 && mAlphaBand != mBand )
    {
      readNextRasterPart( mAlphaBand, oversamplingX, oversamplingY, viewPort, nCols, nRows, nRasterCols, nRasterRows,
                          &transparencyData, topLeftCol, topLeftRow );
    }
    else if ( mAlphaBand == mBand )
    {
      transparencyData = rasterData;
    }

    //create image
    QImage img( nRasterCols, nRasterRows, QImage::Format_ARGB32_Premultiplied );
    QRgb* imageScanLine = 0;
    double val = 0;

    int currentRasterPos = 0;
    for ( int i = 0; i < nRasterRows; ++i )
    {
      imageScanLine = ( QRgb* )( img.scanLine( i ) );
      for ( int j = 0; j < nRasterCols; ++j )
      {
        val = readValue( rasterData, rasterType, currentRasterPos );
        if ( !mShader->shade( val, &red, &green, &blue ) )
        {
          imageScanLine[j] = myDefaultColor;
          ++currentRasterPos;
          continue;
        }

        if ( !hasTransparency )
        {
          imageScanLine[j] = qRgba( red, green, blue, 255 );
        }
        else
        {
          //opacity
          currentOpacity = mOpacity;
          if ( mRasterTransparency )
          {
            currentOpacity = mRasterTransparency->alphaValue( val, mOpacity * 255 ) / 255.0;
          }
          if ( mAlphaBand > 0 )
          {
            currentOpacity *= ( readValue( transparencyData, transparencyType, currentRasterPos ) / 255.0 );
          }

          imageScanLine[j] = qRgba( currentOpacity * red, currentOpacity * green, currentOpacity * blue, currentOpacity * 255 );
        }
        ++currentRasterPos;
      }
    }

    drawImage( p, viewPort, img, topLeftCol, topLeftRow, nCols, nRows, oversamplingX, oversamplingY );
  }

  stopRasterRead( mBand );
  if ( mAlphaBand > 0 && mAlphaBand != mBand )
  {
    stopRasterRead( mAlphaBand );
  }
}

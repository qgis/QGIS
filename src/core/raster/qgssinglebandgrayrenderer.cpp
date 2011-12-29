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
#include "qgsrasterresampler.h"
#include "qgsrasterviewport.h"
#include <QImage>
#include <QPainter>

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

  int nCols = 0;
  int nRows = 0;
  int topLeftCol = 0;
  int topLeftRow = 0;
  QgsRasterDataProvider::DataType rasterType = ( QgsRasterDataProvider::DataType )mProvider->dataType( mGrayBand );
  void* rasterData;
  //double currentOpacity = mOpacity;
  int grayVal;

  while ( readNextRasterPart( mGrayBand, viewPort, nCols, nRows, &rasterData, topLeftCol, topLeftRow ) )
  {
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
          grayVal = mContrastEnhancement->enhanceContrast( grayVal );
        }

        if ( mInvertColor )
        {
          grayVal = 255 - grayVal;
        }

        imageScanLine[j] = qRgba( grayVal, grayVal, grayVal, 255 );
        ++currentRasterPos;
      }
    }

    //draw image
    //top left position in device coords
    QPointF tlPoint = QPointF( viewPort->topLeftPoint.x(), viewPort->topLeftPoint.y() );
    tlPoint += QPointF( topLeftCol / oversamplingX, topLeftRow / oversamplingY );

    if ( mResampler ) //resample to output resolution
    {
      QImage dstImg( nCols / oversamplingX + 1.0, nRows / oversamplingY + 1.0, QImage::Format_ARGB32_Premultiplied );
      mResampler->resample( img, dstImg );
      p->drawImage( tlPoint, dstImg );
    }
    else //use original image
    {
      p->drawImage( tlPoint, img );
    }
  }

  stopRasterRead( mGrayBand );
}

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

  double oversampling;
  startRasterRead( mBandNumber, viewPort, theQgsMapToPixel, oversampling );

  int nCols = 0;
  int nRows = 0;
  int topLeftCol = 0;
  int topLeftRow = 0;
  int currentRasterPos = 0;
  QgsRasterDataProvider::DataType rasterType = ( QgsRasterDataProvider::DataType )mProvider->dataType( mBandNumber );
  void* rasterData;

  while ( readNextRasterPart( mBandNumber, viewPort, nCols, nRows, &rasterData, topLeftCol, topLeftRow ) )
  {
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
        imageScanLine[j] = mColors[ val ].rgba();
        ++currentRasterPos;
      }
    }

    //top left position in device coords
    QPointF tlPoint = QPointF( viewPort->topLeftPoint.x(), viewPort->topLeftPoint.y() );
    tlPoint += QPointF( topLeftCol / oversampling, topLeftRow / oversampling );

    //draw image
    if ( mResampler ) //resample to output resolution
    {
      QImage dstImg( viewPort->drawableAreaXDim, viewPort->drawableAreaYDim, QImage::Format_ARGB32_Premultiplied );
      mResampler->resample( img, dstImg );
      p->drawImage( tlPoint, dstImg );
    }
    else //use original image
    {
      p->drawImage( tlPoint, img );
    }
  }
  stopRasterRead( mBandNumber );
}

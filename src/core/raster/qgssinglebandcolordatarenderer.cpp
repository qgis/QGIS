/***************************************************************************
                         qgssinglebandcolordatarenderer.cpp
                         ----------------------------------
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

#include "qgssinglebandcolordatarenderer.h"
#include <QImage>

QgsSingleBandColorDataRenderer::QgsSingleBandColorDataRenderer( QgsRasterDataProvider* provider, int band, QgsRasterResampler* resampler ):
    QgsRasterRenderer( provider, resampler ), mBand( band )
{

}

QgsSingleBandColorDataRenderer::~QgsSingleBandColorDataRenderer()
{
}

void QgsSingleBandColorDataRenderer::draw( QPainter* p, QgsRasterViewPort* viewPort, const QgsMapToPixel* theQgsMapToPixel )
{
  if ( !p || !mProvider || !viewPort || !theQgsMapToPixel )
  {
    return;
  }

  double oversamplingX, oversamplingY;
  startRasterRead( mBand, viewPort, theQgsMapToPixel, oversamplingX, oversamplingY );

  int topLeftCol, topLeftRow, nCols, nRows, currentRasterPos;
  void* rasterData;

  while ( readNextRasterPart( mBand, viewPort, nCols, nRows, &rasterData, topLeftCol, topLeftRow ) )
  {
    currentRasterPos = 0;
    QImage img( nCols, nRows, QImage::Format_ARGB32_Premultiplied );
    for ( int i = 0; i < nRows; ++i )
    {
      memcpy( img.scanLine( i ), &((( uint* )rasterData )[currentRasterPos] ), nCols * 4 );
      for ( int j = 0; j < nCols; ++j )
      {
        ++currentRasterPos;
      }
    }

    drawImage( p, viewPort, img, topLeftCol, topLeftRow, nCols, nRows, oversamplingX, oversamplingY );
  }

  stopRasterRead( mBand );
}

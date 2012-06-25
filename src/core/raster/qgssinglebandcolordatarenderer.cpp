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
#include "qgsrasterviewport.h"
#include <QDomDocument>
#include <QDomElement>
#include <QImage>

QgsSingleBandColorDataRenderer::QgsSingleBandColorDataRenderer( QgsRasterDataProvider* provider, int band ):
    QgsRasterRenderer( provider, "singlebandcolordata" ), mBand( band )
{

}

QgsSingleBandColorDataRenderer::~QgsSingleBandColorDataRenderer()
{
}

QgsRasterRenderer* QgsSingleBandColorDataRenderer::create( const QDomElement& elem, QgsRasterDataProvider* provider )
{
  if ( elem.isNull() )
  {
    return 0;
  }

  int band = elem.attribute( "band", "-1" ).toInt();
  QgsRasterRenderer* r = new QgsSingleBandColorDataRenderer( provider, band );
  r->readXML( elem );
  return r;
}

void QgsSingleBandColorDataRenderer::draw( QPainter* p, QgsRasterViewPort* viewPort, const QgsMapToPixel* theQgsMapToPixel )
{
  if ( !p || !mProvider || !viewPort || !theQgsMapToPixel )
  {
    return;
  }

  double oversamplingX, oversamplingY;
  startRasterRead( mBand, viewPort, theQgsMapToPixel, oversamplingX, oversamplingY );

  //number of cols/rows in output pixels
  int nCols = 0;
  int nRows = 0;
  //number of raster cols/rows with oversampling
  int nRasterCols = 0;
  int nRasterRows = 0;
  //shift to top left point for the raster part
  int topLeftCol = 0;
  int topLeftRow = 0;
  int currentRasterPos;
  void* rasterData;

  bool hasTransparency = usesTransparency( viewPort->mSrcCRS, viewPort->mDestCRS );

  while ( readNextRasterPart( mBand, oversamplingX, oversamplingY, viewPort, nCols, nRows, nRasterCols, nRasterRows, &rasterData, topLeftCol, topLeftRow ) )
  {
    currentRasterPos = 0;
    QImage img( nRasterCols, nRasterRows, QImage::Format_ARGB32 );
    uchar* scanLine = 0;
    for ( int i = 0; i < nRasterRows; ++i )
    {
      scanLine = img.scanLine( i );
      if ( !hasTransparency )
      {
        memcpy( scanLine, &((( uint* )rasterData )[currentRasterPos] ), nCols * 4 );
        currentRasterPos += nRasterCols;
      }
      else
      {
        QRgb pixelColor;
        for ( int j = 0; j < nRasterCols; ++j )
        {
          QRgb c((( uint* )( rasterData ) )[currentRasterPos] );
          pixelColor = qRgba( qRed( c ), qGreen( c ), qBlue( c ), 255 );
          memcpy( &( scanLine[j*4] ), &pixelColor, 4 );
          ++currentRasterPos;
        }
      }
    }

    drawImage( p, viewPort, img, topLeftCol, topLeftRow, nCols, nRows, oversamplingX, oversamplingY );
  }

  stopRasterRead( mBand );
}

void QgsSingleBandColorDataRenderer::writeXML( QDomDocument& doc, QDomElement& parentElem ) const
{
  if ( parentElem.isNull() )
  {
    return;
  }

  QDomElement rasterRendererElem = doc.createElement( "rasterrenderer" );
  _writeXML( doc, rasterRendererElem );
  rasterRendererElem.setAttribute( "band", mBand );
  parentElem.appendChild( rasterRendererElem );
}

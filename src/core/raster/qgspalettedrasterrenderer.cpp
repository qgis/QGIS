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
#include "qgsrastertransparency.h"
#include "qgsrasterviewport.h"
#include <QColor>
#include <QDomDocument>
#include <QDomElement>
#include <QImage>

QgsPalettedRasterRenderer::QgsPalettedRasterRenderer( QgsRasterDataProvider* provider, int bandNumber,
    QColor* colorArray, int nColors ):
    QgsRasterRenderer( provider, "paletted" ), mBandNumber( bandNumber ), mColors( colorArray ), mNColors( nColors )
{
}

QgsPalettedRasterRenderer::~QgsPalettedRasterRenderer()
{
  delete[] mColors;
}

QgsRasterRenderer* QgsPalettedRasterRenderer::create( const QDomElement& elem )
{
  return 0;
}

QColor* QgsPalettedRasterRenderer::colors() const
{
  if ( mNColors < 1 )
  {
    return 0;
  }
  QColor* colorArray = new QColor[ mNColors ];
  for ( int i = 0; i < mNColors; ++i )
  {
    colorArray[i] = mColors[i];
  }
  return colorArray;
}

void QgsPalettedRasterRenderer::draw( QPainter* p, QgsRasterViewPort* viewPort, const QgsMapToPixel* theQgsMapToPixel )
{
  if ( !p || !mProvider || !viewPort || !theQgsMapToPixel )
  {
    return;
  }

  double oversamplingX, oversamplingY;
  QgsRasterDataProvider::DataType transparencyType = QgsRasterDataProvider::UnknownDataType;
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

  //number of cols/rows in output pixels
  int nCols = 0;
  int nRows = 0;
  //number of raster cols/rows with oversampling
  int nRasterCols = 0;
  int nRasterRows = 0;
  //shift to top left point for the raster part
  int topLeftCol = 0;
  int topLeftRow = 0;
  int currentRasterPos = 0;
  QgsRasterDataProvider::DataType rasterType = ( QgsRasterDataProvider::DataType )mProvider->dataType( mBandNumber );
  void* rasterData;
  double currentOpacity = mOpacity;

  //rendering is faster without considering user-defined transparency
  bool hasTransparency = usesTransparency( viewPort->mSrcCRS, viewPort->mDestCRS );
  void* transparencyData;

  while ( readNextRasterPart( mBandNumber, oversamplingX, oversamplingY, viewPort, nCols, nRows, nRasterCols, nRasterRows,
                              &rasterData, topLeftCol, topLeftRow ) )
  {
    if ( mAlphaBand > 0 && mAlphaBand != mBandNumber )
    {
      readNextRasterPart( mAlphaBand, oversamplingX, oversamplingY, viewPort, nCols, nRows, nRasterCols, nRasterRows,
                          &transparencyData, topLeftCol, topLeftRow );
    }
    else if ( mAlphaBand == mBandNumber )
    {
      transparencyData = rasterData;
    }

    //create image
    QImage img( nRasterCols, nRasterRows, QImage::Format_ARGB32_Premultiplied );
    QRgb* imageScanLine = 0;
    int val = 0;
    currentRasterPos = 0;

    for ( int i = 0; i < nRasterRows; ++i )
    {
      imageScanLine = ( QRgb* )( img.scanLine( i ) );
      for ( int j = 0; j < nRasterCols; ++j )
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

          if ( mInvertColor )
          {
            imageScanLine[j] = qRgba( currentOpacity * currentColor.blue(), currentOpacity * currentColor.green(), currentOpacity * currentColor.red(), currentOpacity * 255 );
          }
          else
          {
            imageScanLine[j] = qRgba( currentOpacity * currentColor.red(), currentOpacity * currentColor.green(), currentOpacity * currentColor.blue(), currentOpacity * 255 );
          }
        }
        ++currentRasterPos;
      }
    }

    drawImage( p, viewPort, img, topLeftCol, topLeftRow, nCols, nRows, oversamplingX, oversamplingY );
  }

  //stop raster reading
  stopRasterRead( mBandNumber );
  if ( mAlphaBand > 0 && mAlphaBand != mBandNumber )
  {
    stopRasterRead( mAlphaBand );
  }
}

void QgsPalettedRasterRenderer::writeXML( QDomDocument& doc, QDomElement& parentElem ) const
{
  QDomElement rasterRendererElem = doc.createElement( "rasterrenderer" );
  _writeXML( doc, rasterRendererElem );

  rasterRendererElem.setAttribute( "band", mBandNumber );
  QDomElement colorPaletteElem = doc.createElement( "colorPalette" );
  for ( int i = 0; i < mNColors; ++i )
  {
    QDomElement colorElem = doc.createElement( "paletteEntry" );
    colorElem.setAttribute( "value", i );
    colorElem.setAttribute( "color", mColors[i].name() );
    colorPaletteElem.appendChild( colorElem );
  }
  rasterRendererElem.appendChild( colorPaletteElem );

  parentElem.appendChild( rasterRendererElem );
}

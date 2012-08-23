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

QgsPalettedRasterRenderer::QgsPalettedRasterRenderer( QgsRasterInterface* input, int bandNumber,
    QColor* colorArray, int nColors ):
    QgsRasterRenderer( input, "paletted" ), mBandNumber( bandNumber ), mColors( colorArray ), mNColors( nColors )
{
}

QgsPalettedRasterRenderer::~QgsPalettedRasterRenderer()
{
  delete[] mColors;
}

QgsRasterInterface * QgsPalettedRasterRenderer::clone() const
{
  QgsPalettedRasterRenderer * renderer = new QgsPalettedRasterRenderer( 0, mBandNumber, colors(), mNColors );
  return renderer;
}

QgsRasterRenderer* QgsPalettedRasterRenderer::create( const QDomElement& elem, QgsRasterInterface* input )
{
  if ( elem.isNull() )
  {
    return 0;
  }

  int bandNumber = elem.attribute( "band", "-1" ).toInt();
  int nColors = 0;
  QColor* colors = 0;

  QDomElement paletteElem = elem.firstChildElement( "colorPalette" );
  if ( !paletteElem.isNull() )
  {
    QDomNodeList paletteEntries = paletteElem.elementsByTagName( "paletteEntry" );
    nColors = paletteEntries.size();
    colors = new QColor[ nColors ];

    int value = 0;
    QDomElement entryElem;
    for ( int i = 0; i < nColors; ++i )
    {
      entryElem = paletteEntries.at( i ).toElement();
      value = entryElem.attribute( "value", "0" ).toInt();
      QgsDebugMsg( entryElem.attribute( "color", "#000000" ) );
      colors[value] = QColor( entryElem.attribute( "color", "#000000" ) );
    }
  }
  QgsRasterRenderer* r = new QgsPalettedRasterRenderer( input, bandNumber, colors, nColors );
  r->readXML( elem );
  return r;
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

void * QgsPalettedRasterRenderer::readBlock( int bandNo, QgsRectangle  const & extent, int width, int height )
{
  if ( !mInput )
  {
    return 0;
  }
  QRgb myDefaultColor = qRgba( 0, 0, 0, 0 );

  QgsRasterInterface::DataType transparencyType = QgsRasterInterface::UnknownDataType;
  if ( mAlphaBand > 0 )
  {
    transparencyType = ( QgsRasterInterface::DataType )mInput->dataType( mAlphaBand );
  }

  QgsRasterInterface::DataType rasterType = ( QgsRasterInterface::DataType )mInput->dataType( mBandNumber );
  void* rasterData = mInput->block( bandNo, extent, width, height );
  double currentOpacity = mOpacity;

  //rendering is faster without considering user-defined transparency
  bool hasTransparency = usesTransparency();
  void* transparencyData = 0;

  if ( mAlphaBand > 0 && mAlphaBand != mBandNumber )
  {
    transparencyData = mInput->block( mAlphaBand, extent, width, height );
  }
  else if ( mAlphaBand == mBandNumber )
  {
    transparencyData = rasterData;
  }

  //create image
  QImage img( width, height, QImage::Format_ARGB32_Premultiplied );
  QRgb* imageScanLine = 0;
  int val = 0;
  int currentRasterPos = 0;

  for ( int i = 0; i < height; ++i )
  {
    imageScanLine = ( QRgb* )( img.scanLine( i ) );
    for ( int j = 0; j < width; ++j )
    {
      val = readValue( rasterData, rasterType, currentRasterPos );
      if ( mInput->isNoDataValue( mBandNumber, val ) )
      {
        imageScanLine[j] = myDefaultColor;
        ++currentRasterPos;
        continue;
      }
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

  VSIFree( rasterData );
  void * data = VSIMalloc( img.byteCount() );
  return memcpy( data, img.bits(), img.byteCount() );
}

void QgsPalettedRasterRenderer::writeXML( QDomDocument& doc, QDomElement& parentElem ) const
{
  if ( parentElem.isNull() )
  {
    return;
  }

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

void QgsPalettedRasterRenderer::legendSymbologyItems( QList< QPair< QString, QColor > >& symbolItems ) const
{
  for ( int i = 0; i < mNColors; ++i )
  {
    symbolItems.push_back( qMakePair( QString::number( i ), mColors[i] ) );
  }
}

QList<int> QgsPalettedRasterRenderer::usesBands() const
{
  QList<int> bandList;
  if ( mBandNumber != -1 )
  {
    bandList << mBandNumber;
  }
  return bandList;
}

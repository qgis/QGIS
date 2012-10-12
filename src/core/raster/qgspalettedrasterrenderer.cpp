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
    QgsRasterRenderer( input, "paletted" ), mBand( bandNumber ), mColors( colorArray ), mNColors( nColors )
{
}

QgsPalettedRasterRenderer::~QgsPalettedRasterRenderer()
{
  delete[] mColors;
}

QgsRasterInterface * QgsPalettedRasterRenderer::clone() const
{
  QgsPalettedRasterRenderer * renderer = new QgsPalettedRasterRenderer( 0, mBand, colors(), mNColors );
  renderer->setOpacity( mOpacity );
  renderer->setAlphaBand( mAlphaBand );
  renderer->setInvertColor( mInvertColor );
  renderer->setRasterTransparency( mRasterTransparency );
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

QgsRasterBlock * QgsPalettedRasterRenderer::block( int bandNo, QgsRectangle  const & extent, int width, int height )
{
  QgsRasterBlock *outputBlock = new QgsRasterBlock();
  if ( !mInput )
  {
    return outputBlock;
  }
  QRgb myDefaultColor = NODATA_COLOR;

  //QgsRasterBlock::DataType transparencyType = QgsRasterBlock::UnknownDataType;
  //if ( mAlphaBand > 0 )
  //{
  //  transparencyType = ( QgsRasterBlock::DataType )mInput->dataType( mAlphaBand );
  //}

  //QgsRasterBlock::DataType rasterType = ( QgsRasterBlock::DataType )mInput->dataType( mBand );
  //void* rasterData = mInput->block( bandNo, extent, width, height );
  QgsRasterBlock *inputBlock = mInput->block( bandNo, extent, width, height );

  if ( !inputBlock || inputBlock->isEmpty() )
  {
    QgsDebugMsg( "No raster data!" );
    delete inputBlock;
    return outputBlock;
  }

  double currentOpacity = mOpacity;

  //rendering is faster without considering user-defined transparency
  bool hasTransparency = usesTransparency();
  //void* transparencyData = 0;
  QgsRasterBlock *alphaBlock = 0;

  if ( mAlphaBand > 0 && mAlphaBand != mBand )
  {
    alphaBlock = mInput->block( mAlphaBand, extent, width, height );
    if ( !alphaBlock || alphaBlock->isEmpty() )
    {
      delete inputBlock;
      delete alphaBlock;
      return outputBlock;
    }
  }
  else if ( mAlphaBand == mBand )
  {
    //transparencyData = rasterData;
    alphaBlock = inputBlock;
  }

  //create image
  //QImage img( width, height, QImage::Format_ARGB32_Premultiplied );
  //if ( img.isNull() )
  //{
  //QgsDebugMsg( "Could not create QImage" );
  //VSIFree( rasterData );
  //return 0;
  //}
  if ( !outputBlock->reset( QgsRasterBlock::ARGB32_Premultiplied, width, height ) )
  {
    delete inputBlock;
    delete alphaBlock;
    return outputBlock;
  }

  for ( size_t i = 0; i < ( size_t )width*height; i++ )
  {
    int val = ( int ) inputBlock->value( i );
    if ( inputBlock->isNoDataValue( val ) )
    {
      outputBlock->setColor( i, myDefaultColor );
      continue;
    }
    if ( !hasTransparency )
    {
      if ( val < 0 || val > mNColors )
      {
        outputBlock->setColor( i, myDefaultColor );
      }
      else
      {
        outputBlock->setColor( i, mColors[ val ].rgba() );
      }
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
        currentOpacity *=  alphaBlock->value( i ) / 255.0;
      }
      QColor& currentColor = mColors[val];

      if ( mInvertColor )
      {
        outputBlock->setColor( i, qRgba( currentOpacity * currentColor.blue(), currentOpacity * currentColor.green(), currentOpacity * currentColor.red(), currentOpacity * 255 ) );
      }
      else
      {
        outputBlock->setColor( i, qRgba( currentOpacity * currentColor.red(), currentOpacity * currentColor.green(), currentOpacity * currentColor.blue(), currentOpacity * 255 ) );
      }
    }
  }

  delete inputBlock;
  if ( mAlphaBand > 0 && mBand != mAlphaBand )
  {
    delete alphaBlock;
  }

  return outputBlock;
}

void QgsPalettedRasterRenderer::writeXML( QDomDocument& doc, QDomElement& parentElem ) const
{
  if ( parentElem.isNull() )
  {
    return;
  }

  QDomElement rasterRendererElem = doc.createElement( "rasterrenderer" );
  _writeXML( doc, rasterRendererElem );

  rasterRendererElem.setAttribute( "band", mBand );
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
  if ( mBand != -1 )
  {
    bandList << mBand;
  }
  return bandList;
}

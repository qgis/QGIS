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
    QgsRasterRenderer( input, "paletted" ), mBand( bandNumber ), mNColors( nColors )
{
  mColors = new QRgb[nColors];
  for ( int i = 0; i < nColors; ++i )
  {
    mColors[i] = colorArray[i].rgba();
  }
  delete[] colorArray;
}

QgsPalettedRasterRenderer::QgsPalettedRasterRenderer( QgsRasterInterface* input, int bandNumber, QRgb* colorArray, int nColors ):
    QgsRasterRenderer( input, "paletted" ), mBand( bandNumber ), mColors( colorArray ), mNColors( nColors )
{
}

QgsPalettedRasterRenderer::~QgsPalettedRasterRenderer()
{
  delete[] mColors;
}

QgsRasterInterface * QgsPalettedRasterRenderer::clone() const
{
  QgsPalettedRasterRenderer * renderer = new QgsPalettedRasterRenderer( 0, mBand, rgbArray(), mNColors );
  renderer->setOpacity( mOpacity );
  renderer->setAlphaBand( mAlphaBand );
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
  QRgb* colors = 0;

  QDomElement paletteElem = elem.firstChildElement( "colorPalette" );
  if ( !paletteElem.isNull() )
  {
    QDomNodeList paletteEntries = paletteElem.elementsByTagName( "paletteEntry" );

    QDomElement entryElem;
    int value;
    nColors = 0;

    // We cannot believe that data are correct, check first max value
    for ( int i = 0; i < paletteEntries.size(); ++i )
    {
      entryElem = paletteEntries.at( i ).toElement();
      // Could be written as doubles (with .0000) in old project files
      value = ( int )entryElem.attribute( "value", "0" ).toDouble();
      if ( value >= nColors && value <= 10000 ) nColors = value + 1;
    }
    QgsDebugMsg( QString( "nColors = %1" ).arg( nColors ) );

    colors = new QRgb[ nColors ];

    for ( int i = 0; i < nColors; ++i )
    {
      entryElem = paletteEntries.at( i ).toElement();
      value = ( int )entryElem.attribute( "value", "0" ).toDouble();
      QgsDebugMsg( entryElem.attribute( "color", "#000000" ) );
      if ( value >= 0 && value < nColors )
      {
        colors[value] = QColor( entryElem.attribute( "color", "#000000" ) ).rgba();
      }
      else
      {
        QgsDebugMsg( QString( "value %1 out of range" ).arg( value ) );
      }
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
    colorArray[i] = QColor( mColors[i] );
  }
  return colorArray;
}

QRgb* QgsPalettedRasterRenderer::rgbArray() const
{
  if ( mNColors < 1 )
  {
    return 0;
  }
  QRgb* rgbValues = new QRgb[mNColors];
  for ( int i = 0; i < mNColors; ++i )
  {
    rgbValues[i] = mColors[i];
  }
  return rgbValues;
}

QgsRasterBlock * QgsPalettedRasterRenderer::block( int bandNo, QgsRectangle  const & extent, int width, int height )
{
  QgsRasterBlock *outputBlock = new QgsRasterBlock();
  if ( !mInput )
  {
    return outputBlock;
  }

  //QGis::DataType transparencyType = QGis::UnknownDataType;
  //if ( mAlphaBand > 0 )
  //{
  //  transparencyType = ( QGis::DataType )mInput->dataType( mAlphaBand );
  //}

  //QGis::DataType rasterType = ( QGis::DataType )mInput->dataType( mBand );
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

  if ( !outputBlock->reset( QGis::ARGB32_Premultiplied, width, height ) )
  {
    delete inputBlock;
    delete alphaBlock;
    return outputBlock;
  }

  //create copy of color table with nodata values replaced by fully transparent color
  // We dont have no data value anymore
#if 0
  QVector<QRgb> colorTable( mNColors );
  for ( int i = 0; i < mNColors; ++i )
  {
    if ( inputBlock->isNoDataValue( i ) )
    {
      colorTable[i] = QColor( 0, 0, 0, 0 ).rgba();
    }
    else
    {
      colorTable[i] = mColors[i];
    }
  }
#endif

  QRgb myDefaultColor = NODATA_COLOR;

  //use direct data access instead of QgsRasterBlock::setValue
  //because of performance
  unsigned int* outputData = ( unsigned int* )( outputBlock->bits() );

  size_t rasterSize = ( size_t )width * height;
  for ( size_t i = 0; i < rasterSize; ++i )
  {
    if ( inputBlock->isNoData( i ) )
    {
      outputData[i] = myDefaultColor;
    }
    int val = ( int ) inputBlock->value( i );
    if ( !hasTransparency )
    {
      outputData[i] = mColors[val];
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
      QColor currentColor = QColor( mColors[val] );
      outputData[i] = qRgba( currentOpacity * currentColor.red(), currentOpacity * currentColor.green(), currentOpacity * currentColor.blue(), currentOpacity * 255 );
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
    colorElem.setAttribute( "color", QColor( mColors[i] ).name() );
    colorPaletteElem.appendChild( colorElem );
  }
  rasterRendererElem.appendChild( colorPaletteElem );

  parentElem.appendChild( rasterRendererElem );
}

void QgsPalettedRasterRenderer::legendSymbologyItems( QList< QPair< QString, QColor > >& symbolItems ) const
{
  for ( int i = 0; i < mNColors; ++i )
  {
    symbolItems.push_back( qMakePair( QString::number( i ), QColor( mColors[i] ) ) );
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

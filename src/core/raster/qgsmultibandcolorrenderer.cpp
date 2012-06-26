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
#include "qgscontrastenhancement.h"
#include "qgsrastertransparency.h"
#include "qgsrasterviewport.h"
#include <QDomDocument>
#include <QDomElement>
#include <QImage>
#include <QSet>

QgsMultiBandColorRenderer::QgsMultiBandColorRenderer( QgsRasterInterface* input, int redBand, int greenBand, int blueBand,
    QgsContrastEnhancement* redEnhancement,
    QgsContrastEnhancement* greenEnhancement,
    QgsContrastEnhancement* blueEnhancement ):
    QgsRasterRenderer( input, "multibandcolor" ), mRedBand( redBand ), mGreenBand( greenBand ), mBlueBand( blueBand ),
    mRedContrastEnhancement( redEnhancement ), mGreenContrastEnhancement( greenEnhancement ), mBlueContrastEnhancement( blueEnhancement )
{
}

QgsMultiBandColorRenderer::~QgsMultiBandColorRenderer()
{
  delete mRedContrastEnhancement;
  delete mGreenContrastEnhancement;
  delete mBlueContrastEnhancement;
}

void QgsMultiBandColorRenderer::setRedContrastEnhancement( QgsContrastEnhancement* ce )
{
  delete mRedContrastEnhancement; mRedContrastEnhancement = ce;
}

void QgsMultiBandColorRenderer::setGreenContrastEnhancement( QgsContrastEnhancement* ce )
{
  delete mGreenContrastEnhancement; mGreenContrastEnhancement = ce;
}

void QgsMultiBandColorRenderer::setBlueContrastEnhancement( QgsContrastEnhancement* ce )
{
  delete mBlueContrastEnhancement; mBlueContrastEnhancement = ce;
}

QgsRasterRenderer* QgsMultiBandColorRenderer::create( const QDomElement& elem, QgsRasterInterface* input )
{
  if ( elem.isNull() )
  {
    return 0;
  }

  //red band, green band, blue band
  int redBand = elem.attribute( "redBand", "-1" ).toInt();
  int greenBand = elem.attribute( "greenBand", "-1" ).toInt();
  int blueBand = elem.attribute( "blueBand", "-1" ).toInt();

  //contrast enhancements
  QgsContrastEnhancement* redContrastEnhancement = 0;
  QDomElement redContrastElem = elem.firstChildElement( "redContrastEnhancement" );
  if ( !redContrastElem.isNull() )
  {
    redContrastEnhancement = new QgsContrastEnhancement(( QgsContrastEnhancement::QgsRasterDataType )(
          input->dataType( redBand ) ) );
    redContrastEnhancement->readXML( redContrastElem );
  }

  QgsContrastEnhancement* greenContrastEnhancement = 0;
  QDomElement greenContrastElem = elem.firstChildElement( "greenContrastEnhancement" );
  if ( !greenContrastElem.isNull() )
  {
    greenContrastEnhancement = new QgsContrastEnhancement(( QgsContrastEnhancement::QgsRasterDataType )(
          input->dataType( greenBand ) ) );
    greenContrastEnhancement->readXML( greenContrastElem );
  }

  QgsContrastEnhancement* blueContrastEnhancement = 0;
  QDomElement blueContrastElem = elem.firstChildElement( "blueContrastEnhancement" );
  if ( !blueContrastElem.isNull() )
  {
    blueContrastEnhancement = new QgsContrastEnhancement(( QgsContrastEnhancement::QgsRasterDataType )(
          input->dataType( blueBand ) ) );
    blueContrastEnhancement->readXML( blueContrastElem );
  }

  QgsRasterRenderer* r = new QgsMultiBandColorRenderer( input, redBand, greenBand, blueBand, redContrastEnhancement,
      greenContrastEnhancement, blueContrastEnhancement );
  r->readXML( elem );
  return r;
}

void * QgsMultiBandColorRenderer::readBlock( int bandNo, QgsRectangle  const & extent, int width, int height )
{
  if ( !mInput )
  {
    return 0;
  }

  //In some (common) cases, we can simplify the drawing loop considerably and save render time
  bool fastDraw = ( !usesTransparency()
                    && mRedBand > 0 && mGreenBand > 0 && mBlueBand > 0
                    && mAlphaBand < 1 && !mRedContrastEnhancement && !mGreenContrastEnhancement && !mBlueContrastEnhancement
                    && !mInvertColor );

  QgsRasterInterface::DataType redType = QgsRasterInterface::UnknownDataType;

  if ( mRedBand > 0 )
  {
    redType = ( QgsRasterInterface::DataType )mInput->dataType( mRedBand );
  }
  QgsRasterInterface::DataType greenType = QgsRasterInterface::UnknownDataType;
  if ( mGreenBand > 0 )
  {
    greenType = ( QgsRasterInterface::DataType )mInput->dataType( mGreenBand );
  }
  QgsRasterInterface::DataType blueType = QgsRasterInterface::UnknownDataType;
  if ( mBlueBand > 0 )
  {
    blueType = ( QgsRasterInterface::DataType )mInput->dataType( mBlueBand );
  }
  QgsRasterInterface::DataType transparencyType = QgsRasterInterface::UnknownDataType;
  if ( mAlphaBand > 0 )
  {
    transparencyType = ( QgsRasterInterface::DataType )mInput->dataType( mAlphaBand );
  }

  double oversamplingX = 1.0, oversamplingY = 1.0;
  QSet<int> bands;
  if ( mRedBand > 0 )
  {
    bands << mRedBand;
  }
  if ( mGreenBand > 0 )
  {
    bands << mGreenBand;
  }
  if ( mBlueBand > 0 )
  {
    bands << mBlueBand;
  }
  if ( bands.size() < 1 )
  {
    return 0; //no need to draw anything if no band is set
  }

  if ( mAlphaBand > 0 )
  {
    bands << mAlphaBand;
  }

  QMap<int, void*> bandData;
  void* defaultPointer = 0;
  QSet<int>::const_iterator bandIt = bands.constBegin();
  for ( ; bandIt != bands.constEnd(); ++bandIt )
  {
    bandData.insert( *bandIt, defaultPointer );
  }

  void* redData = 0;
  void* greenData = 0;
  void* blueData = 0;
  void* alphaData = 0;

  bandIt = bands.constBegin();
  for ( ; bandIt != bands.constEnd(); ++bandIt )
  {
    bandData[*bandIt] =  mInput->readBlock( *bandIt, extent, width, height );
    if ( !bandData[*bandIt] ) return 0;
  }

  if ( mRedBand > 0 )
  {
    redData = bandData[mRedBand];
  }
  if ( mGreenBand > 0 )
  {
    greenData = bandData[mGreenBand];
  }
  if ( mBlueBand > 0 )
  {
    blueData = bandData[mBlueBand];
  }
  if ( mAlphaBand > 0 )
  {
    alphaData = bandData[mAlphaBand];
  }

  QImage img( width, height, QImage::Format_ARGB32_Premultiplied );
  QRgb* imageScanLine = 0;
  int currentRasterPos = 0;
  int redVal = 0;
  int greenVal = 0;
  int blueVal = 0;
  QRgb defaultColor = qRgba( 255, 255, 255, 0 );
  double currentOpacity = mOpacity; //opacity (between 0 and 1)

  for ( int i = 0; i < height; ++i )
  {
    imageScanLine = ( QRgb* )( img.scanLine( i ) );
    for ( int j = 0; j < width; ++j )
    {
      if ( fastDraw ) //fast rendering if no transparency, stretching, color inversion, etc.
      {
        redVal = readValue( redData, redType, currentRasterPos );
        greenVal = readValue( greenData, greenType, currentRasterPos );
        blueVal = readValue( blueData, blueType, currentRasterPos );
        imageScanLine[j] = qRgba( redVal, greenVal, blueVal, 255 );
        ++currentRasterPos;
        continue;
      }

      if ( mRedBand > 0 )
      {
        redVal = readValue( redData, redType, currentRasterPos );
      }
      if ( mGreenBand > 0 )
      {
        greenVal = readValue( greenData, greenType, currentRasterPos );
      }
      if ( mBlueBand > 0 )
      {
        blueVal = readValue( blueData, blueType, currentRasterPos );
      }

      //apply default color if red, green or blue not in displayable range
      if (( mRedContrastEnhancement && !mRedContrastEnhancement->isValueInDisplayableRange( redVal ) )
          || ( mGreenContrastEnhancement && !mGreenContrastEnhancement->isValueInDisplayableRange( redVal ) )
          || ( mBlueContrastEnhancement && !mBlueContrastEnhancement->isValueInDisplayableRange( redVal ) ) )
      {
        imageScanLine[j] = defaultColor;
        ++currentRasterPos;
        continue;
      }

      //stretch color values
      if ( mRedContrastEnhancement )
      {
        redVal = mRedContrastEnhancement->enhanceContrast( redVal );
      }
      if ( mGreenContrastEnhancement )
      {
        greenVal = mGreenContrastEnhancement->enhanceContrast( greenVal );
      }
      if ( mBlueContrastEnhancement )
      {
        blueVal = mBlueContrastEnhancement->enhanceContrast( blueVal );
      }

      if ( mInvertColor )
      {
        redVal = 255 - redVal;
        greenVal = 255 - greenVal;
        blueVal = 255 - blueVal;
      }

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

      if ( doubleNear( currentOpacity, 1.0 ) )
      {
        imageScanLine[j] = qRgba( redVal, greenVal, blueVal, 255 );
      }
      else
      {
        imageScanLine[j] = qRgba( currentOpacity * redVal, currentOpacity * greenVal, currentOpacity * blueVal, currentOpacity * 255 );
      }

      //stretch color values
      if ( mRedContrastEnhancement )
      {
        redVal = mRedContrastEnhancement->enhanceContrast( redVal );
      }
      if ( mGreenContrastEnhancement )
      {
        greenVal = mGreenContrastEnhancement->enhanceContrast( greenVal );
      }
      if ( mBlueContrastEnhancement )
      {
        blueVal = mBlueContrastEnhancement->enhanceContrast( blueVal );
      }

      if ( mInvertColor )
      {
        redVal = 255 - redVal;
        greenVal = 255 - greenVal;
        blueVal = 255 - blueVal;
      }

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

  bandIt = bands.constBegin();
  for ( ; bandIt != bands.constEnd(); ++bandIt )
  {
    VSIFree( bandData[*bandIt] );
  }

  void * data = VSIMalloc( img.byteCount() );
  return memcpy( data, img.bits(), img.byteCount() );
}

void QgsMultiBandColorRenderer::writeXML( QDomDocument& doc, QDomElement& parentElem ) const
{
  if ( parentElem.isNull() )
  {
    return;
  }

  QDomElement rasterRendererElem = doc.createElement( "rasterrenderer" );
  _writeXML( doc, rasterRendererElem );
  rasterRendererElem.setAttribute( "redBand", mRedBand );
  rasterRendererElem.setAttribute( "greenBand", mGreenBand );
  rasterRendererElem.setAttribute( "blueBand", mBlueBand );

  //contrast enhancement
  if ( mRedContrastEnhancement )
  {
    QDomElement redContrastElem = doc.createElement( "redContrastEnhancement" );
    mRedContrastEnhancement->writeXML( doc, redContrastElem );
    rasterRendererElem.appendChild( redContrastElem );
  }
  if ( mGreenContrastEnhancement )
  {
    QDomElement greenContrastElem = doc.createElement( "greenContrastEnhancement" );
    mGreenContrastEnhancement->writeXML( doc, greenContrastElem );
    rasterRendererElem.appendChild( greenContrastElem );
  }
  if ( mBlueContrastEnhancement )
  {
    QDomElement blueContrastElem = doc.createElement( "blueContrastEnhancement" );
    mBlueContrastEnhancement->writeXML( doc, blueContrastElem );
    rasterRendererElem.appendChild( blueContrastElem );
  }
  parentElem.appendChild( rasterRendererElem );
}

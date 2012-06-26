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

QgsMultiBandColorRenderer::QgsMultiBandColorRenderer( QgsRasterDataProvider* provider, int redBand, int greenBand, int blueBand,
    QgsContrastEnhancement* redEnhancement,
    QgsContrastEnhancement* greenEnhancement,
    QgsContrastEnhancement* blueEnhancement ):
    QgsRasterRenderer( provider, "multibandcolor" ), mRedBand( redBand ), mGreenBand( greenBand ), mBlueBand( blueBand ),
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

QgsRasterRenderer* QgsMultiBandColorRenderer::create( const QDomElement& elem, QgsRasterDataProvider* provider )
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
          provider->dataType( redBand ) ) );
    redContrastEnhancement->readXML( redContrastElem );
  }

  QgsContrastEnhancement* greenContrastEnhancement = 0;
  QDomElement greenContrastElem = elem.firstChildElement( "greenContrastEnhancement" );
  if ( !greenContrastElem.isNull() )
  {
    greenContrastEnhancement = new QgsContrastEnhancement(( QgsContrastEnhancement::QgsRasterDataType )(
          provider->dataType( greenBand ) ) );
    greenContrastEnhancement->readXML( greenContrastElem );
  }

  QgsContrastEnhancement* blueContrastEnhancement = 0;
  QDomElement blueContrastElem = elem.firstChildElement( "blueContrastEnhancement" );
  if ( !blueContrastElem.isNull() )
  {
    blueContrastEnhancement = new QgsContrastEnhancement(( QgsContrastEnhancement::QgsRasterDataType )(
          provider->dataType( blueBand ) ) );
    blueContrastEnhancement->readXML( blueContrastElem );
  }

  QgsRasterRenderer* r = new QgsMultiBandColorRenderer( provider, redBand, greenBand, blueBand, redContrastEnhancement,
      greenContrastEnhancement, blueContrastEnhancement );
  r->readXML( elem );
  return r;
}

void QgsMultiBandColorRenderer::draw( QPainter* p, QgsRasterViewPort* viewPort, const QgsMapToPixel* theQgsMapToPixel )
{
  if ( !p || !mProvider || !viewPort || !theQgsMapToPixel )
  {
    return;
  }

  //In some (common) cases, we can simplify the drawing loop considerably and save render time
  bool fastDraw = (
                    !usesTransparency( viewPort->mSrcCRS, viewPort->mDestCRS )
                    && mRedBand > 0 && mGreenBand > 0 && mBlueBand > 0
                    && mAlphaBand < 1 && !mRedContrastEnhancement && !mGreenContrastEnhancement && !mBlueContrastEnhancement
                    && !mInvertColor );

  QgsRasterDataProvider::DataType redType = QgsRasterDataProvider::UnknownDataType;
  if ( mRedBand > 0 )
  {
    redType = ( QgsRasterDataProvider::DataType )mProvider->dataType( mRedBand );
  }
  QgsRasterDataProvider::DataType greenType = QgsRasterDataProvider::UnknownDataType;
  if ( mGreenBand > 0 )
  {
    greenType = ( QgsRasterDataProvider::DataType )mProvider->dataType( mGreenBand );
  }
  QgsRasterDataProvider::DataType blueType = QgsRasterDataProvider::UnknownDataType;
  if ( mBlueBand > 0 )
  {
    blueType = ( QgsRasterDataProvider::DataType )mProvider->dataType( mBlueBand );
  }
  QgsRasterDataProvider::DataType transparencyType = QgsRasterDataProvider::UnknownDataType;
  if ( mAlphaBand > 0 )
  {
    transparencyType = ( QgsRasterDataProvider::DataType )mProvider->dataType( mAlphaBand );
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
    return; //no need to draw anything if no band is set
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
    startRasterRead( *bandIt, viewPort, theQgsMapToPixel, oversamplingX, oversamplingY );
  }

  void* redData = 0;
  void* greenData = 0;
  void* blueData = 0;
  void* alphaData = 0;
  //number of cols/rows in output pixels
  int nCols = 0;
  int nRows = 0;
  //number of raster cols/rows with oversampling
  int nRasterCols = 0;
  int nRasterRows = 0;
  //shift to top left point for the raster part
  int topLeftCol = 0;
  int topLeftRow = 0;

  bool readSuccess = true;
  while ( true )
  {
    QSet<int>::const_iterator bandIt = bands.constBegin();
    for ( ; bandIt != bands.constEnd(); ++bandIt )
    {
      readSuccess = readSuccess && readNextRasterPart( *bandIt, oversamplingX, oversamplingY, viewPort, nCols, nRows,
                    nRasterCols, nRasterRows, &bandData[*bandIt], topLeftCol, topLeftRow );
    }

    if ( !readSuccess )
    {
      break;
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

    QImage img( nRasterCols, nRasterRows, QImage::Format_ARGB32_Premultiplied );
    QRgb* imageScanLine = 0;
    int currentRasterPos = 0;
    int redVal = 0;
    int greenVal = 0;
    int blueVal = 0;
    QRgb defaultColor = qRgba( 255, 255, 255, 0 );
    double currentOpacity = mOpacity; //opacity (between 0 and 1)

    for ( int i = 0; i < nRasterRows; ++i )
    {
      imageScanLine = ( QRgb* )( img.scanLine( i ) );
      for ( int j = 0; j < nRasterCols; ++j )
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
        ++currentRasterPos;
      }
    }

    drawImage( p, viewPort, img, topLeftCol, topLeftRow, nCols, nRows, oversamplingX, oversamplingY );
  }

  bandIt = bands.constBegin();
  for ( ; bandIt != bands.constEnd(); ++bandIt )
  {
    stopRasterRead( *bandIt );
  }
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

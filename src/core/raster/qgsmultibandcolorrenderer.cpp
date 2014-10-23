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

QgsRasterInterface * QgsMultiBandColorRenderer::clone() const
{
  QgsMultiBandColorRenderer * renderer = new QgsMultiBandColorRenderer( 0, mRedBand, mGreenBand, mBlueBand );
  if ( mRedContrastEnhancement )
  {
    renderer->setRedContrastEnhancement( new QgsContrastEnhancement( *mRedContrastEnhancement ) );
  }
  if ( mGreenContrastEnhancement )
  {
    renderer->setGreenContrastEnhancement( new QgsContrastEnhancement( *mGreenContrastEnhancement ) );
  }
  if ( mBlueContrastEnhancement )
  {
    renderer->setBlueContrastEnhancement( new QgsContrastEnhancement( *mBlueContrastEnhancement ) );
  }
  renderer->setOpacity( mOpacity );
  renderer->setAlphaBand( mAlphaBand );
  renderer->setRasterTransparency( mRasterTransparency ? new QgsRasterTransparency( *mRasterTransparency ) : 0 );

  return renderer;
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
    redContrastEnhancement = new QgsContrastEnhancement(( QGis::DataType )(
          input->dataType( redBand ) ) );
    redContrastEnhancement->readXML( redContrastElem );
  }

  QgsContrastEnhancement* greenContrastEnhancement = 0;
  QDomElement greenContrastElem = elem.firstChildElement( "greenContrastEnhancement" );
  if ( !greenContrastElem.isNull() )
  {
    greenContrastEnhancement = new QgsContrastEnhancement(( QGis::DataType )(
          input->dataType( greenBand ) ) );
    greenContrastEnhancement->readXML( greenContrastElem );
  }

  QgsContrastEnhancement* blueContrastEnhancement = 0;
  QDomElement blueContrastElem = elem.firstChildElement( "blueContrastEnhancement" );
  if ( !blueContrastElem.isNull() )
  {
    blueContrastEnhancement = new QgsContrastEnhancement(( QGis::DataType )(
          input->dataType( blueBand ) ) );
    blueContrastEnhancement->readXML( blueContrastElem );
  }

  QgsRasterRenderer* r = new QgsMultiBandColorRenderer( input, redBand, greenBand, blueBand, redContrastEnhancement,
      greenContrastEnhancement, blueContrastEnhancement );
  r->readXML( elem );
  return r;
}

QgsRasterBlock* QgsMultiBandColorRenderer::block( int bandNo, QgsRectangle  const & extent, int width, int height )
{
  Q_UNUSED( bandNo );
  QgsRasterBlock *outputBlock = new QgsRasterBlock();
  if ( !mInput )
  {
    return outputBlock;
  }

  //In some (common) cases, we can simplify the drawing loop considerably and save render time
  bool fastDraw = ( !usesTransparency()
                    && mRedBand > 0 && mGreenBand > 0 && mBlueBand > 0
                    && mAlphaBand < 1 && !mRedContrastEnhancement && !mGreenContrastEnhancement && !mBlueContrastEnhancement );

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
    // no need to draw anything if no band is set
    // TODO:: we should probably return default color block
    return outputBlock;
  }

  if ( mAlphaBand > 0 )
  {
    bands << mAlphaBand;
  }

  QMap<int, QgsRasterBlock*> bandBlocks;
  QgsRasterBlock* defaultPointer = 0;
  QSet<int>::const_iterator bandIt = bands.constBegin();
  for ( ; bandIt != bands.constEnd(); ++bandIt )
  {
    bandBlocks.insert( *bandIt, defaultPointer );
  }

  QgsRasterBlock* redBlock = 0;
  QgsRasterBlock* greenBlock = 0;
  QgsRasterBlock* blueBlock = 0;
  QgsRasterBlock* alphaBlock = 0;

  bandIt = bands.constBegin();
  for ( ; bandIt != bands.constEnd(); ++bandIt )
  {
    bandBlocks[*bandIt] =  mInput->block( *bandIt, extent, width, height );
    if ( !bandBlocks[*bandIt] )
    {
      // We should free the alloced mem from block().
      QgsDebugMsg( "No input band" );
      --bandIt;
      for ( ; bandIt != bands.constBegin(); --bandIt )
      {
        delete bandBlocks[*bandIt];
      }
      return outputBlock;
    }
  }

  if ( mRedBand > 0 )
  {
    redBlock = bandBlocks[mRedBand];
  }
  if ( mGreenBand > 0 )
  {
    greenBlock = bandBlocks[mGreenBand];
  }
  if ( mBlueBand > 0 )
  {
    blueBlock = bandBlocks[mBlueBand];
  }
  if ( mAlphaBand > 0 )
  {
    alphaBlock = bandBlocks[mAlphaBand];
  }

  if ( !outputBlock->reset( QGis::ARGB32_Premultiplied, width, height ) )
  {
    for ( int i = 0; i < bandBlocks.size(); i++ )
    {
      delete bandBlocks.value( i );
    }
    return outputBlock;
  }

  QRgb myDefaultColor = NODATA_COLOR;

  for ( qgssize i = 0; i < ( qgssize )width*height; i++ )
  {
    if ( fastDraw ) //fast rendering if no transparency, stretching, color inversion, etc.
    {
      if ( redBlock->isNoData( i ) ||
           greenBlock->isNoData( i ) ||
           blueBlock->isNoData( i ) )
      {
        outputBlock->setColor( i, myDefaultColor );
      }
      else
      {
        int redVal = ( int )redBlock->value( i );
        int greenVal = ( int )greenBlock->value( i );
        int blueVal = ( int )blueBlock->value( i );
        outputBlock->setColor( i, qRgba( redVal, greenVal, blueVal, 255 ) );
      }
      continue;
    }

    bool isNoData = false;
    double redVal = 0;
    double greenVal = 0;
    double blueVal = 0;
    if ( mRedBand > 0 )
    {
      redVal = redBlock->value( i );
      if ( redBlock->isNoData( i ) ) isNoData = true;
    }
    if ( !isNoData && mGreenBand > 0 )
    {
      greenVal = greenBlock->value( i );
      if ( greenBlock->isNoData( i ) ) isNoData = true;
    }
    if ( !isNoData && mBlueBand > 0 )
    {
      blueVal = blueBlock->value( i );
      if ( blueBlock->isNoData( i ) ) isNoData = true;
    }
    if ( isNoData )
    {
      outputBlock->setColor( i, myDefaultColor );
      continue;
    }

    //apply default color if red, green or blue not in displayable range
    if (( mRedContrastEnhancement && !mRedContrastEnhancement->isValueInDisplayableRange( redVal ) )
        || ( mGreenContrastEnhancement && !mGreenContrastEnhancement->isValueInDisplayableRange( redVal ) )
        || ( mBlueContrastEnhancement && !mBlueContrastEnhancement->isValueInDisplayableRange( redVal ) ) )
    {
      outputBlock->setColor( i, myDefaultColor );
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

    //opacity
    double currentOpacity = mOpacity;
    if ( mRasterTransparency )
    {
      currentOpacity = mRasterTransparency->alphaValue( redVal, greenVal, blueVal, mOpacity * 255 ) / 255.0;
    }
    if ( mAlphaBand > 0 )
    {
      currentOpacity *= alphaBlock->value( i ) / 255.0;
    }

    if ( qgsDoubleNear( currentOpacity, 1.0 ) )
    {
      outputBlock->setColor( i, qRgba( redVal, greenVal, blueVal, 255 ) );
    }
    else
    {
      outputBlock->setColor( i, qRgba( currentOpacity * redVal, currentOpacity * greenVal, currentOpacity * blueVal, currentOpacity * 255 ) );
    }
  }

  //delete input blocks
  QMap<int, QgsRasterBlock*>::const_iterator bandDelIt = bandBlocks.constBegin();
  for ( ; bandDelIt != bandBlocks.constEnd(); ++bandDelIt )
  {
    delete bandDelIt.value();
  }

  return outputBlock;
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

QList<int> QgsMultiBandColorRenderer::usesBands() const
{
  QList<int> bandList;
  if ( mRedBand != -1 )
  {
    bandList << mRedBand;
  }
  if ( mGreenBand != -1 )
  {
    bandList << mGreenBand;
  }
  if ( mBlueBand != -1 )
  {
    bandList << mBlueBand;
  }
  return bandList;
}

/***************************************************************************
                         qgssinglebandgrayrenderer.cpp
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

#include "qgssinglebandgrayrenderer.h"
#include "qgscontrastenhancement.h"
#include "qgsrastertransparency.h"
#include <QDomDocument>
#include <QDomElement>
#include <QImage>

QgsSingleBandGrayRenderer::QgsSingleBandGrayRenderer( QgsRasterInterface* input, int grayBand ):
    QgsRasterRenderer( input, "singlebandgray" ), mGrayBand( grayBand ), mGradient( BlackToWhite ), mContrastEnhancement( 0 )
{
}

QgsSingleBandGrayRenderer::~QgsSingleBandGrayRenderer()
{
  delete mContrastEnhancement;
}

QgsRasterInterface * QgsSingleBandGrayRenderer::clone() const
{
  QgsSingleBandGrayRenderer * renderer = new QgsSingleBandGrayRenderer( 0, mGrayBand );
  renderer->setOpacity( mOpacity );
  renderer->setAlphaBand( mAlphaBand );
  renderer->setRasterTransparency( mRasterTransparency );
  renderer->setGradient( mGradient );
  if ( mContrastEnhancement )
  {
    renderer->setContrastEnhancement( new QgsContrastEnhancement( *mContrastEnhancement ) );
  }
  return renderer;
}

QgsRasterRenderer* QgsSingleBandGrayRenderer::create( const QDomElement& elem, QgsRasterInterface* input )
{
  if ( elem.isNull() )
  {
    return 0;
  }

  int grayBand = elem.attribute( "grayBand", "-1" ).toInt();
  QgsSingleBandGrayRenderer* r = new QgsSingleBandGrayRenderer( input, grayBand );
  r->readXML( elem );

  if ( elem.attribute( "gradient" ) == "WhiteToBlack" )
  {
    r->setGradient( WhiteToBlack );  // BlackToWhite is default
  }

  QDomElement contrastEnhancementElem = elem.firstChildElement( "contrastEnhancement" );
  if ( !contrastEnhancementElem.isNull() )
  {
    QgsContrastEnhancement* ce = new QgsContrastEnhancement(( QgsContrastEnhancement::QgsRasterDataType )(
          input->dataType( grayBand ) ) ) ;
    ce->readXML( contrastEnhancementElem );
    r->setContrastEnhancement( ce );
  }
  return r;
}

void QgsSingleBandGrayRenderer::setContrastEnhancement( QgsContrastEnhancement* ce )
{
  delete mContrastEnhancement;
  mContrastEnhancement = ce;
}

QgsRasterBlock* QgsSingleBandGrayRenderer::block( int bandNo, QgsRectangle  const & extent, int width, int height )
{
  Q_UNUSED( bandNo );
  QgsDebugMsg( QString( "width = %1 height = %2" ).arg( width ).arg( height ) );

  QgsRasterBlock *outputBlock = new QgsRasterBlock();
  if ( !mInput )
  {
    return outputBlock;
  }

  QgsRasterBlock *inputBlock = mInput->block( mGrayBand, extent, width, height );
  if ( !inputBlock || inputBlock->isEmpty() )
  {
    QgsDebugMsg( "No raster data!" );
    delete inputBlock;
    return outputBlock;
  }

  QgsRasterBlock *alphaBlock = 0;

  if ( mAlphaBand > 0 && mGrayBand != mAlphaBand )
  {
    alphaBlock = mInput->block( mAlphaBand, extent, width, height );
    if ( !alphaBlock || alphaBlock->isEmpty() )
    {
      // TODO: better to render without alpha
      delete inputBlock;
      delete alphaBlock;
      return outputBlock;
    }
  }
  else if ( mAlphaBand > 0 )
  {
    alphaBlock = inputBlock;
  }

  if ( !outputBlock->reset( QGis::ARGB32_Premultiplied, width, height ) )
  {
    delete inputBlock;
    delete alphaBlock;
    return outputBlock;
  }

  QRgb myDefaultColor = NODATA_COLOR;
  for ( size_t i = 0; i < ( size_t )width*height; i++ )
  {
    if ( inputBlock->isNoData( i ) )
    {
      outputBlock->setColor( i, myDefaultColor );
      continue;
    }
    double grayVal = inputBlock->value( i );

    double currentAlpha = mOpacity;
    if ( mRasterTransparency )
    {
      currentAlpha = mRasterTransparency->alphaValue( grayVal, mOpacity * 255 ) / 255.0;
    }
    if ( mAlphaBand > 0 )
    {
      currentAlpha *= alphaBlock->value( i ) / 255.0;
    }

    if ( mContrastEnhancement )
    {
      if ( !mContrastEnhancement->isValueInDisplayableRange( grayVal ) )
      {
        outputBlock->setColor( i, myDefaultColor );
        continue;
      }
      grayVal = mContrastEnhancement->enhanceContrast( grayVal );
    }

    if ( mGradient == WhiteToBlack )
    {
      grayVal = 255 - grayVal;
    }

    if ( qgsDoubleNear( currentAlpha, 1.0 ) )
    {
      outputBlock->setColor( i, qRgba( grayVal, grayVal, grayVal, 255 ) );
    }
    else
    {
      outputBlock->setColor( i, qRgba( currentAlpha * grayVal, currentAlpha * grayVal, currentAlpha * grayVal, currentAlpha * 255 ) );
    }
  }

  delete inputBlock;
  if ( mAlphaBand > 0 && mGrayBand != mAlphaBand )
  {
    delete alphaBlock;
  }

  return outputBlock;
}

void QgsSingleBandGrayRenderer::writeXML( QDomDocument& doc, QDomElement& parentElem ) const
{
  if ( parentElem.isNull() )
  {
    return;
  }

  QDomElement rasterRendererElem = doc.createElement( "rasterrenderer" );
  _writeXML( doc, rasterRendererElem );

  rasterRendererElem.setAttribute( "grayBand", mGrayBand );

  QString gradient;
  if ( mGradient == BlackToWhite )
  {
    gradient = "BlackToWhite";
  }
  else
  {
    gradient = "WhiteToBlack";
  }
  rasterRendererElem.setAttribute( "gradient", gradient );

  if ( mContrastEnhancement )
  {
    QDomElement contrastElem = doc.createElement( "contrastEnhancement" );
    mContrastEnhancement->writeXML( doc, contrastElem );
    rasterRendererElem.appendChild( contrastElem );
  }
  parentElem.appendChild( rasterRendererElem );
}

void QgsSingleBandGrayRenderer::legendSymbologyItems( QList< QPair< QString, QColor > >& symbolItems ) const
{
  if ( mContrastEnhancement && mContrastEnhancement->contrastEnhancementAlgorithm() != QgsContrastEnhancement::NoEnhancement )
  {
    symbolItems.push_back( qMakePair( QString::number( mContrastEnhancement->minimumValue() ), QColor( 0, 0, 0 ) ) );
    symbolItems.push_back( qMakePair( QString::number( mContrastEnhancement->maximumValue() ), QColor( 255, 255, 255 ) ) );
  }
}

QList<int> QgsSingleBandGrayRenderer::usesBands() const
{
  QList<int> bandList;
  if ( mGrayBand != -1 )
  {
    bandList << mGrayBand;
  }
  return bandList;
}

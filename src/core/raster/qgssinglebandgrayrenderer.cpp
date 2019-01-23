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
#include <QColor>
#include <memory>

QgsSingleBandGrayRenderer::QgsSingleBandGrayRenderer( QgsRasterInterface *input, int grayBand )
  : QgsRasterRenderer( input, QStringLiteral( "singlebandgray" ) )
  , mGrayBand( grayBand )
  , mGradient( BlackToWhite )
  , mContrastEnhancement( nullptr )
{
}

QgsSingleBandGrayRenderer *QgsSingleBandGrayRenderer::clone() const
{
  QgsSingleBandGrayRenderer *renderer = new QgsSingleBandGrayRenderer( nullptr, mGrayBand );
  renderer->copyCommonProperties( this );

  renderer->setGradient( mGradient );
  if ( mContrastEnhancement )
  {
    renderer->setContrastEnhancement( new QgsContrastEnhancement( *mContrastEnhancement ) );
  }
  return renderer;
}

QgsRasterRenderer *QgsSingleBandGrayRenderer::create( const QDomElement &elem, QgsRasterInterface *input )
{
  if ( elem.isNull() )
  {
    return nullptr;
  }

  int grayBand = elem.attribute( QStringLiteral( "grayBand" ), QStringLiteral( "-1" ) ).toInt();
  QgsSingleBandGrayRenderer *r = new QgsSingleBandGrayRenderer( input, grayBand );
  r->readXml( elem );

  if ( elem.attribute( QStringLiteral( "gradient" ) ) == QLatin1String( "WhiteToBlack" ) )
  {
    r->setGradient( WhiteToBlack );  // BlackToWhite is default
  }

  QDomElement contrastEnhancementElem = elem.firstChildElement( QStringLiteral( "contrastEnhancement" ) );
  if ( !contrastEnhancementElem.isNull() )
  {
    QgsContrastEnhancement *ce = new QgsContrastEnhancement( ( Qgis::DataType )(
          input->dataType( grayBand ) ) );
    ce->readXml( contrastEnhancementElem );
    r->setContrastEnhancement( ce );
  }
  return r;
}

void QgsSingleBandGrayRenderer::setContrastEnhancement( QgsContrastEnhancement *ce )
{
  mContrastEnhancement.reset( ce );
}

QgsRasterBlock *QgsSingleBandGrayRenderer::block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback )
{
  Q_UNUSED( bandNo );
  QgsDebugMsgLevel( QStringLiteral( "width = %1 height = %2" ).arg( width ).arg( height ), 4 );

  std::unique_ptr< QgsRasterBlock > outputBlock( new QgsRasterBlock() );
  if ( !mInput )
  {
    return outputBlock.release();
  }

  std::shared_ptr< QgsRasterBlock > inputBlock( mInput->block( mGrayBand, extent, width, height, feedback ) );
  if ( !inputBlock || inputBlock->isEmpty() )
  {
    QgsDebugMsg( QStringLiteral( "No raster data!" ) );
    return outputBlock.release();
  }

  std::shared_ptr< QgsRasterBlock > alphaBlock;

  if ( mAlphaBand > 0 && mGrayBand != mAlphaBand )
  {
    alphaBlock.reset( mInput->block( mAlphaBand, extent, width, height, feedback ) );
    if ( !alphaBlock || alphaBlock->isEmpty() )
    {
      // TODO: better to render without alpha
      return outputBlock.release();
    }
  }
  else if ( mAlphaBand > 0 )
  {
    alphaBlock = inputBlock;
  }

  if ( !outputBlock->reset( Qgis::ARGB32_Premultiplied, width, height ) )
  {
    return outputBlock.release();
  }

  QRgb myDefaultColor = NODATA_COLOR;
  bool isNoData = false;
  for ( qgssize i = 0; i < ( qgssize )width * height; i++ )
  {
    double grayVal = inputBlock->valueAndNoData( i, isNoData );

    if ( isNoData )
    {
      outputBlock->setColor( i, myDefaultColor );
      continue;
    }

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

  return outputBlock.release();
}

void QgsSingleBandGrayRenderer::writeXml( QDomDocument &doc, QDomElement &parentElem ) const
{
  if ( parentElem.isNull() )
  {
    return;
  }

  QDomElement rasterRendererElem = doc.createElement( QStringLiteral( "rasterrenderer" ) );
  _writeXml( doc, rasterRendererElem );

  rasterRendererElem.setAttribute( QStringLiteral( "grayBand" ), mGrayBand );

  QString gradient;
  if ( mGradient == BlackToWhite )
  {
    gradient = QStringLiteral( "BlackToWhite" );
  }
  else
  {
    gradient = QStringLiteral( "WhiteToBlack" );
  }
  rasterRendererElem.setAttribute( QStringLiteral( "gradient" ), gradient );

  if ( mContrastEnhancement )
  {
    QDomElement contrastElem = doc.createElement( QStringLiteral( "contrastEnhancement" ) );
    mContrastEnhancement->writeXml( doc, contrastElem );
    rasterRendererElem.appendChild( contrastElem );
  }
  parentElem.appendChild( rasterRendererElem );
}

void QgsSingleBandGrayRenderer::legendSymbologyItems( QList< QPair< QString, QColor > > &symbolItems ) const
{
  if ( mContrastEnhancement && mContrastEnhancement->contrastEnhancementAlgorithm() != QgsContrastEnhancement::NoEnhancement )
  {
    QColor minColor = ( mGradient == BlackToWhite ) ? Qt::black : Qt::white;
    QColor maxColor = ( mGradient == BlackToWhite ) ? Qt::white : Qt::black;
    symbolItems.push_back( qMakePair( QString::number( mContrastEnhancement->minimumValue() ), minColor ) );
    symbolItems.push_back( qMakePair( QString::number( mContrastEnhancement->maximumValue() ), maxColor ) );
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

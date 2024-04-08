/***************************************************************************
                         qgsrastersinglecolorrenderer.cpp
                         -----------------------------
    begin                : April 2024
    copyright            : (C) 2024 by Mathieu Pellerin
    email                : mathieu at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrastersinglecolorrenderer.h"
#include "qgsrastertransparency.h"
#include "qgscolorutils.h"

#include <QDomDocument>
#include <QDomElement>

QgsRasterSingleColorRenderer::QgsRasterSingleColorRenderer( QgsRasterInterface *input, int band, const QColor &color )
  : QgsRasterRenderer( input, QStringLiteral( "singlecolor" ) )
  , mInputBand( band )
  , mColor( color )
{
}

QgsRasterSingleColorRenderer *QgsRasterSingleColorRenderer::clone() const
{
  QgsRasterSingleColorRenderer *renderer = new QgsRasterSingleColorRenderer( nullptr, mInputBand, mColor );
  renderer->copyCommonProperties( this );
  return renderer;
}

Qgis::RasterRendererFlags QgsRasterSingleColorRenderer::flags() const
{
  return Qgis::RasterRendererFlag::InternalLayerOpacityHandling;
}

QgsRasterRenderer *QgsRasterSingleColorRenderer::create( const QDomElement &elem, QgsRasterInterface *input )
{
  if ( elem.isNull() )
  {
    return nullptr;
  }

  const QColor color = QgsColorUtils::colorFromString( elem.attribute( QStringLiteral( "color" ), QStringLiteral( "0,0,0" ) ) );
  const int band = elem.attribute( QStringLiteral( "band" ), QStringLiteral( "1" ) ).toInt();
  QgsRasterSingleColorRenderer *r = new QgsRasterSingleColorRenderer( input, band, color );
  r->readXml( elem );

  return r;
}

QgsRasterBlock *QgsRasterSingleColorRenderer::block( int, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback )
{
  QgsDebugMsgLevel( QStringLiteral( "width = %1 height = %2" ).arg( width ).arg( height ), 4 );

  std::unique_ptr< QgsRasterBlock > outputBlock( new QgsRasterBlock() );
  if ( !mInput || mInputBand == -1 )
  {
    return outputBlock.release();
  }

  const std::shared_ptr< QgsRasterBlock > inputBlock( mInput->block( mInputBand, extent, width, height, feedback ) );
  if ( !inputBlock || inputBlock->isEmpty() )
  {
    QgsDebugError( QStringLiteral( "No raster data!" ) );
    return outputBlock.release();
  }

  std::shared_ptr< QgsRasterBlock > alphaBlock;
  if ( mAlphaBand > 0 )
  {
    alphaBlock = inputBlock;
  }

  if ( !outputBlock->reset( Qgis::DataType::ARGB32_Premultiplied, width, height ) )
  {
    return outputBlock.release();
  }

  const QRgb defaultColor = renderColorForNodataPixel();
  const QRgb rendererColor = qRgba( mColor.red(), mColor.green(), mColor.blue(), mColor.alpha() );

  bool isNoData = false;
  const qgssize blockSize = static_cast< qgssize >( width ) * height;
  for ( qgssize i = 0; i < blockSize; i++ )
  {
    double value = inputBlock->valueAndNoData( i, isNoData );
    if ( isNoData )
    {
      outputBlock->setColor( i, defaultColor );
      continue;
    }

    double currentAlpha = mOpacity;
    if ( mRasterTransparency )
    {
      currentAlpha *= mRasterTransparency->opacityForValue( value );
    }
    if ( mAlphaBand > 0 )
    {
      const double alpha = alphaBlock->value( i );
      if ( alpha == 0 )
      {
        outputBlock->setColor( i, defaultColor );
        continue;
      }
      else
      {
        currentAlpha *= alpha / 255.0;
      }
    }

    if ( qgsDoubleNear( currentAlpha, 1.0 ) )
    {
      outputBlock->setColor( i, rendererColor );
    }
    else
    {
      outputBlock->setColor( i, qRgba( static_cast<int>( currentAlpha * mColor.red() ),
                                       static_cast<int>( currentAlpha * mColor.green() ),
                                       static_cast<int>( currentAlpha * mColor.blue() ),
                                       static_cast<int>( currentAlpha * mColor.alpha() ) ) );
    }
  }

  return outputBlock.release();
}

void QgsRasterSingleColorRenderer::setColor( const QColor &color )
{
  mColor = color;;
}

QColor QgsRasterSingleColorRenderer::color() const
{
  return mColor;
}

void QgsRasterSingleColorRenderer::writeXml( QDomDocument &doc, QDomElement &parentElem ) const
{
  if ( parentElem.isNull() )
  {
    return;
  }

  QDomElement rasterRendererElem = doc.createElement( QStringLiteral( "rasterrenderer" ) );
  _writeXml( doc, rasterRendererElem );

  rasterRendererElem.setAttribute( QStringLiteral( "color" ), QgsColorUtils::colorToString( mColor ) );
  rasterRendererElem.setAttribute( QStringLiteral( "band" ), mInputBand );

  parentElem.appendChild( rasterRendererElem );
}

int QgsRasterSingleColorRenderer::inputBand() const
{
  return mInputBand;
}

bool QgsRasterSingleColorRenderer::setInputBand( int band )
{
  if ( !mInput || ( band > 0 && band <= mInput->bandCount() ) )
  {
    mInputBand = band;
    return true;
  }
  return false;
}

QList<int> QgsRasterSingleColorRenderer::usesBands() const
{
  QList<int> bands;
  if ( mInputBand != -1 )
  {
    bands << mInputBand;
  }
  return bands;
}

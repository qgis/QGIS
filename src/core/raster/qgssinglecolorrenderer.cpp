/***************************************************************************
                         qgssinglecolorrenderer.cpp
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

#include "qgssinglecolorrenderer.h"
#include "qgsrastertransparency.h"
#include "qgscolorutils.h"

#include <QDomDocument>
#include <QDomElement>

QgsSingleColorRenderer::QgsSingleColorRenderer( QgsRasterInterface *input, QColor color )
  : QgsRasterRenderer( input, QStringLiteral( "singlecolor" ) )
  , mColor( color )
{
}

QgsSingleColorRenderer *QgsSingleColorRenderer::clone() const
{
  QgsSingleColorRenderer *renderer = new QgsSingleColorRenderer( nullptr, mColor );
  renderer->copyCommonProperties( this );
  return renderer;
}

Qgis::RasterRendererFlags QgsSingleColorRenderer::flags() const
{
  return Qgis::RasterRendererFlag::InternalLayerOpacityHandling;
}

QgsRasterRenderer *QgsSingleColorRenderer::create( const QDomElement &elem, QgsRasterInterface *input )
{
  if ( elem.isNull() )
  {
    return nullptr;
  }

  const QColor color = QgsColorUtils::colorFromString( elem.attribute( QStringLiteral( "color" ), QStringLiteral( "0,0,0" ) ) );
  QgsSingleColorRenderer *r = new QgsSingleColorRenderer( input, color );
  r->readXml( elem );

  return r;
}

QgsRasterBlock *QgsSingleColorRenderer::block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback )
{
  QgsDebugMsgLevel( QStringLiteral( "width = %1 height = %2" ).arg( width ).arg( height ), 4 );

  std::unique_ptr< QgsRasterBlock > outputBlock( new QgsRasterBlock() );
  if ( !mInput )
  {
    return outputBlock.release();
  }

  const std::shared_ptr< QgsRasterBlock > inputBlock( mInput->block( bandNo, extent, width, height, feedback ) );
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

  const QRgb myDefaultColor = renderColorForNodataPixel();
  bool isNoData = false;
  for ( qgssize i = 0; i < ( qgssize )width * height; i++ )
  {
    double value = inputBlock->valueAndNoData( i, isNoData );
    if ( isNoData )
    {
      outputBlock->setColor( i, myDefaultColor );
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
        outputBlock->setColor( i, myDefaultColor );
        continue;
      }
      else
      {
        currentAlpha *= alpha / 255.0;
      }
    }

    if ( qgsDoubleNear( currentAlpha, 1.0 ) )
    {
      outputBlock->setColor( i, qRgba( mColor.red(), mColor.green(), mColor.blue(), mColor.alpha() ) );
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

void QgsSingleColorRenderer::setColor( QColor &color )
{
  mColor = color;;
}

QColor QgsSingleColorRenderer::color() const
{
  return mColor;
}

void QgsSingleColorRenderer::writeXml( QDomDocument &doc, QDomElement &parentElem ) const
{
  if ( parentElem.isNull() )
  {
    return;
  }

  QDomElement rasterRendererElem = doc.createElement( QStringLiteral( "rasterrenderer" ) );
  _writeXml( doc, rasterRendererElem );

  rasterRendererElem.setAttribute( QStringLiteral( "color" ), QgsColorUtils::colorToString( mColor ) );

  parentElem.appendChild( rasterRendererElem );
}

QList<int> QgsSingleColorRenderer::usesBands() const
{
  QList<int> bandList;
  for ( int i = 0; i <= bandCount(); i++ )
  {
    bandList << i;
  }
  return bandList;
}

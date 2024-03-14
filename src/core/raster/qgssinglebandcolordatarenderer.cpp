/***************************************************************************
                         qgssinglebandcolordatarenderer.cpp
                         ----------------------------------
    begin                : January 2012
    copyright            : (C) 2012 by Marco Hugentobler
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

#include "qgssinglebandcolordatarenderer.h"
#include "qgsrastertransparency.h"
#include "qgsrasterviewport.h"
#include <QDomDocument>
#include <QDomElement>
#include <QImage>
#include <memory>

QgsSingleBandColorDataRenderer::QgsSingleBandColorDataRenderer( QgsRasterInterface *input, int band )
  : QgsRasterRenderer( input, QStringLiteral( "singlebandcolordata" ) )
  , mBand( band )
{

}

QgsSingleBandColorDataRenderer *QgsSingleBandColorDataRenderer::clone() const
{
  QgsSingleBandColorDataRenderer *renderer = new QgsSingleBandColorDataRenderer( nullptr, mBand );
  renderer->copyCommonProperties( this );
  return renderer;
}

Qgis::RasterRendererFlags QgsSingleBandColorDataRenderer::flags() const
{
  return Qgis::RasterRendererFlag::InternalLayerOpacityHandling;
}

QgsRasterRenderer *QgsSingleBandColorDataRenderer::create( const QDomElement &elem, QgsRasterInterface *input )
{
  if ( elem.isNull() )
  {
    return nullptr;
  }

  const int band = elem.attribute( QStringLiteral( "band" ), QStringLiteral( "-1" ) ).toInt();
  QgsRasterRenderer *r = new QgsSingleBandColorDataRenderer( input, band );
  r->readXml( elem );
  return r;
}

QgsRasterBlock *QgsSingleBandColorDataRenderer::block( int bandNo, QgsRectangle  const &extent, int width, int height, QgsRasterBlockFeedback *feedback )
{
  Q_UNUSED( bandNo )

  std::unique_ptr< QgsRasterBlock > outputBlock( new QgsRasterBlock() );
  if ( !mInput )
  {
    return outputBlock.release();
  }

  std::unique_ptr< QgsRasterBlock > inputBlock( mInput->block( mBand, extent, width, height, feedback ) );
  if ( !inputBlock || inputBlock->isEmpty() )
  {
    QgsDebugError( QStringLiteral( "No raster data!" ) );
    return outputBlock.release();
  }

  const bool hasTransparency = usesTransparency();
  if ( !hasTransparency )
  {
    // Nothing to do, just retype if necessary
    inputBlock->convert( Qgis::DataType::ARGB32_Premultiplied );
    return inputBlock.release();
  }

  if ( !outputBlock->reset( Qgis::DataType::ARGB32_Premultiplied, width, height ) )
  {
    return outputBlock.release();
  }

  // make sure input is also premultiplied!
  inputBlock->convert( Qgis::DataType::ARGB32_Premultiplied );

  QRgb *inputBits = ( QRgb * )inputBlock->bits();
  QRgb *outputBits = ( QRgb * )outputBlock->bits();
  for ( qgssize i = 0; i < ( qgssize )width * height; i++ )
  {
    const QRgb c = inputBits[i];
    outputBits[i] = qRgba( mOpacity * qRed( c ), mOpacity * qGreen( c ), mOpacity * qBlue( c ), mOpacity * qAlpha( c ) );
  }

  return outputBlock.release();
}

void QgsSingleBandColorDataRenderer::writeXml( QDomDocument &doc, QDomElement &parentElem ) const
{
  if ( parentElem.isNull() )
    return;

  QDomElement rasterRendererElem = doc.createElement( QStringLiteral( "rasterrenderer" ) );
  _writeXml( doc, rasterRendererElem );
  rasterRendererElem.setAttribute( QStringLiteral( "band" ), mBand );
  parentElem.appendChild( rasterRendererElem );
}

QList<int> QgsSingleBandColorDataRenderer::usesBands() const
{
  QList<int> bandList;
  if ( mBand != -1 )
  {
    bandList << mBand;
  }
  return bandList;
}

bool QgsSingleBandColorDataRenderer::setInput( QgsRasterInterface *input )
{
  // Renderer can only work with numerical values in at least 1 band
  if ( !input ) return false;

  if ( !mOn )
  {
    // In off mode we can connect to anything
    mInput = input;
    return true;
  }

  if ( input->dataType( 1 ) == Qgis::DataType::ARGB32 ||
       input->dataType( 1 ) == Qgis::DataType::ARGB32_Premultiplied )
  {
    mInput = input;
    return true;
  }
  return false;
}

int QgsSingleBandColorDataRenderer::inputBand() const
{
  return mBand;
}

bool QgsSingleBandColorDataRenderer::setInputBand( int band )
{
  if ( !mInput )
  {
    mBand = band;
    return true;
  }
  else if ( band > 0 && band <= mInput->bandCount() )
  {
    mBand = band;
    return true;
  }
  return false;
}

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

QgsSingleBandColorDataRenderer::QgsSingleBandColorDataRenderer( QgsRasterInterface* input, int band ):
    QgsRasterRenderer( input, "singlebandcolordata" ), mBand( band )
{

}

QgsSingleBandColorDataRenderer::~QgsSingleBandColorDataRenderer()
{
}

QgsRasterInterface * QgsSingleBandColorDataRenderer::clone() const
{
  QgsSingleBandColorDataRenderer * renderer = new QgsSingleBandColorDataRenderer( 0, mBand );
  renderer->setOpacity( mOpacity );
  renderer->setAlphaBand( mAlphaBand );
  renderer->setRasterTransparency( mRasterTransparency ? new QgsRasterTransparency( *mRasterTransparency ) : 0 );
  return renderer;
}

QgsRasterRenderer* QgsSingleBandColorDataRenderer::create( const QDomElement& elem, QgsRasterInterface* input )
{
  if ( elem.isNull() )
  {
    return 0;
  }

  int band = elem.attribute( "band", "-1" ).toInt();
  QgsRasterRenderer* r = new QgsSingleBandColorDataRenderer( input, band );
  r->readXML( elem );
  return r;
}

QgsRasterBlock* QgsSingleBandColorDataRenderer::block( int bandNo, QgsRectangle  const & extent, int width, int height )
{
  Q_UNUSED( bandNo );

  QgsRasterBlock *outputBlock = new QgsRasterBlock();
  if ( !mInput )
  {
    return outputBlock;
  }

  QgsRasterBlock *inputBlock = mInput->block( mBand, extent, width, height );
  if ( !inputBlock || inputBlock->isEmpty() )
  {
    QgsDebugMsg( "No raster data!" );
    delete inputBlock;
    return outputBlock;
  }

  bool hasTransparency = usesTransparency();
  if ( !hasTransparency )
  {
    // Nothing to do, just retype if necessary
    inputBlock->convert( QGis::ARGB32_Premultiplied );
    delete outputBlock;
    return inputBlock;
  }

  if ( !outputBlock->reset( QGis::ARGB32_Premultiplied, width, height ) )
  {
    delete inputBlock;
    return outputBlock;
  }

  for ( qgssize i = 0; i < ( qgssize )width*height; i++ )
  {
    QRgb pixelColor;
    QRgb c = inputBlock->color( i );
    double alpha = qAlpha( c );
    pixelColor = qRgba( mOpacity * qRed( c ), mOpacity * qGreen( c ), mOpacity * qBlue( c ), mOpacity * alpha );
    outputBlock->setColor( i,  pixelColor );
  }

  delete inputBlock;
  return outputBlock;
}

void QgsSingleBandColorDataRenderer::writeXML( QDomDocument& doc, QDomElement& parentElem ) const
{
  if ( parentElem.isNull() )
    return;

  QDomElement rasterRendererElem = doc.createElement( "rasterrenderer" );
  _writeXML( doc, rasterRendererElem );
  rasterRendererElem.setAttribute( "band", mBand );
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

bool QgsSingleBandColorDataRenderer::setInput( QgsRasterInterface* input )
{
  // Renderer can only work with numerical values in at least 1 band
  if ( !input ) return false;

  if ( !mOn )
  {
    // In off mode we can connect to anything
    mInput = input;
    return true;
  }

  if ( input->dataType( 1 ) == QGis::ARGB32 ||
       input->dataType( 1 ) == QGis::ARGB32_Premultiplied )
  {
    mInput = input;
    return true;
  }
  return false;
}

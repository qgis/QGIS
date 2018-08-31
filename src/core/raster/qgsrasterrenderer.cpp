/***************************************************************************
                         qgsrasterrenderer.cpp
                         ---------------------
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

#include "qgsrasterrenderer.h"
#include "qgsrastertransparency.h"

#include <QCoreApplication>
#include <QDomDocument>
#include <QDomElement>
#include <QImage>
#include <QPainter>

// See #9101 before any change of NODATA_COLOR!
const QRgb QgsRasterRenderer::NODATA_COLOR = qRgba( 0, 0, 0, 0 );

QgsRasterRenderer::QgsRasterRenderer( QgsRasterInterface *input, const QString &type )
  : QgsRasterInterface( input )
  , mType( type )
{
}

QgsRasterRenderer::~QgsRasterRenderer()
{
  delete mRasterTransparency;
}

int QgsRasterRenderer::bandCount() const
{
  if ( mOn ) return 1;

  if ( mInput ) return mInput->bandCount();

  return 0;
}

Qgis::DataType QgsRasterRenderer::dataType( int bandNo ) const
{
  QgsDebugMsgLevel( QStringLiteral( "Entered" ), 4 );

  if ( mOn ) return Qgis::ARGB32_Premultiplied;

  if ( mInput ) return mInput->dataType( bandNo );

  return Qgis::UnknownDataType;
}

bool QgsRasterRenderer::setInput( QgsRasterInterface *input )
{
  // Renderer can only work with numerical values in at least 1 band
  if ( !input ) return false;

  if ( !mOn )
  {
    // In off mode we can connect to anything
    mInput = input;
    return true;
  }

  for ( int i = 1; i <= input->bandCount(); i++ )
  {
    if ( !QgsRasterBlock::typeIsNumeric( input->dataType( i ) ) )
    {
      return false;
    }
  }
  mInput = input;
  return true;
}

bool QgsRasterRenderer::usesTransparency() const
{
  if ( !mInput )
  {
    return true;
  }
  return ( mAlphaBand > 0 || ( mRasterTransparency && !mRasterTransparency->isEmpty() ) || !qgsDoubleNear( mOpacity, 1.0 ) );
}

void QgsRasterRenderer::setRasterTransparency( QgsRasterTransparency *t )
{
  delete mRasterTransparency;
  mRasterTransparency = t;
}

void QgsRasterRenderer::_writeXml( QDomDocument &doc, QDomElement &rasterRendererElem ) const
{
  if ( rasterRendererElem.isNull() )
  {
    return;
  }

  rasterRendererElem.setAttribute( QStringLiteral( "type" ), mType );
  rasterRendererElem.setAttribute( QStringLiteral( "opacity" ), QString::number( mOpacity ) );
  rasterRendererElem.setAttribute( QStringLiteral( "alphaBand" ), mAlphaBand );

  if ( mRasterTransparency )
  {
    mRasterTransparency->writeXml( doc, rasterRendererElem );
  }

  QDomElement minMaxOriginElem = doc.createElement( QStringLiteral( "minMaxOrigin" ) );
  mMinMaxOrigin.writeXml( doc, minMaxOriginElem );
  rasterRendererElem.appendChild( minMaxOriginElem );
}

void QgsRasterRenderer::readXml( const QDomElement &rendererElem )
{
  if ( rendererElem.isNull() )
  {
    return;
  }

  mType = rendererElem.attribute( QStringLiteral( "type" ) );
  mOpacity = rendererElem.attribute( QStringLiteral( "opacity" ), QStringLiteral( "1.0" ) ).toDouble();
  mAlphaBand = rendererElem.attribute( QStringLiteral( "alphaBand" ), QStringLiteral( "-1" ) ).toInt();

  QDomElement rasterTransparencyElem = rendererElem.firstChildElement( QStringLiteral( "rasterTransparency" ) );
  if ( !rasterTransparencyElem.isNull() )
  {
    delete mRasterTransparency;
    mRasterTransparency = new QgsRasterTransparency();
    mRasterTransparency->readXml( rasterTransparencyElem );
  }

  QDomElement minMaxOriginElem = rendererElem.firstChildElement( QStringLiteral( "minMaxOrigin" ) );
  if ( !minMaxOriginElem.isNull() )
  {
    mMinMaxOrigin.readXml( minMaxOriginElem );
  }
}

void QgsRasterRenderer::copyCommonProperties( const QgsRasterRenderer *other, bool copyMinMaxOrigin )
{
  if ( !other )
    return;

  setOpacity( other->opacity() );
  setAlphaBand( other->alphaBand() );
  setRasterTransparency( other->rasterTransparency() ? new QgsRasterTransparency( *other->rasterTransparency() ) : nullptr );
  if ( copyMinMaxOrigin )
    setMinMaxOrigin( other->minMaxOrigin() );
}

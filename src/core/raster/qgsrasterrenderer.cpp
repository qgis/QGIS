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
#include "qgsrasterresampler.h"
#include "qgsrasterprojector.h"
#include "qgsrastertransparency.h"
#include "qgsrasterviewport.h"
#include "qgsmaptopixel.h"

//resamplers
#include "qgsbilinearrasterresampler.h"
#include "qgscubicrasterresampler.h"

#include <QDomDocument>
#include <QDomElement>
#include <QImage>
#include <QPainter>

QgsRasterRenderer::QgsRasterRenderer( QgsRasterInterface* input, const QString& type )
    : QgsRasterInterface( input ),
    mType( type ), mOpacity( 1.0 ), mRasterTransparency( 0 ),
    mAlphaBand( -1 ), mInvertColor( false ), mMaxOversampling( 2.0 )
{
}

QgsRasterRenderer::~QgsRasterRenderer()
{
}

int QgsRasterRenderer::bandCount() const
{
  if ( mOn ) return 1;

  if ( mInput ) return mInput->bandCount();

  return 0;
}

QgsRasterInterface::DataType QgsRasterRenderer::dataType( int bandNo ) const
{
  if ( mOn ) return QgsRasterInterface::ARGB32_Premultiplied;

  if ( mInput ) return mInput->dataType( bandNo );

  return QgsRasterInterface::UnknownDataType;
}

bool QgsRasterRenderer::setInput( QgsRasterInterface* input )
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
    if ( typeIsNumeric( input->dataType( i ) ) )
    {
      mInput = input;
      return true;
    }
  }
  return false;
}

bool QgsRasterRenderer::usesTransparency( ) const
{
  if ( !mInput )
  {
    return true;
  }
  return ( mAlphaBand > 0 || ( mRasterTransparency && !mRasterTransparency->isEmpty( mInput->noDataValue() ) ) || !doubleNear( mOpacity, 1.0 ) );
}

void QgsRasterRenderer::setRasterTransparency( QgsRasterTransparency* t )
{
  delete mRasterTransparency;
  mRasterTransparency = t;
}

void QgsRasterRenderer::_writeXML( QDomDocument& doc, QDomElement& rasterRendererElem ) const
{
  if ( rasterRendererElem.isNull() )
  {
    return;
  }

  rasterRendererElem.setAttribute( "type", mType );
  rasterRendererElem.setAttribute( "opacity", QString::number( mOpacity ) );
  rasterRendererElem.setAttribute( "alphaBand", mAlphaBand );
  rasterRendererElem.setAttribute( "invertColor", mInvertColor );

  if ( mRasterTransparency )
  {
    mRasterTransparency->writeXML( doc, rasterRendererElem );
  }
}

void QgsRasterRenderer::readXML( const QDomElement& rendererElem )
{
  if ( rendererElem.isNull() )
  {
    return;
  }

  mType = rendererElem.attribute( "type" );
  mOpacity = rendererElem.attribute( "opacity", "1.0" ).toDouble();
  mAlphaBand = rendererElem.attribute( "alphaBand", "-1" ).toInt();
  mInvertColor = rendererElem.attribute( "invertColor", "0" ).toInt();

  //todo: read mRasterTransparency
  QDomElement rasterTransparencyElem = rendererElem.firstChildElement( "rasterTransparency" );
  if ( !rasterTransparencyElem.isNull() )
  {
    delete mRasterTransparency;
    mRasterTransparency = new QgsRasterTransparency();
    mRasterTransparency->readXML( rasterTransparencyElem );
  }
}

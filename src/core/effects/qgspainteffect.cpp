/***************************************************************************
                              qgspainteffect.cpp
                             -------------------
    begin                : December 2014
    copyright            : (C) 2014 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspainteffect.h"
#include "qgsimageoperation.h"
#include "qgslogger.h"
#include <QPicture>

Q_GUI_EXPORT extern int qt_defaultDpiX();
Q_GUI_EXPORT extern int qt_defaultDpiY();

QgsPaintEffect::QgsPaintEffect()
    : mEnabled( true )
    , mDrawMode( ModifyAndRender )
    , requiresQPainterDpiFix( true )
    , mPicture( 0 )
    , mSourceImage( 0 )
    , mOwnsImage( false )
    , mPrevPainter( 0 )
    , mEffectPainter( 0 )
    , mTempPicture( 0 )
{

}

QgsPaintEffect::QgsPaintEffect( const QgsPaintEffect &other )
    : mEnabled( other.enabled() )
    , mDrawMode( other.drawMode() )
    , requiresQPainterDpiFix( true )
    , mPicture( 0 )
    , mSourceImage( 0 )
    , mOwnsImage( false )
    , mPrevPainter( 0 )
    , mEffectPainter( 0 )
    , mTempPicture( 0 )
{

}

QgsPaintEffect::~QgsPaintEffect()
{
  if ( mOwnsImage )
  {
    delete mSourceImage;
  }
  delete mEffectPainter;
  delete mTempPicture;
}

void QgsPaintEffect::setEnabled( const bool enabled )
{
  mEnabled = enabled;
}

void QgsPaintEffect::setDrawMode( const QgsPaintEffect::DrawMode drawMode )
{
  mDrawMode = drawMode;
}

bool QgsPaintEffect::saveProperties( QDomDocument &doc, QDomElement &element ) const
{
  if ( element.isNull() )
  {
    return false;
  }

  QDomElement effectElement = doc.createElement( "effect" );
  effectElement.setAttribute( QString( "type" ), type() );

  QgsStringMap props = properties();
  for ( QgsStringMap::iterator it = props.begin(); it != props.end(); ++it )
  {
    QDomElement propEl = doc.createElement( "prop" );
    propEl.setAttribute( "k", it.key() );
    propEl.setAttribute( "v", it.value() );
    effectElement.appendChild( propEl );
  }

  element.appendChild( effectElement );
  return true;
}

bool QgsPaintEffect::readProperties( const QDomElement &element )
{
  if ( element.isNull() )
  {
    return false;
  }

  //default implementation converts to a string map
  QgsStringMap props;

  QDomElement e = element.firstChildElement();
  while ( !e.isNull() )
  {
    if ( e.tagName() != "prop" )
    {
      QgsDebugMsg( "unknown tag " + e.tagName() );
    }
    else
    {
      QString propKey = e.attribute( "k" );
      QString propValue = e.attribute( "v" );
      props[propKey] = propValue;
    }
    e = e.nextSiblingElement();
  }

  readProperties( props );
  return true;
}

void QgsPaintEffect::render( QPicture &picture, QgsRenderContext &context )
{
  //set source picture
  mPicture = &picture;
  delete mSourceImage;
  mSourceImage = 0;

  draw( context );
}

void QgsPaintEffect::begin( QgsRenderContext &context )
{
  //temporarily replace painter and direct paint operations for context to a QPicture
  mPrevPainter = context.painter();

  delete mTempPicture;
  mTempPicture = new QPicture();

  delete mEffectPainter;
  mEffectPainter = new QPainter();
  mEffectPainter->begin( mTempPicture );

  context.setPainter( mEffectPainter );
}

void QgsPaintEffect::end( QgsRenderContext &context )
{
  if ( !mEffectPainter )
    return;

  mEffectPainter->end();
  delete mEffectPainter;
  mEffectPainter = 0;

  //restore previous painter for context
  context.setPainter( mPrevPainter );
  mPrevPainter = 0;

  //draw using effect
  render( *mTempPicture, context );

  //clean up
  delete mTempPicture;
  mTempPicture = 0;
}

void QgsPaintEffect::drawSource( QPainter &painter )
{
  if ( requiresQPainterDpiFix )
  {
    painter.save();
    fixQPictureDpi( &painter );
    painter.drawPicture( 0, 0, *mPicture );
    painter.restore();
  }
  else
  {
    painter.drawPicture( 0, 0, *mPicture );
  }
}

QImage* QgsPaintEffect::sourceAsImage( QgsRenderContext &context )
{
  //have we already created a source image? if so, return it
  if ( mSourceImage )
  {
    return mSourceImage;
  }

  if ( !mPicture )
    return 0;

  //else create it
  //TODO - test with premultiplied image for speed
  QRectF bounds = imageBoundingRect( context );
  mSourceImage = new QImage( bounds.width(), bounds.height(), QImage::Format_ARGB32 );
  mSourceImage->fill( Qt::transparent );
  QPainter imagePainter( mSourceImage );
  imagePainter.setRenderHint( QPainter::Antialiasing );
  imagePainter.translate( -bounds.left(), -bounds.top() );
  imagePainter.drawPicture( 0, 0, *mPicture );
  imagePainter.end();
  mOwnsImage = true;
  return mSourceImage;
}

QPointF QgsPaintEffect::imageOffset( const QgsRenderContext& context ) const
{
  return imageBoundingRect( context ).topLeft();
}

QRectF QgsPaintEffect::boundingRect( const QRectF &rect, const QgsRenderContext &context ) const
{
  Q_UNUSED( context );
  return rect;
}

void QgsPaintEffect::fixQPictureDpi( QPainter *painter ) const
{
  // QPicture makes an assumption that we drawing to it with system DPI.
  // Then when being drawn, it scales the painter. The following call
  // negates the effect. There is no way of setting QPicture's DPI.
  // See QTBUG-20361
  painter->scale(( double )qt_defaultDpiX() / painter->device()->logicalDpiX(),
                 ( double )qt_defaultDpiY() / painter->device()->logicalDpiY() );
}

QRectF QgsPaintEffect::imageBoundingRect( const QgsRenderContext &context ) const
{
  return boundingRect( mPicture->boundingRect(), context );
}


//
// QgsDrawSourceEffect
//

QgsDrawSourceEffect::QgsDrawSourceEffect()
    : QgsPaintEffect()
    , mTransparency( 0.0 )
    , mBlendMode( QPainter::CompositionMode_SourceOver )
{

}

QgsDrawSourceEffect::~QgsDrawSourceEffect()
{

}

QgsPaintEffect *QgsDrawSourceEffect::create( const QgsStringMap &map )
{
  QgsDrawSourceEffect* effect = new QgsDrawSourceEffect();
  effect->readProperties( map );
  return effect;
}

void QgsDrawSourceEffect::draw( QgsRenderContext &context )
{
  if ( !enabled() || !context.painter() )
    return;

  QPainter* painter = context.painter();

  if ( mBlendMode == QPainter::CompositionMode_SourceOver && qgsDoubleNear( mTransparency, 0.0 ) )
  {
    //just draw unmodified source
    drawSource( *painter );
  }
  else
  {
    //rasterise source and apply modifications
    QImage image = sourceAsImage( context )->copy();
    QgsImageOperation::multiplyOpacity( image, 1.0 - mTransparency );
    painter->save();
    painter->setCompositionMode( mBlendMode );
    painter->drawImage( imageOffset( context ), image );
    painter->restore();
  }
}

QgsPaintEffect *QgsDrawSourceEffect::clone() const
{
  return new QgsDrawSourceEffect( *this );
}

QgsStringMap QgsDrawSourceEffect::properties() const
{
  QgsStringMap props;
  props.insert( "enabled", mEnabled ? "1" : "0" );
  props.insert( "draw_mode", QString::number( int( mDrawMode ) ) );
  props.insert( "blend_mode", QString::number( int( mBlendMode ) ) );
  props.insert( "transparency", QString::number( mTransparency ) );
  return props;
}

void QgsDrawSourceEffect::readProperties( const QgsStringMap &props )
{
  bool ok;
  QPainter::CompositionMode mode = ( QPainter::CompositionMode )props.value( "blend_mode" ).toInt( &ok );
  if ( ok )
  {
    mBlendMode = mode;
  }
  double transparency = props.value( "transparency" ).toDouble( &ok );
  if ( ok )
  {
    mTransparency = transparency;
  }
  mEnabled = props.value( "enabled", "1" ).toInt();
  mDrawMode = ( QgsPaintEffect::DrawMode )props.value( "draw_mode", "2" ).toInt();
}

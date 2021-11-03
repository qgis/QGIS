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
#include "qgsrendercontext.h"
#include "qgssymbollayerutils.h"
#include <QPicture>

Q_GUI_EXPORT extern int qt_defaultDpiX();
Q_GUI_EXPORT extern int qt_defaultDpiY();

QgsPaintEffect::QgsPaintEffect( const QgsPaintEffect &other )
  : mEnabled( other.enabled() )
  , mDrawMode( other.drawMode() )
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
  QDomElement effectElement = doc.createElement( QStringLiteral( "effect" ) );
  effectElement.setAttribute( QStringLiteral( "type" ), type() );
  QgsSymbolLayerUtils::saveProperties( properties(), doc, effectElement );
  element.appendChild( effectElement );
  return true;
}

bool QgsPaintEffect::readProperties( const QDomElement &element )
{
  if ( element.isNull() )
  {
    return false;
  }

  const QVariantMap props = QgsSymbolLayerUtils::parseProperties( element );
  readProperties( props );
  return true;
}

void QgsPaintEffect::render( QPicture &picture, QgsRenderContext &context )
{
  //set source picture
  mPicture = &picture;
  delete mSourceImage;
  mSourceImage = nullptr;

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
  mEffectPainter = nullptr;

  //restore previous painter for context
  context.setPainter( mPrevPainter );
  mPrevPainter = nullptr;

  // clear any existing pen/brush - sometimes these are not correctly restored when restoring a painter
  // with a QPicture destination - see #15696
  context.painter()->setPen( Qt::NoPen );
  context.painter()->setBrush( Qt::NoBrush );

  //draw using effect
  render( *mTempPicture, context );

  //clean up
  delete mTempPicture;
  mTempPicture = nullptr;
}

void QgsPaintEffect::drawSource( QPainter &painter )
{
  if ( requiresQPainterDpiFix )
  {
    const QgsScopedQPainterState painterState( &painter );
    fixQPictureDpi( &painter );
    painter.drawPicture( 0, 0, *mPicture );
  }
  else
  {
    painter.drawPicture( 0, 0, *mPicture );
  }
}

QImage *QgsPaintEffect::sourceAsImage( QgsRenderContext &context )
{
  //have we already created a source image? if so, return it
  if ( mSourceImage )
  {
    return mSourceImage;
  }

  if ( !mPicture )
    return nullptr;

  //else create it
  //TODO - test with premultiplied image for speed
  const QRectF bounds = imageBoundingRect( context );
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

QPointF QgsPaintEffect::imageOffset( const QgsRenderContext &context ) const
{
  return imageBoundingRect( context ).topLeft();
}

QRectF QgsPaintEffect::boundingRect( const QRectF &rect, const QgsRenderContext &context ) const
{
  Q_UNUSED( context )
  return rect;
}

void QgsPaintEffect::fixQPictureDpi( QPainter *painter ) const
{
  // QPicture makes an assumption that we drawing to it with system DPI.
  // Then when being drawn, it scales the painter. The following call
  // negates the effect. There is no way of setting QPicture's DPI.
  // See QTBUG-20361
  painter->scale( static_cast< double >( qt_defaultDpiX() ) / painter->device()->logicalDpiX(),
                  static_cast< double >( qt_defaultDpiY() ) / painter->device()->logicalDpiY() );
}

QRectF QgsPaintEffect::imageBoundingRect( const QgsRenderContext &context ) const
{
  return boundingRect( mPicture->boundingRect(), context );
}


//
// QgsDrawSourceEffect
//

QgsPaintEffect *QgsDrawSourceEffect::create( const QVariantMap &map )
{
  QgsDrawSourceEffect *effect = new QgsDrawSourceEffect();
  effect->readProperties( map );
  return effect;
}

void QgsDrawSourceEffect::draw( QgsRenderContext &context )
{
  if ( !enabled() || !context.painter() )
    return;

  QPainter *painter = context.painter();

  if ( mBlendMode == QPainter::CompositionMode_SourceOver && qgsDoubleNear( mOpacity, 1.0 ) )
  {
    //just draw unmodified source
    drawSource( *painter );
  }
  else
  {
    //rasterize source and apply modifications
    QImage image = sourceAsImage( context )->copy();
    QgsImageOperation::multiplyOpacity( image, mOpacity, context.feedback() );
    const QgsScopedQPainterState painterState( painter );
    painter->setCompositionMode( mBlendMode );
    painter->drawImage( imageOffset( context ), image );
  }
}

QgsDrawSourceEffect *QgsDrawSourceEffect::clone() const
{
  return new QgsDrawSourceEffect( *this );
}

QVariantMap QgsDrawSourceEffect::properties() const
{
  QVariantMap props;
  props.insert( QStringLiteral( "enabled" ), mEnabled ? "1" : "0" );
  props.insert( QStringLiteral( "draw_mode" ), QString::number( int( mDrawMode ) ) );
  props.insert( QStringLiteral( "blend_mode" ), QString::number( int( mBlendMode ) ) );
  props.insert( QStringLiteral( "opacity" ), QString::number( mOpacity ) );
  return props;
}

void QgsDrawSourceEffect::readProperties( const QVariantMap &props )
{
  bool ok;
  const QPainter::CompositionMode mode = static_cast< QPainter::CompositionMode >( props.value( QStringLiteral( "blend_mode" ) ).toInt( &ok ) );
  if ( ok )
  {
    mBlendMode = mode;
  }
  if ( props.contains( QStringLiteral( "transparency" ) ) )
  {
    const double transparency = props.value( QStringLiteral( "transparency" ) ).toDouble( &ok );
    if ( ok )
    {
      mOpacity = 1.0 - transparency;
    }
  }
  else
  {
    const double opacity = props.value( QStringLiteral( "opacity" ) ).toDouble( &ok );
    if ( ok )
    {
      mOpacity = opacity;
    }
  }
  mEnabled = props.value( QStringLiteral( "enabled" ), QStringLiteral( "1" ) ).toInt();
  mDrawMode = static_cast< QgsPaintEffect::DrawMode >( props.value( QStringLiteral( "draw_mode" ), QStringLiteral( "2" ) ).toInt() );
}


//
// QgsEffectPainter
//

QgsEffectPainter::QgsEffectPainter( QgsRenderContext &renderContext )
  : mRenderContext( renderContext )

{
  mPainter = renderContext.painter();
  mPainter->save();
}

QgsEffectPainter::QgsEffectPainter( QgsRenderContext &renderContext, QgsPaintEffect *effect )
  : mRenderContext( renderContext )
  , mEffect( effect )
{
  mPainter = mRenderContext.painter();
  mPainter->save();
  mEffect->begin( mRenderContext );
}

void QgsEffectPainter::setEffect( QgsPaintEffect *effect )
{
  Q_ASSERT( !mEffect );

  mEffect = effect;
  mEffect->begin( mRenderContext );
}

QgsEffectPainter::~QgsEffectPainter()
{
  Q_ASSERT( mEffect );

  mEffect->end( mRenderContext );
  mPainter->restore();
}

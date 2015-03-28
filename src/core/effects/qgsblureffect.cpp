/***************************************************************************
                              qgsblureffect.cpp
                              -----------------
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

#include "qgsblureffect.h"
#include "qgsimageoperation.h"

QgsPaintEffect *QgsBlurEffect::create( const QgsStringMap &map )
{
  QgsBlurEffect* newEffect = new QgsBlurEffect();
  newEffect->readProperties( map );
  return newEffect;
}

QgsBlurEffect::QgsBlurEffect()
    : QgsPaintEffect()
    , mBlurLevel( 10 )
    , mBlurMethod( StackBlur )
    , mTransparency( 0.0 )
    , mBlendMode( QPainter::CompositionMode_SourceOver )
{

}

QgsBlurEffect::~QgsBlurEffect()
{

}

void QgsBlurEffect::draw( QgsRenderContext &context )
{
  if ( !source() || !enabled() || !context.painter() )
    return;

  switch ( mBlurMethod )
  {
    case StackBlur:
      drawStackBlur( context );
      break;
    case GaussianBlur:
      drawGaussianBlur( context );
      break;
  }
}

void QgsBlurEffect::drawStackBlur( QgsRenderContext& context )
{
  QImage im = sourceAsImage( context )->copy();
  QgsImageOperation::stackBlur( im, mBlurLevel );
  drawBlurredImage( context, im );
}

void QgsBlurEffect::drawGaussianBlur( QgsRenderContext &context )
{
  QImage* im = QgsImageOperation::gaussianBlur( *sourceAsImage( context ), mBlurLevel );
  drawBlurredImage( context, *im );
  delete im;
}

void QgsBlurEffect::drawBlurredImage( QgsRenderContext& context, QImage& image )
{
  //transparency
  QgsImageOperation::multiplyOpacity( image, 1.0 - mTransparency );

  QPainter* painter = context.painter();
  painter->save();
  painter->setCompositionMode( mBlendMode );
  painter->drawImage( imageOffset( context ), image );
  painter->restore();
}

QgsStringMap QgsBlurEffect::properties() const
{
  QgsStringMap props;
  props.insert( "enabled", mEnabled ? "1" : "0" );
  props.insert( "draw_mode", QString::number( int( mDrawMode ) ) );
  props.insert( "blend_mode", QString::number( int( mBlendMode ) ) );
  props.insert( "transparency", QString::number( mTransparency ) );
  props.insert( "blur_level", QString::number( mBlurLevel ) );
  props.insert( "blur_method", QString::number(( int )mBlurMethod ) );
  return props;
}

void QgsBlurEffect::readProperties( const QgsStringMap &props )
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
  int level = props.value( "blur_level" ).toInt( &ok );
  if ( ok )
  {
    mBlurLevel = level;
  }
  QgsBlurEffect::BlurMethod method = ( QgsBlurEffect::BlurMethod )props.value( "blur_method" ).toInt( &ok );
  if ( ok )
  {
    mBlurMethod = method;
  }
}

QgsPaintEffect *QgsBlurEffect::clone() const
{
  QgsBlurEffect* newEffect = new QgsBlurEffect( *this );
  return newEffect;
}

QRectF QgsBlurEffect::boundingRect( const QRectF &rect, const QgsRenderContext& context ) const
{
  Q_UNUSED( context );

  //plus possible extension due to blur, with a couple of extra pixels thrown in for safety
  double spread = mBlurLevel * 2.0 + 10;
  return rect.adjusted( -spread, -spread, spread, spread );
}

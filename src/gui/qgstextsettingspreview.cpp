/***************************************************************************
    qgstextsettingspreview.cpp
    ---------------------
    begin                : May 2010
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstextsettingspreview.h"

#include <QPainter>
#include <QFontMetrics>

QgsTextSettingsPreview::QgsTextSettingsPreview( QWidget* parent )
    : QLabel( parent )
{
  // construct a device-based render context
  QgsMapToPixel newCoordXForm;
  newCoordXForm.setParameters( 1, 0, 0, 0 );
  mContext = new QgsRenderContext();
  mContext->setMapToPixel( newCoordXForm );
  mContext->setScaleFactor( 96 / 25.4 );
  mContext->setUseAdvancedEffects( true );
}

QgsTextSettingsPreview::~QgsTextSettingsPreview()
{
  delete mContext;
}

void QgsTextSettingsPreview::paintEvent( QPaintEvent *e )
{
  Q_UNUSED( e );
  QPainter p( this );

  p.setRenderHint( QPainter::Antialiasing );
  QFontMetrics fm( mTextSettings.textFont );

  // otherwise thin buffers don't look like those on canvas
  if ( mTextSettings.bufferSize != 0 && mTextSettings.bufferSize < 1 )
  {
    mTextSettings.bufferSize = 1;
  }

  //just assume everything is in mm for now
  double xtrans = 2;
  double ytrans = 2;
  if ( mTextSettings.bufferSize != 0 )
  {
    xtrans += mTextSettings.bufferSize * 96 / 25.4;
    ytrans += mTextSettings.bufferSize * 96 / 25.4;
  }
  if ( mTextSettings.shapeDraw && mTextSettings.shapeSizeType == QgsTextRendererSettings::SizeBuffer )
  {
    xtrans += mTextSettings.shapeSize.x() * 96 / 25.4;
    ytrans += mTextSettings.shapeSize.y() * 96 / 25.4;
  }
  if ( mTextSettings.shapeDraw && mTextSettings.shapeBorderWidth > 0 )
  {
    xtrans += 0.5 * mTextSettings.shapeBorderWidth * 96 / 25.4;
    ytrans += 0.5 * mTextSettings.shapeBorderWidth * 96 / 25.4;
  }

  QRectF textRect;
  switch ( mTextSettings.multilineAlign )
  {
    case QgsTextRendererSettings::MultiLeft:
      textRect = QRectF( xtrans, ytrans, width() - xtrans, height() );
      break;
    case QgsTextRendererSettings::MultiCenter:
      textRect = QRectF( 0, ytrans, width(), height() );
      break;
    case QgsTextRendererSettings::MultiRight:
      textRect = QRectF( 0, ytrans, width() - xtrans, height() );
      break;
  }

  mContext->setPainter( &p );

  QgsTextRenderer::drawText( textRect, 0, text(), *mContext, mTextSettings );
}

void QgsTextSettingsPreview::setTextRendererSettings( const QgsTextRendererSettings &textSettings )
{
  mTextSettings = QgsTextRendererSettings( textSettings );
  update();
}

void QgsTextSettingsPreview::setMapUnitScale( const double scale )
{
  QgsMapToPixel newCoordXForm;
  newCoordXForm.setParameters( scale, 0, 0, 0 );
  mContext->setMapToPixel( newCoordXForm );
  update();
}

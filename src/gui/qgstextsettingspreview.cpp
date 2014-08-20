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

  double xtrans = 0;
  if ( mTextSettings.bufferSize != 0 )
  {
    xtrans = mTextSettings.bufferSize / 4;
  }

  mContext->setPainter( &p );

  QgsTextRenderer::drawText( QRectF( xtrans, fm.ascent() + 4, width() - xtrans, height()), 0, text(), *mContext, mTextSettings );
}

void QgsTextSettingsPreview::setTextRendererSettings(const QgsTextRendererSettings &textSettings)
{
    mTextSettings = QgsTextRendererSettings( textSettings );
    update();
}

void QgsTextSettingsPreview::setMapUnitScale(const double scale)
{
    QgsMapToPixel newCoordXForm;
    newCoordXForm.setParameters( scale, 0, 0, 0 );
    mContext->setMapToPixel( newCoordXForm );
    update();
}

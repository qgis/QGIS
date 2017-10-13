/***************************************************************************
    qgstextpreview.cpp
    ------------------
    begin                : October 2016
    copyright            : (C) 2016 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstextpreview.h"
#include <QDesktopWidget>
#include <QPainter>

QgsTextPreview::QgsTextPreview( QWidget *parent )
  : QLabel( parent )
{
  // initially use a basic transform with no scale
  QgsMapToPixel newCoordXForm;
  newCoordXForm.setParameters( 1, 0, 0, 0, 0, 0 );
  mContext.setMapToPixel( newCoordXForm );

  mContext.setScaleFactor( QgsApplication::desktop()->logicalDpiX() / 25.4 );
  mContext.setUseAdvancedEffects( true );
}


void QgsTextPreview::paintEvent( QPaintEvent *e )
{
  Q_UNUSED( e );
  QPainter p( this );

  p.setRenderHint( QPainter::Antialiasing );

  // slightly inset text
  double xtrans = 0;
  if ( mFormat.buffer().enabled() )
    xtrans = mContext.convertToPainterUnits( mFormat.buffer().size(), mFormat.buffer().sizeUnit(), mFormat.buffer().sizeMapUnitScale() );
  if ( mFormat.background().enabled() && mFormat.background().sizeType() != QgsTextBackgroundSettings::SizeFixed )
    xtrans = std::max( xtrans, mContext.convertToPainterUnits( mFormat.background().size().width(), mFormat.background().sizeUnit(), mFormat.background().sizeMapUnitScale() ) );
  xtrans += 4;

  double ytrans = 0.0;
  if ( mFormat.buffer().enabled() )
    ytrans = std::max( ytrans, mContext.convertToPainterUnits( mFormat.buffer().size(), mFormat.buffer().sizeUnit(), mFormat.buffer().sizeMapUnitScale() ) );
  if ( mFormat.background().enabled() )
    ytrans = std::max( ytrans, mContext.convertToPainterUnits( mFormat.background().size().height(), mFormat.background().sizeUnit(), mFormat.background().sizeMapUnitScale() ) );
  ytrans += 4;

  QRectF textRect = rect();
  textRect.setLeft( xtrans );
  textRect.setWidth( textRect.width() - xtrans );
  textRect.setTop( ytrans );
  if ( textRect.height() > 300 )
    textRect.setHeight( 300 );
  if ( textRect.width() > 2000 )
    textRect.setWidth( 2000 );

  mContext.setPainter( &p );
  QgsTextRenderer::drawText( textRect, 0, QgsTextRenderer::AlignLeft, QStringList() << text(),
                             mContext, mFormat );
}

void QgsTextPreview::setFormat( const QgsTextFormat &format )
{
  mFormat = format;
  update();
}

void QgsTextPreview::updateContext()
{
  if ( mScale >= 0 )
  {
    QgsMapToPixel newCoordXForm = QgsMapToPixel::fromScale( mScale, mMapUnits, QgsApplication::desktop()->logicalDpiX() );
    mContext.setMapToPixel( newCoordXForm );
  }
  update();
}

void QgsTextPreview::setScale( double scale )
{
  mScale = scale;
  updateContext();
}

void QgsTextPreview::setMapUnits( QgsUnitTypes::DistanceUnit unit )
{
  mMapUnits = unit;
  updateContext();
}

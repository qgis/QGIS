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
#include "qgstextrenderer.h"
#include "qgsscreenhelper.h"

#include <QPainter>

QgsTextPreview::QgsTextPreview( QWidget *parent )
  : QLabel( parent )
{
  mScreenHelper = new QgsScreenHelper( this );
  connect( mScreenHelper, &QgsScreenHelper::screenDpiChanged, this, [ = ]( double dpi )
  {
    mContext.setScaleFactor( dpi / 25.4 );
    updateContext();
  } );

  // initially use a basic transform with no scale
  QgsMapToPixel newCoordXForm;
  newCoordXForm.setParameters( 1, 0, 0, 0, 0, 0 );
  mContext.setMapToPixel( newCoordXForm );

  mContext.setScaleFactor( mScreenHelper->screenDpi() / 25.4 );
  mContext.setUseAdvancedEffects( true );

  mContext.setFlag( Qgis::RenderContextFlag::Antialiasing, true );

  mContext.setIsGuiPreview( true );
}

void QgsTextPreview::paintEvent( QPaintEvent *e )
{
  Q_UNUSED( e )
  QPainter p( this );

  p.setRenderHint( QPainter::Antialiasing );

  // slightly inset text
  const double fontSize = mContext.convertToPainterUnits( mFormat.size(), mFormat.sizeUnit(), mFormat.sizeMapUnitScale() );
  double xtrans = 0;
  if ( mFormat.buffer().enabled() )
    xtrans = mFormat.buffer().sizeUnit() == QgsUnitTypes::RenderPercentage
             ? fontSize * mFormat.buffer().size() / 100
             : mContext.convertToPainterUnits( mFormat.buffer().size(), mFormat.buffer().sizeUnit(), mFormat.buffer().sizeMapUnitScale() );
  if ( mFormat.background().enabled() && mFormat.background().sizeType() != QgsTextBackgroundSettings::SizeFixed )
    xtrans = std::max( xtrans, mContext.convertToPainterUnits( mFormat.background().size().width(), mFormat.background().sizeUnit(), mFormat.background().sizeMapUnitScale() ) );
  xtrans += 4;

  double ytrans = 0.0;
  if ( mFormat.buffer().enabled() )
    ytrans = std::max( ytrans, mFormat.buffer().sizeUnit() == QgsUnitTypes::RenderPercentage
                       ? fontSize * mFormat.buffer().size() / 100
                       : mContext.convertToPainterUnits( mFormat.buffer().size(), mFormat.buffer().sizeUnit(), mFormat.buffer().sizeMapUnitScale() ) );
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
  QgsTextRenderer::drawText( textRect, 0, Qgis::TextHorizontalAlignment::Left, QStringList() << text(),
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
    const QgsMapToPixel newCoordXForm = QgsMapToPixel::fromScale( mScale, mMapUnits, mScreenHelper->screenDpi() );
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

/***************************************************************************
    qgslabelpreview.cpp
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
#include "qgslabelpreview.h"

#include <QPainter>
#include <QFontMetrics>

#include "qgspallabeling.h"

QgsLabelPreview::QgsLabelPreview( QWidget* parent )
    : QLabel( parent )
{
  // construct a device-based render context
  QgsMapToPixel newCoordXForm( 1 );
  mContext.setMapToPixel( newCoordXForm );
}

void QgsLabelPreview::setTextColor( const QColor& color )
{
  mTextColor = color;
  update();
}

void QgsLabelPreview::setBuffer( double size, const QColor& color, Qt::PenJoinStyle joinStyle, bool noFill )
{
  QgsTextBufferSettings buffer = mFormat.buffer();
  buffer.setSize( size * 88 / 25.4 ); //assume standard dpi for preview;
  buffer.setSizeUnit( QgsUnitTypes::RenderMillimeters );
  buffer.setColor( color );
  buffer.setJoinStyle( joinStyle );
  buffer.setFillBufferInterior( !noFill );
  mFormat.setBuffer( buffer );

  mFormat.setFont( font() );
  update();
}

void QgsLabelPreview::paintEvent( QPaintEvent *e )
{
  Q_UNUSED( e );
  QPainter p( this );

  // TODO: draw all label components when this preview is an actual map canvas
  // for now, only preview label's text and buffer
  mFormat.shadow().setEnabled( false );

  p.setRenderHint( QPainter::Antialiasing );
  p.setFont( font() );
  QFontMetrics fm( font() );

  // otherwise thin buffers don't look like those on canvas
  if ( mFormat.buffer().size() != 0 && mFormat.buffer().size() < 1 )
    mFormat.buffer().setSize( 1 );

  double xtrans = 0;
  if ( mFormat.buffer().size() != 0 )
    xtrans = mFormat.buffer().size() / 4;

  p.translate( xtrans, fm.ascent() + 4 );

  if ( mFormat.buffer().size() != 0 )
  {
    mContext.setPainter( &p );
    QgsTextRenderer::Component component;
    component.text = text();
    QgsTextRenderer::drawBuffer( mContext, component, mFormat );
  }

  QPainterPath path;
  path.addText( 0, 0, font(), text() );
  p.setPen( Qt::NoPen );
  p.setBrush( mTextColor );
  p.drawPath( path );

//  p.setPen( mTextColor );
//  p.drawText( 0, 0, text() );
}

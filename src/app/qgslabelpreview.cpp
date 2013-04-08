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
  mTmpLyr = new QgsPalLayerSettings();

  // construct a device-based render context
  QgsMapToPixel newCoordXForm;
  newCoordXForm.setParameters( 0, 0, 0, 0 );
  mContext = new QgsRenderContext();
  mContext->setMapToPixel( newCoordXForm );
}

QgsLabelPreview::~QgsLabelPreview()
{
  delete mTmpLyr;
  delete mContext;
}

void QgsLabelPreview::setTextColor( QColor color )
{
  mTextColor = color;
  update();
}

void QgsLabelPreview::setBuffer( double size, QColor color, Qt::PenJoinStyle joinStyle, bool noFill )
{
  mTmpLyr->bufferSize = size * 88 / 25.4; //assume standard dpi for preview;
  mTmpLyr->bufferSizeInMapUnits = false;
  mTmpLyr->bufferColor = color;
  mTmpLyr->bufferJoinStyle = joinStyle;
  mTmpLyr->bufferNoFill = noFill;

  mTmpLyr->textFont = font();
  update();
}

void QgsLabelPreview::paintEvent( QPaintEvent *e )
{
  Q_UNUSED( e );
  QPainter p( this );

  p.setRenderHint( QPainter::Antialiasing );
  p.setFont( font() );
  QFontMetrics fm( font() );

  // otherwise thin buffers don't look like those on canvas
  if ( mTmpLyr->bufferSize != 0 && mTmpLyr->bufferSize < 1 )
    mTmpLyr->bufferSize = 1;

  double xtrans = 0;
  if ( mTmpLyr->bufferSize != 0 )
    xtrans = mTmpLyr->bufferSize / 4;

  p.translate( xtrans, fm.ascent() + 4 );

  if ( mTmpLyr->bufferSize != 0 )
  {
    mContext->setPainter( &p );
    QgsPalLabeling::drawLabelBuffer( *mContext, text(), *mTmpLyr );
  }

  QPainterPath path;
  path.addText( 0, 0, font(), text() );
  p.setPen( Qt::NoPen );
  p.setBrush( mTextColor );
  p.drawPath( path );

//  p.setPen( mTextColor );
//  p.drawText( 0, 0, text() );
}

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
}

void QgsLabelPreview::setTextColor( QColor color )
{
  mTextColor = color;
  update();
}

void QgsLabelPreview::setBuffer( double size, QColor color )
{
  mBufferSize = size * 88 / 25.4; //assume standard dpi for preview
  mBufferColor = color;
  update();
}

void QgsLabelPreview::paintEvent( QPaintEvent *e )
{
  Q_UNUSED( e );
  QPainter p( this );

  p.setRenderHint( QPainter::Antialiasing );
  p.setFont( font() );
  QFontMetrics fm( font() );
  p.translate( 0, fm.ascent() + 4 );

//   mBufferColor.setAlpha( 125 );

  if ( mBufferSize != 0 )
    QgsPalLabeling::drawLabelBuffer( &p, text(), font(), mBufferSize, mBufferColor );

  p.setPen( mTextColor );
  p.drawText( 0, 0, text() );
}

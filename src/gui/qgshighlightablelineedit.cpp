/***************************************************************************
    qgshighlightablelineedit.h
     -------------------------
    Date                 : March 2020
    Copyright            : (C) 2020 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgshighlightablelineedit.h"
#include <QPainter>

QgsHighlightableLineEdit::QgsHighlightableLineEdit( QWidget *parent )
  : QgsFilterLineEdit( parent )
{}

void QgsHighlightableLineEdit::paintEvent( QPaintEvent *e )
{
  QgsFilterLineEdit::paintEvent( e );
  if ( mHighlight )
  {
    QPainter p( this );
    const int width = 2;  // width of highlight rectangle inside frame
    p.setPen( QPen( palette().highlight(), width ) );
    const QRect r = rect().adjusted( width, width, -width, -width );
    p.drawRect( r );
  }
}

void QgsHighlightableLineEdit::setHighlighted( bool highlighted )
{
  mHighlight = highlighted;
  update();
}

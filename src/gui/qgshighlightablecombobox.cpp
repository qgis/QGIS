/***************************************************************************
    qgshighlightablecombobox.cpp
     ---------------------------
    Date                 : 20/12/2019
    Copyright            : (C) 2019 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgshighlightablecombobox.h"
#include <QPainter>

QgsHighlightableComboBox::QgsHighlightableComboBox( QWidget *parent )
  : QComboBox( parent )
{}

void QgsHighlightableComboBox::paintEvent( QPaintEvent *e )
{
  QComboBox::paintEvent( e );
  if ( mHighlight )
  {
    QPainter p( this );
    const int width = 2;  // width of highlight rectangle inside frame
    p.setPen( QPen( palette().highlight(), width ) );
    const QRect r = rect().adjusted( width, width, -width, -width );
    p.drawRect( r );
  }
}

void QgsHighlightableComboBox::setHighlighted( bool highlighted )
{
  mHighlight = highlighted;
  update();
}

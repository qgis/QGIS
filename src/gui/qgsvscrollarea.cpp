/***************************************************************************
                              qgsvscrollarea.cpp
                              ------------------------
  begin                : September 2017
  copyright            : (C) 2017 Sandro Mani
  email                : manisandro at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvscrollarea.h"
#include "moc_qgsvscrollarea.cpp"
#include <QEvent>
#include <QScrollBar>

QgsVScrollArea::QgsVScrollArea( QWidget *parent )
  : QgsScrollArea( parent )
{
  setWidgetResizable( true );
  setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  setVerticalScrollBarPolicy( Qt::ScrollBarAsNeeded );
}

bool QgsVScrollArea::eventFilter( QObject *o, QEvent *e )
{
  // This works because QScrollArea::setWidget installs an eventFilter on the widget
  if ( o && o == widget() && e->type() == QEvent::Resize )
    setMinimumWidth( widget()->minimumSizeHint().width() + verticalScrollBar()->width() );
  return QgsScrollArea::eventFilter( o, e );
}

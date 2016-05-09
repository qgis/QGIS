/***************************************************************************
    qgsfocuswatcher.h
    -----------------
    Date                 : April 2016
    Copyright            : (C) 2016 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfocuswatcher.h"
#include <QEvent>

QgsFocusWatcher::QgsFocusWatcher( QObject* parent )
    : QObject( parent )
{
  Q_ASSERT( parent );
  parent->installEventFilter( this );
}

bool QgsFocusWatcher::eventFilter( QObject*, QEvent* event )
{
  switch ( event->type() )
  {
    case QEvent::FocusIn:
      emit focusChanged( true );
      emit focusIn();
      break;
    case QEvent::FocusOut:
      emit focusChanged( false );
      emit focusOut();
      break;
    default:
      break;
  }
  return false;
}

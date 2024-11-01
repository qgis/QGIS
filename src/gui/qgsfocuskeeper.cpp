/***************************************************************************
    qgsfocuskeeper.cpp
     -----------------
    Date                 : May 2020
    Copyright            : (C) 2020 Even Rouault
    Email                : even dot rouault at spatialys dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfocuskeeper.h"
#include "moc_qgsfocuskeeper.cpp"

#include <QApplication>
#include <QEvent>
#include <QWidget>

QgsFocusKeeper::QgsFocusKeeper()
  : mWidgetToKeepFocused( QApplication::focusWidget() )
{
  if ( mWidgetToKeepFocused )
    mWidgetToKeepFocused->installEventFilter( this );
}

QgsFocusKeeper::~QgsFocusKeeper()
{
  if ( mWidgetToKeepFocused )
    mWidgetToKeepFocused->removeEventFilter( this );
}

bool QgsFocusKeeper::eventFilter( QObject *obj, QEvent *event )
{
  if ( obj == mWidgetToKeepFocused && event && ( event->type() == QEvent::FocusOut || event->type() == QEvent::FocusAboutToChange ) )
  {
    return true;
  }
  return QObject::eventFilter( obj, event );
}

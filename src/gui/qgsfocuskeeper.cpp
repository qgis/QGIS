/***************************************************************************
    qgsfocuskeeper.cpp
     -----------------
    Date                 : May 2020
    Copyright            : (C) 2020 Even Rouault
    Email                : even dot rouault at spatialys dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsfocuskeeper.h"

#include <QApplication>
#include <QEvent>
#include <QWidget>

QgsFocusKeeper::QgsFocusKeeper(): mWidgetToKeepFocused( QApplication::focusWidget() )
{
  mWidgetToKeepFocused->installEventFilter( this );
}

QgsFocusKeeper::~QgsFocusKeeper()
{
  mWidgetToKeepFocused->removeEventFilter( this );
}

bool QgsFocusKeeper::eventFilter( QObject *obj, QEvent *event )
{
  if ( obj == mWidgetToKeepFocused && event &&
       ( event->type() == QEvent::FocusOut ||  event->type() == QEvent::FocusAboutToChange ) )
  {
    return true;
  }
  return QObject::eventFilter( obj, event );
}

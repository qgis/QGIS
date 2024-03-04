/***************************************************************************
  qgswidgetstatehelper_p.cpp - QgsWidgetStateHelper

 ---------------------
 begin                : 3.12.2017
 copyright            : (C) 2017 by Nathan Woodrow
 Email                : woodrow.nathan at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgswidgetstatehelper_p.h"
#include <QWindow>
#include <QWidget>
#include <QEvent>
#include <QObject>
#include <QVariant>
#include "qgsguiutils.h"
#include "qgslogger.h"

QgsWidgetStateHelper::QgsWidgetStateHelper( QObject *parent ) : QObject( parent )
{

}

bool QgsWidgetStateHelper::eventFilter( QObject *object, QEvent *event )
{
  if ( event->type() == QEvent::Close ||
       event->type() == QEvent::Destroy ||
       event->type() == QEvent::Hide )
  {
    QWidget *widget = qobject_cast<QWidget *>( object );

    // don't save geometry for windows which were never shown
    if ( widget->property( "widgetStateHelperWasShown" ).toBool() )
    {
      const QString name = widgetSafeName( widget );
      const QString key = mKeys[name];
      QgsGuiUtils::saveGeometry( widget, key );
    }
  }
  else if ( event->type() == QEvent::Show )
  {
    QWidget *widget = qobject_cast<QWidget *>( object );
    const QString name = widgetSafeName( widget );

    // If window is already maximized by Window Manager,
    // there is no need to restore its geometry as it might lead to
    // an incorrect state of QFlags<Qt::WindowState>(WindowMinimized|WindowMaximized)
    // thus minimizing window after it just has been restored by WM.
    // Inability to restore minimized windows has been observed with
    // KWin 5.19 and Qt 5.15 running under X11.
    QWindow *win = widget->windowHandle();
    if ( !win )
      return QObject::eventFilter( object, event );

    if ( !( win->windowStates() & Qt::WindowMaximized ) )
    {
      const QString key = mKeys[name];
      QgsGuiUtils::restoreGeometry( widget, key );
    }

    widget->setProperty( "widgetStateHelperWasShown", QVariant( true ) );
  }
  return QObject::eventFilter( object, event );
}

void QgsWidgetStateHelper::registerWidget( QWidget *widget, const QString &key )
{
  const QString name = widgetSafeName( widget );
  mKeys[name] = key;
  widget->installEventFilter( this );
}

QString QgsWidgetStateHelper::widgetSafeName( QWidget *widget )
{
  if ( widget->objectName().isEmpty() )
  {
    return widget->metaObject()->className();
  }
  return widget->objectName();
}

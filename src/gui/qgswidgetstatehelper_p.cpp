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
      QString name = widgetSafeName( widget );
      QString key = mKeys[name];
      QgsGuiUtils::saveGeometry( widget, key );
    }
  }
  else if ( event->type() == QEvent::Show )
  {
    QWidget *widget = qobject_cast<QWidget *>( object );
    QString name = widgetSafeName( widget );
    QString key = mKeys[name];
    QgsGuiUtils::restoreGeometry( widget, key );
    widget->setProperty( "widgetStateHelperWasShown", QVariant( true ) );
  }
  return QObject::eventFilter( object, event );
}

void QgsWidgetStateHelper::registerWidget( QWidget *widget, const QString &key )
{
  QString name = widgetSafeName( widget );
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

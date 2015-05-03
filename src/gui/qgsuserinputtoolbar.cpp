/***************************************************************************
    qgsuserinputtoolbar.h
     --------------------------------------
    Date                 : 04.2015
    Copyright            : (C) 2015 Denis Rouzaud
    Email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsuserinputtoolbar.h"

#include <QAction>


QgsUserInputToolBar::QgsUserInputToolBar( QWidget *parent )
    : QToolBar( tr( "User input tool bar" ), parent )
{
  setAllowedAreas( Qt::BottomToolBarArea | Qt::TopToolBarArea );

  // add spacer to align right
  QWidget* spacer = new QWidget();
  spacer->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
  addWidget( spacer );
}

QgsUserInputToolBar::~QgsUserInputToolBar()
{

}

void QgsUserInputToolBar::addUserInputWidget( QWidget *widget )
{
  QAction* sep = 0;
  if ( mWidgetList.count() > 0 )
  {
    sep = addSeparator();
  }
  addWidget( widget );

  connect( widget, SIGNAL( destroyed( QObject* ) ), this, SLOT( widgetDestroyed( QObject* ) ) );

  mWidgetList.insert( widget, sep );

  show();
}

void QgsUserInputToolBar::widgetDestroyed( QObject *obj )
{
  if ( obj->isWidgetType() )
  {
    QWidget* w = qobject_cast<QWidget*>( obj );
    QMap<QWidget*, QAction*>::iterator i = mWidgetList.find( w );
    while ( i != mWidgetList.end() )
    {
      if ( i.value() )
      {
        i.value()->deleteLater();
      }
      mWidgetList.remove( i.key() );
      ++i;
    }
  }
  if ( mWidgetList.count() == 0 )
  {
    hide();
  }
}

void QgsUserInputToolBar::paintEvent( QPaintEvent * event )
{
  QToolBar::paintEvent( event );
  if ( mWidgetList.count() == 0 )
  {
    hide();
  }
}

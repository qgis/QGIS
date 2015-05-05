/***************************************************************************
    qgsuserinputdockwidget.h
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

#include "qgsuserinputdockwidget.h"

#include <QFrame>
#include <QBoxLayout>

QgsUserInputDockWidget::QgsUserInputDockWidget( QWidget *parent )
    : QDockWidget( tr( "User input" ), parent )
    , mDockArea( Qt::BottomDockWidgetArea )
{
  QWidget* w = new QWidget( this );
  mLayout = new QBoxLayout( QBoxLayout::LeftToRight, this );
  mLayout->setAlignment( Qt::AlignLeft | Qt::AlignTop );
  w->setLayout( mLayout );
  setWidget( w );

  connect( this, SIGNAL( dockLocationChanged( Qt::DockWidgetArea ) ), this, SLOT( areaChanged( Qt::DockWidgetArea ) ) );

  hide();
}

QgsUserInputDockWidget::~QgsUserInputDockWidget()
{
}

void QgsUserInputDockWidget::addUserInputWidget( QWidget *widget )
{
  QFrame* line = 0;
  if ( mWidgetList.count() > 0 )
  {
    line = new QFrame( this );
    line->setFrameShadow( QFrame::Sunken );
    line->setFrameShape( isLayoutHorizontal() ? QFrame::VLine : QFrame::HLine );
    mLayout->addWidget( line );
  }
  mLayout->addWidget( widget );

  connect( widget, SIGNAL( destroyed( QObject* ) ), this, SLOT( widgetDestroyed( QObject* ) ) );

  mWidgetList.insert( widget, line );

  adjustSize();
  show();
}

void QgsUserInputDockWidget::widgetDestroyed( QObject *obj )
{
  if ( obj->isWidgetType() )
  {
    QWidget* w = qobject_cast<QWidget*>( obj );
    QMap<QWidget*, QFrame*>::iterator i = mWidgetList.find( w );
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

void QgsUserInputDockWidget::areaChanged( Qt::DockWidgetArea area )
{
  mDockArea = area;

  mLayout->setDirection( isLayoutHorizontal() ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom );

  QMap<QWidget*, QFrame*>::iterator i = mWidgetList.begin();
  while ( i != mWidgetList.end() )
  {
    if ( i.value() )
    {
      i.value()->setFrameShape( isLayoutHorizontal() ? QFrame::VLine : QFrame::HLine );
    }
    ++i;
  }

  adjustSize();
}

bool QgsUserInputDockWidget::isLayoutHorizontal()
{
  if ( mDockArea & Qt::BottomDockWidgetArea || mDockArea & Qt::TopDockWidgetArea || mDockArea & Qt::NoDockWidgetArea )
  {
    return true;
  }
  else
  {
    return false;
  }
}

void QgsUserInputDockWidget::paintEvent( QPaintEvent * event )
{
  QDockWidget::paintEvent( event );
  if ( mWidgetList.count() == 0 )
  {
    hide();
  }
}

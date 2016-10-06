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
    : QgsDockWidget( tr( "User Input Panel" ), parent )
    , mLayoutHorizontal( true )
{
  QWidget* w = new QWidget( nullptr );
  mLayout = new QBoxLayout( QBoxLayout::LeftToRight );
  mLayout->setAlignment( Qt::AlignLeft | Qt::AlignTop );
  w->setLayout( mLayout );
  setWidget( w );

  connect( this, SIGNAL( dockLocationChanged( Qt::DockWidgetArea ) ), this, SLOT( areaChanged( Qt::DockWidgetArea ) ) );
  connect( this, SIGNAL( topLevelChanged( bool ) ), this, SLOT( floatingChanged( bool ) ) );
  hide();
}

QgsUserInputDockWidget::~QgsUserInputDockWidget()
{
}

void QgsUserInputDockWidget::addUserInputWidget( QWidget *widget )
{
  QFrame* line = nullptr;
  if ( mWidgetList.count() > 0 )
  {
    line = new QFrame( this );
    line->setFrameShadow( QFrame::Sunken );
    line->setFrameShape( mLayoutHorizontal ? QFrame::VLine : QFrame::HLine );
    mLayout->addWidget( line );
  }
  mLayout->addWidget( widget );

  connect( widget, SIGNAL( destroyed( QObject* ) ), this, SLOT( widgetDestroyed( QObject* ) ) );

  mWidgetList.insert( widget, line );

  show();
  adjustSize();
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
      i = mWidgetList.erase( i );
    }
  }
}

void QgsUserInputDockWidget::areaChanged( Qt::DockWidgetArea area )
{
  bool newLayoutHorizontal = area & Qt::BottomDockWidgetArea || area & Qt::TopDockWidgetArea;
  if ( mLayoutHorizontal == newLayoutHorizontal )
  {
    // no change
    adjustSize();
    return;
  }
  mLayoutHorizontal = newLayoutHorizontal;
  updateLayoutDirection();
}

void QgsUserInputDockWidget::floatingChanged( bool floating )
{
  if ( mLayoutHorizontal == floating )
  {
    adjustSize();
    return;
  }
  mLayoutHorizontal = floating;
  updateLayoutDirection();
}

void QgsUserInputDockWidget::updateLayoutDirection()
{
  mLayout->setDirection( mLayoutHorizontal ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom );

  QMap<QWidget*, QFrame*>::const_iterator i = mWidgetList.constBegin();
  while ( i != mWidgetList.constEnd() )
  {
    if ( i.value() )
    {
      i.value()->setFrameShape( mLayoutHorizontal ? QFrame::VLine : QFrame::HLine );
    }
    ++i;
  }

  adjustSize();
}

void QgsUserInputDockWidget::paintEvent( QPaintEvent * event )
{
  if ( mWidgetList.count() == 0 )
  {
    hide();
  }
  else
  {
    QgsDockWidget::paintEvent( event );
  }
}

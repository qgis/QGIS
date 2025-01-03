/***************************************************************************
                             qgsdockwidget.cpp
                             -----------------
    begin                : June 2016
    copyright            : (C) 2016 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsdockwidget.h"
#include "moc_qgsdockwidget.cpp"
#include <QAction>


QgsDockWidget::QgsDockWidget( QWidget *parent, Qt::WindowFlags flags )
  : QDockWidget( parent, flags )
{
  connect( this, &QDockWidget::visibilityChanged, this, &QgsDockWidget::handleVisibilityChanged );
}

QgsDockWidget::QgsDockWidget( const QString &title, QWidget *parent, Qt::WindowFlags flags )
  : QDockWidget( title, parent, flags )
{
  connect( this, &QDockWidget::visibilityChanged, this, &QgsDockWidget::handleVisibilityChanged );
}

void QgsDockWidget::setUserVisible( bool visible )
{
  if ( visible )
  {
    show();
    raise();
  }
  else
  {
    if ( !mVisibleAndActive )
      return;

    hide();
  }
}

void QgsDockWidget::toggleUserVisible()
{
  setUserVisible( !isUserVisible() );
}

bool QgsDockWidget::isUserVisible() const
{
  return mVisibleAndActive;
}

void QgsDockWidget::setToggleVisibilityAction( QAction *action )
{
  mAction = action;
  if ( !mAction->isCheckable() )
    mAction->setCheckable( true );
  mAction->setChecked( isUserVisible() );
  connect( mAction, &QAction::toggled, this, [=]( bool visible ) {
    setUserVisible( visible );
  } );
  connect( this, &QgsDockWidget::visibilityChanged, mAction, [=]( bool visible ) {
    mAction->setChecked( visible );
  } );
}

QAction *QgsDockWidget::toggleVisibilityAction()
{
  return mAction;
}

void QgsDockWidget::closeEvent( QCloseEvent *e )
{
  emit closed();
  emit closedStateChanged( true );
  emit openedStateChanged( false );
  QDockWidget::closeEvent( e );
}

void QgsDockWidget::showEvent( QShowEvent *e )
{
  emit opened();
  emit closedStateChanged( false );
  emit openedStateChanged( true );
  QDockWidget::showEvent( e );
}

void QgsDockWidget::handleVisibilityChanged( bool visible )
{
  mVisibleAndActive = visible;
}

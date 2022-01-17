/***************************************************************************
  qgsdockablewidgethelper.cpp
  --------------------------------------
  Date                 : January 2022
  Copyright            : (C) 2022 by Belgacem Nedjima
  Email                : belgacem dot nedjima at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdockablewidgethelper.h"

#include "qgisapp.h"
#include "qgsdockwidget.h"

#include <QWidget>

QgsDockableWidgetHelper::QgsDockableWidgetHelper( bool isDocked, QWidget *widget )
  : QWidget( nullptr )
  , mWidget( widget )
  , mDialogGeometry( 0, 0, 200, 200 )
  , mDockGeometry( QRect() )
  , mIsDockFloating( true )
  , mDockArea( Qt::RightDockWidgetArea )
{
  mWidget->setParent( this );

  toggleDockMode( isDocked );
}

QgsDockableWidgetHelper::~QgsDockableWidgetHelper()
{
  if ( mDock )
  {
    mDockGeometry = mDock->geometry();
    mIsDockFloating = mDock->isFloating();
    mDockArea = QgisApp::instance()->dockWidgetArea( mDock );

    mDock->setWidget( nullptr );
    QgisApp::instance()->removeDockWidget( mDock );
    delete mDock;
    mDock = nullptr;
  }

  if ( mDialog )
  {
    mDialogGeometry = mDialog->geometry();

    mDialog->layout()->removeWidget( mWidget );
    delete mDialog;
    mDialog = nullptr;
  }
}

void QgsDockableWidgetHelper::setWidget( QWidget *widget )
{
  if ( mDialog )
  {
    mDialog->layout()->removeWidget( mWidget );
  }
  if ( mDock )
  {
    mDock->setWidget( nullptr );
  }

  mWidget = widget;
  toggleDockMode( mIsDocked );
}

bool QgsDockableWidgetHelper::isDocked() const
{
  return mIsDocked;
}

void QgsDockableWidgetHelper::toggleDockMode( bool docked )
{
  mWidget->setParent( this );

  if ( mDock )
  {
    mDockGeometry = mDock->geometry();
    mIsDockFloating = mDock->isFloating();
    mDockArea = QgisApp::instance()->dockWidgetArea( mDock );

    mDock->setWidget( nullptr );
    QgisApp::instance()->removeDockWidget( mDock );
    delete mDock;
    mDock = nullptr;
  }

  if ( mDialog )
  {
    mDialogGeometry = mDialog->geometry();

    mDialog->layout()->removeWidget( mWidget );
    delete mDialog;
    mDialog = nullptr;
  }

  mIsDocked = docked;

  if ( docked )
  {
    // going from window -> dock
    mDock = new QgsDockWidget( QgisApp::instance() );
    mDock->setWindowTitle( this->windowTitle() );
    mDock->setWidget( mWidget );
    setupDockWidget();

    connect( mDock, &QgsDockWidget::closed, [ = ]()
    {
      mDockGeometry = mDock->geometry();
      mIsDockFloating = mDock->isFloating();
      mDockArea = QgisApp::instance()->dockWidgetArea( mDock );
      QgisApp::instance()->close3DMapView( this->windowTitle() );
    } );
  }
  else
  {
    // going from dock -> window
    mDialog = new QDialog( QgisApp::instance(), Qt::Window );

    mDialog->setWindowTitle( this->windowTitle() );
    QVBoxLayout *vl = new QVBoxLayout();
    vl->setContentsMargins( 0, 0, 0, 0 );
    vl->addWidget( mWidget );
    mDialog->setLayout( vl );
    mDialog->raise();
    mDialog->show();

    connect( mDialog, &QDialog::finished, [ = ]()
    {
      mDialogGeometry = mDialog->geometry();
      QgisApp::instance()->close3DMapView( this->windowTitle() );
    } );

    mDialog->setGeometry( mDialogGeometry );
  }
}

void QgsDockableWidgetHelper::setWindowTitle( const QString &title )
{
  QWidget::setWindowTitle( title );
  if ( mDialog )
  {
    mDialog->setWindowTitle( title );
  }
  if ( mDock )
  {
    mDock->setWindowTitle( title );
  }
}

void QgsDockableWidgetHelper::setDialogGeometry( const QRect &geom )
{
  mDialogGeometry = geom;
  if ( !mDialog )
    return;
  mDialog->setGeometry( geom );
}

void QgsDockableWidgetHelper::setDockGeometry( const QRect &geom, bool isFloating, Qt::DockWidgetArea area )
{
  mDockGeometry = geom;
  mIsDockFloating = isFloating;
  mDockArea = area;

  setupDockWidget();
}

QRect QgsDockableWidgetHelper::dialogGeometry() const
{
  if ( mDialog )
    return mDialog->geometry();
  return mDialogGeometry;
}

QRect QgsDockableWidgetHelper::dockGeometry() const
{
  if ( mDock )
    return mDock->geometry();
  return mDockGeometry;
}

bool QgsDockableWidgetHelper::isDockFloating() const
{
  if ( mDock )
    return mDock->isFloating();
  return mIsDockFloating;
}

Qt::DockWidgetArea QgsDockableWidgetHelper::dockFloatingArea() const
{
  if ( mDock )
    return QgisApp::instance()->dockWidgetArea( mDock );
  return mDockArea;
}

void QgsDockableWidgetHelper::closeEvent( QCloseEvent *e )
{
  if ( mDialog )
  {
    mDialog->layout()->removeWidget( mWidget );
  }
  if ( mDock )
  {
    mDock->setWidget( nullptr );
  }
  mWidget->setParent( this );

  emit closed();
  QWidget::closeEvent( e );
}

void QgsDockableWidgetHelper::setupDockWidget()
{
  if ( !mDock )
    return;
  mDock->setFloating( mIsDockFloating );
  if ( mDockGeometry.isEmpty() )
  {
    // try to guess a nice initial placement for view - about 3/4 along, half way down
    mDock->setGeometry( QRect( static_cast< int >( rect().width() * 0.75 ), static_cast< int >( rect().height() * 0.5 ), 400, 400 ) );
    QgisApp::instance()->addDockWidget( mDockArea, mDock );
  }
  else
  {
    if ( !mIsDockFloating )
    {
      // ugly hack, but only way to set dock size correctly for Qt < 5.6
      mDock->setFixedSize( mDockGeometry.size() );
      QgisApp::instance()->addDockWidget( mDockArea, mDock );
      mDock->resize( mDockGeometry.size() );
//      QgsApplication::processEvents(); // required!
      mDock->setFixedSize( QWIDGETSIZE_MAX, QWIDGETSIZE_MAX );
    }
    else
    {
      mDock->setGeometry( mDockGeometry );
      QgisApp::instance()->addDockWidget( mDockArea, mDock );
    }
  }
}

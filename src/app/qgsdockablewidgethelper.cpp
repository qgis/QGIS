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

#include "qgsdockwidget.h"
#include "qgsapplication.h"

#include <QLayout>

QgsDockableWidgetHelper::QgsDockableWidgetHelper( bool isDocked, const QString &windowTitle, QWidget *widget, QMainWindow *ownerWindow )
  : QObject( nullptr )
  , mWidget( widget )
  , mDialogGeometry( 0, 0, 200, 200 )
  , mDockGeometry( QRect() )
  , mIsDockFloating( true )
  , mDockArea( Qt::RightDockWidgetArea )
  , mWindowTitle( windowTitle )
  , mOwnerWindow( ownerWindow )
{
  toggleDockMode( isDocked );
  mToggleButton.setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mDockify.svg" ) ) );
  mToggleButton.setToolTip( tr( "Dock 3D Map View" ) );
  mToggleButton.setCheckable( true );
  mToggleButton.setChecked( isDocked );
  mToggleButton.setEnabled( true );

  connect( &mToggleButton, &QToolButton::toggled, this, &QgsDockableWidgetHelper::toggleDockMode );
}

QgsDockableWidgetHelper::~QgsDockableWidgetHelper()
{
  if ( mDock )
  {
    mDockGeometry = mDock->geometry();
    mIsDockFloating = mDock->isFloating();
    mDockArea = mOwnerWindow->dockWidgetArea( mDock );

    mDock->setWidget( nullptr );
    mOwnerWindow->removeDockWidget( mDock );
    mDock->deleteLater();
    mDock = nullptr;
  }

  if ( mDialog )
  {
    mDialogGeometry = mDialog->geometry();

    mDialog->layout()->removeWidget( mWidget );
    mDialog->deleteLater();
    mDialog = nullptr;
  }
}

void QgsDockableWidgetHelper::setWidget( QWidget *widget )
{
  // Make sure the old mWidget is not stuck as a child of mDialog or mDock
  if ( mWidget )
  {
    mWidget->setParent( mOwnerWindow );
  }
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
  // Make sure the old mWidget is not stuck as a child of mDialog or mDock
  if ( mWidget )
  {
    mWidget->setParent( mOwnerWindow );
  }

  // Remove both the dialog and the dock widget first
  if ( mDock )
  {
    mDockGeometry = mDock->geometry();
    mIsDockFloating = mDock->isFloating();
    mDockArea = mOwnerWindow->dockWidgetArea( mDock );

    mDock->setWidget( nullptr );
    mOwnerWindow->removeDockWidget( mDock );
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

  // If there is no widget set, do not create a dock or a dialog
  if ( mWidget == nullptr )
    return;

  if ( docked )
  {
    // going from window -> dock
    mDock = new QgsDockWidget( mOwnerWindow );
    mDock->setWindowTitle( mWindowTitle );
    mDock->setWidget( mWidget );
    setupDockWidget();

    connect( mDock, &QgsDockWidget::closed, [ = ]()
    {
      mDockGeometry = mDock->geometry();
      mIsDockFloating = mDock->isFloating();
      mDockArea = mOwnerWindow->dockWidgetArea( mDock );
      emit closed();
    } );
  }
  else
  {
    // going from dock -> window
    mDialog = new QDialog( mOwnerWindow, Qt::Window );

    mDialog->setWindowTitle( mWindowTitle );
    QVBoxLayout *vl = new QVBoxLayout();
    vl->setContentsMargins( 0, 0, 0, 0 );
    vl->addWidget( mWidget );
    mDialog->setLayout( vl );
    mDialog->raise();
    mDialog->show();

    connect( mDialog, &QDialog::finished, [ = ]()
    {
      mDialogGeometry = mDialog->geometry();
      emit closed();
    } );

    mDialog->setGeometry( mDialogGeometry );
  }
}

void QgsDockableWidgetHelper::setWindowTitle( const QString &title )
{
  mWindowTitle = title;
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
    return mOwnerWindow->dockWidgetArea( mDock );
  return mDockArea;
}

void QgsDockableWidgetHelper::setupDockWidget()
{
  if ( !mDock )
    return;
  mDock->setFloating( mIsDockFloating );
  if ( mDockGeometry.isEmpty() )
  {
    // try to guess a nice initial placement for view - about 3/4 along, half way down
    mDock->setGeometry( QRect( static_cast< int >( mWidget->rect().width() * 0.75 ), static_cast< int >( mWidget->rect().height() * 0.5 ), 400, 400 ) );
    mOwnerWindow->addDockWidget( mDockArea, mDock );
  }
  else
  {
    if ( !mIsDockFloating )
    {
      // ugly hack, but only way to set dock size correctly for Qt < 5.6
      mDock->setFixedSize( mDockGeometry.size() );
      mOwnerWindow->addDockWidget( mDockArea, mDock );
      mDock->resize( mDockGeometry.size() );
      QgsApplication::processEvents(); // required!
      mDock->setFixedSize( QWIDGETSIZE_MAX, QWIDGETSIZE_MAX );
    }
    else
    {
      mDock->setGeometry( mDockGeometry );
      mOwnerWindow->addDockWidget( mDockArea, mDock );
    }
  }
}

QToolButton *QgsDockableWidgetHelper::toggleButton()
{
  return &mToggleButton;
}

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
  , mDialogGeometry( 0, 0, 0, 0 )
  , mDockGeometry( QRect() )
  , mIsDockFloating( true )
  , mDockArea( Qt::RightDockWidgetArea )
  , mWindowTitle( windowTitle )
  , mOwnerWindow( ownerWindow )
{
  toggleDockMode( isDocked );
}

QgsDockableWidgetHelper::~QgsDockableWidgetHelper()
{
  setWidget( nullptr );
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

void QgsDockableWidgetHelper::writeXml( QDomElement &viewDom )
{
  viewDom.setAttribute( QStringLiteral( "isDocked" ), mIsDocked );

  if ( mDock )
  {
    mDockGeometry = mDock->geometry();
    mIsDockFloating = mDock->isFloating();
    mDockArea = mOwnerWindow->dockWidgetArea( mDock );
  }

  viewDom.setAttribute( QStringLiteral( "x" ), mDockGeometry.x() );
  viewDom.setAttribute( QStringLiteral( "y" ), mDockGeometry.y() );
  viewDom.setAttribute( QStringLiteral( "width" ), mDockGeometry.width() );
  viewDom.setAttribute( QStringLiteral( "height" ), mDockGeometry.height() );
  viewDom.setAttribute( QStringLiteral( "floating" ), mIsDockFloating );
  viewDom.setAttribute( QStringLiteral( "area" ), mDockArea );

  if ( mDialog )
    mDialogGeometry = mDialog->geometry();

  viewDom.setAttribute( QStringLiteral( "d_x" ), mDialogGeometry.x() );
  viewDom.setAttribute( QStringLiteral( "d_y" ), mDialogGeometry.y() );
  viewDom.setAttribute( QStringLiteral( "d_width" ), mDialogGeometry.width() );
  viewDom.setAttribute( QStringLiteral( "d_height" ), mDialogGeometry.height() );
}

void QgsDockableWidgetHelper::readXml( QDomElement &viewDom )
{
  {
    int x = viewDom.attribute( QStringLiteral( "d_x" ), QStringLiteral( "0" ) ).toInt();
    int y = viewDom.attribute( QStringLiteral( "d_x" ), QStringLiteral( "0" ) ).toInt();
    int w = viewDom.attribute( QStringLiteral( "d_width" ), QStringLiteral( "200" ) ).toInt();
    int h = viewDom.attribute( QStringLiteral( "d_height" ), QStringLiteral( "200" ) ).toInt();
    mDialogGeometry = QRect( x, y, w, h );
    if ( mDialog )
      mDialog->setGeometry( mDialogGeometry );
  }

  {
    int x = viewDom.attribute( QStringLiteral( "x" ), QStringLiteral( "0" ) ).toInt();
    int y = viewDom.attribute( QStringLiteral( "y" ), QStringLiteral( "0" ) ).toInt();
    int w = viewDom.attribute( QStringLiteral( "width" ), QStringLiteral( "200" ) ).toInt();
    int h = viewDom.attribute( QStringLiteral( "height" ), QStringLiteral( "200" ) ).toInt();
    mDockGeometry = QRect( x, y, w, h );
    mIsDockFloating = viewDom.attribute( QStringLiteral( "floating" ), QStringLiteral( "0" ) ).toInt();
    mDockArea = static_cast< Qt::DockWidgetArea >( viewDom.attribute( QStringLiteral( "area" ), QString::number( Qt::RightDockWidgetArea ) ).toInt() );
    setupDockWidget();
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
    if ( mDialogGeometry.isEmpty() )
      mDialog->setGeometry( mDockGeometry );
    else
      mDialog->setGeometry( mDialogGeometry );
    mDialog->setLayout( vl );
    mDialog->raise();
    mDialog->show();

    connect( mDialog, &QDialog::finished, [ = ]()
    {
      mDialogGeometry = mDialog->geometry();
      emit closed();
    } );
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

void QgsDockableWidgetHelper::setupDockWidget()
{
  if ( !mDock )
    return;
  mDock->setFloating( mIsDockFloating );
  if ( mDockGeometry.isEmpty() )
  {
    mDockGeometry = QRect( static_cast< int >( mWidget->rect().width() * 0.75 ), static_cast< int >( mWidget->rect().height() * 0.5 ), 400, 400 );
  }
  mOwnerWindow->addDockWidget( mDockArea, mDock );
  QgsApplication::processEvents(); // required to resize properly!
  mDock->setGeometry( mDockGeometry );
}

QToolButton *QgsDockableWidgetHelper::createDockUndockToolButton()
{
  QToolButton *toggleButton = new QToolButton;
  toggleButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mDockify.svg" ) ) );
  toggleButton->setCheckable( true );
  toggleButton->setChecked( mIsDocked );
  toggleButton->setEnabled( true );

  connect( toggleButton, &QToolButton::toggled, this, &QgsDockableWidgetHelper::toggleDockMode );
  return toggleButton;
}

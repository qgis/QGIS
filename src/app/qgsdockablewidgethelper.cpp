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
#include "qgisapp.h"
#include <QLayout>
#include <QAction>

QgsDockableWidgetHelper::QgsDockableWidgetHelper( bool isDocked, const QString &windowTitle, QWidget *widget, QMainWindow *ownerWindow,
    Qt::DockWidgetArea defaultDockArea,
    const QStringList &tabifyWith,
    bool raiseTab, const QString &windowGeometrySettingsKey )
  : QObject( nullptr )
  , mWidget( widget )
  , mDialogGeometry( 0, 0, 0, 0 )
  , mIsDockFloating( defaultDockArea == Qt::DockWidgetArea::NoDockWidgetArea )
  , mDockArea( defaultDockArea == Qt::DockWidgetArea::NoDockWidgetArea ? Qt::DockWidgetArea::RightDockWidgetArea : defaultDockArea )
  , mWindowTitle( windowTitle )
  , mOwnerWindow( ownerWindow )
  , mTabifyWith( tabifyWith )
  , mRaiseTab( raiseTab )
  , mWindowGeometrySettingsKey( windowGeometrySettingsKey )
  , mUuid( QUuid::createUuid().toString() )
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

    if ( !mWindowGeometrySettingsKey.isEmpty() )
    {
      QgsSettings().setValue( mWindowGeometrySettingsKey, mDialog->saveGeometry() );
    }

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
  viewDom.setAttribute( QStringLiteral( "uuid" ), mUuid );

  if ( mDock )
  {
    const QList<QDockWidget * > tabSiblings = mOwnerWindow->tabifiedDockWidgets( mDock );
    QDomElement tabSiblingsElement = viewDom.ownerDocument().createElement( QStringLiteral( "tab_siblings" ) );
    for ( QDockWidget *dock : tabSiblings )
    {
      QDomElement siblingElement = viewDom.ownerDocument().createElement( QStringLiteral( "sibling" ) );
      siblingElement.setAttribute( QStringLiteral( "uuid" ), dock->property( "dock_uuid" ).toString() );
      siblingElement.setAttribute( QStringLiteral( "object_name" ), dock->objectName() );
      tabSiblingsElement.appendChild( siblingElement );
    }
    viewDom.appendChild( tabSiblingsElement );
  }

  if ( mDialog )
    mDialogGeometry = mDialog->geometry();

  viewDom.setAttribute( QStringLiteral( "d_x" ), mDialogGeometry.x() );
  viewDom.setAttribute( QStringLiteral( "d_y" ), mDialogGeometry.y() );
  viewDom.setAttribute( QStringLiteral( "d_width" ), mDialogGeometry.width() );
  viewDom.setAttribute( QStringLiteral( "d_height" ), mDialogGeometry.height() );
}

void QgsDockableWidgetHelper::readXml( const QDomElement &viewDom )
{
  mUuid = viewDom.attribute( QStringLiteral( "uuid" ), mUuid );

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

    QStringList tabSiblings;
    const QDomElement tabSiblingsElement = viewDom.firstChildElement( QStringLiteral( "tab_siblings" ) );
    const QDomNodeList tabSiblingNodes = tabSiblingsElement.childNodes();
    for ( int i = 0; i < tabSiblingNodes.size(); ++i )
    {
      const QDomElement tabSiblingElement = tabSiblingNodes.at( i ).toElement();
      // prefer uuid if set, as it's always unique
      QString tabId = tabSiblingElement.attribute( QStringLiteral( "uuid" ) );
      if ( tabId.isEmpty() )
        tabId = tabSiblingElement.attribute( QStringLiteral( "object_name" ) );
      if ( !tabId.isEmpty() )
        tabSiblings.append( tabId );
    }

    setupDockWidget( tabSiblings );
  }

  if ( mDock )
  {
    mDock->setProperty( "dock_uuid", mUuid );
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
    // going from window -> dock, so save current window geometry
    if ( !mWindowGeometrySettingsKey.isEmpty() )
      QgsSettings().setValue( mWindowGeometrySettingsKey, mDialog->saveGeometry() );

    mDialogGeometry = mDialog->geometry();

    if ( mWidget )
      mDialog->layout()->removeWidget( mWidget );

    delete mDialog;
    mDialog = nullptr;
  }

  mIsDocked = docked;

  // If there is no widget set, do not create a dock or a dialog
  if ( !mWidget )
    return;

  if ( docked )
  {
    // going from window -> dock
    mDock = new QgsDockWidget( mOwnerWindow );
    mDock->setWindowTitle( mWindowTitle );
    mDock->setWidget( mWidget );
    mDock->setProperty( "dock_uuid", mUuid );
    setupDockWidget();

    connect( mDock, &QgsDockWidget::closed, this, [ = ]()
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
    // note -- we explicitly DO NOT set the parent for the dialog, as we want these treated as
    // proper top level windows and have their own taskbar entries. See https://github.com/qgis/QGIS/issues/49286
    mDialog = new QDialog( nullptr, Qt::Window );
    mDialog->setStyleSheet( QgisApp::instance()->styleSheet() );

    mDialog->setWindowTitle( mWindowTitle );
    QVBoxLayout *vl = new QVBoxLayout();
    vl->setContentsMargins( 0, 0, 0, 0 );
    vl->addWidget( mWidget );

    if ( !mWindowGeometrySettingsKey.isEmpty() )
    {
      QgsSettings settings;
      mDialog->restoreGeometry( settings.value( mWindowGeometrySettingsKey ).toByteArray() );
    }
    else
    {
      if ( !mDockGeometry.isEmpty() )
        mDialog->setGeometry( mDockGeometry );
      else if ( !mDialogGeometry.isEmpty() )
        mDialog->setGeometry( mDialogGeometry );
    }
    mDialog->setLayout( vl );
    mDialog->raise();
    mDialog->show();

    connect( mDialog, &QDialog::finished, this, [ = ]()
    {
      mDialogGeometry = mDialog->geometry();
      emit closed();
    } );
  }
  emit dockModeToggled( docked );
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

void QgsDockableWidgetHelper::setupDockWidget( const QStringList &tabSiblings )
{
  if ( !mDock )
    return;

  mDock->setFloating( mIsDockFloating );
  if ( mDockGeometry.isEmpty() )
  {
    const QFontMetrics fm( mOwnerWindow->font() );
    const int initialDockSize = fm.horizontalAdvance( '0' ) * 50;
    mDockGeometry = QRect( static_cast< int >( mOwnerWindow->rect().width() * 0.75 ),
                           static_cast< int >( mOwnerWindow->rect().height() * 0.5 ),
                           initialDockSize, initialDockSize );
  }
  if ( !tabSiblings.isEmpty() )
  {
    QgisApp::instance()->addTabifiedDockWidget( mDockArea, mDock, tabSiblings, false );
  }
  else if ( mRaiseTab )
  {
    QgisApp::instance()->addTabifiedDockWidget( mDockArea, mDock, mTabifyWith, mRaiseTab );
  }
  else
  {
    mOwnerWindow->addDockWidget( mDockArea, mDock );
  }
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

QAction *QgsDockableWidgetHelper::createDockUndockAction( const QString &title, QWidget *parent )
{
  QAction *toggleAction = new QAction( title, parent );
  toggleAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mDockify.svg" ) ) );
  toggleAction->setCheckable( true );
  toggleAction->setChecked( mIsDocked );
  toggleAction->setEnabled( true );

  connect( toggleAction, &QAction::toggled, this, &QgsDockableWidgetHelper::toggleDockMode );
  return toggleAction;
}

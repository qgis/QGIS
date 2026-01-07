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

#include "qgsapplication.h"
#include "qgsdockwidget.h"

#include <QAction>
#include <QLayout>
#include <QUuid>

#include "moc_qgsdockablewidgethelper.cpp"

///@cond PRIVATE

const QgsSettingsEntryBool *QgsDockableWidgetHelper::sSettingsIsDocked = new QgsSettingsEntryBool( u"is-docked"_s, QgsDockableWidgetHelper::sTtreeDockConfigs, false );
const QgsSettingsEntryVariant *QgsDockableWidgetHelper::sSettingsDockGeometry = new QgsSettingsEntryVariant( u"dock-geometry"_s, QgsDockableWidgetHelper::sTtreeDockConfigs );
const QgsSettingsEntryVariant *QgsDockableWidgetHelper::sSettingsDialogGeometry = new QgsSettingsEntryVariant( u"dialog-geometry"_s, QgsDockableWidgetHelper::sTtreeDockConfigs );
const QgsSettingsEntryEnumFlag<Qt::DockWidgetArea> *QgsDockableWidgetHelper::sSettingsDockArea = new QgsSettingsEntryEnumFlag<Qt::DockWidgetArea>( u"dock-area"_s, QgsDockableWidgetHelper::sTtreeDockConfigs, Qt::RightDockWidgetArea );

std::function<void( Qt::DockWidgetArea, QDockWidget *, const QStringList &, bool )> QgsDockableWidgetHelper::sAddTabifiedDockWidgetFunction = []( Qt::DockWidgetArea, QDockWidget *, const QStringList &, bool ) {};
std::function<QString()> QgsDockableWidgetHelper::sAppStylesheetFunction = [] { return QString(); };
QMainWindow *QgsDockableWidgetHelper::sOwnerWindow = nullptr;

QgsDockableWidgetHelper::QgsDockableWidgetHelper( const QString &windowTitle, QWidget *widget, QMainWindow *ownerWindow, const QString &dockId, const QStringList &tabifyWith, OpeningMode openingMode, bool defaultIsDocked, Qt::DockWidgetArea defaultDockArea, Options options )
  : QObject( nullptr )
  , mWidget( widget )
  , mDialogGeometry( 0, 0, 0, 0 )
  , mWindowTitle( windowTitle )
  , mOwnerWindow( ownerWindow )
  , mTabifyWith( tabifyWith )
  , mOptions( options )
  , mUuid( QUuid::createUuid().toString() )
  , mSettingKeyDockId( dockId )
{
  bool isDocked = sSettingsIsDocked->valueWithDefaultOverride( defaultIsDocked, mSettingKeyDockId );
  if ( openingMode == OpeningMode::ForceDocked )
    isDocked = true;
  else if ( openingMode == OpeningMode::ForceDialog )
    isDocked = false;

  mDockArea = sSettingsDockArea->valueWithDefaultOverride( defaultDockArea, mSettingKeyDockId );
  mIsDockFloating = mDockArea == Qt::DockWidgetArea::NoDockWidgetArea;
  toggleDockMode( isDocked );
}

QgsDockableWidgetHelper::~QgsDockableWidgetHelper()
{
  if ( mDock )
  {
    if ( !mSettingKeyDockId.isEmpty() )
      sSettingsDockGeometry->setValue( mDock->saveGeometry(), mSettingKeyDockId );

    if ( mOwnerWindow )
      mOwnerWindow->removeDockWidget( mDock );

    mDock->setWidget( nullptr );
    mWidget->setParent( nullptr );
    // TODO -- potentially "deleteLater" would be safer here, see eg note
    // in QgsElevationProfileWidget destructor
    delete mDock.data();
    mDock = nullptr;
  }

  if ( mDialog )
  {
    mDialogGeometry = mDialog->geometry();

    if ( !mSettingKeyDockId.isEmpty() )
      sSettingsDialogGeometry->setValue( mDialog->saveGeometry(), mSettingKeyDockId );

    mDialog->layout()->removeWidget( mWidget );
    mDialog->deleteLater();
    mDialog = nullptr;
  }
}

void QgsDockableWidgetHelper::writeXml( QDomElement &viewDom )
{
  viewDom.setAttribute( u"isDocked"_s, mIsDocked );

  if ( mDock )
  {
    mDockGeometry = mDock->geometry();
    mIsDockFloating = mDock->isFloating();
    if ( mOwnerWindow )
      mDockArea = mOwnerWindow->dockWidgetArea( mDock );
  }

  viewDom.setAttribute( u"x"_s, mDockGeometry.x() );
  viewDom.setAttribute( u"y"_s, mDockGeometry.y() );
  viewDom.setAttribute( u"width"_s, mDockGeometry.width() );
  viewDom.setAttribute( u"height"_s, mDockGeometry.height() );
  viewDom.setAttribute( u"floating"_s, mIsDockFloating );
  viewDom.setAttribute( u"area"_s, mDockArea );
  viewDom.setAttribute( u"uuid"_s, mUuid );

  if ( mDock )
  {
    const QList<QDockWidget *> tabSiblings = mOwnerWindow ? mOwnerWindow->tabifiedDockWidgets( mDock ) : QList<QDockWidget *>();
    QDomElement tabSiblingsElement = viewDom.ownerDocument().createElement( u"tab_siblings"_s );
    for ( QDockWidget *dock : tabSiblings )
    {
      QDomElement siblingElement = viewDom.ownerDocument().createElement( u"sibling"_s );
      siblingElement.setAttribute( u"uuid"_s, dock->property( "dock_uuid" ).toString() );
      siblingElement.setAttribute( u"object_name"_s, dock->objectName() );
      tabSiblingsElement.appendChild( siblingElement );
    }
    viewDom.appendChild( tabSiblingsElement );
  }

  if ( mDialog )
    mDialogGeometry = mDialog->geometry();

  viewDom.setAttribute( u"d_x"_s, mDialogGeometry.x() );
  viewDom.setAttribute( u"d_y"_s, mDialogGeometry.y() );
  viewDom.setAttribute( u"d_width"_s, mDialogGeometry.width() );
  viewDom.setAttribute( u"d_height"_s, mDialogGeometry.height() );
}

void QgsDockableWidgetHelper::readXml( const QDomElement &viewDom )
{
  mUuid = viewDom.attribute( u"uuid"_s, mUuid );

  {
    int x = viewDom.attribute( u"d_x"_s, u"0"_s ).toInt();
    int y = viewDom.attribute( u"d_x"_s, u"0"_s ).toInt();
    int w = viewDom.attribute( u"d_width"_s, u"200"_s ).toInt();
    int h = viewDom.attribute( u"d_height"_s, u"200"_s ).toInt();
    mDialogGeometry = QRect( x, y, w, h );
    if ( mDialog )
      mDialog->setGeometry( mDialogGeometry );
  }

  {
    int x = viewDom.attribute( u"x"_s, u"0"_s ).toInt();
    int y = viewDom.attribute( u"y"_s, u"0"_s ).toInt();
    int w = viewDom.attribute( u"width"_s, u"200"_s ).toInt();
    int h = viewDom.attribute( u"height"_s, u"200"_s ).toInt();
    mDockGeometry = QRect( x, y, w, h );
    mIsDockFloating = viewDom.attribute( u"floating"_s, u"0"_s ).toInt();
    mDockArea = static_cast<Qt::DockWidgetArea>( viewDom.attribute( u"area"_s, QString::number( Qt::RightDockWidgetArea ) ).toInt() );

    if ( mDockArea == Qt::DockWidgetArea::NoDockWidgetArea && !mIsDockFloating )
    {
      mDockArea = Qt::RightDockWidgetArea;
    }

    QStringList tabSiblings;
    const QDomElement tabSiblingsElement = viewDom.firstChildElement( u"tab_siblings"_s );
    const QDomNodeList tabSiblingNodes = tabSiblingsElement.childNodes();
    for ( int i = 0; i < tabSiblingNodes.size(); ++i )
    {
      const QDomElement tabSiblingElement = tabSiblingNodes.at( i ).toElement();
      // prefer uuid if set, as it's always unique
      QString tabId = tabSiblingElement.attribute( u"uuid"_s );
      if ( tabId.isEmpty() )
        tabId = tabSiblingElement.attribute( u"object_name"_s );
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
  if ( mWidget && mOwnerWindow )
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

QgsDockWidget *QgsDockableWidgetHelper::dockWidget()
{
  return mDock.data();
}

QDialog *QgsDockableWidgetHelper::dialog()
{
  return mDialog.data();
}

void QgsDockableWidgetHelper::toggleDockMode( bool docked )
{
  // Make sure the old mWidget is not stuck as a child of mDialog or mDock
  if ( mWidget && mOwnerWindow )
  {
    mWidget->setParent( mOwnerWindow );
  }

  // Remove both the dialog and the dock widget first
  if ( mDock )
  {
    mDockGeometry = mDock->geometry();
    mIsDockFloating = mDock->isFloating();
    if ( mOwnerWindow )
      mDockArea = mOwnerWindow->dockWidgetArea( mDock );

    mDock->setWidget( nullptr );
    if ( mOwnerWindow )
      mOwnerWindow->removeDockWidget( mDock );
    delete mDock;
    mDock = nullptr;
  }

  if ( mDialog )
  {
    // going from window -> dock, so save current window geometry
    if ( !mSettingKeyDockId.isEmpty() )
      sSettingsDialogGeometry->setValue( mDialog->saveGeometry(), mSettingKeyDockId );

    mDialogGeometry = mDialog->geometry();

    if ( mWidget )
      mDialog->layout()->removeWidget( mWidget );

    delete mDialog;
    mDialog = nullptr;
  }

  mIsDocked = docked;
  if ( !mSettingKeyDockId.isEmpty() )
    sSettingsIsDocked->setValue( mIsDocked, mSettingKeyDockId );

  // If there is no widget set, do not create a dock or a dialog
  if ( !mWidget )
    return;

  if ( docked )
  {
    // going from window -> dock
    mDock = new QgsDockWidget( mOwnerWindow );
    mDock->setWindowTitle( mWindowTitle );
    mDock->setWidget( mWidget );
    mDock->setObjectName( mObjectName );
    mDock->setProperty( "dock_uuid", mUuid );
    setupDockWidget();

    if ( !mSettingKeyDockId.isEmpty() )
    {
      connect( mDock, &QgsDockWidget::dockLocationChanged, this, [this]( Qt::DockWidgetArea area ) {
        sSettingsDockArea->setValue( area, mSettingKeyDockId );
      } );
    }

    connect( mDock, &QgsDockWidget::closed, this, [this]() {
      mDockGeometry = mDock->geometry();
      mIsDockFloating = mDock->isFloating();
      if ( mOwnerWindow )
        mDockArea = mOwnerWindow->dockWidgetArea( mDock );
      emit closed();
    } );

    if ( mOptions.testFlag( Option::PermanentWidget ) )
      mDock->installEventFilter( this );

    connect( mDock, &QgsDockWidget::visibilityChanged, this, &QgsDockableWidgetHelper::visibilityChanged );
    mDock->setUserVisible( true );
    emit visibilityChanged( true );
  }
  else
  {
    // going from dock -> window
    // note -- we explicitly DO NOT set the parent for the dialog, as we want these treated as
    // proper top level windows and have their own taskbar entries. See https://github.com/qgis/QGIS/issues/49286
    if ( mOptions.testFlag( Option::PermanentWidget ) )
      mDialog = new QgsNonRejectableDialog( nullptr, Qt::Window );
    else
      mDialog = new QDialog( nullptr, Qt::Window );
    mDialog->setStyleSheet( sAppStylesheetFunction() );

    mDialog->setWindowTitle( mWindowTitle );
    mDialog->setObjectName( mObjectName );

    if ( mOptions.testFlag( Option::PermanentWidget ) )
      mDialog->installEventFilter( this );

    QVBoxLayout *vl = new QVBoxLayout();
    vl->setContentsMargins( 0, 0, 0, 0 );
    vl->addWidget( mWidget );

    if ( !mSettingKeyDockId.isEmpty() )
    {
      mDialog->restoreGeometry( sSettingsDialogGeometry->value( mSettingKeyDockId ).toByteArray() );
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

    connect( mDialog, &QDialog::finished, this, [this]() {
      mDialogGeometry = mDialog->geometry();
      emit closed();
      emit visibilityChanged( false );
    } );

    emit visibilityChanged( true );
  }
  emit dockModeToggled( docked );
}

void QgsDockableWidgetHelper::setUserVisible( bool visible )
{
  if ( mDialog )
  {
    if ( visible )
    {
      mDialog->show();
      mDialog->raise();
      mDialog->setWindowState( mDialog->windowState() & ~Qt::WindowMinimized );
      mDialog->activateWindow();
    }
    else
    {
      mDialog->hide();
    }
  }
  if ( mDock )
  {
    mDock->setUserVisible( visible );
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

void QgsDockableWidgetHelper::setDockObjectName( const QString &name )
{
  mObjectName = name;
  if ( mDialog )
  {
    mDialog->setObjectName( name );
  }
  if ( mDock )
  {
    mDock->setObjectName( name );
  }
}

QString QgsDockableWidgetHelper::dockObjectName() const { return mObjectName; }

bool QgsDockableWidgetHelper::isUserVisible() const
{
  if ( mDialog )
  {
    return mDialog->isVisible();
  }
  if ( mDock )
  {
    return mDock->isUserVisible();
  }
  return false;
}

void QgsDockableWidgetHelper::setupDockWidget( const QStringList &tabSiblings )
{
  if ( !mDock )
    return;

  mDock->setFloating( mIsDockFloating );
  // default dock geometry
  if ( mDockGeometry.isEmpty() && mOwnerWindow )
  {
    const QFontMetrics fm( mOwnerWindow->font() );
    const int initialDockSize = fm.horizontalAdvance( '0' ) * 75;
    mDockGeometry = QRect( static_cast<int>( mOwnerWindow->rect().width() * 0.75 ), static_cast<int>( mOwnerWindow->rect().height() * 0.5 ), initialDockSize, initialDockSize );
  }
  if ( !tabSiblings.isEmpty() )
  {
    sAddTabifiedDockWidgetFunction( mDockArea, mDock, tabSiblings, false );
  }
  else if ( mOptions.testFlag( Option::RaiseTab ) )
  {
    sAddTabifiedDockWidgetFunction( mDockArea, mDock, mTabifyWith, true );
  }
  else if ( mOwnerWindow )
  {
    mOwnerWindow->addDockWidget( mDockArea, mDock );
  }

  // can only resize properly and set the dock geometry after pending events have been processed,
  // so queue the geometry setting on the end of the event loop
  QMetaObject::invokeMethod( mDock, [this] {
    if (mIsDockFloating && sSettingsDockGeometry->exists( mSettingKeyDockId ) )
        mDock->restoreGeometry( sSettingsDockGeometry->value( mSettingKeyDockId ).toByteArray() );
    else if ( mIsDockFloating )
      mDock->setGeometry( mDockGeometry ); }, Qt::QueuedConnection );
}

QToolButton *QgsDockableWidgetHelper::createDockUndockToolButton()
{
  QToolButton *toggleButton = new QToolButton;
  toggleButton->setIcon( QgsApplication::getThemeIcon( u"mDockify.svg"_s ) );
  toggleButton->setCheckable( true );
  toggleButton->setChecked( mIsDocked );
  toggleButton->setEnabled( true );

  connect( toggleButton, &QToolButton::toggled, this, &QgsDockableWidgetHelper::toggleDockMode );
  return toggleButton;
}

QAction *QgsDockableWidgetHelper::createDockUndockAction( const QString &title, QWidget *parent )
{
  QAction *toggleAction = new QAction( title, parent );
  toggleAction->setIcon( QgsApplication::getThemeIcon( u"mDockify.svg"_s ) );
  toggleAction->setCheckable( true );
  toggleAction->setChecked( mIsDocked );
  toggleAction->setEnabled( true );

  connect( toggleAction, &QAction::toggled, this, &QgsDockableWidgetHelper::toggleDockMode );
  return toggleAction;
}

bool QgsDockableWidgetHelper::eventFilter( QObject *watched, QEvent *event )
{
  if ( watched == mDialog )
  {
    if ( event->type() == QEvent::Close )
    {
      event->ignore();
      mDialog->hide();
      emit visibilityChanged( false );
      return true;
    }
  }
  else if ( watched == mDock )
  {
    if ( event->type() == QEvent::Close )
    {
      event->ignore();
      mDock->hide();
      emit visibilityChanged( false );
      return true;
    }
  }
  return QObject::eventFilter( watched, event );
}

//
// QgsNonRejectableDialog
//

QgsNonRejectableDialog::QgsNonRejectableDialog( QWidget *parent, Qt::WindowFlags f )
  : QDialog( parent, f )
{
}

void QgsNonRejectableDialog::reject()
{
  // swallow rejection -- we don't want this dialog to be closable via escape key
}


///@endcond

/***************************************************************************
                             qgslayoutdesignerdialog.cpp
                             ---------------------------
    begin                : July 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#include "qgslayoutdesignerdialog.h"
#include "qgslayoutitemregistry.h"
#include "qgssettings.h"
#include "qgisapp.h"
#include "qgslogger.h"
#include "qgslayoutview.h"
#include "qgslayoutviewtooladditem.h"
#include "qgslayoutviewtoolpan.h"

QgsAppLayoutDesignerInterface::QgsAppLayoutDesignerInterface( QgsLayoutDesignerDialog *dialog )
  : QgsLayoutDesignerInterface( dialog )
  , mDesigner( dialog )
{}

QgsLayout *QgsAppLayoutDesignerInterface::layout()
{
  return mDesigner->currentLayout();
}

void QgsAppLayoutDesignerInterface::close()
{
  mDesigner->close();
}


QgsLayoutDesignerDialog::QgsLayoutDesignerDialog( QWidget *parent, Qt::WindowFlags flags )
  : QMainWindow( parent, flags )
  , mInterface( new QgsAppLayoutDesignerInterface( this ) )
  , mToolsActionGroup( new QActionGroup( this ) )
{
  QgsSettings settings;
  int size = settings.value( QStringLiteral( "IconSize" ), QGIS_ICON_SIZE ).toInt();
  setIconSize( QSize( size, size ) );
  setStyleSheet( QgisApp::instance()->styleSheet() );

  setupUi( this );
  setWindowTitle( tr( "QGIS Layout Designer" ) );

  setAttribute( Qt::WA_DeleteOnClose );
#if QT_VERSION >= 0x050600
  setDockOptions( dockOptions() | QMainWindow::GroupedDragging ) ;
#endif

  //create layout view
  QGridLayout *viewLayout = new QGridLayout();
  viewLayout->setSpacing( 0 );
  viewLayout->setMargin( 0 );
  viewLayout->setContentsMargins( 0, 0, 0, 0 );
  centralWidget()->layout()->setSpacing( 0 );
  centralWidget()->layout()->setMargin( 0 );
  centralWidget()->layout()->setContentsMargins( 0, 0, 0, 0 );
  mView = new QgsLayoutView();
  //mView->setMapCanvas( mQgis->mapCanvas() );
  mView->setContentsMargins( 0, 0, 0, 0 );
  //mView->setHorizontalRuler( mHorizontalRuler );
  //mView->setVerticalRuler( mVerticalRuler );
  viewLayout->addWidget( mView, 1, 1 );
  //view does not accept focus via tab
  mView->setFocusPolicy( Qt::ClickFocus );
  mViewFrame->setLayout( viewLayout );
  mViewFrame->setContentsMargins( 0, 0, 0, 1 ); // 1 is deliberate!
  mView->setFrameShape( QFrame::NoFrame );

  connect( mActionClose, &QAction::triggered, this, &QWidget::close );

  // populate with initial items...
  QMap< int, QString> types = QgsApplication::layoutItemRegistry()->itemTypes();
  QMap< int, QString>::const_iterator typeIt = types.constBegin();
  for ( ; typeIt != types.constEnd(); ++typeIt )
  {
    itemTypeAdded( typeIt.key(), typeIt.value() );
  }
  //..and listen out for new item types
  connect( QgsApplication::layoutItemRegistry(), &QgsLayoutItemRegistry::typeAdded, this, &QgsLayoutDesignerDialog::itemTypeAdded );

  mAddItemTool = new QgsLayoutViewToolAddItem( mView );
  mPanTool = new QgsLayoutViewToolPan( mView );
  mPanTool->setAction( mActionPan );
  mToolsActionGroup->addAction( mActionPan );
  connect( mActionPan, &QAction::triggered, mPanTool, [ = ] { mView->setTool( mPanTool ); } );

  restoreWindowState();
}

QgsAppLayoutDesignerInterface *QgsLayoutDesignerDialog::iface()
{
  return mInterface;
}

QgsLayout *QgsLayoutDesignerDialog::currentLayout()
{
  return mLayout;
}

void QgsLayoutDesignerDialog::setCurrentLayout( QgsLayout *layout )
{
  mLayout = layout;
  mView->setCurrentLayout( layout );
}

void QgsLayoutDesignerDialog::setIconSizes( int size )
{
  //Set the icon size of for all the toolbars created in the future.
  setIconSize( QSize( size, size ) );

  //Change all current icon sizes.
  QList<QToolBar *> toolbars = findChildren<QToolBar *>();
  Q_FOREACH ( QToolBar *toolbar, toolbars )
  {
    toolbar->setIconSize( QSize( size, size ) );
  }
}

void QgsLayoutDesignerDialog::open()
{
  show();
  activate();
#if 0 // TODO
  zoomFull(); // zoomFull() does not work properly until we have called show()
  if ( mView )
  {
    mView->updateRulers();
  }
#endif
}

void QgsLayoutDesignerDialog::activate()
{
  // bool shown = isVisible();
  show();
  raise();
  setWindowState( windowState() & ~Qt::WindowMinimized );
  activateWindow();

#if 0 // TODO
  if ( !shown )
  {
    on_mActionZoomAll_triggered();
  }
#endif
}

void QgsLayoutDesignerDialog::closeEvent( QCloseEvent * )
{
  emit aboutToClose();
  saveWindowState();
}

void QgsLayoutDesignerDialog::itemTypeAdded( int type, const QString &name )
{
  // update UI for new item type
  QAction *action = new QAction( tr( "Add %1" ).arg( name ), this );
  action->setToolTip( tr( "Adds a new %1 to the layout" ).arg( name ) );
  action->setCheckable( true );
  action->setData( type );
  action->setIcon( QgsApplication::layoutItemRegistry()->itemMetadata( type )->icon() );
  mToolsActionGroup->addAction( action );
  mItemMenu->addAction( action );
  mItemToolbar->addAction( action );
  connect( action, &QAction::triggered, this, [this, type]()
  {
    activateNewItemCreationTool( type );
  } );
}

void QgsLayoutDesignerDialog::saveWindowState()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "LayoutDesigner/geometry" ), saveGeometry() );
  // store the toolbar/dock widget settings using Qt settings API
  settings.setValue( QStringLiteral( "LayoutDesigner/state" ), saveState() );
}

void QgsLayoutDesignerDialog::restoreWindowState()
{
  // restore the toolbar and dock widgets positions using Qt settings API
  QgsSettings settings;

  //TODO - defaults
  if ( !restoreState( settings.value( QStringLiteral( "LayoutDesigner/state" ) /*, QByteArray::fromRawData( ( char * )defaultComposerUIstate, sizeof defaultComposerUIstate ) */ ).toByteArray() ) )
  {
    QgsDebugMsg( "restore of layout UI state failed" );
  }
  // restore window geometry
  if ( !restoreGeometry( settings.value( QStringLiteral( "LayoutDesigner/geometry" ) /*, QByteArray::fromRawData( ( char * )defaultComposerUIgeometry, sizeof defaultComposerUIgeometry ) */ ).toByteArray() ) )
  {
    QgsDebugMsg( "restore of layout UI geometry failed" );
  }
}

void QgsLayoutDesignerDialog::activateNewItemCreationTool( int type )
{
  mAddItemTool->setItemType( type );
  if ( mView )
  {
    mView->setTool( mAddItemTool );
  }
}



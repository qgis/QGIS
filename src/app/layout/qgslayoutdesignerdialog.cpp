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
#include "qgslayout.h"
#include "qgslayoutview.h"
#include "qgslayoutviewtooladditem.h"
#include "qgslayoutviewtoolpan.h"
#include "qgslayoutviewtoolzoom.h"
#include "qgslayoutviewtoolselect.h"
#include "qgsgui.h"
#include "qgslayoutitemguiregistry.h"
#include "qgslayoutruler.h"
#include <QShortcut>
#include <QComboBox>
#include <QLineEdit>
#include <QDesktopWidget>
#include <QSlider>
#include <QLabel>


//add some nice zoom levels for zoom comboboxes
QList<double> QgsLayoutDesignerDialog::sStatusZoomLevelsList { 0.125, 0.25, 0.5, 1.0, 2.0, 4.0, 8.0};
#define FIT_LAYOUT -101
#define FIT_LAYOUT_WIDTH -102

QgsAppLayoutDesignerInterface::QgsAppLayoutDesignerInterface( QgsLayoutDesignerDialog *dialog )
  : QgsLayoutDesignerInterface( dialog )
  , mDesigner( dialog )
{}

QgsLayout *QgsAppLayoutDesignerInterface::layout()
{
  return mDesigner->currentLayout();
}

QgsLayoutView *QgsAppLayoutDesignerInterface::view()
{
  return mDesigner->view();
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

  mHorizontalRuler = new QgsLayoutRuler( nullptr, Qt::Horizontal );
  mVerticalRuler = new QgsLayoutRuler( nullptr, Qt::Vertical );
  mRulerLayoutFix = new QWidget();
  mRulerLayoutFix->setAttribute( Qt::WA_NoMousePropagation );
  mRulerLayoutFix->setBackgroundRole( QPalette::Window );
  mRulerLayoutFix->setFixedSize( mVerticalRuler->rulerSize(), mHorizontalRuler->rulerSize() );
  viewLayout->addWidget( mRulerLayoutFix, 0, 0 );
  viewLayout->addWidget( mHorizontalRuler, 0, 1 );
  viewLayout->addWidget( mVerticalRuler, 1, 0 );

  //initial state of rulers
  bool showRulers = settings.value( QStringLiteral( "LayoutDesigner/showRulers" ), true ).toBool();
  mActionShowRulers->setChecked( showRulers );
  mHorizontalRuler->setVisible( showRulers );
  mVerticalRuler->setVisible( showRulers );
  mRulerLayoutFix->setVisible( showRulers );
  mActionShowRulers->blockSignals( false );
  connect( mActionShowRulers, &QAction::triggered, this, &QgsLayoutDesignerDialog::showRulers );

  mView = new QgsLayoutView();
  //mView->setMapCanvas( mQgis->mapCanvas() );
  mView->setContentsMargins( 0, 0, 0, 0 );
  mView->setHorizontalRuler( mHorizontalRuler );
  mView->setVerticalRuler( mVerticalRuler );
  viewLayout->addWidget( mView, 1, 1 );
  //view does not accept focus via tab
  mView->setFocusPolicy( Qt::ClickFocus );
  mViewFrame->setLayout( viewLayout );
  mViewFrame->setContentsMargins( 0, 0, 0, 1 ); // 1 is deliberate!
  mView->setFrameShape( QFrame::NoFrame );

  connect( mActionClose, &QAction::triggered, this, &QWidget::close );

  // populate with initial items...
  Q_FOREACH ( int type,  QgsGui::layoutItemGuiRegistry()->itemTypes() )
  {
    itemTypeAdded( type );
  }
  //..and listen out for new item types
  connect( QgsGui::layoutItemGuiRegistry(), &QgsLayoutItemGuiRegistry::typeAdded, this, &QgsLayoutDesignerDialog::itemTypeAdded );

  mAddItemTool = new QgsLayoutViewToolAddItem( mView );
  mPanTool = new QgsLayoutViewToolPan( mView );
  mPanTool->setAction( mActionPan );
  mToolsActionGroup->addAction( mActionPan );
  connect( mActionPan, &QAction::triggered, mPanTool, [ = ] { mView->setTool( mPanTool ); } );
  mZoomTool = new QgsLayoutViewToolZoom( mView );
  mZoomTool->setAction( mActionZoomTool );
  mToolsActionGroup->addAction( mActionZoomTool );
  connect( mActionZoomTool, &QAction::triggered, mZoomTool, [ = ] { mView->setTool( mZoomTool ); } );
  mSelectTool = new QgsLayoutViewToolSelect( mView );
  mSelectTool->setAction( mActionSelectMoveItem );
  mToolsActionGroup->addAction( mActionSelectMoveItem );
  connect( mActionSelectMoveItem, &QAction::triggered, mSelectTool, [ = ] { mView->setTool( mSelectTool ); } );

  //Ctrl+= should also trigger zoom in
  QShortcut *ctrlEquals = new QShortcut( QKeySequence( QStringLiteral( "Ctrl+=" ) ), this );
  connect( ctrlEquals, &QShortcut::activated, mActionZoomIn, &QAction::trigger );

  connect( mActionZoomIn, &QAction::triggered, mView, &QgsLayoutView::zoomIn );
  connect( mActionZoomOut, &QAction::triggered, mView, &QgsLayoutView::zoomOut );
  connect( mActionZoomAll, &QAction::triggered, mView, &QgsLayoutView::zoomFull );
  connect( mActionZoomActual, &QAction::triggered, mView, &QgsLayoutView::zoomActual );
  connect( mActionZoomToWidth, &QAction::triggered, mView, &QgsLayoutView::zoomWidth );

  //create status bar labels
  mStatusCursorXLabel = new QLabel( mStatusBar );
  mStatusCursorXLabel->setMinimumWidth( 100 );
  mStatusCursorYLabel = new QLabel( mStatusBar );
  mStatusCursorYLabel->setMinimumWidth( 100 );
  mStatusCursorPageLabel = new QLabel( mStatusBar );
  mStatusCursorPageLabel->setMinimumWidth( 100 );

  mStatusBar->addPermanentWidget( mStatusCursorXLabel );
  mStatusBar->addPermanentWidget( mStatusCursorXLabel );
  mStatusBar->addPermanentWidget( mStatusCursorYLabel );
  mStatusBar->addPermanentWidget( mStatusCursorPageLabel );

  mStatusZoomCombo = new QComboBox();
  mStatusZoomCombo->setEditable( true );
  mStatusZoomCombo->setInsertPolicy( QComboBox::NoInsert );
  mStatusZoomCombo->setCompleter( nullptr );
  mStatusZoomCombo->setMinimumWidth( 100 );
  //zoom combo box accepts decimals in the range 1-9999, with an optional decimal point and "%" sign
  QRegularExpression zoomRx( "\\s*\\d{1,4}(\\.\\d?)?\\s*%?" );
  QValidator *zoomValidator = new QRegularExpressionValidator( zoomRx, mStatusZoomCombo );
  mStatusZoomCombo->lineEdit()->setValidator( zoomValidator );

  Q_FOREACH ( double level, sStatusZoomLevelsList )
  {
    mStatusZoomCombo->insertItem( 0, tr( "%1%" ).arg( level * 100.0, 0, 'f', 1 ), level );
  }
  mStatusZoomCombo->insertItem( 0, tr( "Fit Layout" ), FIT_LAYOUT );
  mStatusZoomCombo->insertItem( 0, tr( "Fit Layout Width" ), FIT_LAYOUT_WIDTH );
  connect( mStatusZoomCombo, static_cast<void ( QComboBox::* )( int )>( &QComboBox::activated ), this, &QgsLayoutDesignerDialog::statusZoomCombo_currentIndexChanged );
  connect( mStatusZoomCombo->lineEdit(), &QLineEdit::returnPressed, this, &QgsLayoutDesignerDialog::statusZoomCombo_zoomEntered );

  mStatusZoomSlider = new QSlider();
  mStatusZoomSlider->setFixedWidth( mStatusZoomCombo->width() );
  mStatusZoomSlider->setOrientation( Qt::Horizontal );
  mStatusZoomSlider->setMinimum( 20 );
  mStatusZoomSlider->setMaximum( 800 );
  connect( mStatusZoomSlider, &QSlider::valueChanged, this, &QgsLayoutDesignerDialog::sliderZoomChanged );

  mStatusZoomCombo->setToolTip( tr( "Zoom level" ) );
  mStatusZoomSlider->setToolTip( tr( "Zoom level" ) );

  mStatusBar->addPermanentWidget( mStatusZoomCombo );
  mStatusBar->addPermanentWidget( mStatusZoomSlider );

  //hide borders from child items in status bar under Windows
  mStatusBar->setStyleSheet( QStringLiteral( "QStatusBar::item {border: none;}" ) );

  mView->setTool( mSelectTool );
  mView->setFocus();
  connect( mView, &QgsLayoutView::zoomLevelChanged, this, &QgsLayoutDesignerDialog::updateStatusZoom );
  connect( mView, &QgsLayoutView::cursorPosChanged, this, &QgsLayoutDesignerDialog::updateStatusCursorPos );
  //also listen out for position updates from the horizontal/vertical rulers
  connect( mHorizontalRuler, &QgsLayoutRuler::cursorPosChanged, this, &QgsLayoutDesignerDialog::updateStatusCursorPos );
  connect( mVerticalRuler, &QgsLayoutRuler::cursorPosChanged, this, &QgsLayoutDesignerDialog::updateStatusCursorPos );

  // Panel and toolbar submenus
  mToolbarMenu->addAction( mLayoutToolbar->toggleViewAction() );
  mToolbarMenu->addAction( mNavigationToolbar->toggleViewAction() );
  mToolbarMenu->addAction( mToolsToolbar->toggleViewAction() );

  connect( mActionToggleFullScreen, &QAction::toggled, this, &QgsLayoutDesignerDialog::toggleFullScreen );

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
  mView->zoomFull(); // zoomFull() does not work properly until we have called show()

#if 0 // TODO

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

void QgsLayoutDesignerDialog::showRulers( bool visible )
{
  //show or hide rulers
  mHorizontalRuler->setVisible( visible );
  mVerticalRuler->setVisible( visible );
  mRulerLayoutFix->setVisible( visible );

  QgsSettings settings;
  settings.setValue( QStringLiteral( "LayoutDesigner/showRulers" ), visible );
}

void QgsLayoutDesignerDialog::closeEvent( QCloseEvent * )
{
  emit aboutToClose();
  saveWindowState();
}

void QgsLayoutDesignerDialog::itemTypeAdded( int type )
{
  QString name = QgsApplication::layoutItemRegistry()->itemMetadata( type )->visibleName();
  QString groupId = QgsGui::layoutItemGuiRegistry()->itemMetadata( type )->groupId();
  QToolButton *groupButton = nullptr;
  QMenu *itemSubmenu = nullptr;
  if ( !groupId.isEmpty() )
  {
    // find existing group toolbutton and submenu, or create new ones if this is the first time the group has been encountered
    const QgsLayoutItemGuiGroup &group = QgsGui::layoutItemGuiRegistry()->itemGroup( groupId );
    QIcon groupIcon = group.icon.isNull() ? QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddBasicShape.svg" ) ) : group.icon;
    QString groupText = tr( "Add %1" ).arg( group.name );
    if ( mItemGroupToolButtons.contains( groupId ) )
    {
      groupButton = mItemGroupToolButtons.value( groupId );
    }
    else
    {
      QToolButton *groupToolButton = new QToolButton( mToolsToolbar );
      groupToolButton->setIcon( groupIcon );
      groupToolButton->setCheckable( true );
      groupToolButton->setPopupMode( QToolButton::InstantPopup );
      groupToolButton->setAutoRaise( true );
      groupToolButton->setToolButtonStyle( Qt::ToolButtonIconOnly );
      groupToolButton->setToolTip( groupText );
      mToolsToolbar->addWidget( groupToolButton );
      mItemGroupToolButtons.insert( groupId, groupToolButton );
      groupButton = groupToolButton;
    }

    if ( mItemGroupSubmenus.contains( groupId ) )
    {
      itemSubmenu = mItemGroupSubmenus.value( groupId );
    }
    else
    {
      QMenu *groupSubmenu = mItemMenu->addMenu( groupText );
      groupSubmenu->setIcon( groupIcon );
      mItemMenu->addMenu( groupSubmenu );
      mItemGroupSubmenus.insert( groupId, groupSubmenu );
      itemSubmenu = groupSubmenu;
    }
  }

  // update UI for new item type
  QAction *action = new QAction( tr( "Add %1" ).arg( name ), this );
  action->setToolTip( tr( "Adds a new %1 to the layout" ).arg( name ) );
  action->setCheckable( true );
  action->setData( type );
  action->setIcon( QgsGui::layoutItemGuiRegistry()->itemMetadata( type )->creationIcon() );

  mToolsActionGroup->addAction( action );
  if ( itemSubmenu )
    itemSubmenu->addAction( action );
  else
    mItemMenu->addAction( action );

  if ( groupButton )
    groupButton->addAction( action );
  else
    mToolsToolbar->addAction( action );

  connect( action, &QAction::triggered, this, [this, type]()
  {
    activateNewItemCreationTool( type );
  } );
}

void QgsLayoutDesignerDialog::statusZoomCombo_currentIndexChanged( int index )
{
  QVariant data = mStatusZoomCombo->itemData( index );
  if ( data.toInt() == FIT_LAYOUT )
  {
    mView->zoomFull();
  }
  else if ( data.toInt() == FIT_LAYOUT_WIDTH )
  {
    mView->zoomWidth();
  }
  else
  {
    double selectedZoom = data.toDouble();
    if ( mView )
    {
      mView->setZoomLevel( selectedZoom );
      //update zoom combobox text for correct format (one decimal place, trailing % sign)
      whileBlocking( mStatusZoomCombo )->lineEdit()->setText( tr( "%1%" ).arg( selectedZoom * 100.0, 0, 'f', 1 ) );
    }
  }
}

void QgsLayoutDesignerDialog::statusZoomCombo_zoomEntered()
{
  if ( !mView )
  {
    return;
  }

  //need to remove spaces and "%" characters from input text
  QString zoom = mStatusZoomCombo->currentText().remove( QChar( '%' ) ).trimmed();
  mView->setZoomLevel( zoom.toDouble() / 100 );
}

void QgsLayoutDesignerDialog::sliderZoomChanged( int value )
{
  mView->setZoomLevel( value / 100.0 );
}

void QgsLayoutDesignerDialog::updateStatusZoom()
{
  double dpi = QgsApplication::desktop()->logicalDpiX();
  //monitor dpi is not always correct - so make sure the value is sane
  if ( ( dpi < 60 ) || ( dpi > 1200 ) )
    dpi = 72;

  //pixel width for 1mm on screen
  double scale100 = dpi / 25.4;
  //current zoomLevel
  double zoomLevel = mView->transform().m11() * 100 / scale100;

  whileBlocking( mStatusZoomCombo )->lineEdit()->setText( tr( "%1%" ).arg( zoomLevel, 0, 'f', 1 ) );
  whileBlocking( mStatusZoomSlider )->setValue( zoomLevel );
}

void QgsLayoutDesignerDialog::updateStatusCursorPos( QPointF position )
{
  if ( !mView->currentLayout() )
  {
    return;
  }

  //convert cursor position to position on current page
#if 0 // TODO
  QPointF pagePosition = mView->currentLayout()->positionOnPage( cursorPosition );
  int currentPage = mView->currentLayout()->pageNumberForPoint( cursorPosition );
#endif
  QPointF pagePosition = position;
  int currentPage = 1;

  mStatusCursorXLabel->setText( QString( tr( "x: %1 mm" ) ).arg( pagePosition.x() ) );
  mStatusCursorYLabel->setText( QString( tr( "y: %1 mm" ) ).arg( pagePosition.y() ) );
  mStatusCursorPageLabel->setText( QString( tr( "page: %1" ) ).arg( currentPage ) );
}

void QgsLayoutDesignerDialog::toggleFullScreen( bool enabled )
{
  if ( enabled )
  {
    showFullScreen();
  }
  else
  {
    showNormal();
  }
}

QgsLayoutView *QgsLayoutDesignerDialog::view()
{
  return mView;
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



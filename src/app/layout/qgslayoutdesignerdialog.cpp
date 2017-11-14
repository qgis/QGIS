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
#include "qgslayoutappmenuprovider.h"
#include "qgslayoutview.h"
#include "qgslayoutviewtooladditem.h"
#include "qgslayoutviewtooladdnodeitem.h"
#include "qgslayoutviewtoolpan.h"
#include "qgslayoutviewtoolmoveitemcontent.h"
#include "qgslayoutviewtoolzoom.h"
#include "qgslayoutviewtoolselect.h"
#include "qgslayoutviewtooleditnodes.h"
#include "qgslayoutitemwidget.h"
#include "qgsgui.h"
#include "qgslayoutitemguiregistry.h"
#include "qgslayoutpropertieswidget.h"
#include "qgslayoutruler.h"
#include "qgslayoutaddpagesdialog.h"
#include "qgspanelwidgetstack.h"
#include "qgspanelwidget.h"
#include "qgsdockwidget.h"
#include "qgslayoutpagepropertieswidget.h"
#include "qgslayoutguidewidget.h"
#include "qgslayoutmousehandles.h"
#include "qgslayoutmodel.h"
#include "qgslayoutitemslistview.h"
#include <QShortcut>
#include <QComboBox>
#include <QLineEdit>
#include <QDesktopWidget>
#include <QSlider>
#include <QLabel>
#include <QUndoView>
#include <QTreeView>

#ifdef ENABLE_MODELTEST
#include "modeltest.h"
#endif

//add some nice zoom levels for zoom comboboxes
QList<double> QgsLayoutDesignerDialog::sStatusZoomLevelsList { 0.125, 0.25, 0.5, 1.0, 2.0, 4.0, 8.0};
#define FIT_LAYOUT -101
#define FIT_LAYOUT_WIDTH -102

bool QgsLayoutDesignerDialog::sInitializedRegistry = false;

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
  if ( !sInitializedRegistry )
  {
    initializeRegistry();
  }
  QgsSettings settings;
  int size = settings.value( QStringLiteral( "IconSize" ), QGIS_ICON_SIZE ).toInt();
  setIconSize( QSize( size, size ) );
  setStyleSheet( QgisApp::instance()->styleSheet() );

  setupUi( this );
  setWindowTitle( tr( "QGIS Layout Designer" ) );

  setAttribute( Qt::WA_DeleteOnClose );
#if QT_VERSION >= 0x050600
  setDockOptions( dockOptions() | QMainWindow::GroupedDragging );
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

  QMenu *rulerMenu = new QMenu( this );
  rulerMenu->addAction( mActionShowGuides );
  rulerMenu->addAction( mActionSnapGuides );
  rulerMenu->addAction( mActionManageGuides );
  rulerMenu->addAction( mActionClearGuides );
  rulerMenu->addSeparator();
  rulerMenu->addAction( mActionShowRulers );
  mHorizontalRuler->setContextMenu( rulerMenu );
  mVerticalRuler->setContextMenu( rulerMenu );

  connect( mActionRefreshView, &QAction::triggered, this, &QgsLayoutDesignerDialog::refreshLayout );

  connect( mActionShowGrid, &QAction::triggered, this, &QgsLayoutDesignerDialog::showGrid );
  connect( mActionSnapGrid, &QAction::triggered, this, &QgsLayoutDesignerDialog::snapToGrid );

  connect( mActionShowGuides, &QAction::triggered, this, &QgsLayoutDesignerDialog::showGuides );
  connect( mActionSnapGuides, &QAction::triggered, this, &QgsLayoutDesignerDialog::snapToGuides );
  connect( mActionSmartGuides, &QAction::triggered, this, &QgsLayoutDesignerDialog::snapToItems );

  connect( mActionShowBoxes, &QAction::triggered, this, &QgsLayoutDesignerDialog::showBoxes );
  connect( mActionShowPage, &QAction::triggered, this, &QgsLayoutDesignerDialog::showPages );

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
  const QList< int > itemMetadataIds = QgsGui::layoutItemGuiRegistry()->itemMetadataIds();
  for ( int id : itemMetadataIds )
  {
    itemTypeAdded( id );
  }
  //..and listen out for new item types
  connect( QgsGui::layoutItemGuiRegistry(), &QgsLayoutItemGuiRegistry::typeAdded, this, &QgsLayoutDesignerDialog::itemTypeAdded );

  QToolButton *orderingToolButton = new QToolButton( this );
  orderingToolButton->setPopupMode( QToolButton::InstantPopup );
  orderingToolButton->setAutoRaise( true );
  orderingToolButton->setToolButtonStyle( Qt::ToolButtonIconOnly );
  orderingToolButton->addAction( mActionRaiseItems );
  orderingToolButton->addAction( mActionLowerItems );
  orderingToolButton->addAction( mActionMoveItemsToTop );
  orderingToolButton->addAction( mActionMoveItemsToBottom );
  orderingToolButton->setDefaultAction( mActionRaiseItems );
  mActionsToolbar->addWidget( orderingToolButton );

  QToolButton *alignToolButton = new QToolButton( this );
  alignToolButton->setPopupMode( QToolButton::InstantPopup );
  alignToolButton->setAutoRaise( true );
  alignToolButton->setToolButtonStyle( Qt::ToolButtonIconOnly );
  alignToolButton->addAction( mActionAlignLeft );
  alignToolButton->addAction( mActionAlignHCenter );
  alignToolButton->addAction( mActionAlignRight );
  alignToolButton->addAction( mActionAlignTop );
  alignToolButton->addAction( mActionAlignVCenter );
  alignToolButton->addAction( mActionAlignBottom );
  alignToolButton->setDefaultAction( mActionAlignLeft );
  mActionsToolbar->addWidget( alignToolButton );

  QToolButton *distributeToolButton = new QToolButton( this );
  distributeToolButton->setPopupMode( QToolButton::InstantPopup );
  distributeToolButton->setAutoRaise( true );
  distributeToolButton->setToolButtonStyle( Qt::ToolButtonIconOnly );
  distributeToolButton->addAction( mActionDistributeLeft );
  distributeToolButton->addAction( mActionDistributeHCenter );
  distributeToolButton->addAction( mActionDistributeRight );
  distributeToolButton->addAction( mActionDistributeTop );
  distributeToolButton->addAction( mActionDistributeVCenter );
  distributeToolButton->addAction( mActionDistributeBottom );
  distributeToolButton->setDefaultAction( mActionDistributeLeft );
  mActionsToolbar->addWidget( distributeToolButton );

  QToolButton *resizeToolButton = new QToolButton( this );
  resizeToolButton->setPopupMode( QToolButton::InstantPopup );
  resizeToolButton->setAutoRaise( true );
  resizeToolButton->setToolButtonStyle( Qt::ToolButtonIconOnly );
  resizeToolButton->addAction( mActionResizeNarrowest );
  resizeToolButton->addAction( mActionResizeWidest );
  resizeToolButton->addAction( mActionResizeShortest );
  resizeToolButton->addAction( mActionResizeTallest );
  resizeToolButton->addAction( mActionResizeToSquare );
  resizeToolButton->setDefaultAction( mActionResizeNarrowest );
  mActionsToolbar->addWidget( resizeToolButton );

  mAddItemTool = new QgsLayoutViewToolAddItem( mView );
  mAddNodeItemTool = new QgsLayoutViewToolAddNodeItem( mView );
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
  // after creating an item with the add item tool, switch immediately to select tool
  connect( mAddItemTool, &QgsLayoutViewToolAddItem::createdItem, this, [ = ] { mView->setTool( mSelectTool ); } );
  connect( mAddNodeItemTool, &QgsLayoutViewToolAddNodeItem::createdItem, this, [ = ] { mView->setTool( mSelectTool ); } );

  mNodesTool = new QgsLayoutViewToolEditNodes( mView );
  mNodesTool->setAction( mActionEditNodesItem );
  mToolsActionGroup->addAction( mActionEditNodesItem );
  connect( mActionEditNodesItem, &QAction::triggered, mNodesTool, [ = ] { mView->setTool( mNodesTool ); } );

  mMoveContentTool = new QgsLayoutViewToolMoveItemContent( mView );
  mMoveContentTool->setAction( mActionMoveItemContent );
  mToolsActionGroup->addAction( mActionMoveItemContent );
  connect( mActionMoveItemContent, &QAction::triggered, mMoveContentTool, [ = ] { mView->setTool( mMoveContentTool ); } );

  //Ctrl+= should also trigger zoom in
  QShortcut *ctrlEquals = new QShortcut( QKeySequence( QStringLiteral( "Ctrl+=" ) ), this );
  connect( ctrlEquals, &QShortcut::activated, mActionZoomIn, &QAction::trigger );
  //Backspace should also trigger delete selection
  QShortcut *backSpace = new QShortcut( QKeySequence( QStringLiteral( "Backspace" ) ), this );
  connect( backSpace, &QShortcut::activated, mActionDeleteSelection, &QAction::trigger );

  mActionPreviewModeOff->setChecked( true );
  connect( mActionPreviewModeOff, &QAction::triggered, this, [ = ]
  {
    mView->setPreviewModeEnabled( false );
  } );
  connect( mActionPreviewModeGrayscale, &QAction::triggered, this, [ = ]
  {
    mView->setPreviewMode( QgsPreviewEffect::PreviewGrayscale );
    mView->setPreviewModeEnabled( true );
  } );
  connect( mActionPreviewModeMono, &QAction::triggered, this, [ = ]
  {
    mView->setPreviewMode( QgsPreviewEffect::PreviewMono );
    mView->setPreviewModeEnabled( true );
  } );
  connect( mActionPreviewProtanope, &QAction::triggered, this, [ = ]
  {
    mView->setPreviewMode( QgsPreviewEffect::PreviewProtanope );
    mView->setPreviewModeEnabled( true );
  } );
  connect( mActionPreviewDeuteranope, &QAction::triggered, this, [ = ]
  {
    mView->setPreviewMode( QgsPreviewEffect::PreviewDeuteranope );
    mView->setPreviewModeEnabled( true );
  } );
  QActionGroup *previewGroup = new QActionGroup( this );
  previewGroup->setExclusive( true );
  mActionPreviewModeOff->setActionGroup( previewGroup );
  mActionPreviewModeGrayscale->setActionGroup( previewGroup );
  mActionPreviewModeMono->setActionGroup( previewGroup );
  mActionPreviewProtanope->setActionGroup( previewGroup );
  mActionPreviewDeuteranope->setActionGroup( previewGroup );

  connect( mActionZoomIn, &QAction::triggered, mView, &QgsLayoutView::zoomIn );
  connect( mActionZoomOut, &QAction::triggered, mView, &QgsLayoutView::zoomOut );
  connect( mActionZoomAll, &QAction::triggered, mView, &QgsLayoutView::zoomFull );
  connect( mActionZoomActual, &QAction::triggered, mView, &QgsLayoutView::zoomActual );
  connect( mActionZoomToWidth, &QAction::triggered, mView, &QgsLayoutView::zoomWidth );

  connect( mActionSelectAll, &QAction::triggered, mView, &QgsLayoutView::selectAll );
  connect( mActionDeselectAll, &QAction::triggered, mView, &QgsLayoutView::deselectAll );
  connect( mActionInvertSelection, &QAction::triggered, mView, &QgsLayoutView::invertSelection );
  connect( mActionSelectNextAbove, &QAction::triggered, mView, &QgsLayoutView::selectNextItemAbove );
  connect( mActionSelectNextBelow, &QAction::triggered, mView, &QgsLayoutView::selectNextItemBelow );

  connect( mActionRaiseItems, &QAction::triggered, this, &QgsLayoutDesignerDialog::raiseSelectedItems );
  connect( mActionLowerItems, &QAction::triggered, this, &QgsLayoutDesignerDialog::lowerSelectedItems );
  connect( mActionMoveItemsToTop, &QAction::triggered, this, &QgsLayoutDesignerDialog::moveSelectedItemsToTop );
  connect( mActionMoveItemsToBottom, &QAction::triggered, this, &QgsLayoutDesignerDialog::moveSelectedItemsToBottom );
  connect( mActionAlignLeft, &QAction::triggered, this, [ = ]
  {
    mView->alignSelectedItems( QgsLayoutAligner::AlignLeft );
  } );
  connect( mActionAlignHCenter, &QAction::triggered, this, [ = ]
  {
    mView->alignSelectedItems( QgsLayoutAligner::AlignHCenter );
  } );
  connect( mActionAlignRight, &QAction::triggered, this, [ = ]
  {
    mView->alignSelectedItems( QgsLayoutAligner::AlignRight );
  } );
  connect( mActionAlignTop, &QAction::triggered, this, [ = ]
  {
    mView->alignSelectedItems( QgsLayoutAligner::AlignTop );
  } );
  connect( mActionAlignVCenter, &QAction::triggered, this, [ = ]
  {
    mView->alignSelectedItems( QgsLayoutAligner::AlignVCenter );
  } );
  connect( mActionAlignBottom, &QAction::triggered, this, [ = ]
  {
    mView->alignSelectedItems( QgsLayoutAligner::AlignBottom );
  } );
  connect( mActionDistributeLeft, &QAction::triggered, this, [ = ]
  {
    mView->distributeSelectedItems( QgsLayoutAligner::DistributeLeft );
  } );
  connect( mActionDistributeHCenter, &QAction::triggered, this, [ = ]
  {
    mView->distributeSelectedItems( QgsLayoutAligner::DistributeHCenter );
  } );
  connect( mActionDistributeRight, &QAction::triggered, this, [ = ]
  {
    mView->distributeSelectedItems( QgsLayoutAligner::DistributeRight );
  } );
  connect( mActionDistributeTop, &QAction::triggered, this, [ = ]
  {
    mView->distributeSelectedItems( QgsLayoutAligner::DistributeTop );
  } );
  connect( mActionDistributeVCenter, &QAction::triggered, this, [ = ]
  {
    mView->distributeSelectedItems( QgsLayoutAligner::DistributeVCenter );
  } );
  connect( mActionDistributeBottom, &QAction::triggered, this, [ = ]
  {
    mView->distributeSelectedItems( QgsLayoutAligner::DistributeBottom );
  } );
  connect( mActionResizeNarrowest, &QAction::triggered, this, [ = ]
  {
    mView->resizeSelectedItems( QgsLayoutAligner::ResizeNarrowest );
  } );
  connect( mActionResizeWidest, &QAction::triggered, this, [ = ]
  {
    mView->resizeSelectedItems( QgsLayoutAligner::ResizeWidest );
  } );
  connect( mActionResizeShortest, &QAction::triggered, this, [ = ]
  {
    mView->resizeSelectedItems( QgsLayoutAligner::ResizeShortest );
  } );
  connect( mActionResizeTallest, &QAction::triggered, this, [ = ]
  {
    mView->resizeSelectedItems( QgsLayoutAligner::ResizeTallest );
  } );
  connect( mActionResizeToSquare, &QAction::triggered, this, [ = ]
  {
    mView->resizeSelectedItems( QgsLayoutAligner::ResizeToSquare );
  } );

  connect( mActionAddPages, &QAction::triggered, this, &QgsLayoutDesignerDialog::addPages );

  connect( mActionUnlockAll, &QAction::triggered, this, &QgsLayoutDesignerDialog::unlockAllItems );
  connect( mActionLockItems, &QAction::triggered, this, &QgsLayoutDesignerDialog::lockSelectedItems );

  connect( mActionHidePanels, &QAction::toggled, this, &QgsLayoutDesignerDialog::setPanelVisibility );

  connect( mActionDeleteSelection, &QAction::triggered, this, [ = ]
  {
    if ( mView->tool() == mNodesTool )
      mNodesTool->deleteSelectedNode();
    else
      mView->deleteSelectedItems();
  } );
  connect( mActionGroupItems, &QAction::triggered, this, [ = ]
  {
    mView->groupSelectedItems();
  } );
  connect( mActionUngroupItems, &QAction::triggered, this, [ = ]
  {
    mView->ungroupSelectedItems();
  } );

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
  QRegularExpression zoomRx( QStringLiteral( "\\s*\\d{1,4}(\\.\\d?)?\\s*%?" ) );
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

  connect( mView, &QgsLayoutView::itemFocused, this, [ = ]( QgsLayoutItem * item )
  {
    showItemOptions( item, false );
  } );

  // Panel and toolbar submenus
  mToolbarMenu->addAction( mLayoutToolbar->toggleViewAction() );
  mToolbarMenu->addAction( mNavigationToolbar->toggleViewAction() );
  mToolbarMenu->addAction( mToolsToolbar->toggleViewAction() );

  connect( mActionToggleFullScreen, &QAction::toggled, this, &QgsLayoutDesignerDialog::toggleFullScreen );

  mMenuProvider = new QgsLayoutAppMenuProvider( this );
  mView->setMenuProvider( mMenuProvider );

  int minDockWidth( fontMetrics().width( QStringLiteral( "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" ) ) );

  setTabPosition( Qt::AllDockWidgetAreas, QTabWidget::North );
  mGeneralDock = new QgsDockWidget( tr( "Layout" ), this );
  mGeneralDock->setObjectName( QStringLiteral( "LayoutDock" ) );
  mGeneralDock->setMinimumWidth( minDockWidth );
  mGeneralPropertiesStack = new QgsPanelWidgetStack();
  mGeneralDock->setWidget( mGeneralPropertiesStack );
  mPanelsMenu->addAction( mGeneralDock->toggleViewAction() );
  connect( mActionLayoutProperties, &QAction::triggered, this, [ = ]
  {
    mGeneralDock->setUserVisible( true );
  } );

  mItemDock = new QgsDockWidget( tr( "Item properties" ), this );
  mItemDock->setObjectName( QStringLiteral( "ItemDock" ) );
  mItemDock->setMinimumWidth( minDockWidth );
  mItemPropertiesStack = new QgsPanelWidgetStack();
  mItemDock->setWidget( mItemPropertiesStack );
  mPanelsMenu->addAction( mItemDock->toggleViewAction() );

  mGuideDock = new QgsDockWidget( tr( "Guides" ), this );
  mGuideDock->setObjectName( QStringLiteral( "GuideDock" ) );
  mGuideDock->setMinimumWidth( minDockWidth );
  mGuideStack = new QgsPanelWidgetStack();
  mGuideDock->setWidget( mGuideStack );
  mPanelsMenu->addAction( mGuideDock->toggleViewAction() );
  connect( mActionManageGuides, &QAction::triggered, this, [ = ]
  {
    mGuideDock->setUserVisible( true );
  } );

  mUndoDock = new QgsDockWidget( tr( "Command history" ), this );
  mUndoDock->setObjectName( QStringLiteral( "CommandDock" ) );
  mPanelsMenu->addAction( mUndoDock->toggleViewAction() );
  mUndoView = new QUndoView( this );
  mUndoDock->setWidget( mUndoView );

  mItemsDock = new QgsDockWidget( tr( "Items" ), this );
  mItemsDock->setObjectName( QStringLiteral( "ItemsDock" ) );
  mPanelsMenu->addAction( mItemsDock->toggleViewAction() );

  //items tree widget
  mItemsTreeView = new QgsLayoutItemsListView( mItemsDock, this );
  mItemsDock->setWidget( mItemsTreeView );

  const QList<QDockWidget *> docks = findChildren<QDockWidget *>();
  for ( QDockWidget *dock : docks )
  {
    connect( dock, &QDockWidget::visibilityChanged, this, &QgsLayoutDesignerDialog::dockVisibilityChanged );
  }

  addDockWidget( Qt::RightDockWidgetArea, mItemDock );
  addDockWidget( Qt::RightDockWidgetArea, mGeneralDock );
  addDockWidget( Qt::RightDockWidgetArea, mGuideDock );
  addDockWidget( Qt::RightDockWidgetArea, mUndoDock );
  addDockWidget( Qt::RightDockWidgetArea, mItemsDock );

  createLayoutPropertiesWidget();

  mUndoDock->show();
  mItemDock->show();
  mGeneralDock->show();
  mItemsDock->show();

  tabifyDockWidget( mGeneralDock, mUndoDock );
  tabifyDockWidget( mItemDock, mUndoDock );
  tabifyDockWidget( mGeneralDock, mItemDock );
  tabifyDockWidget( mItemDock, mItemsDock );

  restoreWindowState();

  //listen out to status bar updates from the view
  connect( mView, &QgsLayoutView::statusMessage, this, &QgsLayoutDesignerDialog::statusMessageReceived );
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

  // add undo/redo actions which apply to the correct layout undo stack
  delete mUndoAction;
  delete mRedoAction;
  mUndoAction = layout->undoStack()->stack()->createUndoAction( this );
  mUndoAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionUndo.svg" ) ) );
  mUndoAction->setShortcuts( QKeySequence::Undo );
  mRedoAction = layout->undoStack()->stack()->createRedoAction( this );
  mRedoAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionRedo.svg" ) ) );
  mRedoAction->setShortcuts( QKeySequence::Redo );
  menuEdit->insertAction( menuEdit->actions().at( 0 ), mRedoAction );
  menuEdit->insertAction( mRedoAction, mUndoAction );
  mLayoutToolbar->addAction( mUndoAction );
  mLayoutToolbar->addAction( mRedoAction );

  connect( mLayout->undoStack(), &QgsLayoutUndoStack::undoRedoOccurredForItems, this, &QgsLayoutDesignerDialog::undoRedoOccurredForItems );
  connect( mActionClearGuides, &QAction::triggered, &mLayout->guides(), [ = ]
  {
    mLayout->guides().clear();
  } );

  mActionShowGrid->setChecked( mLayout->context().gridVisible() );
  mActionSnapGrid->setChecked( mLayout->snapper().snapToGrid() );
  mActionShowGuides->setChecked( mLayout->guides().visible() );
  mActionSnapGuides->setChecked( mLayout->snapper().snapToGuides() );
  mActionSmartGuides->setChecked( mLayout->snapper().snapToItems() );
  mActionShowBoxes->setChecked( mLayout->context().boundingBoxesVisible() );
  mActionShowPage->setChecked( mLayout->context().pagesVisible() );

  mUndoView->setStack( mLayout->undoStack()->stack() );

  mSelectTool->setLayout( layout );
  mItemsTreeView->setCurrentLayout( mLayout );
#ifdef ENABLE_MODELTEST
  new ModelTest( mLayout->itemsModel(), this );
#endif

  createLayoutPropertiesWidget();
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

void QgsLayoutDesignerDialog::showItemOptions( QgsLayoutItem *item, bool bringPanelToFront )
{
  if ( mBlockItemOptions )
    return;

  if ( !item )
  {
    delete mItemPropertiesStack->takeMainPanel();
    return;
  }

  if ( auto widget = qobject_cast< QgsLayoutItemBaseWidget * >( mItemPropertiesStack->mainPanel() ) )
  {
    if ( widget->layoutObject() == item )
    {
      // already showing properties for this item - we don't want to create a new panel
      if ( bringPanelToFront )
        mItemDock->setUserVisible( true );

      return;
    }
    else
    {
      // try to reuse
      if ( widget->setItem( item ) )
      {
        if ( bringPanelToFront )
          mItemDock->setUserVisible( true );

        return;
      }
    }
  }

  std::unique_ptr< QgsLayoutItemBaseWidget > widget( QgsGui::layoutItemGuiRegistry()->createItemWidget( item ) );
  delete mItemPropertiesStack->takeMainPanel();

  if ( ! widget )
    return;

  widget->setDockMode( true );
  connect( item, &QgsLayoutItem::destroyed, widget.get(), [this]
  {
    delete mItemPropertiesStack->takeMainPanel();
  } );

  mItemPropertiesStack->setMainPanel( widget.release() );
  if ( bringPanelToFront )
    mItemDock->setUserVisible( true );

}

void QgsLayoutDesignerDialog::open()
{
  show();
  activate();
  if ( mView )
  {
    mView->zoomFull(); // zoomFull() does not work properly until we have called show()
  }
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

void QgsLayoutDesignerDialog::showGrid( bool visible )
{
  mLayout->context().setGridVisible( visible );
  mLayout->pageCollection()->redraw();
}

void QgsLayoutDesignerDialog::showBoxes( bool visible )
{
  mLayout->context().setBoundingBoxesVisible( visible );
  mSelectTool->mouseHandles()->update();
}

void QgsLayoutDesignerDialog::showPages( bool visible )
{
  mLayout->context().setPagesVisible( visible );
  mLayout->pageCollection()->redraw();
}

void QgsLayoutDesignerDialog::snapToGrid( bool enabled )
{
  mLayout->snapper().setSnapToGrid( enabled );
}

void QgsLayoutDesignerDialog::showGuides( bool visible )
{
  mLayout->guides().setVisible( visible );
}

void QgsLayoutDesignerDialog::snapToGuides( bool enabled )
{
  mLayout->snapper().setSnapToGuides( enabled );
}

void QgsLayoutDesignerDialog::snapToItems( bool enabled )
{
  mLayout->snapper().setSnapToItems( enabled );
}

void QgsLayoutDesignerDialog::unlockAllItems()
{
  mView->unlockAllItems();
}

void QgsLayoutDesignerDialog::lockSelectedItems()
{
  mView->lockSelectedItems();
}

void QgsLayoutDesignerDialog::setPanelVisibility( bool hidden )
{
  /*
  workaround the limited Qt dock widget API
  see http://qt-project.org/forums/viewthread/1141/
  and http://qt-project.org/faq/answer/how_can_i_check_which_tab_is_the_current_one_in_a_tabbed_qdockwidget
  */

  const QList<QDockWidget *> docks = findChildren<QDockWidget *>();
  const QList<QTabBar *> tabBars = findChildren<QTabBar *>();

  if ( hidden )
  {
    mPanelStatus.clear();
    //record status of all docks

    for ( QDockWidget *dock : docks )
    {
      mPanelStatus.insert( dock->windowTitle(), PanelStatus( dock->isVisible(), false ) );
      dock->setVisible( false );
    }

    //record active dock tabs
    for ( QTabBar *tabBar : tabBars )
    {
      QString currentTabTitle = tabBar->tabText( tabBar->currentIndex() );
      mPanelStatus[ currentTabTitle ].isActive = true;
    }
  }
  else
  {
    //restore visibility of all docks
    for ( QDockWidget *dock : docks )
    {
      if ( ! mPanelStatus.contains( dock->windowTitle() ) )
      {
        dock->setVisible( true );
        continue;
      }
      dock->setVisible( mPanelStatus.value( dock->windowTitle() ).isVisible );
    }

    //restore previously active dock tabs
    for ( QTabBar *tabBar : tabBars )
    {
      //loop through all tabs in tab bar
      for ( int i = 0; i < tabBar->count(); ++i )
      {
        QString tabTitle = tabBar->tabText( i );
        if ( mPanelStatus.value( tabTitle ).isActive )
        {
          tabBar->setCurrentIndex( i );
        }
      }
    }
  }
}

void QgsLayoutDesignerDialog::raiseSelectedItems()
{
  mView->raiseSelectedItems();
}

void QgsLayoutDesignerDialog::lowerSelectedItems()
{
  mView->lowerSelectedItems();
}

void QgsLayoutDesignerDialog::moveSelectedItemsToTop()
{
  mView->moveSelectedItemsToTop();
}

void QgsLayoutDesignerDialog::moveSelectedItemsToBottom()
{
  mView->moveSelectedItemsToBottom();
}

void QgsLayoutDesignerDialog::refreshLayout()
{
  if ( !currentLayout() )
  {
    return;
  }

#if 0 //TODO
  //refresh atlas feature first, to update attributes
  if ( mComposition->atlasMode() == QgsComposition::PreviewAtlas )
  {
    //block signals from atlas, since the later call to mComposition->refreshItems() will
    //also trigger items to refresh atlas dependent properties
    mComposition->atlasComposition().blockSignals( true );
    mComposition->atlasComposition().refreshFeature();
    mComposition->atlasComposition().blockSignals( false );
  }
#endif

  currentLayout()->refresh();
}

void QgsLayoutDesignerDialog::closeEvent( QCloseEvent * )
{
  emit aboutToClose();
  saveWindowState();
}

void QgsLayoutDesignerDialog::itemTypeAdded( int id )
{
  if ( QgsGui::layoutItemGuiRegistry()->itemMetadata( id )->flags() & QgsLayoutItemAbstractGuiMetadata::FlagNoCreationTools )
    return;

  QString name = QgsGui::layoutItemGuiRegistry()->itemMetadata( id )->visibleName();
  QString groupId = QgsGui::layoutItemGuiRegistry()->itemMetadata( id )->groupId();
  bool nodeBased = QgsGui::layoutItemGuiRegistry()->itemMetadata( id )->isNodeBased();
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
  action->setData( id );
  action->setIcon( QgsGui::layoutItemGuiRegistry()->itemMetadata( id )->creationIcon() );

  mToolsActionGroup->addAction( action );
  if ( itemSubmenu )
    itemSubmenu->addAction( action );
  else
    mItemMenu->addAction( action );

  if ( groupButton )
    groupButton->addAction( action );
  else
    mToolsToolbar->addAction( action );

  connect( action, &QAction::triggered, this, [this, id, nodeBased]()
  {
    activateNewItemCreationTool( id, nodeBased );
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
  double zoomLevel = 0;
  if ( currentLayout()->units() == QgsUnitTypes::LayoutPixels )
  {
    zoomLevel = mView->transform().m11() * 100;
  }
  else
  {
    double dpi = QgsApplication::desktop()->logicalDpiX();
    //monitor dpi is not always correct - so make sure the value is sane
    if ( ( dpi < 60 ) || ( dpi > 1200 ) )
      dpi = 72;

    //pixel width for 1mm on screen
    double scale100 = dpi / 25.4;
    scale100 = currentLayout()->convertFromLayoutUnits( scale100, QgsUnitTypes::LayoutMillimeters ).length();
    //current zoomLevel
    zoomLevel = mView->transform().m11() * 100 / scale100;
  }
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
  QPointF pagePosition = mLayout->pageCollection()->positionOnPage( position );
  int currentPage = mLayout->pageCollection()->pageNumberForPoint( position );

  QString unit = QgsUnitTypes::toAbbreviatedString( mLayout->units() );
  mStatusCursorXLabel->setText( tr( "x: %1 %2" ).arg( pagePosition.x() ).arg( unit ) );
  mStatusCursorYLabel->setText( tr( "y: %1 %2" ).arg( pagePosition.y() ).arg( unit ) );
  mStatusCursorPageLabel->setText( tr( "page: %1" ).arg( currentPage + 1 ) );
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

void QgsLayoutDesignerDialog::addPages()
{
  QgsLayoutAddPagesDialog dlg( this );
  dlg.setLayout( mLayout );

  if ( dlg.exec() )
  {
    int firstPagePosition = dlg.beforePage() - 1;
    switch ( dlg.pagePosition() )
    {
      case QgsLayoutAddPagesDialog::BeforePage:
        break;

      case QgsLayoutAddPagesDialog::AfterPage:
        firstPagePosition = firstPagePosition + 1;
        break;

      case QgsLayoutAddPagesDialog::AtEnd:
        firstPagePosition = mLayout->pageCollection()->pageCount();
        break;

    }

    if ( dlg.numberPages() > 1 )
      mLayout->undoStack()->beginMacro( tr( "Add Pages" ) );
    for ( int i = 0; i < dlg.numberPages(); ++i )
    {
      QgsLayoutItemPage *page = new QgsLayoutItemPage( mLayout );
      page->setPageSize( dlg.pageSize() );
      mLayout->pageCollection()->insertPage( page, firstPagePosition + i );
    }
    if ( dlg.numberPages() > 1 )
      mLayout->undoStack()->endMacro();

  }
}

void QgsLayoutDesignerDialog::statusMessageReceived( const QString &message )
{
  mStatusBar->showMessage( message );
}

void QgsLayoutDesignerDialog::dockVisibilityChanged( bool visible )
{
  if ( visible )
  {
    whileBlocking( mActionHidePanels )->setChecked( false );
  }
}

void QgsLayoutDesignerDialog::undoRedoOccurredForItems( const QSet<QString> itemUuids )
{
  mBlockItemOptions = true;

  mLayout->deselectAll();
  QgsLayoutItem *focusItem = nullptr;
  for ( const QString &uuid : itemUuids )
  {
    QgsLayoutItem *item = mLayout->itemByUuid( uuid );
    if ( !item )
      continue;

    item->setSelected( true );
    focusItem = item;
  }
  mBlockItemOptions = false;

  if ( focusItem )
    showItemOptions( focusItem );
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

void QgsLayoutDesignerDialog::activateNewItemCreationTool( int id, bool nodeBasedItem )
{
  if ( !nodeBasedItem )
  {
    mAddItemTool->setItemMetadataId( id );
    if ( mView )
      mView->setTool( mAddItemTool );
  }
  else
  {
    mAddNodeItemTool->setItemMetadataId( id );
    if ( mView )
      mView->setTool( mAddNodeItemTool );
  }
}

void QgsLayoutDesignerDialog::createLayoutPropertiesWidget()
{
  if ( !mLayout )
  {
    return;
  }

  // update layout based widgets
  QgsLayoutPropertiesWidget *oldLayoutWidget = qobject_cast<QgsLayoutPropertiesWidget *>( mGeneralPropertiesStack->takeMainPanel() );
  delete oldLayoutWidget;
  QgsLayoutGuideWidget *oldGuideWidget = qobject_cast<QgsLayoutGuideWidget *>( mGuideStack->takeMainPanel() );
  delete oldGuideWidget;

  QgsLayoutPropertiesWidget *widget = new QgsLayoutPropertiesWidget( mGeneralDock, mLayout );
  widget->setDockMode( true );
  mGeneralPropertiesStack->setMainPanel( widget );

  QgsLayoutGuideWidget *guideWidget = new QgsLayoutGuideWidget( mGuideDock, mLayout, mView );
  guideWidget->setDockMode( true );
  mGuideStack->setMainPanel( guideWidget );
}

void QgsLayoutDesignerDialog::initializeRegistry()
{
  sInitializedRegistry = true;
  auto createPageWidget = ( []( QgsLayoutItem * item )->QgsLayoutItemBaseWidget *
  {
    return new QgsLayoutPagePropertiesWidget( nullptr, item );
  } );

  QgsGui::layoutItemGuiRegistry()->addLayoutItemGuiMetadata( new QgsLayoutItemGuiMetadata( QgsLayoutItemRegistry::LayoutPage, QObject::tr( "Page" ), QIcon(), createPageWidget, nullptr, QString(), false, QgsLayoutItemAbstractGuiMetadata::FlagNoCreationTools ) );

}



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
#include "qgsfileutils.h"
#include "qgslogger.h"
#include "qgslayout.h"
#include "qgslayoutatlas.h"
#include "qgslayoutappmenuprovider.h"
#include "qgslayoutcustomdrophandler.h"
#include "qgslayoutmanager.h"
#include "qgslayoutview.h"
#include "qgslayoutviewtooladditem.h"
#include "qgslayoutviewtooladdnodeitem.h"
#include "qgslayoutviewtoolpan.h"
#include "qgslayoutviewtoolmoveitemcontent.h"
#include "qgslayoutviewtoolzoom.h"
#include "qgslayoutviewtoolselect.h"
#include "qgslayoutviewtooleditnodes.h"
#include "qgslayoutitemwidget.h"
#include "qgslayoutimageexportoptionsdialog.h"
#include "qgslayoutitemmap.h"
#include "qgsprintlayout.h"
#include "qgsmapcanvas.h"
#include "qgsmessageviewer.h"
#include "qgsgui.h"
#include "qgsfeedback.h"
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
#include "qgsproject.h"
#include "qgsbusyindicatordialog.h"
#include "qgslayoutundostack.h"
#include "qgslayoutatlaswidget.h"
#include "qgslayoutpagecollection.h"
#include "qgsreport.h"
#include "qgsreportorganizerwidget.h"
#include "ui_qgssvgexportoptions.h"
#include <QShortcut>
#include <QComboBox>
#include <QLineEdit>
#include <QDesktopWidget>
#include <QSlider>
#include <QLabel>
#include <QUndoView>
#include <QTreeView>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QPrinter>
#include <QPrintDialog>
#include <QPageSetupDialog>
#ifdef Q_OS_MACX
#include <ApplicationServices/ApplicationServices.h>
#endif

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

QgsMasterLayoutInterface *QgsAppLayoutDesignerInterface::masterLayout()
{
  return mDesigner->masterLayout();
}

QgsLayoutView *QgsAppLayoutDesignerInterface::view()
{
  return mDesigner->view();
}

QgsMessageBar *QgsAppLayoutDesignerInterface::messageBar()
{
  return mDesigner->messageBar();
}

void QgsAppLayoutDesignerInterface::selectItems( const QList<QgsLayoutItem *> items )
{
  mDesigner->selectItems( items );
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
  setAcceptDrops( true );

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

  mMessageBar = new QgsMessageBar( centralWidget() );
  mMessageBar->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );
  static_cast< QGridLayout * >( centralWidget()->layout() )->addWidget( mMessageBar, 0, 0, 1, 1, Qt::AlignTop );

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
  bool showRulers = settings.value( QStringLiteral( "LayoutDesigner/showRulers" ), true, QgsSettings::App ).toBool();
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
  connect( mActionSaveProject, &QAction::triggered, this, &QgsLayoutDesignerDialog::saveProject );
  connect( mActionNewLayout, &QAction::triggered, this, &QgsLayoutDesignerDialog::newLayout );
  connect( mActionLayoutManager, &QAction::triggered, this, &QgsLayoutDesignerDialog::showManager );
  connect( mActionRemoveLayout, &QAction::triggered, this, &QgsLayoutDesignerDialog::deleteLayout );

  connect( mActionPrint, &QAction::triggered, this, &QgsLayoutDesignerDialog::print );
  connect( mActionExportAsImage, &QAction::triggered, this, &QgsLayoutDesignerDialog::exportToRaster );
  connect( mActionExportAsPDF, &QAction::triggered, this, &QgsLayoutDesignerDialog::exportToPdf );
  connect( mActionExportAsSVG, &QAction::triggered, this, &QgsLayoutDesignerDialog::exportToSvg );

  connect( mActionShowGrid, &QAction::triggered, this, &QgsLayoutDesignerDialog::showGrid );
  connect( mActionSnapGrid, &QAction::triggered, this, &QgsLayoutDesignerDialog::snapToGrid );

  connect( mActionShowGuides, &QAction::triggered, this, &QgsLayoutDesignerDialog::showGuides );
  connect( mActionSnapGuides, &QAction::triggered, this, &QgsLayoutDesignerDialog::snapToGuides );
  connect( mActionSmartGuides, &QAction::triggered, this, &QgsLayoutDesignerDialog::snapToItems );

  connect( mActionShowBoxes, &QAction::triggered, this, &QgsLayoutDesignerDialog::showBoxes );
  connect( mActionShowPage, &QAction::triggered, this, &QgsLayoutDesignerDialog::showPages );

  connect( mActionPasteInPlace, &QAction::triggered, this, &QgsLayoutDesignerDialog::pasteInPlace );

  connect( mActionAtlasSettings, &QAction::triggered, this, &QgsLayoutDesignerDialog::showAtlasSettings );
  connect( mActionAtlasPreview, &QAction::triggered, this, &QgsLayoutDesignerDialog::atlasPreviewTriggered );
  connect( mActionAtlasNext, &QAction::triggered, this, &QgsLayoutDesignerDialog::atlasNext );
  connect( mActionAtlasPrev, &QAction::triggered, this, &QgsLayoutDesignerDialog::atlasPrevious );
  connect( mActionAtlasFirst, &QAction::triggered, this, &QgsLayoutDesignerDialog::atlasFirst );
  connect( mActionAtlasLast, &QAction::triggered, this, &QgsLayoutDesignerDialog::atlasLast );
  connect( mActionPrintAtlas, &QAction::triggered, this, &QgsLayoutDesignerDialog::printAtlas );
  connect( mActionExportAtlasAsImage, &QAction::triggered, this, &QgsLayoutDesignerDialog::exportAtlasToRaster );
  connect( mActionExportAtlasAsSVG, &QAction::triggered, this, &QgsLayoutDesignerDialog::exportAtlasToSvg );
  connect( mActionExportAtlasAsPDF, &QAction::triggered, this, &QgsLayoutDesignerDialog::exportAtlasToPdf );

  connect( mActionReportSettings, &QAction::triggered, this, &QgsLayoutDesignerDialog::showReportSettings );
  connect( mActionExportReportAsImage, &QAction::triggered, this, &QgsLayoutDesignerDialog::exportReportToRaster );
  connect( mActionExportReportAsSVG, &QAction::triggered, this, &QgsLayoutDesignerDialog::exportReportToSvg );
  connect( mActionExportReportAsPDF, &QAction::triggered, this, &QgsLayoutDesignerDialog::exportReportToPdf );
  connect( mActionPrintReport, &QAction::triggered, this, &QgsLayoutDesignerDialog::printReport );

  connect( mActionPageSetup, &QAction::triggered, this, &QgsLayoutDesignerDialog::pageSetup );

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

  QToolButton *atlasExportToolButton = new QToolButton( mAtlasToolbar );
  atlasExportToolButton->setIcon( QgsApplication::getThemeIcon( "mActionExport.svg" ) );
  atlasExportToolButton->setPopupMode( QToolButton::InstantPopup );
  atlasExportToolButton->setAutoRaise( true );
  atlasExportToolButton->setToolButtonStyle( Qt::ToolButtonIconOnly );
  atlasExportToolButton->addAction( mActionExportAtlasAsImage );
  atlasExportToolButton->addAction( mActionExportAtlasAsSVG );
  atlasExportToolButton->addAction( mActionExportAtlasAsPDF );
  atlasExportToolButton->setToolTip( tr( "Export Atlas" ) );
  mAtlasToolbar->insertWidget( mActionAtlasSettings, atlasExportToolButton );
  mAtlasPageComboBox = new QComboBox();
  mAtlasPageComboBox->setEditable( true );
  mAtlasPageComboBox->addItem( QString::number( 1 ) );
  mAtlasPageComboBox->setCurrentIndex( 0 );
  mAtlasPageComboBox->setMinimumHeight( mAtlasToolbar->height() );
  mAtlasPageComboBox->setMinimumContentsLength( 6 );
  mAtlasPageComboBox->setMaxVisibleItems( 20 );
  mAtlasPageComboBox->setSizeAdjustPolicy( QComboBox::AdjustToContents );
  mAtlasPageComboBox->setInsertPolicy( QComboBox::NoInsert );
  connect( mAtlasPageComboBox->lineEdit(), &QLineEdit::editingFinished, this, &QgsLayoutDesignerDialog::atlasPageComboEditingFinished );
  connect( mAtlasPageComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutDesignerDialog::atlasPageComboEditingFinished );
  mAtlasToolbar->insertWidget( mActionAtlasNext, mAtlasPageComboBox );

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

  connect( mActionSaveAsTemplate, &QAction::triggered, this, &QgsLayoutDesignerDialog::saveAsTemplate );
  connect( mActionLoadFromTemplate, &QAction::triggered, this, &QgsLayoutDesignerDialog::addItemsFromTemplate );
  connect( mActionDuplicateLayout, &QAction::triggered, this, &QgsLayoutDesignerDialog::duplicate );
  connect( mActionRenameLayout, &QAction::triggered, this, &QgsLayoutDesignerDialog::renameLayout );

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

  //cut/copy/paste actions. Note these are not included in the ui file
  //as ui files have no support for QKeySequence shortcuts
  mActionCut = new QAction( tr( "Cu&t" ), this );
  mActionCut->setShortcuts( QKeySequence::Cut );
  mActionCut->setStatusTip( tr( "Cut" ) );
  mActionCut->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionEditCut.svg" ) ) );
  connect( mActionCut, &QAction::triggered, this, [ = ]
  {
    mView->copySelectedItems( QgsLayoutView::ClipboardCut );
  } );

  mActionCopy = new QAction( tr( "&Copy" ), this );
  mActionCopy->setShortcuts( QKeySequence::Copy );
  mActionCopy->setStatusTip( tr( "Copy" ) );
  mActionCopy->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionEditCopy.svg" ) ) );
  connect( mActionCopy, &QAction::triggered, this, [ = ]
  {
    mView->copySelectedItems( QgsLayoutView::ClipboardCopy );
  } );

  mActionPaste = new QAction( tr( "&Paste" ), this );
  mActionPaste->setShortcuts( QKeySequence::Paste );
  mActionPaste->setStatusTip( tr( "Paste" ) );
  mActionPaste->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionEditPaste.svg" ) ) );
  connect( mActionPaste, &QAction::triggered, this, &QgsLayoutDesignerDialog::paste );

  menuEdit->insertAction( mActionPasteInPlace, mActionCut );
  menuEdit->insertAction( mActionPasteInPlace, mActionCopy );
  menuEdit->insertAction( mActionPasteInPlace, mActionPaste );

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

  mAtlasDock = new QgsDockWidget( tr( "Atlas" ), this );
  mAtlasDock->setObjectName( QStringLiteral( "AtlasDock" ) );
  connect( mAtlasDock, &QDockWidget::visibilityChanged, mActionAtlasSettings, &QAction::setChecked );

  mReportDock = new QgsDockWidget( tr( "Report" ), this );
  mReportDock->setObjectName( QStringLiteral( "ReportDock" ) );
  connect( mReportDock, &QDockWidget::visibilityChanged, mActionReportSettings, &QAction::setChecked );

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
  addDockWidget( Qt::RightDockWidgetArea, mAtlasDock );
  addDockWidget( Qt::LeftDockWidgetArea, mReportDock );

  createLayoutPropertiesWidget();

  mUndoDock->show();
  mItemDock->show();
  mGeneralDock->show();
  mAtlasDock->show();
  mReportDock->show();
  mItemsDock->show();

  tabifyDockWidget( mGeneralDock, mUndoDock );
  tabifyDockWidget( mItemDock, mUndoDock );
  tabifyDockWidget( mGeneralDock, mItemDock );
  tabifyDockWidget( mItemDock, mItemsDock );
  tabifyDockWidget( mItemDock, mAtlasDock );

  toggleActions( false );

  //set initial state of atlas controls
  mActionAtlasPreview->setEnabled( false );
  mActionAtlasPreview->setChecked( false );
  mActionAtlasFirst->setEnabled( false );
  mActionAtlasLast->setEnabled( false );
  mActionAtlasNext->setEnabled( false );
  mActionAtlasPrev->setEnabled( false );
  mActionPrintAtlas->setEnabled( false );
  mAtlasPageComboBox->setEnabled( false );
  mActionExportAtlasAsImage->setEnabled( false );
  mActionExportAtlasAsSVG->setEnabled( false );
  mActionExportAtlasAsPDF->setEnabled( false );

  mLayoutsMenu->setObjectName( QStringLiteral( "mLayoutsMenu" ) );
  connect( mLayoutsMenu, &QMenu::aboutToShow, this, &QgsLayoutDesignerDialog::populateLayoutsMenu );

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

void QgsLayoutDesignerDialog::setMasterLayout( QgsMasterLayoutInterface *layout )
{
  mMasterLayout = layout;

  QObject *obj = dynamic_cast< QObject * >( mMasterLayout );
  if ( obj )
    connect( obj, &QObject::destroyed, this, &QgsLayoutDesignerDialog::close );

  setTitle( mMasterLayout->name() );

  if ( QgsPrintLayout *l = dynamic_cast< QgsPrintLayout * >( layout ) )
  {
    connect( l, &QgsPrintLayout::nameChanged, this, &QgsLayoutDesignerDialog::setTitle );
    setCurrentLayout( l );
  }
  else if ( QgsReport *r = dynamic_cast< QgsReport * >( layout ) )
  {
    connect( r, &QgsReport::nameChanged, this, &QgsLayoutDesignerDialog::setTitle );
  }

  if ( dynamic_cast< QgsPrintLayout * >( layout ) )
  {
    createAtlasWidget();
  }
  else
  {
    // ideally we'd only create mAtlasDock in createAtlasWidget() -
    // but if we do that, then it's always brought to the focus
    // in tab widgets
    mAtlasDock->hide();
    mPanelsMenu->removeAction( mAtlasDock->toggleViewAction() );
    delete mMenuAtlas;
    mMenuAtlas = nullptr;
    mAtlasToolbar->hide();
  }

  if ( dynamic_cast< QgsReport * >( layout ) )
  {
    createReportWidget();
  }
  else
  {
    // ideally we'd only create mReportDock in createReportWidget() -
    // but if we do that, then it's always brought to the focus
    // in tab widgets
    mReportDock->hide();
    mPanelsMenu->removeAction( mReportDock->toggleViewAction() );
    delete mMenuReport;
    mMenuReport = nullptr;
    mReportToolbar->hide();
  }

  updateActionNames( mMasterLayout->layoutType() );
}

QgsMasterLayoutInterface *QgsLayoutDesignerDialog::masterLayout()
{
  return mMasterLayout;
}

void QgsLayoutDesignerDialog::setCurrentLayout( QgsLayout *layout )
{
  if ( !layout )
  {
    toggleActions( false );
  }
  else
  {
    layout->deselectAll();
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

    mActionShowGrid->setChecked( mLayout->renderContext().gridVisible() );
    mActionSnapGrid->setChecked( mLayout->snapper().snapToGrid() );
    mActionShowGuides->setChecked( mLayout->guides().visible() );
    mActionSnapGuides->setChecked( mLayout->snapper().snapToGuides() );
    mActionSmartGuides->setChecked( mLayout->snapper().snapToItems() );
    mActionShowBoxes->setChecked( mLayout->renderContext().boundingBoxesVisible() );
    mActionShowPage->setChecked( mLayout->renderContext().pagesVisible() );

    mUndoView->setStack( mLayout->undoStack()->stack() );

    mSelectTool->setLayout( layout );
    mItemsTreeView->setCurrentLayout( mLayout );
#ifdef ENABLE_MODELTEST
    new ModelTest( mLayout->itemsModel(), this );
#endif

    createLayoutPropertiesWidget();
    toggleActions( true );
  }
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

  widget->setReportTypeString( reportTypeString() );

  if ( QgsLayoutPagePropertiesWidget *ppWidget = qobject_cast< QgsLayoutPagePropertiesWidget * >( widget.get() ) )
    connect( ppWidget, &QgsLayoutPagePropertiesWidget::pageOrientationChanged, this, &QgsLayoutDesignerDialog::pageOrientationChanged );

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

  if ( mMasterLayout && mMasterLayout->layoutType() == QgsMasterLayoutInterface::Report )
  {
    mReportDock->show();
    mReportDock->raise();
    mReportDock->setUserVisible( true );
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
  settings.setValue( QStringLiteral( "LayoutDesigner/showRulers" ), visible, QgsSettings::App );
}

void QgsLayoutDesignerDialog::showGrid( bool visible )
{
  mLayout->renderContext().setGridVisible( visible );
  mLayout->pageCollection()->redraw();
}

void QgsLayoutDesignerDialog::showBoxes( bool visible )
{
  mLayout->renderContext().setBoundingBoxesVisible( visible );
  mSelectTool->mouseHandles()->update();
}

void QgsLayoutDesignerDialog::showPages( bool visible )
{
  mLayout->renderContext().setPagesVisible( visible );
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

  //refresh atlas feature first, to force an update of feature
  //in case feature attributes or geometry has changed
  if ( QgsLayoutAtlas *printAtlas = atlas() )
  {
    if ( printAtlas->enabled() && mActionAtlasPreview->isChecked() )
    {
      //block signals from atlas, since the later call to mComposition->refreshItems() will
      //also trigger items to refresh atlas dependent properties
      whileBlocking( printAtlas )->refreshCurrentFeature();
    }
  }

  currentLayout()->refresh();
}

void QgsLayoutDesignerDialog::closeEvent( QCloseEvent * )
{
  emit aboutToClose();
  saveWindowState();
}

void QgsLayoutDesignerDialog::dropEvent( QDropEvent *event )
{
  // dragging app is locked for the duration of dropEvent. This causes explorer windows to hang
  // while large projects/layers are loaded. So instead we return from dropEvent as quickly as possible
  // and do the actual handling of the drop after a very short timeout
  QTimer *timer = new QTimer( this );
  timer->setSingleShot( true );
  timer->setInterval( 50 );

  // get the file list
  QList<QUrl>::iterator i;
  QList<QUrl>urls = event->mimeData()->urls();
  QStringList files;
  for ( i = urls.begin(); i != urls.end(); ++i )
  {
    QString fileName = i->toLocalFile();
#ifdef Q_OS_MAC
    // Mac OS X 10.10, under Qt4.8 ,changes dropped URL format
    // https://bugreports.qt.io/browse/QTBUG-40449
    // [pzion 20150805] Work around
    if ( fileName.startsWith( "/.file/id=" ) )
    {
      QgsDebugMsg( "Mac dropped URL with /.file/id= (converting)" );
      CFStringRef relCFStringRef =
        CFStringCreateWithCString(
          kCFAllocatorDefault,
          fileName.toUtf8().constData(),
          kCFStringEncodingUTF8
        );
      CFURLRef relCFURL =
        CFURLCreateWithFileSystemPath(
          kCFAllocatorDefault,
          relCFStringRef,
          kCFURLPOSIXPathStyle,
          false // isDirectory
        );
      CFErrorRef error = 0;
      CFURLRef absCFURL =
        CFURLCreateFilePathURL(
          kCFAllocatorDefault,
          relCFURL,
          &error
        );
      if ( !error )
      {
        static const CFIndex maxAbsPathCStrBufLen = 4096;
        char absPathCStr[maxAbsPathCStrBufLen];
        if ( CFURLGetFileSystemRepresentation(
               absCFURL,
               true, // resolveAgainstBase
               reinterpret_cast<UInt8 *>( &absPathCStr[0] ),
               maxAbsPathCStrBufLen ) )
        {
          fileName = QString( absPathCStr );
        }
      }
      CFRelease( absCFURL );
      CFRelease( relCFURL );
      CFRelease( relCFStringRef );
    }
#endif
    // seems that some drag and drop operations include an empty url
    // so we test for length to make sure we have something
    if ( !fileName.isEmpty() )
    {
      files << fileName;
    }
  }

  connect( timer, &QTimer::timeout, this, [this, timer, files]
  {
    for ( const QString &file : qgis::as_const( files ) )
    {
      const QVector<QPointer<QgsLayoutCustomDropHandler >> handlers = QgisApp::instance()->customLayoutDropHandlers();
      for ( QgsLayoutCustomDropHandler *handler : handlers )
      {
        if ( handler && handler->handleFileDrop( iface(), file ) )
        {
          break;
        }
      }
    }

    timer->deleteLater();
  } );

  event->acceptProposedAction();
  timer->start();
}

void QgsLayoutDesignerDialog::dragEnterEvent( QDragEnterEvent *event )
{
  if ( event->mimeData()->hasUrls() )
  {
    event->acceptProposedAction();
  }
}

void QgsLayoutDesignerDialog::setTitle( const QString &title )
{
  mTitle = title;
  updateWindowTitle();
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
  if ( !currentLayout() )
    return;

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

void QgsLayoutDesignerDialog::saveAsTemplate()
{
  //show file dialog
  QgsSettings settings;
  QString lastSaveDir = settings.value( QStringLiteral( "lastComposerTemplateDir" ), QDir::homePath(), QgsSettings::App ).toString();
#ifdef Q_OS_MAC
  QgisApp::instance()->activateWindow();
  this->raise();
#endif
  QString saveFileName = QFileDialog::getSaveFileName(
                           this,
                           tr( "Save template" ),
                           lastSaveDir,
                           tr( "Layout templates" ) + " (*.qpt *.QPT)" );
  if ( saveFileName.isEmpty() )
    return;

  QFileInfo saveFileInfo( saveFileName );
  //check if suffix has been added
  if ( saveFileInfo.suffix().isEmpty() )
  {
    QString saveFileNameWithSuffix = saveFileName.append( ".qpt" );
    saveFileInfo = QFileInfo( saveFileNameWithSuffix );
  }
  settings.setValue( QStringLiteral( "lastComposerTemplateDir" ), saveFileInfo.absolutePath(), QgsSettings::App );

  QgsReadWriteContext context;
  context.setPathResolver( QgsProject::instance()->pathResolver() );
  if ( !currentLayout()->saveAsTemplate( saveFileName, context ) )
  {
    QMessageBox::warning( this, tr( "Save template" ), tr( "Error creating template file." ) );
  }
}

void QgsLayoutDesignerDialog::addItemsFromTemplate()
{
  if ( !currentLayout() )
    return;

  QgsSettings settings;
  QString openFileDir = settings.value( QStringLiteral( "lastComposerTemplateDir" ), QDir::homePath(), QgsSettings::App ).toString();
  QString openFileString = QFileDialog::getOpenFileName( nullptr, tr( "Load template" ), openFileDir, tr( "Layout templates" ) + " (*.qpt *.QPT)" );

  if ( openFileString.isEmpty() )
  {
    return; //canceled by the user
  }

  QFileInfo openFileInfo( openFileString );
  settings.setValue( QStringLiteral( "LastComposerTemplateDir" ), openFileInfo.absolutePath(), QgsSettings::App );

  QFile templateFile( openFileString );
  if ( !templateFile.open( QIODevice::ReadOnly ) )
  {
    QMessageBox::warning( this, tr( "Load from template" ), tr( "Could not read template file." ) );
    return;
  }

  QDomDocument templateDoc;
  QgsReadWriteContext context;
  context.setPathResolver( QgsProject::instance()->pathResolver() );
  if ( templateDoc.setContent( &templateFile ) )
  {
    bool ok = false;
    QList< QgsLayoutItem * > items = currentLayout()->loadFromTemplate( templateDoc, context, false, &ok );
    if ( !ok )
    {
      QMessageBox::warning( this, tr( "Load from template" ), tr( "Could not read template file." ) );
      return;
    }
    else
    {
      whileBlocking( currentLayout() )->deselectAll();
      selectItems( items );
    }
  }
}

void QgsLayoutDesignerDialog::duplicate()
{
  QString newTitle;
  if ( !QgisApp::instance()->uniqueLayoutTitle( this, newTitle, false, masterLayout()->layoutType(), tr( "%1 copy" ).arg( masterLayout()->name() ) ) )
  {
    return;
  }

  // provide feedback, since loading of template into duplicate layout will be hidden
  QDialog *dlg = new QgsBusyIndicatorDialog( tr( "Duplicating layout…" ) );
  dlg->setStyleSheet( QgisApp::instance()->styleSheet() );
  dlg->show();

  QgsLayoutDesignerDialog *newDialog = QgisApp::instance()->duplicateLayout( mMasterLayout, newTitle );

  dlg->close();
  delete dlg;
  dlg = nullptr;

  if ( !newDialog )
  {
    QMessageBox::warning( this, tr( "Duplicate layout" ),
                          tr( "Layout duplication failed." ) );
  }
}

void QgsLayoutDesignerDialog::saveProject()
{
  QgisApp::instance()->actionSaveProject()->trigger();
}

void QgsLayoutDesignerDialog::newLayout()
{
  QString title;
  if ( !QgisApp::instance()->uniqueLayoutTitle( this, title, true, mMasterLayout->layoutType() ) )
  {
    return;
  }

  switch ( mMasterLayout->layoutType() )
  {
    case QgsMasterLayoutInterface::PrintLayout:
      QgisApp::instance()->createNewPrintLayout( title );
      break;

    case QgsMasterLayoutInterface::Report:
      QgisApp::instance()->createNewReport( title );
      break;
  }
}

void QgsLayoutDesignerDialog::showManager()
{
  // NOTE: Avoid crash where composer that spawned modal manager from toolbar ends up
  // being deleted by user, but event loop tries to return to layout on manager close
  // (does not seem to be an issue for menu action)
  QTimer::singleShot( 0, this, [ = ]
  {
    QgisApp::instance()->showLayoutManager();
  } );
}

void QgsLayoutDesignerDialog::renameLayout()
{
  QString currentTitle = masterLayout()->name();
  QString newTitle;
  if ( !QgisApp::instance()->uniqueLayoutTitle( this, newTitle, false, masterLayout()->layoutType(), currentTitle ) )
  {
    return;
  }
  masterLayout()->setName( newTitle );
}

void QgsLayoutDesignerDialog::deleteLayout()
{
  if ( QMessageBox::question( this, tr( "Delete Layout" ), tr( "Are you sure you want to delete the layout “%1”?" ).arg( masterLayout()->name() ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  masterLayout()->layoutProject()->layoutManager()->removeLayout( masterLayout() );
  close();
}

void QgsLayoutDesignerDialog::print()
{
  if ( containsWmsLayers() )
  {
    showWmsPrintingWarning();
  }

  if ( requiresRasterization() )
  {
    showRasterizationWarning();
  }

  if ( currentLayout()->pageCollection()->pageCount() == 0 )
    return;

  // get orientation from first page
  QgsLayoutItemPage::Orientation orientation = currentLayout()->pageCollection()->page( 0 )->orientation();

  //set printer page orientation
  setPrinterPageOrientation( orientation );

  QPrintDialog printDialog( printer(), nullptr );
  if ( printDialog.exec() != QDialog::Accepted )
  {
    return;
  }


  mView->setPaintingEnabled( false );
  QApplication::setOverrideCursor( Qt::BusyCursor );

  QgsLayoutExporter::PrintExportSettings printSettings;
  printSettings.rasterizeWholeImage = mLayout->customProperty( QStringLiteral( "rasterize" ), false ).toBool();

  // force a refresh, to e.g. update data defined properties, tables, etc
  mLayout->refresh();

  QgsLayoutExporter exporter( mLayout );
  QString printerName = printer()->printerName();
  switch ( exporter.print( *printer(), printSettings ) )
  {
    case QgsLayoutExporter::Success:
    {
      QString message;
      if ( !printerName.isEmpty() )
      {
        message =   tr( "Successfully printed layout to %1" ).arg( printerName );
      }
      else
      {
        message = tr( "Successfully printed layout" );
      }
      mMessageBar->pushMessage( tr( "Print layout" ),
                                message,
                                QgsMessageBar::SUCCESS, 0 );
      break;
    }

    case QgsLayoutExporter::PrintError:
    {
      QString message;
      if ( !printerName.isEmpty() )
      {
        message =   tr( "Could not create print device for %1" ).arg( printerName );
      }
      else
      {
        message = tr( "Could not create print device" );
      }
      QMessageBox::warning( this, tr( "Print layout" ),
                            message,
                            QMessageBox::Ok,
                            QMessageBox::Ok );
      break;
    }

    case QgsLayoutExporter::MemoryError:
      QMessageBox::warning( this, tr( "Memory Allocation Error" ),
                            tr( "Printing the layout "
                                "resulted in a memory overflow.\n\n"
                                "Please try a lower resolution or a smaller paper size." ),
                            QMessageBox::Ok, QMessageBox::Ok );
      break;

    case QgsLayoutExporter::FileError:
    case QgsLayoutExporter::SvgLayerError:
    case QgsLayoutExporter::IteratorError:
    case QgsLayoutExporter::Canceled:
      // no meaning for PDF exports, will not be encountered
      break;
  }

  mView->setPaintingEnabled( true );
  QApplication::restoreOverrideCursor();
}

void QgsLayoutDesignerDialog::exportToRaster()
{
  if ( containsWmsLayers() )
    showWmsPrintingWarning();

  if ( !showFileSizeWarning() )
    return;

  QgsSettings s;
  QString outputFileName = QgsFileUtils::stringToSafeFilename( mMasterLayout->name() );
  QgsLayoutAtlas *printAtlas = atlas();
  if ( printAtlas && printAtlas->enabled() && mActionAtlasPreview->isChecked() )
  {
    QString lastUsedDir = s.value( QStringLiteral( "lastSaveAsImageDir" ), QDir::homePath(), QgsSettings::App ).toString();
    outputFileName = QDir( lastUsedDir ).filePath( QgsFileUtils::stringToSafeFilename( printAtlas->currentFilename() ) );
  }

#ifdef Q_OS_MAC
  QgisApp::instance()->activateWindow();
  this->raise();
#endif
  QPair<QString, QString> fileNExt = QgsGuiUtils::getSaveAsImageName( this, tr( "Save layout as" ), outputFileName );
  this->activateWindow();

  if ( fileNExt.first.isEmpty() )
  {
    return;
  }

  QgsLayoutExporter::ImageExportSettings settings;
  QSize imageSize;
  if ( !getRasterExportSettings( settings, imageSize ) )
    return;

  mView->setPaintingEnabled( false );
  QApplication::setOverrideCursor( Qt::BusyCursor );

  // force a refresh, to e.g. update data defined properties, tables, etc
  mLayout->refresh();

  QgsLayoutExporter exporter( mLayout );

  QFileInfo fi( fileNExt.first );
  switch ( exporter.exportToImage( fileNExt.first, settings ) )
  {
    case QgsLayoutExporter::Success:
      mMessageBar->pushMessage( tr( "Export layout" ),
                                tr( "Successfully exported layout to <a href=\"%1\">%2</a>" ).arg( QUrl::fromLocalFile( fi.path() ).toString(), fileNExt.first ),
                                QgsMessageBar::SUCCESS, 0 );
      break;

    case QgsLayoutExporter::PrintError:
    case QgsLayoutExporter::SvgLayerError:
    case QgsLayoutExporter::IteratorError:
    case QgsLayoutExporter::Canceled:
      // no meaning for raster exports, will not be encountered
      break;

    case QgsLayoutExporter::FileError:
      QMessageBox::warning( this, tr( "Image Export Error" ),
                            tr( "Cannot write to %1.\n\nThis file may be open in another application." ).arg( exporter.errorFile() ),
                            QMessageBox::Ok,
                            QMessageBox::Ok );
      break;

    case QgsLayoutExporter::MemoryError:
      QMessageBox::warning( this, tr( "Memory Allocation Error" ),
                            tr( "Trying to create image %1 (%2×%3 @ %4dpi ) "
                                "resulted in a memory overflow.\n\n"
                                "Please try a lower resolution or a smaller paper size." )
                            .arg( exporter.errorFile() ).arg( imageSize.width() ).arg( imageSize.height() ).arg( settings.dpi ),
                            QMessageBox::Ok, QMessageBox::Ok );
      break;


  }
  QApplication::restoreOverrideCursor();
  mView->setPaintingEnabled( true );
}

void QgsLayoutDesignerDialog::exportToPdf()
{
  if ( containsWmsLayers() )
  {
    showWmsPrintingWarning();
  }

  if ( requiresRasterization() )
  {
    showRasterizationWarning();
  }

  if ( containsAdvancedEffects() && ( mLayout->customProperty( QStringLiteral( "forceVector" ), false ).toBool() ) )
  {
    showForceVectorWarning();
  }

  QgsSettings settings;
  QString lastUsedFile = settings.value( QStringLiteral( "lastSaveAsPdfFile" ), QStringLiteral( "qgis.pdf" ), QgsSettings::App ).toString();
  QFileInfo file( lastUsedFile );
  QString outputFileName;

  QgsLayoutAtlas *printAtlas = atlas();
  if ( printAtlas && printAtlas->enabled() && mActionAtlasPreview->isChecked() )
  {
    outputFileName = QDir( file.path() ).filePath( QgsFileUtils::stringToSafeFilename( printAtlas->currentFilename() ) + QStringLiteral( ".pdf" ) );
  }
  else
  {
    outputFileName = file.path() + '/' + QgsFileUtils::stringToSafeFilename( mMasterLayout->name() ) + QStringLiteral( ".pdf" );
  }

#ifdef Q_OS_MAC
  QgisApp::instance()->activateWindow();
  this->raise();
#endif
  outputFileName = QFileDialog::getSaveFileName(
                     this,
                     tr( "Export to PDF" ),
                     outputFileName,
                     tr( "PDF Format" ) + " (*.pdf *.PDF)" );
  this->activateWindow();
  if ( outputFileName.isEmpty() )
  {
    return;
  }

  if ( !outputFileName.endsWith( QLatin1String( ".pdf" ), Qt::CaseInsensitive ) )
  {
    outputFileName += QLatin1String( ".pdf" );
  }

  settings.setValue( QStringLiteral( "lastSaveAsPdfFile" ), outputFileName, QgsSettings::App );

  mView->setPaintingEnabled( false );
  QApplication::setOverrideCursor( Qt::BusyCursor );

  QgsLayoutExporter::PdfExportSettings pdfSettings;
  pdfSettings.rasterizeWholeImage = mLayout->customProperty( QStringLiteral( "rasterize" ), false ).toBool();
  pdfSettings.forceVectorOutput = mLayout->customProperty( QStringLiteral( "forceVector" ), false ).toBool();

  // force a refresh, to e.g. update data defined properties, tables, etc
  mLayout->refresh();

  QFileInfo fi( outputFileName );
  QgsLayoutExporter exporter( mLayout );
  switch ( exporter.exportToPdf( outputFileName, pdfSettings ) )
  {
    case QgsLayoutExporter::Success:
    {
      mMessageBar->pushMessage( tr( "Export layout" ),
                                tr( "Successfully exported layout to <a href=\"%1\">%2</a>" ).arg( QUrl::fromLocalFile( fi.path() ).toString(), outputFileName ),
                                QgsMessageBar::SUCCESS, 0 );
      break;
    }

    case QgsLayoutExporter::FileError:
      QMessageBox::warning( this, tr( "Export to PDF" ),
                            tr( "Cannot write to %1.\n\nThis file may be open in another application." ).arg( outputFileName ),
                            QMessageBox::Ok,
                            QMessageBox::Ok );
      break;

    case QgsLayoutExporter::PrintError:
      QMessageBox::warning( this, tr( "Export to PDF" ),
                            tr( "Could not create print device." ),
                            QMessageBox::Ok,
                            QMessageBox::Ok );
      break;


    case QgsLayoutExporter::MemoryError:
      QMessageBox::warning( this, tr( "Memory Allocation Error" ),
                            tr( "Exporting the PDF "
                                "resulted in a memory overflow.\n\n"
                                "Please try a lower resolution or a smaller paper size." ),
                            QMessageBox::Ok, QMessageBox::Ok );
      break;

    case QgsLayoutExporter::SvgLayerError:
    case QgsLayoutExporter::IteratorError:
    case QgsLayoutExporter::Canceled:
      // no meaning for PDF exports, will not be encountered
      break;
  }

  mView->setPaintingEnabled( true );
  QApplication::restoreOverrideCursor();
}

void QgsLayoutDesignerDialog::exportToSvg()
{
  if ( containsWmsLayers() )
  {
    showWmsPrintingWarning();
  }

  showSvgExportWarning();

  QgsSettings settings;
  QString lastUsedFile = settings.value( QStringLiteral( "lastSaveAsSvgFile" ), QStringLiteral( "qgis.svg" ), QgsSettings::App ).toString();
  QFileInfo file( lastUsedFile );
  QString outputFileName = QgsFileUtils::stringToSafeFilename( mMasterLayout->name() );

  QgsLayoutAtlas *printAtlas = atlas();
  if ( printAtlas && printAtlas->enabled() && mActionAtlasPreview->isChecked() )
  {
    outputFileName = QDir( file.path() ).filePath( QgsFileUtils::stringToSafeFilename( printAtlas->currentFilename() + QStringLiteral( ".svg" ) ) );
  }
  else
  {
    outputFileName = file.path() + '/' + QgsFileUtils::stringToSafeFilename( mMasterLayout->name() ) + QStringLiteral( ".svg" );
  }

#ifdef Q_OS_MAC
  QgisApp::instance()->activateWindow();
  this->raise();
#endif
  outputFileName = QFileDialog::getSaveFileName(
                     this,
                     tr( "Export to SVG" ),
                     outputFileName,
                     tr( "SVG Format" ) + " (*.svg *.SVG)" );
  this->activateWindow();
  if ( outputFileName.isEmpty() )
  {
    return;
  }

  if ( !outputFileName.endsWith( QLatin1String( ".svg" ), Qt::CaseInsensitive ) )
  {
    outputFileName += QLatin1String( ".svg" );
  }

  bool prevSettingLabelsAsOutlines = mLayout->project()->readBoolEntry( QStringLiteral( "PAL" ), QStringLiteral( "/DrawOutlineLabels" ), true );
  settings.setValue( QStringLiteral( "lastSaveAsSvgFile" ), outputFileName, QgsSettings::App );

  QgsLayoutExporter::SvgExportSettings svgSettings;
  bool exportAsText = false;
  if ( !getSvgExportSettings( svgSettings, exportAsText ) )
    return;

  //temporarily override label draw outlines setting
  mLayout->project()->writeEntry( QStringLiteral( "PAL" ), QStringLiteral( "/DrawOutlineLabels" ), exportAsText );

  mView->setPaintingEnabled( false );
  QApplication::setOverrideCursor( Qt::BusyCursor );

  // force a refresh, to e.g. update data defined properties, tables, etc
  mLayout->refresh();

  QFileInfo fi( outputFileName );
  QgsLayoutExporter exporter( mLayout );
  switch ( exporter.exportToSvg( outputFileName, svgSettings ) )
  {
    case QgsLayoutExporter::Success:
    {
      mMessageBar->pushMessage( tr( "Export layout" ),
                                tr( "Successfully exported layout to <a href=\"%1\">%2</a>" ).arg( QUrl::fromLocalFile( fi.path() ).toString(), outputFileName ),
                                QgsMessageBar::SUCCESS, 0 );
      break;
    }

    case QgsLayoutExporter::FileError:
      QMessageBox::warning( this, tr( "Export to SVG" ),
                            tr( "Cannot write to %1.\n\nThis file may be open in another application." ).arg( outputFileName ),
                            QMessageBox::Ok,
                            QMessageBox::Ok );
      break;

    case QgsLayoutExporter::SvgLayerError:
      QMessageBox::warning( this, tr( "Export to SVG" ),
                            tr( "Cannot create layered SVG file %1." ).arg( outputFileName ),
                            QMessageBox::Ok,
                            QMessageBox::Ok );
      break;

    case QgsLayoutExporter::PrintError:
      QMessageBox::warning( this, tr( "Export to SVG" ),
                            tr( "Could not create print device." ),
                            QMessageBox::Ok,
                            QMessageBox::Ok );
      break;


    case QgsLayoutExporter::MemoryError:
      QMessageBox::warning( this, tr( "Memory Allocation Error" ),
                            tr( "Exporting the SVG "
                                "resulted in a memory overflow.\n\n"
                                "Please try a lower resolution or a smaller paper size." ),
                            QMessageBox::Ok, QMessageBox::Ok );
      break;

    case QgsLayoutExporter::IteratorError:
    case QgsLayoutExporter::Canceled:
      // no meaning here
      break;
  }

  mView->setPaintingEnabled( true );
  mLayout->project()->writeEntry( QStringLiteral( "PAL" ), QStringLiteral( "/DrawOutlineLabels" ), prevSettingLabelsAsOutlines );
  QApplication::restoreOverrideCursor();
}

void QgsLayoutDesignerDialog::showAtlasSettings( bool checked )
{
  if ( !mAtlasDock )
    return;

  mAtlasDock->setUserVisible( checked );
}

void QgsLayoutDesignerDialog::atlasPreviewTriggered( bool checked )
{
  QgsPrintLayout *printLayout = qobject_cast< QgsPrintLayout * >( mLayout );
  if ( !printLayout )
    return;
  QgsLayoutAtlas *atlas = printLayout->atlas();

  //check if composition has an atlas map enabled
  if ( checked && !atlas->enabled() )
  {
    //no atlas current enabled
    mMessageBar->pushWarning( tr( "Atlas" ),
                              tr( "Atlas is not enabled for this layout!" ) );
    whileBlocking( mActionAtlasPreview )->setChecked( false );
    return;
  }

  //toggle other controls depending on whether atlas preview is active
  mActionAtlasFirst->setEnabled( checked );
  mActionAtlasLast->setEnabled( checked );
  mActionAtlasNext->setEnabled( checked );
  mActionAtlasPrev->setEnabled( checked );
  mAtlasPageComboBox->setEnabled( checked );

  if ( checked )
  {
    loadAtlasPredefinedScalesFromProject();
  }

  if ( checked )
  {
    if ( !atlas->beginRender() )
    {
      atlas->endRender();
      //something went wrong, e.g., no matching features
      mMessageBar->pushWarning( tr( "Atlas" ), tr( "No matching atlas features found!" ) );
      mActionAtlasPreview->blockSignals( true );
      mActionAtlasPreview->setChecked( false );
      mActionAtlasFirst->setEnabled( false );
      mActionAtlasLast->setEnabled( false );
      mActionAtlasNext->setEnabled( false );
      mActionAtlasPrev->setEnabled( false );
      mAtlasPageComboBox->setEnabled( false );
      mActionAtlasPreview->blockSignals( false );
    }
    else
    {
      QgisApp::instance()->mapCanvas()->stopRendering();
      atlas->first();
    }
  }
  else
  {
    atlas->endRender();
    mView->setSectionLabel( QString() );
  }
}

void QgsLayoutDesignerDialog::atlasPageComboEditingFinished()
{
  QString text = mAtlasPageComboBox->lineEdit()->text();

  //find matching record in combo box
  int page = -1; //note - first page starts at 1, not 0
  for ( int i = 0; i < mAtlasPageComboBox->count(); ++i )
  {
    if ( text.compare( mAtlasPageComboBox->itemData( i, Qt::UserRole + 1 ).toString(), Qt::CaseInsensitive ) == 0
         || text.compare( mAtlasPageComboBox->itemData( i, Qt::UserRole + 2 ).toString(), Qt::CaseInsensitive ) == 0
         || QString::number( i + 1 ) == text )
    {
      page = i + 1;
      break;
    }
  }
  bool ok = ( page > 0 );

  QgsPrintLayout *printLayout = qobject_cast< QgsPrintLayout * >( mLayout );
  if ( !printLayout )
    return;
  QgsLayoutAtlas *atlas = printLayout->atlas();

  if ( !ok || page > atlas->count() || page < 1 )
  {
    whileBlocking( mAtlasPageComboBox )->setCurrentIndex( atlas->currentFeatureNumber() );
  }
  else if ( page != atlas->currentFeatureNumber() + 1 )
  {
    QgisApp::instance()->mapCanvas()->stopRendering();
    loadAtlasPredefinedScalesFromProject();
    atlas->seekTo( page - 1 );
  }
}

void QgsLayoutDesignerDialog::atlasNext()
{
  QgsLayoutAtlas *printAtlas = atlas();
  if ( !printAtlas )
    return;

  QgisApp::instance()->mapCanvas()->stopRendering();

  loadAtlasPredefinedScalesFromProject();
  printAtlas->next();
}

void QgsLayoutDesignerDialog::atlasPrevious()
{
  QgsLayoutAtlas *printAtlas = atlas();
  if ( !printAtlas )
    return;

  QgisApp::instance()->mapCanvas()->stopRendering();

  loadAtlasPredefinedScalesFromProject();
  printAtlas->previous();
}

void QgsLayoutDesignerDialog::atlasFirst()
{
  QgsLayoutAtlas *printAtlas = atlas();
  if ( !printAtlas )
    return;

  QgisApp::instance()->mapCanvas()->stopRendering();

  loadAtlasPredefinedScalesFromProject();
  printAtlas->first();
}

void QgsLayoutDesignerDialog::atlasLast()
{
  QgsLayoutAtlas *printAtlas = atlas();
  if ( !printAtlas )
    return;

  QgisApp::instance()->mapCanvas()->stopRendering();

  loadAtlasPredefinedScalesFromProject();
  printAtlas->last();
}

void QgsLayoutDesignerDialog::printAtlas()
{
  QgsLayoutAtlas *printAtlas = atlas();
  if ( !printAtlas || !printAtlas->enabled() )
    return;

  loadAtlasPredefinedScalesFromProject();

  if ( containsWmsLayers() )
  {
    showWmsPrintingWarning();
  }

  if ( requiresRasterization() )
  {
    showRasterizationWarning();
  }

  if ( currentLayout()->pageCollection()->pageCount() == 0 )
    return;

  // get orientation from first page
  QgsLayoutItemPage::Orientation orientation = currentLayout()->pageCollection()->page( 0 )->orientation();

  //set printer page orientation
  setPrinterPageOrientation( orientation );

  QPrintDialog printDialog( printer(), nullptr );
  if ( printDialog.exec() != QDialog::Accepted )
  {
    return;
  }

  mView->setPaintingEnabled( false );
  QApplication::setOverrideCursor( Qt::BusyCursor );

  QgsLayoutExporter::PrintExportSettings printSettings;
  printSettings.rasterizeWholeImage = mLayout->customProperty( QStringLiteral( "rasterize" ), false ).toBool();

  QString error;
  std::unique_ptr< QgsFeedback > feedback = qgis::make_unique< QgsFeedback >();
  std::unique_ptr< QProgressDialog > progressDialog = qgis::make_unique< QProgressDialog >( tr( "Printing maps..." ), tr( "Abort" ), 0, 100, this );
  progressDialog->setWindowTitle( tr( "Printing Atlas" ) );
  connect( feedback.get(), &QgsFeedback::progressChanged, this, [ & ]( double progress )
  {
    progressDialog->setValue( progress );
    progressDialog->setLabelText( feedback->property( "progress" ).toString() ) ;

#ifdef Q_OS_LINUX
    // For some reason on Windows hasPendingEvents() always return true,
    // but one iteration is actually enough on Windows to get good interactivity
    // whereas on Linux we must allow for far more iterations.
    // For safety limit the number of iterations
    int nIters = 0;
    while ( QCoreApplication::hasPendingEvents() && ++nIters < 100 )
#endif
    {
      QCoreApplication::processEvents();
    }

  } );
  connect( progressDialog.get(), &QProgressDialog::canceled, this, [ & ]
  {
    feedback->cancel();
  } );

  QString printerName = printer()->printerName();
  switch ( QgsLayoutExporter::print( printAtlas, *printer(), printSettings, error, feedback.get() ) )
  {
    case QgsLayoutExporter::Success:
    {
      QString message;
      if ( !printerName.isEmpty() )
      {
        message =   tr( "Successfully printed atlas to %1" ).arg( printerName );
      }
      else
      {
        message = tr( "Successfully printed atlas" );
      }
      mMessageBar->pushMessage( tr( "Print atlas" ),
                                message,
                                QgsMessageBar::SUCCESS, 0 );
      break;
    }

    case QgsLayoutExporter::PrintError:
    {
      QString message;
      if ( !printerName.isEmpty() )
      {
        message =   tr( "Could not create print device for %1" ).arg( printerName );
      }
      else
      {
        message = tr( "Could not create print device" );
      }
      QMessageBox::warning( this, tr( "Print atlas" ),
                            message,
                            QMessageBox::Ok,
                            QMessageBox::Ok );
      break;
    }

    case QgsLayoutExporter::MemoryError:
      QMessageBox::warning( this, tr( "Memory Allocation Error" ),
                            tr( "Printing the layout "
                                "resulted in a memory overflow.\n\n"
                                "Please try a lower resolution or a smaller paper size." ),
                            QMessageBox::Ok, QMessageBox::Ok );
      break;

    case QgsLayoutExporter::IteratorError:
      QMessageBox::warning( this, tr( "Print Atlas" ),
                            tr( "Error encountered while printing atlas" ),
                            QMessageBox::Ok,
                            QMessageBox::Ok );
      break;

    case QgsLayoutExporter::FileError:
    case QgsLayoutExporter::SvgLayerError:
    case QgsLayoutExporter::Canceled:
      // no meaning for PDF exports, will not be encountered
      break;
  }

  mView->setPaintingEnabled( true );
  QApplication::restoreOverrideCursor();
}

void QgsLayoutDesignerDialog::exportAtlasToRaster()
{
  QgsLayoutAtlas *printAtlas = atlas();
  if ( !printAtlas || !printAtlas->enabled() )
    return;

  loadAtlasPredefinedScalesFromProject();

  // else, it has an atlas to render, so a directory must first be selected
  if ( printAtlas->filenameExpression().isEmpty() )
  {
    int res = QMessageBox::warning( nullptr, tr( "Export Atlas" ),
                                    tr( "The filename expression is empty. A default one will be used instead." ),
                                    QMessageBox::Ok | QMessageBox::Cancel,
                                    QMessageBox::Ok );
    if ( res == QMessageBox::Cancel )
    {
      return;
    }
    QString error;
    printAtlas->setFilenameExpression( QStringLiteral( "'output_'||@atlas_featurenumber" ), error );
  }

  QgsSettings s;
  QString lastUsedDir = s.value( QStringLiteral( "lastSaveAtlasAsImagesDir" ), QDir::homePath(), QgsSettings::App ).toString();

  QFileDialog dlg( this, tr( "Export Atlas to Directory" ) );
  dlg.setFileMode( QFileDialog::Directory );
  dlg.setOption( QFileDialog::ShowDirsOnly, true );
  dlg.setDirectory( lastUsedDir );
  if ( !dlg.exec() )
  {
    return;
  }

  const QStringList files = dlg.selectedFiles();
  if ( files.empty() || files.at( 0 ).isEmpty() )
  {
    return;
  }
  QString dir = files.at( 0 );
  QString format = mLayout->customProperty( QStringLiteral( "atlasRasterFormat" ), QStringLiteral( "png" ) ).toString();
  QString fileExt = '.' + format;
  if ( dir.isEmpty() )
  {
    return;
  }
  s.setValue( QStringLiteral( "lastSaveAtlasAsImagesDir" ), dir, QgsSettings::App );

  // test directory (if it exists and is writable)
  if ( !QDir( dir ).exists() || !QFileInfo( dir ).isWritable() )
  {
    QMessageBox::warning( nullptr, tr( "Unable to write into the directory" ),
                          tr( "The given output directory is not writable. Canceling." ),
                          QMessageBox::Ok,
                          QMessageBox::Ok );
    return;
  }

  if ( containsWmsLayers() )
    showWmsPrintingWarning();

  if ( !showFileSizeWarning() )
    return;

#ifdef Q_OS_MAC
  QgisApp::instance()->activateWindow();
  this->raise();
#endif

  QgsLayoutExporter::ImageExportSettings settings;
  QSize imageSize;
  if ( !getRasterExportSettings( settings, imageSize ) )
    return;

  mView->setPaintingEnabled( false );
  QApplication::setOverrideCursor( Qt::BusyCursor );

  QString error;
  std::unique_ptr< QgsFeedback > feedback = qgis::make_unique< QgsFeedback >();
  std::unique_ptr< QProgressDialog > progressDialog = qgis::make_unique< QProgressDialog >( tr( "Rendering maps..." ), tr( "Abort" ), 0, 100, this );
  progressDialog->setWindowTitle( tr( "Exporting Atlas" ) );
  connect( feedback.get(), &QgsFeedback::progressChanged, this, [ & ]( double progress )
  {
    progressDialog->setValue( progress );
    progressDialog->setLabelText( feedback->property( "progress" ).toString() ) ;

#ifdef Q_OS_LINUX
    // For some reason on Windows hasPendingEvents() always return true,
    // but one iteration is actually enough on Windows to get good interactivity
    // whereas on Linux we must allow for far more iterations.
    // For safety limit the number of iterations
    int nIters = 0;
    while ( QCoreApplication::hasPendingEvents() && ++nIters < 100 )
#endif
    {
      QCoreApplication::processEvents();
    }

  } );
  connect( progressDialog.get(), &QProgressDialog::canceled, this, [ & ]
  {
    feedback->cancel();
  } );

  QString fileName = QDir( dir ).filePath( QStringLiteral( "atlas" ) ); // filename is overridden by atlas
  QgsLayoutExporter::ExportResult result = QgsLayoutExporter::exportToImage( printAtlas, fileName, fileExt, settings, error, feedback.get() );
  QApplication::restoreOverrideCursor();

  switch ( result )
  {
    case QgsLayoutExporter::Success:
      mMessageBar->pushMessage( tr( "Export atlas" ),
                                tr( "Successfully exported atlas to <a href=\"%1\">%2</a>" ).arg( QUrl::fromLocalFile( dir ).toString(), dir ),
                                QgsMessageBar::SUCCESS, 0 );
      break;

    case QgsLayoutExporter::IteratorError:
      QMessageBox::warning( this, tr( "Atlas Export Error" ),
                            tr( "Error encountered while exporting atlas" ),
                            QMessageBox::Ok,
                            QMessageBox::Ok );
      break;

    case QgsLayoutExporter::PrintError:
    case QgsLayoutExporter::SvgLayerError:
    case QgsLayoutExporter::Canceled:
      // no meaning for raster exports, will not be encountered
      break;

    case QgsLayoutExporter::FileError:
      QMessageBox::warning( this, tr( "Image Export Error" ),
                            error,
                            QMessageBox::Ok,
                            QMessageBox::Ok );
      break;

    case QgsLayoutExporter::MemoryError:
      QMessageBox::warning( this, tr( "Memory Allocation Error" ),
                            tr( "Trying to create image of %2×%3 @ %4dpi "
                                "resulted in a memory overflow.\n\n"
                                "Please try a lower resolution or a smaller paper size." )
                            .arg( imageSize.width() ).arg( imageSize.height() ).arg( settings.dpi ),
                            QMessageBox::Ok, QMessageBox::Ok );
      break;
  }
  mView->setPaintingEnabled( true );
}

void QgsLayoutDesignerDialog::exportAtlasToSvg()
{
  QgsLayoutAtlas *printAtlas = atlas();
  if ( !printAtlas || !printAtlas->enabled() )
    return;

  loadAtlasPredefinedScalesFromProject();
  if ( containsWmsLayers() )
  {
    showWmsPrintingWarning();
  }

  showSvgExportWarning();

  // else, it has an atlas to render, so a directory must first be selected
  if ( printAtlas->filenameExpression().isEmpty() )
  {
    int res = QMessageBox::warning( nullptr, tr( "Export Atlas" ),
                                    tr( "The filename expression is empty. A default one will be used instead." ),
                                    QMessageBox::Ok | QMessageBox::Cancel,
                                    QMessageBox::Ok );
    if ( res == QMessageBox::Cancel )
    {
      return;
    }
    QString error;
    printAtlas->setFilenameExpression( QStringLiteral( "'output_'||@atlas_featurenumber" ), error );
  }

  QgsSettings s;
  QString lastUsedDir = s.value( QStringLiteral( "lastSaveAtlasAsSvgDir" ), QDir::homePath(), QgsSettings::App ).toString();

  QFileDialog dlg( this, tr( "Export Atlas to Directory" ) );
  dlg.setFileMode( QFileDialog::Directory );
  dlg.setOption( QFileDialog::ShowDirsOnly, true );
  dlg.setDirectory( lastUsedDir );
  if ( !dlg.exec() )
  {
    return;
  }

#ifdef Q_OS_MAC
  QgisApp::instance()->activateWindow();
  this->raise();
#endif

  const QStringList files = dlg.selectedFiles();
  if ( files.empty() || files.at( 0 ).isEmpty() )
  {
    return;
  }
  QString dir = files.at( 0 );
  if ( dir.isEmpty() )
  {
    return;
  }
  s.setValue( QStringLiteral( "lastSaveAtlasAsSvgDir" ), dir, QgsSettings::App );

  // test directory (if it exists and is writable)
  if ( !QDir( dir ).exists() || !QFileInfo( dir ).isWritable() )
  {
    QMessageBox::warning( nullptr, tr( "Unable to write into the directory" ),
                          tr( "The given output directory is not writable. Canceling." ),
                          QMessageBox::Ok,
                          QMessageBox::Ok );
    return;
  }

  bool prevSettingLabelsAsOutlines = mLayout->project()->readBoolEntry( QStringLiteral( "PAL" ), QStringLiteral( "/DrawOutlineLabels" ), true );
  QgsLayoutExporter::SvgExportSettings svgSettings;
  bool exportAsText = false;
  if ( !getSvgExportSettings( svgSettings, exportAsText ) )
    return;

  //temporarily override label draw outlines setting
  mLayout->project()->writeEntry( QStringLiteral( "PAL" ), QStringLiteral( "/DrawOutlineLabels" ), exportAsText );

  mView->setPaintingEnabled( false );
  QApplication::setOverrideCursor( Qt::BusyCursor );

  QString error;
  std::unique_ptr< QgsFeedback > feedback = qgis::make_unique< QgsFeedback >();
  std::unique_ptr< QProgressDialog > progressDialog = qgis::make_unique< QProgressDialog >( tr( "Rendering maps..." ), tr( "Abort" ), 0, 100, this );
  progressDialog->setWindowTitle( tr( "Exporting Atlas" ) );
  connect( feedback.get(), &QgsFeedback::progressChanged, this, [ & ]( double progress )
  {
    progressDialog->setValue( progress );
    progressDialog->setLabelText( feedback->property( "progress" ).toString() ) ;

#ifdef Q_OS_LINUX
    // For some reason on Windows hasPendingEvents() always return true,
    // but one iteration is actually enough on Windows to get good interactivity
    // whereas on Linux we must allow for far more iterations.
    // For safety limit the number of iterations
    int nIters = 0;
    while ( QCoreApplication::hasPendingEvents() && ++nIters < 100 )
#endif
    {
      QCoreApplication::processEvents();
    }

  } );
  connect( progressDialog.get(), &QProgressDialog::canceled, this, [ & ]
  {
    feedback->cancel();
  } );

  QString filename = QDir( dir ).filePath( QStringLiteral( "atlas" ) ); // filename is overridden by atlas
  QgsLayoutExporter::ExportResult result = QgsLayoutExporter::exportToSvg( printAtlas, filename, svgSettings, error, feedback.get() );

  QApplication::restoreOverrideCursor();
  switch ( result )
  {
    case QgsLayoutExporter::Success:
    {
      mMessageBar->pushMessage( tr( "Export atlas" ),
                                tr( "Successfully exported atlas to <a href=\"%1\">%2</a>" ).arg( QUrl::fromLocalFile( dir ).toString(), dir ),
                                QgsMessageBar::SUCCESS, 0 );
      break;
    }

    case QgsLayoutExporter::FileError:
      QMessageBox::warning( this, tr( "Export atlas" ),
                            error, QMessageBox::Ok,
                            QMessageBox::Ok );
      break;

    case QgsLayoutExporter::SvgLayerError:
      QMessageBox::warning( this, tr( "Export atlas" ),
                            tr( "Cannot create layered SVG file." ),
                            QMessageBox::Ok,
                            QMessageBox::Ok );
      break;

    case QgsLayoutExporter::PrintError:
      QMessageBox::warning( this, tr( "Export atlas" ),
                            tr( "Could not create print device." ),
                            QMessageBox::Ok,
                            QMessageBox::Ok );
      break;


    case QgsLayoutExporter::MemoryError:
      QMessageBox::warning( this, tr( "Memory Allocation Error" ),
                            tr( "Exporting the SVG "
                                "resulted in a memory overflow.\n\n"
                                "Please try a lower resolution or a smaller paper size." ),
                            QMessageBox::Ok, QMessageBox::Ok );
      break;

    case QgsLayoutExporter::IteratorError:
      QMessageBox::warning( this, tr( "Atlas Export Error" ),
                            tr( "Error encountered while exporting atlas" ),
                            QMessageBox::Ok,
                            QMessageBox::Ok );
      break;

    case QgsLayoutExporter::Canceled:
      // no meaning here
      break;
  }

  mView->setPaintingEnabled( true );
  mLayout->project()->writeEntry( QStringLiteral( "PAL" ), QStringLiteral( "/DrawOutlineLabels" ), prevSettingLabelsAsOutlines );
}

void QgsLayoutDesignerDialog::exportAtlasToPdf()
{
  QgsLayoutAtlas *printAtlas = atlas();
  if ( !printAtlas || !printAtlas->enabled() )
    return;

  loadAtlasPredefinedScalesFromProject();
  if ( containsWmsLayers() )
  {
    showWmsPrintingWarning();
  }

  if ( requiresRasterization() )
  {
    showRasterizationWarning();
  }

  if ( containsAdvancedEffects() && ( mLayout->customProperty( QStringLiteral( "forceVector" ), false ).toBool() ) )
  {
    showForceVectorWarning();
  }

  bool singleFile = mLayout->customProperty( QStringLiteral( "singleFile" ), true ).toBool();

  QString outputFileName;
  QgsSettings settings;
  if ( singleFile )
  {
    QString lastUsedFile = settings.value( QStringLiteral( "lastSaveAsPdfFile" ), QStringLiteral( "qgis.pdf" ), QgsSettings::App ).toString();
    QFileInfo file( lastUsedFile );
    outputFileName = file.path() + '/' + QgsFileUtils::stringToSafeFilename( mMasterLayout->name() ) + QStringLiteral( ".pdf" );

#ifdef Q_OS_MAC
    QgisApp::instance()->activateWindow();
    this->raise();
#endif
    outputFileName = QFileDialog::getSaveFileName(
                       this,
                       tr( "Export to PDF" ),
                       outputFileName,
                       tr( "PDF Format" ) + " (*.pdf *.PDF)" );
    this->activateWindow();
    if ( outputFileName.isEmpty() )
    {
      return;
    }

    if ( !outputFileName.endsWith( QLatin1String( ".pdf" ), Qt::CaseInsensitive ) )
    {
      outputFileName += QLatin1String( ".pdf" );
    }
    settings.setValue( QStringLiteral( "lastSaveAsPdfFile" ), outputFileName, QgsSettings::App );
  }
  else
  {
    if ( printAtlas->filenameExpression().isEmpty() )
    {
      int res = QMessageBox::warning( nullptr, tr( "Export Atlas" ),
                                      tr( "The filename expression is empty. A default one will be used instead." ),
                                      QMessageBox::Ok | QMessageBox::Cancel,
                                      QMessageBox::Ok );
      if ( res == QMessageBox::Cancel )
      {
        return;
      }
      QString error;
      printAtlas->setFilenameExpression( QStringLiteral( "'output_'||@atlas_featurenumber" ), error );
    }


    QString lastUsedDir = settings.value( QStringLiteral( "lastSaveAtlasAsPdfDir" ), QDir::homePath(), QgsSettings::App ).toString();

    QFileDialog dlg( this, tr( "Export Atlas to Directory" ) );
    dlg.setFileMode( QFileDialog::Directory );
    dlg.setOption( QFileDialog::ShowDirsOnly, true );
    dlg.setDirectory( lastUsedDir );
    if ( !dlg.exec() )
    {
      return;
    }

#ifdef Q_OS_MAC
    QgisApp::instance()->activateWindow();
    this->raise();
#endif

    const QStringList files = dlg.selectedFiles();
    if ( files.empty() || files.at( 0 ).isEmpty() )
    {
      return;
    }
    QString dir = files.at( 0 );
    if ( dir.isEmpty() )
    {
      return;
    }
    settings.setValue( QStringLiteral( "lastSaveAtlasAsPdfDir" ), dir, QgsSettings::App );

    // test directory (if it exists and is writable)
    if ( !QDir( dir ).exists() || !QFileInfo( dir ).isWritable() )
    {
      QMessageBox::warning( nullptr, tr( "Unable to write into the directory" ),
                            tr( "The given output directory is not writable. Canceling." ),
                            QMessageBox::Ok,
                            QMessageBox::Ok );
      return;
    }

    outputFileName = QDir( dir ).filePath( QStringLiteral( "atlas" ) ); // filename is overridden by atlas
  }

  mView->setPaintingEnabled( false );
  QApplication::setOverrideCursor( Qt::BusyCursor );

  QgsLayoutExporter::PdfExportSettings pdfSettings;
  pdfSettings.rasterizeWholeImage = mLayout->customProperty( QStringLiteral( "rasterize" ), false ).toBool();
  pdfSettings.forceVectorOutput = mLayout->customProperty( QStringLiteral( "forceVector" ), false ).toBool();

  QFileInfo fi( outputFileName );

  QString error;
  std::unique_ptr< QgsFeedback > feedback = qgis::make_unique< QgsFeedback >();
  std::unique_ptr< QProgressDialog > progressDialog = qgis::make_unique< QProgressDialog >( tr( "Rendering maps..." ), tr( "Abort" ), 0, 100, this );
  progressDialog->setWindowTitle( tr( "Exporting Atlas" ) );
  connect( feedback.get(), &QgsFeedback::progressChanged, this, [ & ]( double progress )
  {
    progressDialog->setValue( progress );
    progressDialog->setLabelText( feedback->property( "progress" ).toString() ) ;

#ifdef Q_OS_LINUX
    // For some reason on Windows hasPendingEvents() always return true,
    // but one iteration is actually enough on Windows to get good interactivity
    // whereas on Linux we must allow for far more iterations.
    // For safety limit the number of iterations
    int nIters = 0;
    while ( QCoreApplication::hasPendingEvents() && ++nIters < 100 )
#endif
    {
      QCoreApplication::processEvents();
    }

  } );
  connect( progressDialog.get(), &QProgressDialog::canceled, this, [ & ]
  {
    feedback->cancel();
  } );

  QgsLayoutExporter::ExportResult result = QgsLayoutExporter::Success;
  if ( singleFile )
  {
    result = QgsLayoutExporter::exportToPdf( printAtlas, outputFileName, pdfSettings, error, feedback.get() );
  }
  else
  {
    result = QgsLayoutExporter::exportToPdfs( printAtlas, outputFileName, pdfSettings, error, feedback.get() );
  }

  switch ( result )
  {
    case QgsLayoutExporter::Success:
    {
      if ( singleFile )
      {
        mMessageBar->pushMessage( tr( "Export atlas" ),
                                  tr( "Successfully exported atlas to <a href=\"%1\">%2</a>" ).arg( QUrl::fromLocalFile( fi.path() ).toString(), outputFileName ),
                                  QgsMessageBar::SUCCESS, 0 );
      }
      else
      {
        mMessageBar->pushMessage( tr( "Export atlas" ),
                                  tr( "Successfully exported atlas to <a href=\"%1\">%2</a>" ).arg( QUrl::fromLocalFile( outputFileName ).toString(), outputFileName ),
                                  QgsMessageBar::SUCCESS, 0 );
      }
      break;
    }

    case QgsLayoutExporter::FileError:
      QMessageBox::warning( this, tr( "Export atlas" ),
                            error, QMessageBox::Ok,
                            QMessageBox::Ok );
      break;

    case QgsLayoutExporter::SvgLayerError:
      // no meaning
      break;

    case QgsLayoutExporter::PrintError:
      QMessageBox::warning( this, tr( "Export atlas" ),
                            tr( "Could not create print device." ),
                            QMessageBox::Ok,
                            QMessageBox::Ok );
      break;


    case QgsLayoutExporter::MemoryError:
      QMessageBox::warning( this, tr( "Memory Allocation Error" ),
                            tr( "Exporting the PDF "
                                "resulted in a memory overflow.\n\n"
                                "Please try a lower resolution or a smaller paper size." ),
                            QMessageBox::Ok, QMessageBox::Ok );
      break;

    case QgsLayoutExporter::IteratorError:
      QMessageBox::warning( this, tr( "Atlas Export Error" ),
                            tr( "Error encountered while exporting atlas" ),
                            QMessageBox::Ok,
                            QMessageBox::Ok );
      break;

    case QgsLayoutExporter::Canceled:
      // no meaning here
      break;
  }

  mView->setPaintingEnabled( true );
  QApplication::restoreOverrideCursor();
}

void QgsLayoutDesignerDialog::exportReportToRaster()
{
  QgsSettings s;
  QString outputFileName = QgsFileUtils::stringToSafeFilename( mMasterLayout->name() );

  QPair<QString, QString> fileNExt = QgsGuiUtils::getSaveAsImageName( this, tr( "Save report as" ), outputFileName );
  this->activateWindow();

  if ( fileNExt.first.isEmpty() )
  {
    return;
  }

#ifdef Q_OS_MAC
  QgisApp::instance()->activateWindow();
  this->raise();
#endif

  QgsLayoutExporter::ImageExportSettings settings;
  QSize imageSize;
  if ( !getRasterExportSettings( settings, imageSize ) )
    return;

  mView->setPaintingEnabled( false );
  QApplication::setOverrideCursor( Qt::BusyCursor );

  QString error;
  std::unique_ptr< QgsFeedback > feedback = qgis::make_unique< QgsFeedback >();
  std::unique_ptr< QProgressDialog > progressDialog = qgis::make_unique< QProgressDialog >( tr( "Rendering report..." ), tr( "Abort" ), 0, 0, this );
  progressDialog->setWindowTitle( tr( "Exporting Report" ) );
  connect( feedback.get(), &QgsFeedback::progressChanged, this, [ & ]( double )
  {
    //progressDialog->setValue( progress );
    progressDialog->setLabelText( feedback->property( "progress" ).toString() ) ;

#ifdef Q_OS_LINUX
    // For some reason on Windows hasPendingEvents() always return true,
    // but one iteration is actually enough on Windows to get good interactivity
    // whereas on Linux we must allow for far more iterations.
    // For safety limit the number of iterations
    int nIters = 0;
    while ( QCoreApplication::hasPendingEvents() && ++nIters < 100 )
#endif
    {
      QCoreApplication::processEvents();
    }

  } );
  connect( progressDialog.get(), &QProgressDialog::canceled, this, [ & ]
  {
    feedback->cancel();
  } );

  QFileInfo fi( fileNExt.first );
  QString dir = fi.path();
  QString fileName = dir + '/' + fi.baseName();
  QgsLayoutExporter::ExportResult result = QgsLayoutExporter::exportToImage( dynamic_cast< QgsReport * >( mMasterLayout ), fileName, fileNExt.second, settings, error, feedback.get() );
  QApplication::restoreOverrideCursor();

  switch ( result )
  {
    case QgsLayoutExporter::Success:
      mMessageBar->pushMessage( tr( "Export report" ),
                                tr( "Successfully exported report to <a href=\"%1\">%2</a>" ).arg( QUrl::fromLocalFile( dir ).toString(), dir ),
                                QgsMessageBar::SUCCESS, 0 );
      break;

    case QgsLayoutExporter::IteratorError:
      QMessageBox::warning( this, tr( "Report Export Error" ),
                            tr( "Error encountered while exporting report" ),
                            QMessageBox::Ok,
                            QMessageBox::Ok );
      break;

    case QgsLayoutExporter::PrintError:
    case QgsLayoutExporter::SvgLayerError:
    case QgsLayoutExporter::Canceled:
      // no meaning for raster exports, will not be encountered
      break;

    case QgsLayoutExporter::FileError:
      QMessageBox::warning( this, tr( "Image Export Error" ),
                            error,
                            QMessageBox::Ok,
                            QMessageBox::Ok );
      break;

    case QgsLayoutExporter::MemoryError:
      QMessageBox::warning( this, tr( "Memory Allocation Error" ),
                            tr( "Trying to create image of %2×%3 @ %4dpi "
                                "resulted in a memory overflow.\n\n"
                                "Please try a lower resolution or a smaller paper size." )
                            .arg( imageSize.width() ).arg( imageSize.height() ).arg( settings.dpi ),
                            QMessageBox::Ok, QMessageBox::Ok );
      break;
  }
  mView->setPaintingEnabled( true );
}

void QgsLayoutDesignerDialog::exportReportToSvg()
{
  showSvgExportWarning();

  QgsSettings settings;
  QString lastUsedFile = settings.value( QStringLiteral( "lastSaveAsSvgFile" ), QStringLiteral( "qgis.svg" ), QgsSettings::App ).toString();
  QFileInfo file( lastUsedFile );
  QString outputFileName = file.path() + '/' + QgsFileUtils::stringToSafeFilename( mMasterLayout->name() ) + QStringLiteral( ".svg" );

  outputFileName = QFileDialog::getSaveFileName(
                     this,
                     tr( "Export to SVG" ),
                     outputFileName,
                     tr( "SVG Format" ) + " (*.svg *.SVG)" );
  this->activateWindow();
  if ( outputFileName.isEmpty() )
  {
    return;
  }

  if ( !outputFileName.endsWith( QLatin1String( ".svg" ), Qt::CaseInsensitive ) )
  {
    outputFileName += QLatin1String( ".svg" );
  }
#ifdef Q_OS_MAC
  QgisApp::instance()->activateWindow();
  this->raise();
#endif
  bool prevSettingLabelsAsOutlines = mMasterLayout->layoutProject()->readBoolEntry( QStringLiteral( "PAL" ), QStringLiteral( "/DrawOutlineLabels" ), true );
  settings.setValue( QStringLiteral( "lastSaveAsSvgFile" ), outputFileName, QgsSettings::App );

  QgsLayoutExporter::SvgExportSettings svgSettings;
  bool exportAsText = false;
  if ( !getSvgExportSettings( svgSettings, exportAsText ) )
    return;

  //temporarily override label draw outlines setting
  mMasterLayout->layoutProject()->writeEntry( QStringLiteral( "PAL" ), QStringLiteral( "/DrawOutlineLabels" ), exportAsText );

  mView->setPaintingEnabled( false );
  QApplication::setOverrideCursor( Qt::BusyCursor );

  QString error;
  std::unique_ptr< QgsFeedback > feedback = qgis::make_unique< QgsFeedback >();
  std::unique_ptr< QProgressDialog > progressDialog = qgis::make_unique< QProgressDialog >( tr( "Rendering maps..." ), tr( "Abort" ), 0, 0, this );
  progressDialog->setWindowTitle( tr( "Exporting Report" ) );
  connect( feedback.get(), &QgsFeedback::progressChanged, this, [ & ]( double )
  {
    //progressDialog->setValue( progress );
    progressDialog->setLabelText( feedback->property( "progress" ).toString() ) ;

#ifdef Q_OS_LINUX
    // For some reason on Windows hasPendingEvents() always return true,
    // but one iteration is actually enough on Windows to get good interactivity
    // whereas on Linux we must allow for far more iterations.
    // For safety limit the number of iterations
    int nIters = 0;
    while ( QCoreApplication::hasPendingEvents() && ++nIters < 100 )
#endif
    {
      QCoreApplication::processEvents();
    }

  } );
  connect( progressDialog.get(), &QProgressDialog::canceled, this, [ & ]
  {
    feedback->cancel();
  } );

  QFileInfo fi( outputFileName );
  QString outFile = fi.path() + '/' + fi.baseName();
  QString dir = fi.path();
  QgsLayoutExporter::ExportResult result = QgsLayoutExporter::exportToSvg( dynamic_cast< QgsReport * >( mMasterLayout ), outFile, svgSettings, error, feedback.get() );

  QApplication::restoreOverrideCursor();
  switch ( result )
  {
    case QgsLayoutExporter::Success:
    {
      mMessageBar->pushMessage( tr( "Export report" ),
                                tr( "Successfully exported report to <a href=\"%1\">%2</a>" ).arg( QUrl::fromLocalFile( dir ).toString(), dir ),
                                QgsMessageBar::SUCCESS, 0 );
      break;
    }

    case QgsLayoutExporter::FileError:
      QMessageBox::warning( this, tr( "Export report" ),
                            error, QMessageBox::Ok,
                            QMessageBox::Ok );
      break;

    case QgsLayoutExporter::SvgLayerError:
      QMessageBox::warning( this, tr( "Export report" ),
                            tr( "Cannot create layered SVG file." ),
                            QMessageBox::Ok,
                            QMessageBox::Ok );
      break;

    case QgsLayoutExporter::PrintError:
      QMessageBox::warning( this, tr( "Export report" ),
                            tr( "Could not create print device." ),
                            QMessageBox::Ok,
                            QMessageBox::Ok );
      break;


    case QgsLayoutExporter::MemoryError:
      QMessageBox::warning( this, tr( "Memory Allocation Error" ),
                            tr( "Exporting the SVG "
                                "resulted in a memory overflow.\n\n"
                                "Please try a lower resolution or a smaller paper size." ),
                            QMessageBox::Ok, QMessageBox::Ok );
      break;

    case QgsLayoutExporter::IteratorError:
      QMessageBox::warning( this, tr( "Report Export Error" ),
                            tr( "Error encountered while exporting report" ),
                            QMessageBox::Ok,
                            QMessageBox::Ok );
      break;

    case QgsLayoutExporter::Canceled:
      // no meaning here
      break;
  }

  mView->setPaintingEnabled( true );
  mMasterLayout->layoutProject()->writeEntry( QStringLiteral( "PAL" ), QStringLiteral( "/DrawOutlineLabels" ), prevSettingLabelsAsOutlines );
}

void QgsLayoutDesignerDialog::exportReportToPdf()
{
  QgsSettings settings;

  QString lastUsedFile = settings.value( QStringLiteral( "lastSaveAsPdfFile" ), QStringLiteral( "qgis.pdf" ), QgsSettings::App ).toString();
  QFileInfo file( lastUsedFile );

  QString outputFileName = file.path() + '/' + QgsFileUtils::stringToSafeFilename( mMasterLayout->name() ) + QStringLiteral( ".pdf" );

#ifdef Q_OS_MAC
  QgisApp::instance()->activateWindow();
  this->raise();
#endif
  outputFileName = QFileDialog::getSaveFileName(
                     this,
                     tr( "Export to PDF" ),
                     outputFileName,
                     tr( "PDF Format" ) + " (*.pdf *.PDF)" );
  this->activateWindow();
  if ( outputFileName.isEmpty() )
  {
    return;
  }

  if ( !outputFileName.endsWith( QLatin1String( ".pdf" ), Qt::CaseInsensitive ) )
  {
    outputFileName += QLatin1String( ".pdf" );
  }
  settings.setValue( QStringLiteral( "lastSaveAsPdfFile" ), outputFileName, QgsSettings::App );

  mView->setPaintingEnabled( false );
  QApplication::setOverrideCursor( Qt::BusyCursor );

  bool rasterize = false;
  bool forceVectorOutput = false;
  if ( mLayout )
  {
    rasterize = mLayout->customProperty( QStringLiteral( "rasterize" ), false ).toBool();
    forceVectorOutput = mLayout->customProperty( QStringLiteral( "forceVector" ), false ).toBool();
  }
  QgsLayoutExporter::PdfExportSettings pdfSettings;
  pdfSettings.rasterizeWholeImage = rasterize;
  pdfSettings.forceVectorOutput = forceVectorOutput;

  QFileInfo fi( outputFileName );

  QString error;
  std::unique_ptr< QgsFeedback > feedback = qgis::make_unique< QgsFeedback >();
  std::unique_ptr< QProgressDialog > progressDialog = qgis::make_unique< QProgressDialog >( tr( "Rendering maps..." ), tr( "Abort" ), 0, 0, this );
  progressDialog->setWindowTitle( tr( "Exporting Report" ) );
  connect( feedback.get(), &QgsFeedback::progressChanged, this, [ & ]( double )
  {
    //progressDialog->setValue( progress );
    progressDialog->setLabelText( feedback->property( "progress" ).toString() ) ;

#ifdef Q_OS_LINUX
    // For some reason on Windows hasPendingEvents() always return true,
    // but one iteration is actually enough on Windows to get good interactivity
    // whereas on Linux we must allow for far more iterations.
    // For safety limit the number of iterations
    int nIters = 0;
    while ( QCoreApplication::hasPendingEvents() && ++nIters < 100 )
#endif
    {
      QCoreApplication::processEvents();
    }

  } );
  connect( progressDialog.get(), &QProgressDialog::canceled, this, [ & ]
  {
    feedback->cancel();
  } );

  QgsLayoutExporter::ExportResult result = QgsLayoutExporter::exportToPdf( dynamic_cast< QgsReport * >( mMasterLayout ), outputFileName, pdfSettings, error, feedback.get() );

  switch ( result )
  {
    case QgsLayoutExporter::Success:
    {
      mMessageBar->pushMessage( tr( "Export report" ),
                                tr( "Successfully exported report to <a href=\"%1\">%2</a>" ).arg( QUrl::fromLocalFile( fi.path() ).toString(), outputFileName ),
                                QgsMessageBar::SUCCESS, 0 );
      break;
    }

    case QgsLayoutExporter::FileError:
      QMessageBox::warning( this, tr( "Export report" ),
                            error, QMessageBox::Ok,
                            QMessageBox::Ok );
      break;

    case QgsLayoutExporter::SvgLayerError:
      // no meaning
      break;

    case QgsLayoutExporter::PrintError:
      QMessageBox::warning( this, tr( "Export report" ),
                            tr( "Could not create print device." ),
                            QMessageBox::Ok,
                            QMessageBox::Ok );
      break;


    case QgsLayoutExporter::MemoryError:
      QMessageBox::warning( this, tr( "Memory Allocation Error" ),
                            tr( "Exporting the PDF "
                                "resulted in a memory overflow.\n\n"
                                "Please try a lower resolution or a smaller paper size." ),
                            QMessageBox::Ok, QMessageBox::Ok );
      break;

    case QgsLayoutExporter::IteratorError:
      QMessageBox::warning( this, tr( "Report Export Error" ),
                            tr( "Error encountered while exporting report" ),
                            QMessageBox::Ok,
                            QMessageBox::Ok );
      break;

    case QgsLayoutExporter::Canceled:
      // no meaning here
      break;
  }

  mView->setPaintingEnabled( true );
  QApplication::restoreOverrideCursor();
}

void QgsLayoutDesignerDialog::printReport()
{
  QPrintDialog printDialog( printer(), nullptr );
  if ( printDialog.exec() != QDialog::Accepted )
  {
    return;
  }

  mView->setPaintingEnabled( false );
  QApplication::setOverrideCursor( Qt::BusyCursor );

  QgsLayoutExporter::PrintExportSettings printSettings;
  if ( mLayout )
    printSettings.rasterizeWholeImage = mLayout->customProperty( QStringLiteral( "rasterize" ), false ).toBool();

  QString error;
  std::unique_ptr< QgsFeedback > feedback = qgis::make_unique< QgsFeedback >();
  std::unique_ptr< QProgressDialog > progressDialog = qgis::make_unique< QProgressDialog >( tr( "Printing maps..." ), tr( "Abort" ), 0, 0, this );
  progressDialog->setWindowTitle( tr( "Printing Report" ) );
  connect( feedback.get(), &QgsFeedback::progressChanged, this, [ & ]( double )
  {
    //progressDialog->setValue( progress );
    progressDialog->setLabelText( feedback->property( "progress" ).toString() ) ;

#ifdef Q_OS_LINUX
    // For some reason on Windows hasPendingEvents() always return true,
    // but one iteration is actually enough on Windows to get good interactivity
    // whereas on Linux we must allow for far more iterations.
    // For safety limit the number of iterations
    int nIters = 0;
    while ( QCoreApplication::hasPendingEvents() && ++nIters < 100 )
#endif
    {
      QCoreApplication::processEvents();
    }

  } );
  connect( progressDialog.get(), &QProgressDialog::canceled, this, [ & ]
  {
    feedback->cancel();
  } );

  QString printerName = printer()->printerName();
  switch ( QgsLayoutExporter::print( dynamic_cast< QgsReport * >( mMasterLayout ), *printer(), printSettings, error, feedback.get() ) )
  {
    case QgsLayoutExporter::Success:
    {
      QString message;
      if ( !printerName.isEmpty() )
      {
        message =   tr( "Successfully printed report to %1" ).arg( printerName );
      }
      else
      {
        message = tr( "Successfully printed report" );
      }
      mMessageBar->pushMessage( tr( "Print report" ),
                                message,
                                QgsMessageBar::SUCCESS, 0 );
      break;
    }

    case QgsLayoutExporter::PrintError:
    {
      QString message;
      if ( !printerName.isEmpty() )
      {
        message =   tr( "Could not create print device for %1" ).arg( printerName );
      }
      else
      {
        message = tr( "Could not create print device" );
      }
      QMessageBox::warning( this, tr( "Print report" ),
                            message,
                            QMessageBox::Ok,
                            QMessageBox::Ok );
      break;
    }

    case QgsLayoutExporter::MemoryError:
      QMessageBox::warning( this, tr( "Memory Allocation Error" ),
                            tr( "Printing the report "
                                "resulted in a memory overflow.\n\n"
                                "Please try a lower resolution or a smaller paper size." ),
                            QMessageBox::Ok, QMessageBox::Ok );
      break;

    case QgsLayoutExporter::IteratorError:
      QMessageBox::warning( this, tr( "Print Report" ),
                            tr( "Error encountered while printing report" ),
                            QMessageBox::Ok,
                            QMessageBox::Ok );
      break;

    case QgsLayoutExporter::FileError:
    case QgsLayoutExporter::SvgLayerError:
    case QgsLayoutExporter::Canceled:
      // no meaning for PDF exports, will not be encountered
      break;
  }

  mView->setPaintingEnabled( true );
  QApplication::restoreOverrideCursor();
}

void QgsLayoutDesignerDialog::showReportSettings( bool checked )
{
  if ( !mReportDock )
    return;

  mReportDock->setUserVisible( checked );
}

void QgsLayoutDesignerDialog::pageSetup()
{
  if ( currentLayout() && currentLayout()->pageCollection()->pageCount() > 0 )
  {
    // get orientation from first page
    QgsLayoutItemPage::Orientation orientation = currentLayout()->pageCollection()->page( 0 )->orientation();
    //set printer page orientation
    setPrinterPageOrientation( orientation );
  }

  QPageSetupDialog pageSetupDialog( printer(), this );
  pageSetupDialog.exec();
}

void QgsLayoutDesignerDialog::pageOrientationChanged()
{
  mSetPageOrientation = false;
}

void QgsLayoutDesignerDialog::populateLayoutsMenu()
{
  QgisApp::instance()->populateLayoutsMenu( mLayoutsMenu );
}

void QgsLayoutDesignerDialog::paste()
{
  QPointF pt = mView->mapFromGlobal( QCursor::pos() );
  //TODO - use a better way of determining whether paste was triggered by keystroke
  //or menu item
  QList< QgsLayoutItem * > items;
  if ( ( pt.x() < 0 ) || ( pt.y() < 0 ) )
  {
    //action likely triggered by menu, paste items in center of screen
    items = mView->pasteItems( QgsLayoutView::PasteModeCenter );
  }
  else
  {
    //action likely triggered by keystroke, paste items at cursor position
    items = mView->pasteItems( QgsLayoutView::PasteModeCursor );
  }

  whileBlocking( currentLayout() )->deselectAll();
  selectItems( items );

  //switch back to select tool so that pasted items can be moved/resized (#8958)
  mView->setTool( mSelectTool );
}

void QgsLayoutDesignerDialog::pasteInPlace()
{
  QList< QgsLayoutItem * > items = mView->pasteItems( QgsLayoutView::PasteModeInPlace );

  whileBlocking( currentLayout() )->deselectAll();
  selectItems( items );

  //switch back to select tool so that pasted items can be moved/resized (#8958)
  mView->setTool( mSelectTool );
}

QgsLayoutView *QgsLayoutDesignerDialog::view()
{
  return mView;
}

void QgsLayoutDesignerDialog::saveWindowState()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "LayoutDesigner/geometry" ), saveGeometry(), QgsSettings::App );
  // store the toolbar/dock widget settings using Qt settings API
  settings.setValue( QStringLiteral( "LayoutDesigner/state" ), saveState(), QgsSettings::App );
}

void QgsLayoutDesignerDialog::restoreWindowState()
{
  // restore the toolbar and dock widgets positions using Qt settings API
  QgsSettings settings;

  //TODO - defaults
  if ( !restoreState( settings.value( QStringLiteral( "LayoutDesigner/state" ), QVariant(),  QgsSettings::App /*, QByteArray::fromRawData( ( char * )defaultComposerUIstate, sizeof defaultComposerUIstate ) */ ).toByteArray() ) )
  {
    QgsDebugMsg( "restore of layout UI state failed" );
  }
  // restore window geometry
  if ( !restoreGeometry( settings.value( QStringLiteral( "LayoutDesigner/geometry" ), QVariant(),  QgsSettings::App /*, QByteArray::fromRawData( ( char * )defaultComposerUIgeometry, sizeof defaultComposerUIgeometry ) */ ).toByteArray() ) )
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

  mLayoutPropertiesWidget = new QgsLayoutPropertiesWidget( mGeneralDock, mLayout );
  mLayoutPropertiesWidget->setDockMode( true );
  mGeneralPropertiesStack->setMainPanel( mLayoutPropertiesWidget );

  QgsLayoutGuideWidget *guideWidget = new QgsLayoutGuideWidget( mGuideDock, mLayout, mView );
  guideWidget->setDockMode( true );
  mGuideStack->setMainPanel( guideWidget );
}

void QgsLayoutDesignerDialog::createAtlasWidget()
{
  QgsPrintLayout *printLayout = dynamic_cast< QgsPrintLayout * >( mMasterLayout );
  QgsLayoutAtlas *atlas = printLayout->atlas();
  QgsLayoutAtlasWidget *atlasWidget = new QgsLayoutAtlasWidget( mAtlasDock, printLayout );
  atlasWidget->setMessageBar( mMessageBar );
  mAtlasDock->setWidget( atlasWidget );

  mAtlasToolbar->show();
  mPanelsMenu->addAction( mAtlasDock->toggleViewAction() );

  connect( atlas, &QgsLayoutAtlas::messagePushed, mStatusBar, [ = ]( const QString & message )
  {
    mStatusBar->showMessage( message );
  } );
  connect( atlas, &QgsLayoutAtlas::toggled, this, &QgsLayoutDesignerDialog::toggleAtlasControls );
  connect( atlas, &QgsLayoutAtlas::toggled, this, &QgsLayoutDesignerDialog::refreshLayout );
  connect( atlas, &QgsLayoutAtlas::numberFeaturesChanged, this, &QgsLayoutDesignerDialog::updateAtlasPageComboBox );
  connect( atlas, &QgsLayoutAtlas::featureChanged, this, &QgsLayoutDesignerDialog::atlasFeatureChanged );
  toggleAtlasControls( atlas->enabled() && atlas->coverageLayer() );
}

void QgsLayoutDesignerDialog::createReportWidget()
{
  QgsReport *report = dynamic_cast< QgsReport * >( mMasterLayout );
  QgsReportOrganizerWidget *reportWidget = new QgsReportOrganizerWidget( mReportDock, this, report );
  reportWidget->setMessageBar( mMessageBar );
  mReportDock->setWidget( reportWidget );
  mReportToolbar->show();
  mPanelsMenu->addAction( mReportDock->toggleViewAction() );
}

void QgsLayoutDesignerDialog::initializeRegistry()
{
  sInitializedRegistry = true;
  auto createPageWidget = ( [this]( QgsLayoutItem * item )->QgsLayoutItemBaseWidget *
  {
    std::unique_ptr< QgsLayoutPagePropertiesWidget > newWidget = qgis::make_unique< QgsLayoutPagePropertiesWidget >( nullptr, item );
    return newWidget.release();
  } );

  QgsGui::layoutItemGuiRegistry()->addLayoutItemGuiMetadata( new QgsLayoutItemGuiMetadata( QgsLayoutItemRegistry::LayoutPage, QObject::tr( "Page" ), QIcon(), createPageWidget, nullptr, QString(), false, QgsLayoutItemAbstractGuiMetadata::FlagNoCreationTools ) );

}

bool QgsLayoutDesignerDialog::containsWmsLayers() const
{
  QList< QgsLayoutItemMap *> maps;
  mLayout->layoutItems( maps );

  for ( QgsLayoutItemMap *map : qgis::as_const( maps ) )
  {
    if ( map->containsWmsLayer() )
      return true;
  }
  return false;
}

void QgsLayoutDesignerDialog::showWmsPrintingWarning()
{
  QgsSettings settings;
  bool displayWMSWarning = settings.value( QStringLiteral( "/UI/displayComposerWMSWarning" ), true ).toBool();
  if ( displayWMSWarning )
  {
    QgsMessageViewer *m = new QgsMessageViewer( this );
    m->setWindowTitle( tr( "Project Contains WMS Layers" ) );
    m->setMessage( tr( "Some WMS servers (e.g. UMN mapserver) have a limit for the WIDTH and HEIGHT parameter. Printing layers from such servers may exceed this limit. If this is the case, the WMS layer will not be printed" ), QgsMessageOutput::MessageText );
    m->setCheckBoxText( tr( "Don't show this message again" ) );
    m->setCheckBoxState( Qt::Unchecked );
    m->setCheckBoxVisible( true );
    m->setCheckBoxQgsSettingsLabel( QStringLiteral( "/UI/displayComposerWMSWarning" ) );
    m->exec(); //deleted on close
  }
}

void QgsLayoutDesignerDialog::showSvgExportWarning()
{
  QgsSettings settings;

  bool displaySVGWarning = settings.value( QStringLiteral( "/UI/displaySVGWarning" ), true ).toBool();

  if ( displaySVGWarning )
  {
    QgsMessageViewer m( this, QgsGuiUtils::ModalDialogFlags, false );
    m.setWindowTitle( tr( "Export as SVG" ) );
    m.setCheckBoxText( tr( "Don't show this message again" ) );
    m.setCheckBoxState( Qt::Unchecked );
    m.setCheckBoxVisible( true );
    m.setCheckBoxQgsSettingsLabel( QStringLiteral( "/UI/displaySVGWarning" ) );
    m.setMessageAsHtml( tr( "<p>The SVG export function in QGIS has several "
                            "problems due to bugs and deficiencies in the " )
                        + tr( "underlying Qt SVG library. In particular, there are problems "
                              "with layers not being clipped to the map "
                              "bounding box.</p>" )
                        + tr( "If you require a vector-based output file from "
                              "QGIS it is suggested that you try exporting "
                              "to PDF if the SVG output is not "
                              "satisfactory."
                              "</p>" ) );
    m.exec();
  }
}

bool QgsLayoutDesignerDialog::requiresRasterization() const
{
  QList< QgsLayoutItem *> items;
  mLayout->layoutItems( items );

  for ( QgsLayoutItem *currentItem : qgis::as_const( items ) )
  {
    if ( currentItem->requiresRasterization() )
      return true;
  }
  return false;
}

bool QgsLayoutDesignerDialog::containsAdvancedEffects() const
{
  QList< QgsLayoutItem *> items;
  mLayout->layoutItems( items );

  for ( QgsLayoutItem *currentItem : qgis::as_const( items ) )
  {
    if ( currentItem->containsAdvancedEffects() )
      return true;
  }
  return false;
}

void QgsLayoutDesignerDialog::showRasterizationWarning()
{

  if ( mLayout->customProperty( QStringLiteral( "rasterize" ), false ).toBool() ||
       mLayout->customProperty( QStringLiteral( "forceVector" ), false ).toBool() )
    return;

  QgsMessageViewer m( this, QgsGuiUtils::ModalDialogFlags, false );
  m.setWindowTitle( tr( "Composition Effects" ) );
  m.setMessage( tr( "Advanced composition effects such as blend modes or vector layer transparency are enabled in this layout, which cannot be printed as vectors. Printing as a raster is recommended." ), QgsMessageOutput::MessageText );
  m.setCheckBoxText( tr( "Print as raster" ) );
  m.setCheckBoxState( Qt::Checked );
  m.setCheckBoxVisible( true );
  m.showMessage( true );

  mLayout->setCustomProperty( QStringLiteral( "rasterize" ), m.checkBoxState() == Qt::Checked );
  //make sure print as raster checkbox is updated
  mLayoutPropertiesWidget->updateGui();
}

void QgsLayoutDesignerDialog::showForceVectorWarning()
{
  QgsSettings settings;
  if ( settings.value( QStringLiteral( "LayoutDesigner/hideForceVectorWarning" ), false, QgsSettings::App ).toBool() )
    return;

  QgsMessageViewer m( this, QgsGuiUtils::ModalDialogFlags, false );
  m.setWindowTitle( tr( "Force Vector" ) );
  m.setMessage( tr( "This layout has the \"Always export as vectors\" option enabled, but the layout contains effects such as blend modes or vector layer transparency, which cannot be printed as vectors. The generated file will differ from the layout contents." ), QgsMessageOutput::MessageText );
  m.setCheckBoxText( tr( "Never show this message again" ) );
  m.setCheckBoxState( Qt::Unchecked );
  m.setCheckBoxVisible( true );
  m.showMessage( true );

  if ( m.checkBoxState() == Qt::Checked )
  {
    settings.setValue( QStringLiteral( "LayoutDesigner/hideForceVectorWarning" ), true, QgsSettings::App );
  }
}

bool QgsLayoutDesignerDialog::showFileSizeWarning()
{
  // Image size
  double oneInchInLayoutUnits = mLayout->convertToLayoutUnits( QgsLayoutMeasurement( 1, QgsUnitTypes::LayoutInches ) );
  QSizeF maxPageSize = mLayout->pageCollection()->maximumPageSize();
  int width = ( int )( mLayout->renderContext().dpi() * maxPageSize.width() / oneInchInLayoutUnits );
  int height = ( int )( mLayout->renderContext().dpi() * maxPageSize.height() / oneInchInLayoutUnits );
  int memuse = width * height * 3 / 1000000;  // pixmap + image
  QgsDebugMsg( QString( "Image %1x%2" ).arg( width ).arg( height ) );
  QgsDebugMsg( QString( "memuse = %1" ).arg( memuse ) );

  if ( memuse > 400 )   // about 4500x4500
  {
    int answer = QMessageBox::warning( this, tr( "Export layout" ),
                                       tr( "To create an image of %1x%2 requires about %3 MB of memory. Proceed?" )
                                       .arg( width ).arg( height ).arg( memuse ),
                                       QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok );

    raise();
    if ( answer == QMessageBox::Cancel )
      return false;
  }
  return true;
}

bool QgsLayoutDesignerDialog::getRasterExportSettings( QgsLayoutExporter::ImageExportSettings &settings, QSize &imageSize )
{
  QSizeF maxPageSize;
  bool hasUniformPageSizes = false;
  double dpi = 300;
  bool cropToContents = false;
  int marginTop = 0;
  int marginRight = 0;
  int marginBottom = 0;
  int marginLeft = 0;
  bool antialias = true;

  // Image size
  if ( mLayout )
  {
    maxPageSize = mLayout->pageCollection()->maximumPageSize();
    hasUniformPageSizes = mLayout->pageCollection()->hasUniformPageSizes();
    dpi = mLayout->renderContext().dpi();

    //get some defaults from the composition
    cropToContents = mLayout->customProperty( QStringLiteral( "imageCropToContents" ), false ).toBool();
    marginTop = mLayout->customProperty( QStringLiteral( "imageCropMarginTop" ), 0 ).toInt();
    marginRight = mLayout->customProperty( QStringLiteral( "imageCropMarginRight" ), 0 ).toInt();
    marginBottom = mLayout->customProperty( QStringLiteral( "imageCropMarginBottom" ), 0 ).toInt();
    marginLeft = mLayout->customProperty( QStringLiteral( "imageCropMarginLeft" ), 0 ).toInt();
    antialias = mLayout->customProperty( QStringLiteral( "imageAntialias" ), true ).toBool();
  }

  QgsLayoutImageExportOptionsDialog imageDlg( this );
  imageDlg.setImageSize( maxPageSize );
  imageDlg.setResolution( dpi );
  imageDlg.setCropToContents( cropToContents );
  imageDlg.setCropMargins( marginTop, marginRight, marginBottom, marginLeft );
  if ( mLayout )
    imageDlg.setGenerateWorldFile( mLayout->customProperty( QStringLiteral( "exportWorldFile" ), false ).toBool() );
  imageDlg.setAntialiasing( antialias );

  if ( !imageDlg.exec() )
    return false;

  imageSize = QSize( imageDlg.imageWidth(), imageDlg.imageHeight() );
  cropToContents = imageDlg.cropToContents();
  imageDlg.getCropMargins( marginTop, marginRight, marginBottom, marginLeft );
  if ( mLayout )
  {
    mLayout->setCustomProperty( QStringLiteral( "imageCropToContents" ), cropToContents );
    mLayout->setCustomProperty( QStringLiteral( "imageCropMarginTop" ), marginTop );
    mLayout->setCustomProperty( QStringLiteral( "imageCropMarginRight" ), marginRight );
    mLayout->setCustomProperty( QStringLiteral( "imageCropMarginBottom" ), marginBottom );
    mLayout->setCustomProperty( QStringLiteral( "imageCropMarginLeft" ), marginLeft );
    mLayout->setCustomProperty( QStringLiteral( "imageAntialias" ), imageDlg.antialiasing() );
  }

  settings.cropToContents = cropToContents;
  settings.cropMargins = QgsMargins( marginLeft, marginTop, marginRight, marginBottom );
  settings.dpi = imageDlg.resolution();
  if ( hasUniformPageSizes )
  {
    settings.imageSize = imageSize;
  }
  settings.generateWorldFile = imageDlg.generateWorldFile();
  settings.flags = QgsLayoutRenderContext::FlagUseAdvancedEffects;
  if ( imageDlg.antialiasing() )
    settings.flags |= QgsLayoutRenderContext::FlagAntialiasing;

  return true;
}

bool QgsLayoutDesignerDialog::getSvgExportSettings( QgsLayoutExporter::SvgExportSettings &settings, bool &exportAsText )
{
  bool groupLayers = false;
  bool prevSettingLabelsAsOutlines = mMasterLayout->layoutProject()->readBoolEntry( QStringLiteral( "PAL" ), QStringLiteral( "/DrawOutlineLabels" ), true );
  bool clipToContent = false;
  double marginTop = 0.0;
  double marginRight = 0.0;
  double marginBottom = 0.0;
  double marginLeft = 0.0;
  bool previousForceVector = false;
  bool layersAsGroup = false;
  bool cropToContents = false;
  double topMargin = 0.0;
  double rightMargin = 0.0;
  double bottomMargin = 0.0;
  double leftMargin = 0.0;
  if ( mLayout )
  {
    mLayout->customProperty( QStringLiteral( "forceVector" ), false ).toBool();
    layersAsGroup = mLayout->customProperty( QStringLiteral( "svgGroupLayers" ), false ).toBool();
    cropToContents = mLayout->customProperty( QStringLiteral( "svgCropToContents" ), false ).toBool();
    topMargin = mLayout->customProperty( QStringLiteral( "svgCropMarginTop" ), 0 ).toInt();
    rightMargin = mLayout->customProperty( QStringLiteral( "svgCropMarginRight" ), 0 ).toInt();
    bottomMargin = mLayout->customProperty( QStringLiteral( "svgCropMarginBottom" ), 0 ).toInt();
    leftMargin = mLayout->customProperty( QStringLiteral( "svgCropMarginLeft" ), 0 ).toInt();
  }

  // open options dialog
  QDialog dialog;
  Ui::QgsSvgExportOptionsDialog options;
  options.setupUi( &dialog );
  options.chkTextAsOutline->setChecked( prevSettingLabelsAsOutlines );
  options.chkMapLayersAsGroup->setChecked( layersAsGroup );
  options.mClipToContentGroupBox->setChecked( cropToContents );
  options.mForceVectorCheckBox->setChecked( previousForceVector );
  options.mTopMarginSpinBox->setValue( topMargin );
  options.mRightMarginSpinBox->setValue( rightMargin );
  options.mBottomMarginSpinBox->setValue( bottomMargin );
  options.mLeftMarginSpinBox->setValue( leftMargin );

  if ( dialog.exec() != QDialog::Accepted )
    return false;

  groupLayers = options.chkMapLayersAsGroup->isChecked();
  clipToContent = options.mClipToContentGroupBox->isChecked();
  marginTop = options.mTopMarginSpinBox->value();
  marginRight = options.mRightMarginSpinBox->value();
  marginBottom = options.mBottomMarginSpinBox->value();
  marginLeft = options.mLeftMarginSpinBox->value();

  if ( mLayout )
  {
    //save dialog settings
    mLayout->setCustomProperty( QStringLiteral( "svgGroupLayers" ), groupLayers );
    mLayout->setCustomProperty( QStringLiteral( "svgCropToContents" ), clipToContent );
    mLayout->setCustomProperty( QStringLiteral( "svgCropMarginTop" ), marginTop );
    mLayout->setCustomProperty( QStringLiteral( "svgCropMarginRight" ), marginRight );
    mLayout->setCustomProperty( QStringLiteral( "svgCropMarginBottom" ), marginBottom );
    mLayout->setCustomProperty( QStringLiteral( "svgCropMarginLeft" ), marginLeft );
  }

  settings.cropToContents = clipToContent;
  settings.cropMargins = QgsMargins( marginLeft, marginTop, marginRight, marginBottom );
  settings.forceVectorOutput = options.mForceVectorCheckBox->isChecked();
  settings.exportAsLayers = groupLayers;

  exportAsText = options.chkTextAsOutline->isChecked();
  return true;
}

void QgsLayoutDesignerDialog::toggleAtlasControls( bool atlasEnabled )
{
  //preview defaults to unchecked
  mActionAtlasPreview->blockSignals( true );
  mActionAtlasPreview->setChecked( false );
  mActionAtlasFirst->setEnabled( false );
  mActionAtlasLast->setEnabled( false );
  mActionAtlasNext->setEnabled( false );
  mActionAtlasPrev->setEnabled( false );
  mAtlasPageComboBox->setEnabled( false );
  mActionAtlasPreview->blockSignals( false );
  mActionAtlasPreview->setEnabled( atlasEnabled );
  mActionPrintAtlas->setEnabled( atlasEnabled );
  mActionExportAtlasAsImage->setEnabled( atlasEnabled );
  mActionExportAtlasAsSVG->setEnabled( atlasEnabled );
  mActionExportAtlasAsPDF->setEnabled( atlasEnabled );
}

void QgsLayoutDesignerDialog::updateAtlasPageComboBox( int pageCount )
{
  QgsPrintLayout *printLayout = qobject_cast< QgsPrintLayout * >( mLayout );
  if ( !printLayout )
    return;

  QgsLayoutAtlas *atlas = printLayout->atlas();
  mAtlasPageComboBox->blockSignals( true );
  mAtlasPageComboBox->clear();
  for ( int i = 1; i <= pageCount && i < 500; ++i )
  {
    QString name = atlas->nameForPage( i - 1 );
    QString fullName = ( !name.isEmpty() ? QStringLiteral( "%1: %2" ).arg( i ).arg( name ) : QString::number( i ) );

    mAtlasPageComboBox->addItem( fullName, i );
    mAtlasPageComboBox->setItemData( i - 1, name, Qt::UserRole + 1 );
    mAtlasPageComboBox->setItemData( i - 1, fullName, Qt::UserRole + 2 );
  }
  mAtlasPageComboBox->blockSignals( false );

}

void QgsLayoutDesignerDialog::atlasFeatureChanged( const QgsFeature &feature )
{
  //TODO - this should be disabled during an export

  QgsPrintLayout *printLayout = qobject_cast< QgsPrintLayout *>( mLayout );
  if ( !printLayout )
    return;

  QgsLayoutAtlas *atlas = printLayout->atlas();

  mAtlasPageComboBox->blockSignals( true );
  //prefer to set index of current atlas page, if combo box is showing enough page items
  if ( atlas->currentFeatureNumber() < mAtlasPageComboBox->count() )
  {
    mAtlasPageComboBox->setCurrentIndex( atlas->currentFeatureNumber() );
  }
  else
  {
    //fallback to setting the combo text to the page number
    mAtlasPageComboBox->setEditText( QString::number( atlas->currentFeatureNumber() + 1 ) );
  }
  mAtlasPageComboBox->blockSignals( false );

  //update expression context variables in map canvas to allow for previewing atlas feature based rendering
  QgsMapCanvas *mapCanvas = QgisApp::instance()->mapCanvas();
  mapCanvas->expressionContextScope().addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "atlas_featurenumber" ), atlas->currentFeatureNumber() + 1, true ) );
  mapCanvas->expressionContextScope().addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "atlas_pagename" ), atlas->nameForPage( atlas->currentFeatureNumber() ), true ) );
  mapCanvas->expressionContextScope().addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "atlas_feature" ), QVariant::fromValue( feature ), true ) );
  mapCanvas->expressionContextScope().addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "atlas_featureid" ), feature.id(), true ) );
  mapCanvas->expressionContextScope().addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "atlas_geometry" ), QVariant::fromValue( feature.geometry() ), true ) );
  mapCanvas->stopRendering();
  mapCanvas->refreshAllLayers();

  mView->setSectionLabel( atlas->nameForPage( atlas->currentFeatureNumber() ) );
}

void QgsLayoutDesignerDialog::loadAtlasPredefinedScalesFromProject()
{
  QVector<qreal> projectScales;
  // first look at project's scales
  QStringList scales( mLayout->project()->readListEntry( QStringLiteral( "Scales" ), QStringLiteral( "/ScalesList" ) ) );
  bool hasProjectScales( mLayout->project()->readBoolEntry( QStringLiteral( "Scales" ), QStringLiteral( "/useProjectScales" ) ) );
  if ( !hasProjectScales || scales.isEmpty() )
  {
    // default to global map tool scales
    QgsSettings settings;
    QString scalesStr( settings.value( QStringLiteral( "Map/scales" ), PROJECT_SCALES ).toString() );
    scales = scalesStr.split( ',' );
  }

  for ( auto scaleIt = scales.constBegin(); scaleIt != scales.constEnd(); ++scaleIt )
  {
    QStringList parts( scaleIt->split( ':' ) );
    if ( parts.size() == 2 )
    {
      projectScales.push_back( parts[1].toDouble() );
    }
  }
  mLayout->reportContext().setPredefinedScales( projectScales );
}

QgsLayoutAtlas *QgsLayoutDesignerDialog::atlas()
{
  QgsPrintLayout *layout = qobject_cast< QgsPrintLayout *>( mLayout );
  if ( !layout )
    return nullptr;
  return layout->atlas();
}

void QgsLayoutDesignerDialog::toggleActions( bool layoutAvailable )
{
  mActionPan->setEnabled( layoutAvailable );
  mActionZoomTool->setEnabled( layoutAvailable );
  mActionSelectMoveItem->setEnabled( layoutAvailable );
  mActionZoomAll->setEnabled( layoutAvailable );
  mActionZoomIn->setEnabled( layoutAvailable );
  mActionZoomOut->setEnabled( layoutAvailable );
  mActionZoomActual->setEnabled( layoutAvailable );
  mActionZoomToWidth->setEnabled( layoutAvailable );
  mActionAddPages->setEnabled( layoutAvailable );
  mActionShowGrid->setEnabled( layoutAvailable );
  mActionSnapGrid->setEnabled( layoutAvailable );
  mActionShowGuides->setEnabled( layoutAvailable );
  mActionSnapGuides->setEnabled( layoutAvailable );
  mActionClearGuides->setEnabled( layoutAvailable );
  mActionLayoutProperties->setEnabled( layoutAvailable );
  mActionShowBoxes->setEnabled( layoutAvailable );
  mActionSmartGuides->setEnabled( layoutAvailable );
  mActionDeselectAll->setEnabled( layoutAvailable );
  mActionSelectAll->setEnabled( layoutAvailable );
  mActionInvertSelection->setEnabled( layoutAvailable );
  mActionSelectNextBelow->setEnabled( layoutAvailable );
  mActionSelectNextAbove->setEnabled( layoutAvailable );
  mActionLockItems->setEnabled( layoutAvailable );
  mActionUnlockAll->setEnabled( layoutAvailable );
  mActionRaiseItems->setEnabled( layoutAvailable );
  mActionLowerItems->setEnabled( layoutAvailable );
  mActionMoveItemsToTop->setEnabled( layoutAvailable );
  mActionMoveItemsToBottom->setEnabled( layoutAvailable );
  mActionAlignLeft->setEnabled( layoutAvailable );
  mActionAlignHCenter->setEnabled( layoutAvailable );
  mActionAlignRight->setEnabled( layoutAvailable );
  mActionAlignTop->setEnabled( layoutAvailable );
  mActionAlignVCenter->setEnabled( layoutAvailable );
  mActionAlignBottom->setEnabled( layoutAvailable );
  mActionDistributeLeft->setEnabled( layoutAvailable );
  mActionDistributeHCenter->setEnabled( layoutAvailable );
  mActionDistributeRight->setEnabled( layoutAvailable );
  mActionDistributeTop->setEnabled( layoutAvailable );
  mActionDistributeVCenter->setEnabled( layoutAvailable );
  mActionDistributeBottom->setEnabled( layoutAvailable );
  mActionResizeNarrowest->setEnabled( layoutAvailable );
  mActionResizeWidest->setEnabled( layoutAvailable );
  mActionResizeShortest->setEnabled( layoutAvailable );
  mActionResizeTallest->setEnabled( layoutAvailable );
  mActionDeleteSelection->setEnabled( layoutAvailable );
  mActionResizeToSquare->setEnabled( layoutAvailable );
  mActionShowPage->setEnabled( layoutAvailable );
  mActionGroupItems->setEnabled( layoutAvailable );
  mActionUngroupItems->setEnabled( layoutAvailable );
  mActionRefreshView->setEnabled( layoutAvailable );
  mActionEditNodesItem->setEnabled( layoutAvailable );
  mActionMoveItemContent->setEnabled( layoutAvailable );
  mActionPasteInPlace->setEnabled( layoutAvailable );
  mActionSaveAsTemplate->setEnabled( layoutAvailable );
  mActionLoadFromTemplate->setEnabled( layoutAvailable );
  mActionExportAsImage->setEnabled( layoutAvailable );
  mActionExportAsPDF->setEnabled( layoutAvailable );
  mActionExportAsSVG->setEnabled( layoutAvailable );
  mActionCut->setEnabled( layoutAvailable );
  mActionCopy->setEnabled( layoutAvailable );
  mActionPaste->setEnabled( layoutAvailable );
  menuAlign_Items->setEnabled( layoutAvailable );
  menu_Distribute_Items->setEnabled( layoutAvailable );
  menuResize->setEnabled( layoutAvailable );

  const QList<QAction *> itemActions = mToolsActionGroup->actions();
  for ( QAction *action : itemActions )
  {
    action->setEnabled( layoutAvailable );
  }
  for ( auto it = mItemGroupSubmenus.constBegin(); it != mItemGroupSubmenus.constEnd(); ++it )
  {
    it.value()->setEnabled( layoutAvailable );
  }
  for ( auto it = mItemGroupToolButtons.constBegin(); it != mItemGroupToolButtons.constEnd(); ++it )
  {
    it.value()->setEnabled( layoutAvailable );
  }
}

void QgsLayoutDesignerDialog::setPrinterPageOrientation( QgsLayoutItemPage::Orientation orientation )
{
  if ( !mSetPageOrientation )
  {
    switch ( orientation )
    {
      case QgsLayoutItemPage::Landscape:
        printer()->setOrientation( QPrinter::Landscape );
        break;

      case QgsLayoutItemPage::Portrait:
        printer()->setOrientation( QPrinter::Portrait );
        break;
    }

    mSetPageOrientation = true;
  }
}

QPrinter *QgsLayoutDesignerDialog::printer()
{
  //only create the printer on demand - creating a printer object can be very slow
  //due to QTBUG-3033
  if ( !mPrinter )
    mPrinter = qgis::make_unique< QPrinter >();

  return mPrinter.get();
}

QString QgsLayoutDesignerDialog::reportTypeString()
{
  if ( atlas() )
    return tr( "atlas" );
  else
    return tr( "report" );
}

void QgsLayoutDesignerDialog::updateActionNames( QgsMasterLayoutInterface::Type type )
{
  switch ( type )
  {
    case QgsMasterLayoutInterface::PrintLayout:
      mActionDuplicateLayout->setText( tr( "&Duplicate Layout…" ) );
      mActionRemoveLayout->setText( tr( "Delete Layout…" ) );
      mActionRenameLayout->setText( tr( "Rename Layout…" ) );
      mActionNewLayout->setText( tr( "New Layout…" ) );
      break;

    case QgsMasterLayoutInterface::Report:
      mActionDuplicateLayout->setText( tr( "&Duplicate Report…" ) );
      mActionRemoveLayout->setText( tr( "Delete Report…" ) );
      mActionRenameLayout->setText( tr( "Rename Report…" ) );
      mActionNewLayout->setText( tr( "New Report…" ) );
      break;
  }
}

void QgsLayoutDesignerDialog::updateWindowTitle()
{
  if ( mSectionTitle.isEmpty() )
    setWindowTitle( mTitle );
  else
    setWindowTitle( QStringLiteral( "%1 - %2" ).arg( mTitle, mSectionTitle ) );
}

void QgsLayoutDesignerDialog::selectItems( const QList<QgsLayoutItem *> items )
{
  for ( QGraphicsItem *item : items )
  {
    if ( item )
    {
      item->setSelected( true );
    }
  }

  //update item panel
  const QList<QgsLayoutItem *> selectedItemList = currentLayout()->selectedLayoutItems();
  if ( !selectedItemList.isEmpty() )
  {
    showItemOptions( selectedItemList.at( 0 ) );
  }
  else
  {
    showItemOptions( nullptr );
  }
}

QgsMessageBar *QgsLayoutDesignerDialog::messageBar()
{
  return mMessageBar;
}

void QgsLayoutDesignerDialog::setAtlasFeature( QgsMapLayer *layer, const QgsFeature &feat )
{
  QgsLayoutAtlas *layoutAtlas = atlas();
  if ( !layoutAtlas || !layoutAtlas->enabled() || layoutAtlas->coverageLayer() != layer )
  {
    //either atlas isn't enabled, or layer doesn't match
    return;
  }

  if ( !mActionAtlasPreview->isChecked() )
  {
    //update gui controls
    whileBlocking( mActionAtlasPreview )->setChecked( true );
    atlasPreviewTriggered( true );
  }

  //set current preview feature id
  layoutAtlas->seekTo( feat );

  //bring layout window to foreground
  activate();
}

void QgsLayoutDesignerDialog::setSectionTitle( const QString &title )
{
  mSectionTitle = title;
  updateWindowTitle();
  mView->setSectionLabel( title );
}



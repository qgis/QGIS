/***************************************************************************
                         qgscomposer.cpp  -  description
                             -------------------
    begin                : January 2005
    copyright            : (C) 2005 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgscomposer.h"

#include <stdexcept>

#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsbusyindicatordialog.h"
#include "qgscomposerruler.h"
#include "qgscomposerview.h"
#include "qgscomposition.h"
#include "qgscompositionwidget.h"
#include "qgscomposermodel.h"
#include "qgsdockwidget.h"
#include "qgsatlascompositionwidget.h"
#include "qgscomposerarrow.h"
#include "qgscomposerpolygon.h"
#include "qgscomposerpolyline.h"
#include "qgscomposerpolygonwidget.h"
#include "qgscomposerpolylinewidget.h"
#include "qgscomposerarrowwidget.h"
#include "qgscomposerattributetablewidget.h"
#include "qgscomposerframe.h"
#include "qgscomposerhtml.h"
#include "qgscomposerhtmlwidget.h"
#include "qgscomposerlabel.h"
#include "qgscomposerlabelwidget.h"
#include "qgscomposerlegend.h"
#include "qgscomposerlegendwidget.h"
#include "qgscomposermap.h"
#include "qgsatlascomposition.h"
#include "qgscomposermapwidget.h"
#include "qgscomposerpicture.h"
#include "qgscomposerpicturewidget.h"
#include "qgscomposerscalebar.h"
#include "qgscomposerscalebarwidget.h"
#include "qgscomposershape.h"
#include "qgscomposershapewidget.h"
#include "qgscomposerattributetablev2.h"
#include "qgsexception.h"
#include "qgslogger.h"
#include "qgsproject.h"
#include "qgsmapcanvas.h"
#include "qgsmessageviewer.h"
#include "qgscursors.h"
#include "qgsmaplayeractionregistry.h"
#include "qgsgeometry.h"
#include "qgspaperitem.h"
#include "qgsprevieweffect.h"
#include "qgsvectorlayer.h"
#include "qgscomposerimageexportoptionsdialog.h"
#include "ui_qgssvgexportoptions.h"
#include "qgspanelwidgetstack.h"
#include "qgssettings.h"
#include "qgslayoutmanager.h"

#include <QCloseEvent>
#include <QCheckBox>
#include <QDesktopWidget>
#include <QDialog>
#include <QFileDialog>
#include <QFileInfo>
#include <QIcon>
#include <QImageWriter>
#include <QLabel>
#include <QMatrix>
#include <QMenuBar>
#include <QMessageBox>
#include <QPageSetupDialog>
#include <QPainter>
#include <QPixmap>
#include <QPrintDialog>
#include <QPrinter>
#include <QSizeGrip>
#include <QSvgGenerator>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QUndoView>
#include <QPaintEngine>
#include <QProgressBar>
#include <QProgressDialog>
#include <QShortcut>

#ifdef ENABLE_MODELTEST
#include "modeltest.h"
#endif

QgsComposer::QgsComposer( QgsComposition *composition )
  : mInterface( new QgsAppComposerInterface( this ) )
  , mComposition( composition )
  , mQgis( QgisApp::instance() )
{
  setupUi( this );
  connect( mActionZoomAll, &QAction::triggered, this, &QgsComposer::mActionZoomAll_triggered );
  connect( mActionZoomIn, &QAction::triggered, this, &QgsComposer::mActionZoomIn_triggered );
  connect( mActionZoomOut, &QAction::triggered, this, &QgsComposer::mActionZoomOut_triggered );
  connect( mActionZoomActual, &QAction::triggered, this, &QgsComposer::mActionZoomActual_triggered );
  connect( mActionRefreshView, &QAction::triggered, this, &QgsComposer::mActionRefreshView_triggered );
  connect( mActionPrint, &QAction::triggered, this, &QgsComposer::mActionPrint_triggered );
  connect( mActionPageSetup, &QAction::triggered, this, &QgsComposer::mActionPageSetup_triggered );
  connect( mActionExportAsImage, &QAction::triggered, this, &QgsComposer::mActionExportAsImage_triggered );
  connect( mActionExportAsSVG, &QAction::triggered, this, &QgsComposer::mActionExportAsSVG_triggered );
  connect( mActionExportAsPDF, &QAction::triggered, this, &QgsComposer::mActionExportAsPDF_triggered );
  connect( mActionSelectMoveItem, &QAction::triggered, this, &QgsComposer::mActionSelectMoveItem_triggered );
  connect( mActionAddArrow, &QAction::triggered, this, &QgsComposer::mActionAddArrow_triggered );
  connect( mActionAddNewMap, &QAction::triggered, this, &QgsComposer::mActionAddNewMap_triggered );
  connect( mActionAddNewLegend, &QAction::triggered, this, &QgsComposer::mActionAddNewLegend_triggered );
  connect( mActionAddNewLabel, &QAction::triggered, this, &QgsComposer::mActionAddNewLabel_triggered );
  connect( mActionAddNewScalebar, &QAction::triggered, this, &QgsComposer::mActionAddNewScalebar_triggered );
  connect( mActionAddImage, &QAction::triggered, this, &QgsComposer::mActionAddImage_triggered );
  connect( mActionAddRectangle, &QAction::triggered, this, &QgsComposer::mActionAddRectangle_triggered );
  connect( mActionAddTriangle, &QAction::triggered, this, &QgsComposer::mActionAddTriangle_triggered );
  connect( mActionAddEllipse, &QAction::triggered, this, &QgsComposer::mActionAddEllipse_triggered );
  connect( mActionEditNodesItem, &QAction::triggered, this, &QgsComposer::mActionEditNodesItem_triggered );
  connect( mActionAddPolygon, &QAction::triggered, this, &QgsComposer::mActionAddPolygon_triggered );
  connect( mActionAddPolyline, &QAction::triggered, this, &QgsComposer::mActionAddPolyline_triggered );
  connect( mActionAddTable, &QAction::triggered, this, &QgsComposer::mActionAddTable_triggered );
  connect( mActionAddAttributeTable, &QAction::triggered, this, &QgsComposer::mActionAddAttributeTable_triggered );
  connect( mActionAddHtml, &QAction::triggered, this, &QgsComposer::mActionAddHtml_triggered );
  connect( mActionSaveProject, &QAction::triggered, this, &QgsComposer::mActionSaveProject_triggered );
  connect( mActionNewComposer, &QAction::triggered, this, &QgsComposer::mActionNewComposer_triggered );
  connect( mActionDuplicateComposer, &QAction::triggered, this, &QgsComposer::mActionDuplicateComposer_triggered );
  connect( mActionComposerManager, &QAction::triggered, this, &QgsComposer::mActionComposerManager_triggered );
  connect( mActionSaveAsTemplate, &QAction::triggered, this, &QgsComposer::mActionSaveAsTemplate_triggered );
  connect( mActionLoadFromTemplate, &QAction::triggered, this, &QgsComposer::mActionLoadFromTemplate_triggered );
  connect( mActionMoveItemContent, &QAction::triggered, this, &QgsComposer::mActionMoveItemContent_triggered );
  connect( mActionPan, &QAction::triggered, this, &QgsComposer::mActionPan_triggered );
  connect( mActionMouseZoom, &QAction::triggered, this, &QgsComposer::mActionMouseZoom_triggered );
  connect( mActionGroupItems, &QAction::triggered, this, &QgsComposer::mActionGroupItems_triggered );
  connect( mActionPasteInPlace, &QAction::triggered, this, &QgsComposer::mActionPasteInPlace_triggered );
  connect( mActionDeleteSelection, &QAction::triggered, this, &QgsComposer::mActionDeleteSelection_triggered );
  connect( mActionSelectAll, &QAction::triggered, this, &QgsComposer::mActionSelectAll_triggered );
  connect( mActionDeselectAll, &QAction::triggered, this, &QgsComposer::mActionDeselectAll_triggered );
  connect( mActionInvertSelection, &QAction::triggered, this, &QgsComposer::mActionInvertSelection_triggered );
  connect( mActionUngroupItems, &QAction::triggered, this, &QgsComposer::mActionUngroupItems_triggered );
  connect( mActionLockItems, &QAction::triggered, this, &QgsComposer::mActionLockItems_triggered );
  connect( mActionUnlockAll, &QAction::triggered, this, &QgsComposer::mActionUnlockAll_triggered );
  connect( mActionSelectNextAbove, &QAction::triggered, this, &QgsComposer::mActionSelectNextAbove_triggered );
  connect( mActionSelectNextBelow, &QAction::triggered, this, &QgsComposer::mActionSelectNextBelow_triggered );
  connect( mActionRaiseItems, &QAction::triggered, this, &QgsComposer::mActionRaiseItems_triggered );
  connect( mActionLowerItems, &QAction::triggered, this, &QgsComposer::mActionLowerItems_triggered );
  connect( mActionMoveItemsToTop, &QAction::triggered, this, &QgsComposer::mActionMoveItemsToTop_triggered );
  connect( mActionMoveItemsToBottom, &QAction::triggered, this, &QgsComposer::mActionMoveItemsToBottom_triggered );
  connect( mActionAlignLeft, &QAction::triggered, this, &QgsComposer::mActionAlignLeft_triggered );
  connect( mActionAlignHCenter, &QAction::triggered, this, &QgsComposer::mActionAlignHCenter_triggered );
  connect( mActionAlignRight, &QAction::triggered, this, &QgsComposer::mActionAlignRight_triggered );
  connect( mActionAlignTop, &QAction::triggered, this, &QgsComposer::mActionAlignTop_triggered );
  connect( mActionAlignVCenter, &QAction::triggered, this, &QgsComposer::mActionAlignVCenter_triggered );
  connect( mActionAlignBottom, &QAction::triggered, this, &QgsComposer::mActionAlignBottom_triggered );
  connect( mActionUndo, &QAction::triggered, this, &QgsComposer::mActionUndo_triggered );
  connect( mActionRedo, &QAction::triggered, this, &QgsComposer::mActionRedo_triggered );
  connect( mActionShowGrid, &QAction::triggered, this, &QgsComposer::mActionShowGrid_triggered );
  connect( mActionSnapGrid, &QAction::triggered, this, &QgsComposer::mActionSnapGrid_triggered );
  connect( mActionShowGuides, &QAction::triggered, this, &QgsComposer::mActionShowGuides_triggered );
  connect( mActionSnapGuides, &QAction::triggered, this, &QgsComposer::mActionSnapGuides_triggered );
  connect( mActionSmartGuides, &QAction::triggered, this, &QgsComposer::mActionSmartGuides_triggered );
  connect( mActionShowBoxes, &QAction::triggered, this, &QgsComposer::mActionShowBoxes_triggered );
  connect( mActionShowPage, &QAction::triggered, this, &QgsComposer::mActionShowPage_triggered );
  connect( mActionClearGuides, &QAction::triggered, this, &QgsComposer::mActionClearGuides_triggered );
  connect( mActionOptions, &QAction::triggered, this, &QgsComposer::mActionOptions_triggered );
  connect( mActionAtlasPreview, &QAction::triggered, this, &QgsComposer::mActionAtlasPreview_triggered );
  connect( mActionAtlasNext, &QAction::triggered, this, &QgsComposer::mActionAtlasNext_triggered );
  connect( mActionAtlasPrev, &QAction::triggered, this, &QgsComposer::mActionAtlasPrev_triggered );
  connect( mActionAtlasFirst, &QAction::triggered, this, &QgsComposer::mActionAtlasFirst_triggered );
  connect( mActionAtlasLast, &QAction::triggered, this, &QgsComposer::mActionAtlasLast_triggered );
  connect( mActionPrintAtlas, &QAction::triggered, this, &QgsComposer::mActionPrintAtlas_triggered );
  connect( mActionExportAtlasAsImage, &QAction::triggered, this, &QgsComposer::mActionExportAtlasAsImage_triggered );
  connect( mActionExportAtlasAsSVG, &QAction::triggered, this, &QgsComposer::mActionExportAtlasAsSVG_triggered );
  connect( mActionExportAtlasAsPDF, &QAction::triggered, this, &QgsComposer::mActionExportAtlasAsPDF_triggered );
  connect( mActionAtlasSettings, &QAction::triggered, this, &QgsComposer::mActionAtlasSettings_triggered );
  connect( mActionToggleFullScreen, &QAction::triggered, this, &QgsComposer::mActionToggleFullScreen_triggered );
  connect( mActionHidePanels, &QAction::triggered, this, &QgsComposer::mActionHidePanels_triggered );
  setWindowTitle( mComposition->name() );
  setAttribute( Qt::WA_DeleteOnClose );
#if QT_VERSION >= 0x050600
  setDockOptions( dockOptions() | QMainWindow::GroupedDragging );
#endif
  setupTheme();

  QgsSettings settings;
  setStyleSheet( mQgis->styleSheet() );

  int size = settings.value( QStringLiteral( "IconSize" ), QGIS_ICON_SIZE ).toInt();
  setIconSize( QSize( size, size ) );

  QToolButton *orderingToolButton = new QToolButton( this );
  orderingToolButton->setPopupMode( QToolButton::InstantPopup );
  orderingToolButton->setAutoRaise( true );
  orderingToolButton->setToolButtonStyle( Qt::ToolButtonIconOnly );
  orderingToolButton->addAction( mActionRaiseItems );
  orderingToolButton->addAction( mActionLowerItems );
  orderingToolButton->addAction( mActionMoveItemsToTop );
  orderingToolButton->addAction( mActionMoveItemsToBottom );
  orderingToolButton->setDefaultAction( mActionRaiseItems );
  mItemActionToolbar->addWidget( orderingToolButton );

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
  mItemActionToolbar->addWidget( alignToolButton );

  QToolButton *shapeToolButton = new QToolButton( mItemToolbar );
  shapeToolButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddBasicShape.svg" ) ) );
  shapeToolButton->setCheckable( true );
  shapeToolButton->setPopupMode( QToolButton::InstantPopup );
  shapeToolButton->setAutoRaise( true );
  shapeToolButton->setToolButtonStyle( Qt::ToolButtonIconOnly );
  shapeToolButton->addAction( mActionAddRectangle );
  shapeToolButton->addAction( mActionAddTriangle );
  shapeToolButton->addAction( mActionAddEllipse );
  shapeToolButton->setToolTip( tr( "Add Shape" ) );
  mItemToolbar->insertWidget( mActionAddArrow, shapeToolButton );

  QToolButton *nodesItemButton = new QToolButton( mItemToolbar );
  nodesItemButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddNodesItem.svg" ) ) );
  nodesItemButton->setCheckable( true );
  nodesItemButton->setPopupMode( QToolButton::InstantPopup );
  nodesItemButton->setAutoRaise( true );
  nodesItemButton->setToolButtonStyle( Qt::ToolButtonIconOnly );
  nodesItemButton->addAction( mActionAddPolygon );
  nodesItemButton->addAction( mActionAddPolyline );
  nodesItemButton->setToolTip( tr( "Add Nodes item" ) );
  mItemToolbar->insertWidget( mActionAddArrow, nodesItemButton );

  QActionGroup *toggleActionGroup = new QActionGroup( this );
  toggleActionGroup->addAction( mActionMoveItemContent );
  toggleActionGroup->addAction( mActionEditNodesItem );
  toggleActionGroup->addAction( mActionPan );
  toggleActionGroup->addAction( mActionMouseZoom );
  toggleActionGroup->addAction( mActionAddNewMap );
  toggleActionGroup->addAction( mActionAddNewLabel );
  toggleActionGroup->addAction( mActionAddNewLegend );
  toggleActionGroup->addAction( mActionAddNewScalebar );
  toggleActionGroup->addAction( mActionAddImage );
  toggleActionGroup->addAction( mActionSelectMoveItem );
  toggleActionGroup->addAction( mActionAddRectangle );
  toggleActionGroup->addAction( mActionAddTriangle );
  toggleActionGroup->addAction( mActionAddEllipse );
  toggleActionGroup->addAction( mActionAddPolygon );
  toggleActionGroup->addAction( mActionAddPolyline );
  toggleActionGroup->addAction( mActionAddArrow );
  //toggleActionGroup->addAction( mActionAddTable );
  toggleActionGroup->addAction( mActionAddAttributeTable );
  toggleActionGroup->addAction( mActionAddHtml );
  toggleActionGroup->setExclusive( true );

  mActionAddNewMap->setCheckable( true );
  mActionAddNewLabel->setCheckable( true );
  mActionAddNewLegend->setCheckable( true );
  mActionSelectMoveItem->setCheckable( true );
  mActionAddNewScalebar->setCheckable( true );
  mActionAddImage->setCheckable( true );
  mActionMoveItemContent->setCheckable( true );
  mActionEditNodesItem->setCheckable( true );
  mActionPan->setCheckable( true );
  mActionMouseZoom->setCheckable( true );
  mActionAddArrow->setCheckable( true );
  mActionAddHtml->setCheckable( true );

  mActionShowGrid->setCheckable( true );
  mActionSnapGrid->setCheckable( true );
  mActionShowGuides->setCheckable( true );
  mActionSnapGuides->setCheckable( true );
  mActionSmartGuides->setCheckable( true );
  mActionShowRulers->setCheckable( true );
  mActionShowBoxes->setCheckable( true );

  mActionAtlasPreview->setCheckable( true );

#ifdef Q_OS_MAC
  mActionQuit->setText( tr( "Close" ) );
  mActionQuit->setShortcut( QKeySequence::Close );
  QMenu *appMenu = menuBar()->addMenu( tr( "QGIS" ) );
  appMenu->addAction( mQgis->actionAbout() );
  appMenu->addAction( mQgis->actionOptions() );
#endif

  QMenu *composerMenu = menuBar()->addMenu( tr( "&Composer" ) );
  composerMenu->addAction( mActionSaveProject );
  composerMenu->addSeparator();
  composerMenu->addAction( mActionNewComposer );
  composerMenu->addAction( mActionDuplicateComposer );
  composerMenu->addAction( mActionComposerManager );

  mPrintComposersMenu = new QMenu( tr( "Print &Composers" ), this );
  mPrintComposersMenu->setObjectName( QStringLiteral( "mPrintComposersMenu" ) );
  connect( mPrintComposersMenu, &QMenu::aboutToShow, this, &QgsComposer::populatePrintComposersMenu );
  composerMenu->addMenu( mPrintComposersMenu );

  composerMenu->addSeparator();
  composerMenu->addAction( mActionLoadFromTemplate );
  composerMenu->addAction( mActionSaveAsTemplate );
  composerMenu->addSeparator();
  composerMenu->addAction( mActionExportAsImage );
  composerMenu->addAction( mActionExportAsPDF );
  composerMenu->addAction( mActionExportAsSVG );
  composerMenu->addSeparator();
  composerMenu->addAction( mActionPageSetup );
  composerMenu->addAction( mActionPrint );
  composerMenu->addSeparator();
  composerMenu->addAction( mActionQuit );
  connect( mActionQuit, &QAction::triggered, this, &QWidget::close );

  //cut/copy/paste actions. Note these are not included in the ui file
  //as ui files have no support for QKeySequence shortcuts
  mActionCut = new QAction( tr( "Cu&t" ), this );
  mActionCut->setShortcuts( QKeySequence::Cut );
  mActionCut->setStatusTip( tr( "Cut" ) );
  mActionCut->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionEditCut.svg" ) ) );
  connect( mActionCut, &QAction::triggered, this, &QgsComposer::actionCutTriggered );

  mActionCopy = new QAction( tr( "&Copy" ), this );
  mActionCopy->setShortcuts( QKeySequence::Copy );
  mActionCopy->setStatusTip( tr( "Copy" ) );
  mActionCopy->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionEditCopy.svg" ) ) );
  connect( mActionCopy, &QAction::triggered, this, &QgsComposer::actionCopyTriggered );

  mActionPaste = new QAction( tr( "&Paste" ), this );
  mActionPaste->setShortcuts( QKeySequence::Paste );
  mActionPaste->setStatusTip( tr( "Paste" ) );
  mActionPaste->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionEditPaste.svg" ) ) );
  connect( mActionPaste, &QAction::triggered, this, &QgsComposer::actionPasteTriggered );

  QMenu *editMenu = menuBar()->addMenu( tr( "&Edit" ) );
  editMenu->addAction( mActionUndo );
  editMenu->addAction( mActionRedo );
  editMenu->addSeparator();

  //Backspace should also trigger delete selection
  QShortcut *backSpace = new QShortcut( QKeySequence( QStringLiteral( "Backspace" ) ), this );
  connect( backSpace, &QShortcut::activated, mActionDeleteSelection, &QAction::trigger );
  editMenu->addAction( mActionDeleteSelection );
  editMenu->addSeparator();

  editMenu->addAction( mActionCut );
  editMenu->addAction( mActionCopy );
  editMenu->addAction( mActionPaste );
  //TODO : "Ctrl+Shift+V" is one way to paste in place, but on some platforms you can use Shift+Ins and F18
  editMenu->addAction( mActionPasteInPlace );
  editMenu->addSeparator();
  editMenu->addAction( mActionSelectAll );
  editMenu->addAction( mActionDeselectAll );
  editMenu->addAction( mActionInvertSelection );
  editMenu->addAction( mActionSelectNextBelow );
  editMenu->addAction( mActionSelectNextAbove );

  mActionPreviewModeOff = new QAction( tr( "&Normal" ), this );
  mActionPreviewModeOff->setStatusTip( tr( "Normal" ) );
  mActionPreviewModeOff->setCheckable( true );
  mActionPreviewModeOff->setChecked( true );
  connect( mActionPreviewModeOff, &QAction::triggered, this, &QgsComposer::disablePreviewMode );
  mActionPreviewModeGrayscale = new QAction( tr( "Simulate Photocopy (&Grayscale)" ), this );
  mActionPreviewModeGrayscale->setStatusTip( tr( "Simulate photocopy (grayscale)" ) );
  mActionPreviewModeGrayscale->setCheckable( true );
  connect( mActionPreviewModeGrayscale, &QAction::triggered, this, &QgsComposer::activateGrayscalePreview );
  mActionPreviewModeMono = new QAction( tr( "Simulate Fax (&Mono)" ), this );
  mActionPreviewModeMono->setStatusTip( tr( "Simulate fax (mono)" ) );
  mActionPreviewModeMono->setCheckable( true );
  connect( mActionPreviewModeMono, &QAction::triggered, this, &QgsComposer::activateMonoPreview );
  mActionPreviewProtanope = new QAction( tr( "Simulate Color Blindness (&Protanope)" ), this );
  mActionPreviewProtanope->setStatusTip( tr( "Simulate color blindness (Protanope)" ) );
  mActionPreviewProtanope->setCheckable( true );
  connect( mActionPreviewProtanope, &QAction::triggered, this, &QgsComposer::activateProtanopePreview );
  mActionPreviewDeuteranope = new QAction( tr( "Simulate Color Blindness (&Deuteranope)" ), this );
  mActionPreviewDeuteranope->setStatusTip( tr( "Simulate color blindness (Deuteranope)" ) );
  mActionPreviewDeuteranope->setCheckable( true );
  connect( mActionPreviewDeuteranope, &QAction::triggered, this, &QgsComposer::activateDeuteranopePreview );

  QActionGroup *mPreviewGroup = new QActionGroup( this );
  mPreviewGroup->setExclusive( true );
  mActionPreviewModeOff->setActionGroup( mPreviewGroup );
  mActionPreviewModeGrayscale->setActionGroup( mPreviewGroup );
  mActionPreviewModeMono->setActionGroup( mPreviewGroup );
  mActionPreviewProtanope->setActionGroup( mPreviewGroup );
  mActionPreviewDeuteranope->setActionGroup( mPreviewGroup );

  QMenu *viewMenu = menuBar()->addMenu( tr( "&View" ) );
  //Ctrl+= should also trigger zoom in
  QShortcut *ctrlEquals = new QShortcut( QKeySequence( QStringLiteral( "Ctrl+=" ) ), this );
  connect( ctrlEquals, &QShortcut::activated, mActionZoomIn, &QAction::trigger );

  QMenu *previewMenu = viewMenu->addMenu( QStringLiteral( "&Preview" ) );
  previewMenu->addAction( mActionPreviewModeOff );
  previewMenu->addAction( mActionPreviewModeGrayscale );
  previewMenu->addAction( mActionPreviewModeMono );
  previewMenu->addAction( mActionPreviewProtanope );
  previewMenu->addAction( mActionPreviewDeuteranope );

  viewMenu->addSeparator();
  viewMenu->addAction( mActionZoomIn );
  viewMenu->addAction( mActionZoomOut );
  viewMenu->addAction( mActionZoomAll );
  viewMenu->addAction( mActionZoomActual );
  viewMenu->addSeparator();
  viewMenu->addAction( mActionRefreshView );
  viewMenu->addSeparator();
  viewMenu->addAction( mActionShowGrid );
  viewMenu->addAction( mActionSnapGrid );
  viewMenu->addSeparator();
  viewMenu->addAction( mActionShowGuides );
  viewMenu->addAction( mActionSnapGuides );
  viewMenu->addAction( mActionSmartGuides );
  viewMenu->addAction( mActionClearGuides );
  viewMenu->addSeparator();
  viewMenu->addAction( mActionShowBoxes );
  viewMenu->addAction( mActionShowRulers );
  viewMenu->addAction( mActionShowPage );

  // Panel and toolbar submenus
  mPanelMenu = new QMenu( tr( "P&anels" ), this );
  mPanelMenu->setObjectName( QStringLiteral( "mPanelMenu" ) );
  mToolbarMenu = new QMenu( tr( "&Toolbars" ), this );
  mToolbarMenu->setObjectName( QStringLiteral( "mToolbarMenu" ) );
  viewMenu->addSeparator();
  viewMenu->addMenu( mPanelMenu );
  viewMenu->addMenu( mToolbarMenu );
  viewMenu->addAction( mActionToggleFullScreen );
  viewMenu->addAction( mActionHidePanels );
  // toolBar already exists, add other widgets as they are created
  mToolbarMenu->addAction( mComposerToolbar->toggleViewAction() );
  mToolbarMenu->addAction( mPaperNavToolbar->toggleViewAction() );
  mToolbarMenu->addAction( mItemActionToolbar->toggleViewAction() );
  mToolbarMenu->addAction( mItemToolbar->toggleViewAction() );

  QMenu *layoutMenu = menuBar()->addMenu( tr( "&Layout" ) );
  layoutMenu->addAction( mActionAddNewMap );
  layoutMenu->addAction( mActionAddNewLabel );
  layoutMenu->addAction( mActionAddNewScalebar );
  layoutMenu->addAction( mActionAddNewLegend );
  layoutMenu->addAction( mActionAddImage );
  QMenu *shapeMenu = layoutMenu->addMenu( QStringLiteral( "Add Shape" ) );
  shapeMenu->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddBasicShape.svg" ) ) );
  shapeMenu->addAction( mActionAddRectangle );
  shapeMenu->addAction( mActionAddTriangle );
  shapeMenu->addAction( mActionAddEllipse );

  QMenu *nodesItemMenu = layoutMenu->addMenu( QStringLiteral( "Add Nodes Item" ) );
  nodesItemMenu->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddNodesItem.svg" ) ) );
  nodesItemMenu->addAction( mActionAddPolygon );
  nodesItemMenu->addAction( mActionAddPolyline );

  layoutMenu->addAction( mActionAddArrow );
  //layoutMenu->addAction( mActionAddTable );
  layoutMenu->addAction( mActionAddAttributeTable );
  layoutMenu->addAction( mActionAddHtml );
  layoutMenu->addSeparator();
  layoutMenu->addAction( mActionSelectMoveItem );
  layoutMenu->addAction( mActionMoveItemContent );
  layoutMenu->addAction( mActionEditNodesItem );
  layoutMenu->addSeparator();
  layoutMenu->addAction( mActionGroupItems );
  layoutMenu->addAction( mActionUngroupItems );
  layoutMenu->addSeparator();
  layoutMenu->addAction( mActionRaiseItems );
  layoutMenu->addAction( mActionLowerItems );
  layoutMenu->addAction( mActionMoveItemsToTop );
  layoutMenu->addAction( mActionMoveItemsToBottom );
  layoutMenu->addAction( mActionLockItems );
  layoutMenu->addAction( mActionUnlockAll );

  QMenu *atlasMenu = menuBar()->addMenu( tr( "&Atlas" ) );
  atlasMenu->addAction( mActionAtlasPreview );
  atlasMenu->addAction( mActionAtlasFirst );
  atlasMenu->addAction( mActionAtlasPrev );
  atlasMenu->addAction( mActionAtlasNext );
  atlasMenu->addAction( mActionAtlasLast );
  atlasMenu->addSeparator();
  atlasMenu->addAction( mActionPrintAtlas );
  atlasMenu->addAction( mActionExportAtlasAsImage );
  atlasMenu->addAction( mActionExportAtlasAsSVG );
  atlasMenu->addAction( mActionExportAtlasAsPDF );
  atlasMenu->addSeparator();
  atlasMenu->addAction( mActionAtlasSettings );

  QToolButton *atlasExportToolButton = new QToolButton( mAtlasToolbar );
  atlasExportToolButton->setPopupMode( QToolButton::InstantPopup );
  atlasExportToolButton->setAutoRaise( true );
  atlasExportToolButton->setToolButtonStyle( Qt::ToolButtonIconOnly );
  atlasExportToolButton->addAction( mActionExportAtlasAsImage );
  atlasExportToolButton->addAction( mActionExportAtlasAsSVG );
  atlasExportToolButton->addAction( mActionExportAtlasAsPDF );
  atlasExportToolButton->setDefaultAction( mActionExportAtlasAsImage );
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
  connect( mAtlasPageComboBox->lineEdit(), &QLineEdit::editingFinished, this, &QgsComposer::atlasPageComboEditingFinished );
  connect( mAtlasPageComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsComposer::atlasPageComboEditingFinished );
  mAtlasToolbar->insertWidget( mActionAtlasNext, mAtlasPageComboBox );

  QMenu *settingsMenu = menuBar()->addMenu( tr( "&Settings" ) );
  settingsMenu->addAction( mActionOptions );

#ifdef Q_OS_MAC
  // this doesn't work on Mac anymore: menuBar()->addMenu( mQgis->windowMenu() );
  // QgsComposer::populateWithOtherMenu should work recursively with submenus and regardless of Qt version
  mWindowMenu = new QMenu( tr( "Window" ), this );
  mWindowMenu->setObjectName( "mWindowMenu" );
  connect( mWindowMenu, SIGNAL( aboutToShow() ), this, SLOT( populateWindowMenu() ) );
  menuBar()->addMenu( mWindowMenu );

  mHelpMenu = new QMenu( tr( "Help" ), this );
  mHelpMenu->setObjectName( "mHelpMenu" );
  connect( mHelpMenu, SIGNAL( aboutToShow() ), this, SLOT( populateHelpMenu() ) );
  menuBar()->addMenu( mHelpMenu );
#endif

  setMouseTracking( true );
  mViewFrame->setMouseTracking( true );

  mStatusZoomCombo = new QComboBox( mStatusBar );
  mStatusZoomCombo->setEditable( true );
  mStatusZoomCombo->setInsertPolicy( QComboBox::NoInsert );
  mStatusZoomCombo->setCompleter( nullptr );
  mStatusZoomCombo->setMinimumWidth( 100 );
  //zoom combo box accepts decimals in the range 1-9999, with an optional decimal point and "%" sign
  QRegExp zoomRx( "\\s*\\d{1,4}(\\.\\d?)?\\s*%?" );
  QValidator *zoomValidator = new QRegExpValidator( zoomRx, mStatusZoomCombo );
  mStatusZoomCombo->lineEdit()->setValidator( zoomValidator );

  //add some nice zoom levels to the zoom combobox
  mStatusZoomLevelsList << 0.125 << 0.25 << 0.5 << 1.0 << 2.0 << 4.0 << 8.0;
  QList<double>::iterator zoom_it;
  for ( zoom_it = mStatusZoomLevelsList.begin(); zoom_it != mStatusZoomLevelsList.end(); ++zoom_it )
  {
    mStatusZoomCombo->insertItem( 0, tr( "%1%" ).arg( *zoom_it * 100.0, 0, 'f', 1 ) );
  }
  connect( mStatusZoomCombo, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsComposer::statusZoomCombo_currentIndexChanged );
  connect( mStatusZoomCombo->lineEdit(), &QLineEdit::returnPressed, this, &QgsComposer::statusZoomCombo_zoomEntered );

  //create status bar labels
  mStatusCursorXLabel = new QLabel( mStatusBar );
  mStatusCursorXLabel->setMinimumWidth( 100 );
  mStatusCursorYLabel = new QLabel( mStatusBar );
  mStatusCursorYLabel->setMinimumWidth( 100 );
  mStatusCursorPageLabel = new QLabel( mStatusBar );
  mStatusCursorPageLabel->setMinimumWidth( 100 );
  mStatusCompositionLabel = new QLabel( mStatusBar );
  mStatusCompositionLabel->setMinimumWidth( 350 );
  mStatusAtlasLabel = new QLabel( mStatusBar );

  //hide borders from child items in status bar under Windows
  mStatusBar->setStyleSheet( QStringLiteral( "QStatusBar::item {border: none;}" ) );

  mStatusBar->addWidget( mStatusCursorXLabel );
  mStatusBar->addWidget( mStatusCursorYLabel );
  mStatusBar->addWidget( mStatusCursorPageLabel );
  mStatusBar->addWidget( mStatusZoomCombo );
  mStatusBar->addWidget( mStatusCompositionLabel );
  mStatusBar->addWidget( mStatusAtlasLabel );

  //create composer view and layout with rulers
  mView = nullptr;
  mViewLayout = new QGridLayout();
  mViewLayout->setSpacing( 0 );
  mViewLayout->setMargin( 0 );
  mHorizontalRuler = new QgsComposerRuler( QgsComposerRuler::Horizontal );
  mVerticalRuler = new QgsComposerRuler( QgsComposerRuler::Vertical );
  mRulerLayoutFix = new QWidget();
  mRulerLayoutFix->setAttribute( Qt::WA_NoMousePropagation );
  mRulerLayoutFix->setBackgroundRole( QPalette::Window );
  mRulerLayoutFix->setFixedSize( mVerticalRuler->rulerSize(), mHorizontalRuler->rulerSize() );
  mViewLayout->addWidget( mRulerLayoutFix, 0, 0 );
  mViewLayout->addWidget( mHorizontalRuler, 0, 1 );
  mViewLayout->addWidget( mVerticalRuler, 1, 0 );
  createComposerView();
  mViewFrame->setLayout( mViewLayout );

  //initial state of rulers
  QgsSettings myQSettings;
  bool showRulers = myQSettings.value( QStringLiteral( "Composer/showRulers" ), true ).toBool();
  mActionShowRulers->blockSignals( true );
  mActionShowRulers->setChecked( showRulers );
  mHorizontalRuler->setVisible( showRulers );
  mVerticalRuler->setVisible( showRulers );
  mRulerLayoutFix->setVisible( showRulers );
  mActionShowRulers->blockSignals( false );
  connect( mActionShowRulers, &QAction::triggered, this, &QgsComposer::toggleRulers );

  mActionUndo->setEnabled( false );
  mActionRedo->setEnabled( false );
  if ( mComposition->undoStack() )
  {
    connect( mComposition->undoStack(), &QUndoStack::canUndoChanged, mActionUndo, &QAction::setEnabled );
    connect( mComposition->undoStack(), &QUndoStack::canRedoChanged, mActionRedo, &QAction::setEnabled );
  }

  mActionShowPage->setChecked( mComposition->pagesVisible() );
  restoreGridSettings();
  connectViewSlots();
  connectCompositionSlots();
  connectOtherSlots();

  mView->setComposition( mComposition );

  int minDockWidth( 335 );

  setTabPosition( Qt::AllDockWidgetAreas, QTabWidget::North );
  mGeneralDock = new QgsDockWidget( tr( "Composition" ), this );
  mGeneralDock->setObjectName( QStringLiteral( "CompositionDock" ) );
  mGeneralDock->setMinimumWidth( minDockWidth );
  mGeneralPropertiesStack = new QgsPanelWidgetStack();
  mGeneralDock->setWidget( mGeneralPropertiesStack );
  mPanelMenu->addAction( mGeneralDock->toggleViewAction() );
  mItemDock = new QgsDockWidget( tr( "Item properties" ), this );
  mItemDock->setObjectName( QStringLiteral( "ItemDock" ) );
  mItemDock->setMinimumWidth( minDockWidth );
  mItemPropertiesStack = new QgsPanelWidgetStack();
  mItemDock->setWidget( mItemPropertiesStack );
  mPanelMenu->addAction( mItemDock->toggleViewAction() );
  mUndoDock = new QgsDockWidget( tr( "Command history" ), this );
  mUndoDock->setObjectName( QStringLiteral( "CommandDock" ) );
  mPanelMenu->addAction( mUndoDock->toggleViewAction() );
  mAtlasDock = new QgsDockWidget( tr( "Atlas generation" ), this );
  mAtlasDock->setObjectName( QStringLiteral( "AtlasDock" ) );
  mPanelMenu->addAction( mAtlasDock->toggleViewAction() );
  mItemsDock = new QgsDockWidget( tr( "Items" ), this );
  mItemsDock->setObjectName( QStringLiteral( "ItemsDock" ) );
  mPanelMenu->addAction( mItemsDock->toggleViewAction() );

  QList<QDockWidget *> docks = findChildren<QDockWidget *>();
  Q_FOREACH ( QDockWidget *dock, docks )
  {
    connect( dock, &QDockWidget::visibilityChanged, this, &QgsComposer::dockVisibilityChanged );
  }

  createCompositionWidget();

  //undo widget
  mUndoView = new QUndoView( mComposition->undoStack(), this );
  mUndoDock->setWidget( mUndoView );

  //items tree widget
  mItemsTreeView = new QTreeView( mItemsDock );
  mItemsTreeView->setModel( mComposition->itemsModel() );
#ifdef ENABLE_MODELTEST
  new ModelTest( mComposition->itemsModel(), this );
#endif

  mItemsTreeView->setColumnWidth( 0, 30 );
  mItemsTreeView->setColumnWidth( 1, 30 );
  mItemsTreeView->header()->setSectionResizeMode( 0, QHeaderView::Fixed );
  mItemsTreeView->header()->setSectionResizeMode( 1, QHeaderView::Fixed );
  mItemsTreeView->header()->setSectionsMovable( false );

  mItemsTreeView->setDragEnabled( true );
  mItemsTreeView->setAcceptDrops( true );
  mItemsTreeView->setDropIndicatorShown( true );
  mItemsTreeView->setDragDropMode( QAbstractItemView::InternalMove );

  mItemsTreeView->setIndentation( 0 );
  mItemsDock->setWidget( mItemsTreeView );
  connect( mItemsTreeView->selectionModel(), &QItemSelectionModel::currentChanged, mComposition->itemsModel(), &QgsComposerModel::setSelected );

  addDockWidget( Qt::RightDockWidgetArea, mItemDock );
  addDockWidget( Qt::RightDockWidgetArea, mGeneralDock );
  addDockWidget( Qt::RightDockWidgetArea, mUndoDock );
  addDockWidget( Qt::RightDockWidgetArea, mAtlasDock );
  addDockWidget( Qt::RightDockWidgetArea, mItemsDock );

  QgsAtlasCompositionWidget *atlasWidget = new QgsAtlasCompositionWidget( mGeneralDock, mComposition );
  mAtlasDock->setWidget( atlasWidget );

  mItemDock->show();
  mGeneralDock->show();
  mUndoDock->show();
  mAtlasDock->show();
  mItemsDock->show();

  tabifyDockWidget( mGeneralDock, mUndoDock );
  tabifyDockWidget( mItemDock, mUndoDock );
  tabifyDockWidget( mGeneralDock, mItemDock );
  tabifyDockWidget( mItemDock, mAtlasDock );
  tabifyDockWidget( mItemDock, mItemsDock );

  mGeneralDock->raise();

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
  QgsAtlasComposition *atlasMap = &mComposition->atlasComposition();
  connect( atlasMap, &QgsAtlasComposition::toggled, this, &QgsComposer::toggleAtlasControls );
  connect( atlasMap, &QgsAtlasComposition::numberFeaturesChanged, this, &QgsComposer::updateAtlasPageComboBox );
  connect( atlasMap, &QgsAtlasComposition::featureChanged, this, &QgsComposer::atlasFeatureChanged );
  toggleAtlasControls( atlasMap->enabled() && atlasMap->coverageLayer() );

  // Create size grip (needed by Mac OS X for QMainWindow if QStatusBar is not visible)
  //should not be needed now that composer has a status bar?
#if 0
  mSizeGrip = new QSizeGrip( this );
  mSizeGrip->resize( mSizeGrip->sizeHint() );
  mSizeGrip->move( rect().bottomRight() - mSizeGrip->rect().bottomRight() );
#endif

  restoreWindowState();
  setSelectionTool();

  mView->setFocus();

#if defined(ANDROID)
  // fix for Qt Ministro hiding app's menubar in favor of native Android menus
  menuBar()->setNativeMenuBar( false );
  menuBar()->setVisible( true );
#endif
}

QgsComposer::~QgsComposer()
{
  mComposition->setAllDeselected();
  delete mPrinter;
}

QgsComposerInterface *QgsComposer::iface()
{
  return mInterface;
}

void QgsComposer::setupTheme()
{
  //now set all the icons - getThemeIcon will fall back to default theme if its
  //missing from active theme
  mActionQuit->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionFileExit.png" ) ) );
  mActionSaveProject->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionFileSave.svg" ) ) );
  mActionNewComposer->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionNewComposer.svg" ) ) );
  mActionDuplicateComposer->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionDuplicateComposer.svg" ) ) );
  mActionComposerManager->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionComposerManager.svg" ) ) );
  mActionLoadFromTemplate->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionFileOpen.svg" ) ) );
  mActionSaveAsTemplate->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionFileSaveAs.svg" ) ) );
  mActionExportAsImage->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionSaveMapAsImage.svg" ) ) );
  mActionExportAsSVG->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionSaveAsSVG.svg" ) ) );
  mActionExportAsPDF->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionSaveAsPDF.svg" ) ) );
  mActionPrint->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionFilePrint.svg" ) ) );
  mActionZoomAll->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomFullExtent.svg" ) ) );
  mActionZoomIn->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomIn.svg" ) ) );
  mActionZoomOut->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomOut.svg" ) ) );
  mActionZoomActual->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomActual.svg" ) ) );
  mActionMouseZoom->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomToArea.svg" ) ) );
  mActionRefreshView->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionDraw.svg" ) ) );
  mActionUndo->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionUndo.svg" ) ) );
  mActionRedo->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionRedo.svg" ) ) );
  mActionAddImage->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddImage.svg" ) ) );
  mActionAddNewMap->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddMap.svg" ) ) );
  mActionAddNewLabel->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionLabel.svg" ) ) );
  mActionAddNewLegend->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddLegend.svg" ) ) );
  mActionAddNewScalebar->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionScaleBar.svg" ) ) );
  mActionAddRectangle->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddBasicRectangle.svg" ) ) );
  mActionAddTriangle->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddBasicTriangle.svg" ) ) );
  mActionAddEllipse->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddBasicCircle.svg" ) ) );
  mActionAddPolygon->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddPolygon.svg" ) ) );
  mActionAddPolyline->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddPolyline.svg" ) ) );
  mActionAddArrow->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddArrow.svg" ) ) );
  mActionAddTable->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddTable.svg" ) ) );
  mActionAddAttributeTable->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddTable.svg" ) ) );
  mActionAddHtml->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddHtml.svg" ) ) );
  mActionSelectMoveItem->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionSelect.svg" ) ) );
  mActionMoveItemContent->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionMoveItemContent.svg" ) ) );
  mActionEditNodesItem->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionEditNodesItem.svg" ) ) );
  mActionGroupItems->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionGroupItems.svg" ) ) );
  mActionUngroupItems->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionUngroupItems.svg" ) ) );
  mActionRaiseItems->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionRaiseItems.svg" ) ) );
  mActionLowerItems->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionLowerItems.svg" ) ) );
  mActionMoveItemsToTop->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionMoveItemsToTop.svg" ) ) );
  mActionMoveItemsToBottom->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionMoveItemsToBottom.svg" ) ) );
  mActionAlignLeft->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAlignLeft.svg" ) ) );
  mActionAlignHCenter->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAlignHCenter.svg" ) ) );
  mActionAlignRight->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAlignRight.svg" ) ) );
  mActionAlignTop->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAlignTop.svg" ) ) );
  mActionAlignVCenter->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAlignVCenter.svg" ) ) );
  mActionAlignBottom->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAlignBottom.svg" ) ) );
}

void QgsComposer::setIconSizes( int size )
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

void QgsComposer::connectViewSlots()
{
  if ( !mView )
  {
    return;
  }

  connect( mView, &QgsComposerView::selectedItemChanged, this, &QgsComposer::showItemOptions );
  connect( mView, &QgsComposerView::itemRemoved, this, &QgsComposer::deleteItem );
  connect( mView, &QgsComposerView::actionFinished, this, &QgsComposer::setSelectionTool );

  //listen out for position updates from the QgsComposerView
  connect( mView, &QgsComposerView::cursorPosChanged, this, &QgsComposer::updateStatusCursorPos );
  connect( mView, &QgsComposerView::zoomLevelChanged, this, &QgsComposer::updateStatusZoom );

  connect( mView, &QgsComposerView::zoomLevelChanged, this, &QgsComposer::invalidateCachedRenders );
}

void QgsComposer::connectCompositionSlots()
{
  if ( !mComposition )
  {
    return;
  }

  connect( mComposition, &QgsComposition::nameChanged, this, &QgsComposer::setWindowTitle );
  connect( mComposition, &QgsComposition::selectedItemChanged, this, &QgsComposer::showItemOptions );
  connect( mComposition, &QgsComposition::itemRemoved, this, &QgsComposer::deleteItem );
  connect( mComposition, &QgsComposition::paperSizeChanged, this, [ = ]
  {
    mHorizontalRuler->update();
    mVerticalRuler->update();
  } );
  connect( mComposition, &QgsComposition::nPagesChanged, this, [ = ]
  {
    mHorizontalRuler->update();
    mVerticalRuler->update();
  } );

  //listen out to status bar updates from the atlas
  QgsAtlasComposition *atlasMap = &mComposition->atlasComposition();
  connect( atlasMap, &QgsAtlasComposition::statusMsgChanged, this, &QgsComposer::updateStatusAtlasMsg );

  //listen out to status bar updates from the composition
  connect( mComposition, &QgsComposition::statusMsgChanged, this, &QgsComposer::updateStatusCompositionMsg );
}

void QgsComposer::connectOtherSlots()
{
  //also listen out for position updates from the horizontal/vertical rulers
  connect( mHorizontalRuler, &QgsComposerRuler::cursorPosChanged, this, &QgsComposer::updateStatusCursorPos );
  connect( mVerticalRuler, &QgsComposerRuler::cursorPosChanged, this, &QgsComposer::updateStatusCursorPos );
  //listen out for zoom updates
  connect( this, &QgsComposer::zoomLevelChanged, this, &QgsComposer::updateStatusZoom );
  connect( this, &QgsComposer::zoomLevelChanged, this, &QgsComposer::invalidateCachedRenders );
}

void QgsComposer::open()
{
  show();
  activate();
  zoomFull(); // zoomFull() does not work properly until we have called show()
  if ( mView )
  {
    mView->updateRulers();
  }
}

void QgsComposer::activate()
{
  bool shown = isVisible();
  show();
  raise();
  setWindowState( windowState() & ~Qt::WindowMinimized );
  activateWindow();
  if ( !shown )
  {
    mActionZoomAll_triggered();
  }
}

bool QgsComposer::loadFromTemplate( const QDomDocument &templateDoc, bool clearExisting )
{
  // provide feedback, since composer will be hidden when loading template (much faster)
  std::unique_ptr< QDialog > dlg( new QgsBusyIndicatorDialog( tr( "Loading template into composer..." ), this ) );
  dlg->setStyleSheet( mQgis->styleSheet() );
  dlg->show();

  setUpdatesEnabled( false );
  bool result = mComposition->loadFromTemplate( templateDoc, nullptr, false, clearExisting );
  cleanupAfterTemplateRead();
  setUpdatesEnabled( true );

  dlg->close();

  if ( result )
  {
    // update composition widget
    QgsCompositionWidget *oldCompositionWidget = qobject_cast<QgsCompositionWidget *>( mGeneralPropertiesStack->takeMainPanel() );
    delete oldCompositionWidget;
    createCompositionWidget();
  }

  return result;
}

void QgsComposer::updateStatusCursorPos( QPointF cursorPosition )
{
  if ( !mComposition )
  {
    return;
  }

  //convert cursor position to position on current page
  QPointF pagePosition = mComposition->positionOnPage( cursorPosition );
  int currentPage = mComposition->pageNumberForPoint( cursorPosition );

  mStatusCursorXLabel->setText( QString( tr( "x: %1 mm" ) ).arg( pagePosition.x() ) );
  mStatusCursorYLabel->setText( QString( tr( "y: %1 mm" ) ).arg( pagePosition.y() ) );
  mStatusCursorPageLabel->setText( QString( tr( "page: %3" ) ).arg( currentPage ) );
}

void QgsComposer::updateStatusZoom()
{
  double dpi = QgsApplication::desktop()->logicalDpiX();
  //monitor dpi is not always correct - so make sure the value is sane
  if ( ( dpi < 60 ) || ( dpi > 250 ) )
    dpi = 72;

  //pixel width for 1mm on screen
  double scale100 = dpi / 25.4;
  //current zoomLevel
  double zoomLevel = mView->transform().m11() * 100 / scale100;

  whileBlocking( mStatusZoomCombo )->lineEdit()->setText( tr( "%1%" ).arg( zoomLevel, 0, 'f', 1 ) );
}

void QgsComposer::statusZoomCombo_currentIndexChanged( int index )
{
  double selectedZoom = mStatusZoomLevelsList.at( mStatusZoomLevelsList.count() - index - 1 );
  if ( mView )
  {
    mView->setZoomLevel( selectedZoom );
    //update zoom combobox text for correct format (one decimal place, trailing % sign)
    whileBlocking( mStatusZoomCombo )->lineEdit()->setText( tr( "%1%" ).arg( selectedZoom * 100.0, 0, 'f', 1 ) );
  }
}

void QgsComposer::statusZoomCombo_zoomEntered()
{
  if ( !mView )
  {
    return;
  }

  //need to remove spaces and "%" characters from input text
  QString zoom = mStatusZoomCombo->currentText().remove( QChar( '%' ) ).trimmed();
  mView->setZoomLevel( zoom.toDouble() / 100 );
}

void QgsComposer::updateStatusCompositionMsg( const QString &message )
{
  mStatusCompositionLabel->setText( message );
}

void QgsComposer::updateStatusAtlasMsg( const QString &message )
{
  mStatusAtlasLabel->setText( message );
}

void QgsComposer::showItemOptions( QgsComposerItem *item )
{
  if ( !item )
  {
    delete mItemPropertiesStack->takeMainPanel();
    return;
  }

  std::unique_ptr< QgsPanelWidget > widget( createItemWidget( item ) );
  if ( ! widget )
  {
    return;
  }

  delete mItemPropertiesStack->takeMainPanel();
  widget->setDockMode( true );
  mItemPropertiesStack->setMainPanel( widget.release() );
}

void QgsComposer::mActionOptions_triggered()
{
  mQgis->showOptionsDialog( this, QStringLiteral( "mOptionsPageComposer" ) );
}

void QgsComposer::toggleAtlasControls( bool atlasEnabled )
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

void QgsComposer::updateAtlasPageComboBox( int pageCount )
{
  if ( !mComposition )
    return;

  mAtlasPageComboBox->blockSignals( true );
  mAtlasPageComboBox->clear();
  for ( int i = 1; i <= pageCount && i < 500; ++i )
  {
    QString name = mComposition->atlasComposition().nameForPage( i - 1 );
    QString fullName = ( !name.isEmpty() ? QStringLiteral( "%1: %2" ).arg( i ).arg( name ) : QString::number( i ) );

    mAtlasPageComboBox->addItem( fullName, i );
    mAtlasPageComboBox->setItemData( i - 1, name, Qt::UserRole + 1 );
    mAtlasPageComboBox->setItemData( i - 1, fullName, Qt::UserRole + 2 );
  }
  mAtlasPageComboBox->blockSignals( false );
}

void QgsComposer::atlasFeatureChanged( QgsFeature *feature )
{
  Q_UNUSED( feature );

  if ( !mComposition )
    return;

  mAtlasPageComboBox->blockSignals( true );
  //prefer to set index of current atlas page, if combo box is showing enough page items
  if ( mComposition->atlasComposition().currentFeatureNumber() < mAtlasPageComboBox->count() )
  {
    mAtlasPageComboBox->setCurrentIndex( mComposition->atlasComposition().currentFeatureNumber() );
  }
  else
  {
    //fallback to setting the combo text to the page number
    mAtlasPageComboBox->setEditText( QString::number( mComposition->atlasComposition().currentFeatureNumber() + 1 ) );
  }
  mAtlasPageComboBox->blockSignals( false );

  //update expression context variables in map canvas to allow for previewing atlas feature based rendering
  mapCanvas()->expressionContextScope().addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "atlas_featurenumber" ), mComposition->atlasComposition().currentFeatureNumber() + 1, true ) );
  mapCanvas()->expressionContextScope().addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "atlas_pagename" ), mComposition->atlasComposition().currentPageName(), true ) );
  QgsFeature atlasFeature = mComposition->atlasComposition().feature();
  mapCanvas()->expressionContextScope().addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "atlas_feature" ), QVariant::fromValue( atlasFeature ), true ) );
  mapCanvas()->expressionContextScope().addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "atlas_featureid" ), atlasFeature.id(), true ) );
  mapCanvas()->expressionContextScope().addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "atlas_geometry" ), QVariant::fromValue( atlasFeature.geometry() ), true ) );
}

void QgsComposer::invalidateCachedRenders()
{
  //redraw cached map items
  QList< QgsComposerMap *> maps;
  mComposition->composerItems( maps );

  Q_FOREACH ( QgsComposerMap *map, maps )
  {
    map->invalidateCache();
  }
}

QgsPanelWidget *QgsComposer::createItemWidget( QgsComposerItem *item )
{
  if ( !item )
    return nullptr;

  switch ( item->type() )
  {
    case QgsComposerItem::ComposerArrow:
      return new QgsComposerArrowWidget( static_cast< QgsComposerArrow * >( item ) );

    case QgsComposerItem::ComposerPolygon:
      return new QgsComposerPolygonWidget( static_cast< QgsComposerPolygon * >( item ) );

    case QgsComposerItem::ComposerPolyline:
      return new QgsComposerPolylineWidget( static_cast< QgsComposerPolyline * >( item ) );

    case QgsComposerItem::ComposerLabel:
      return new QgsComposerLabelWidget( static_cast< QgsComposerLabel * >( item ) );

    case QgsComposerItem::ComposerMap:
      return new QgsComposerMapWidget( static_cast< QgsComposerMap * >( item ) );

    case QgsComposerItem::ComposerScaleBar:
      return new QgsComposerScaleBarWidget( static_cast< QgsComposerScaleBar * >( item ) );

    case QgsComposerItem::ComposerLegend:
      return new QgsComposerLegendWidget( static_cast< QgsComposerLegend * >( item ) );

    case QgsComposerItem::ComposerPicture:
      return new QgsComposerPictureWidget( static_cast< QgsComposerPicture * >( item ) );

    case QgsComposerItem::ComposerShape:
      return new QgsComposerShapeWidget( static_cast< QgsComposerShape * >( item ) );

    case QgsComposerItem::ComposerFrame:
    {
      QgsComposerFrame *frame = static_cast< QgsComposerFrame * >( item );
      if ( QgsComposerHtml *html = dynamic_cast< QgsComposerHtml * >( frame->multiFrame() ) )
      {
        return new QgsComposerHtmlWidget( html, frame );
      }
      else if ( QgsComposerAttributeTableV2 *table = dynamic_cast< QgsComposerAttributeTableV2 * >( frame->multiFrame() ) )
      {
        return new QgsComposerAttributeTableWidget( table, frame );
      }
      break;
    }

  }
  return nullptr; // no warnings!
}

void QgsComposer::mActionAtlasPreview_triggered( bool checked )
{
  QgsAtlasComposition *atlasMap = &mComposition->atlasComposition();

  //check if composition has an atlas map enabled
  if ( checked && !atlasMap->enabled() )
  {
    //no atlas current enabled
    QMessageBox::warning( nullptr, tr( "Enable atlas preview" ),
                          tr( "Atlas in not currently enabled for this composition!" ),
                          QMessageBox::Ok,
                          QMessageBox::Ok );
    whileBlocking( mActionAtlasPreview )->setChecked( false );
    mStatusAtlasLabel->setText( QString() );
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

  bool previewEnabled = mComposition->setAtlasMode( checked ? QgsComposition::PreviewAtlas : QgsComposition::AtlasOff );
  if ( !previewEnabled )
  {
    //something went wrong, e.g., no matching features
    QMessageBox::warning( nullptr, tr( "Enable atlas preview" ),
                          tr( "No matching atlas features found!" ),
                          QMessageBox::Ok,
                          QMessageBox::Ok );
    mActionAtlasPreview->blockSignals( true );
    mActionAtlasPreview->setChecked( false );
    mActionAtlasFirst->setEnabled( false );
    mActionAtlasLast->setEnabled( false );
    mActionAtlasNext->setEnabled( false );
    mActionAtlasPrev->setEnabled( false );
    mAtlasPageComboBox->setEnabled( false );
    mActionAtlasPreview->blockSignals( false );
    mStatusAtlasLabel->setText( QString() );
    return;
  }

  if ( checked )
  {
    mapCanvas()->stopRendering();
    emit atlasPreviewFeatureChanged();
  }
  else
  {
    mStatusAtlasLabel->setText( QString() );
  }
}

void QgsComposer::mActionAtlasNext_triggered()
{
  QgsAtlasComposition *atlasMap = &mComposition->atlasComposition();
  if ( !atlasMap->enabled() )
  {
    return;
  }

  mapCanvas()->stopRendering();

  loadAtlasPredefinedScalesFromProject();
  atlasMap->nextFeature();
  emit atlasPreviewFeatureChanged();
}

void QgsComposer::mActionAtlasPrev_triggered()
{
  QgsAtlasComposition *atlasMap = &mComposition->atlasComposition();
  if ( !atlasMap->enabled() )
  {
    return;
  }

  mapCanvas()->stopRendering();

  loadAtlasPredefinedScalesFromProject();
  atlasMap->prevFeature();
  emit atlasPreviewFeatureChanged();
}

void QgsComposer::mActionAtlasFirst_triggered()
{
  QgsAtlasComposition *atlasMap = &mComposition->atlasComposition();
  if ( !atlasMap->enabled() )
  {
    return;
  }

  mapCanvas()->stopRendering();

  loadAtlasPredefinedScalesFromProject();
  atlasMap->firstFeature();
  emit atlasPreviewFeatureChanged();
}

void QgsComposer::mActionAtlasLast_triggered()
{
  QgsAtlasComposition *atlasMap = &mComposition->atlasComposition();
  if ( !atlasMap->enabled() )
  {
    return;
  }

  mapCanvas()->stopRendering();

  loadAtlasPredefinedScalesFromProject();
  atlasMap->lastFeature();
  emit atlasPreviewFeatureChanged();
}

void QgsComposer::atlasPageComboEditingFinished()
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

  if ( !ok || page > mComposition->atlasComposition().numFeatures() || page < 1 )
  {
    whileBlocking( mAtlasPageComboBox )->setCurrentIndex( mComposition->atlasComposition().currentFeatureNumber() );
  }
  else if ( page != mComposition->atlasComposition().currentFeatureNumber() + 1 )
  {
    mapCanvas()->stopRendering();
    loadAtlasPredefinedScalesFromProject();
    mComposition->atlasComposition().prepareForFeature( page - 1 );
    emit atlasPreviewFeatureChanged();
  }
}

QgsMapCanvas *QgsComposer::mapCanvas()
{
  return mQgis->mapCanvas();
}

QgsComposerView *QgsComposer::view()
{
  return mView;
}

void QgsComposer::zoomFull()
{
  if ( mView )
  {
    mView->fitInView( mComposition->sceneRect(), Qt::KeepAspectRatio );
  }
}

void QgsComposer::mActionZoomAll_triggered()
{
  zoomFull();
  mView->updateRulers();
  mView->update();
  emit zoomLevelChanged();
}

void QgsComposer::mActionZoomIn_triggered()
{
  mView->scaleSafe( 2 );
  mView->updateRulers();
  mView->update();
  emit zoomLevelChanged();
}

void QgsComposer::mActionZoomOut_triggered()
{
  mView->scaleSafe( 0.5 );
  mView->updateRulers();
  mView->update();
  emit zoomLevelChanged();
}

void QgsComposer::mActionZoomActual_triggered()
{
  mView->setZoomLevel( 1.0 );
}

void QgsComposer::mActionMouseZoom_triggered()
{
  if ( mView )
  {
    mView->setCurrentTool( QgsComposerView::Zoom );
  }
}

void QgsComposer::mActionRefreshView_triggered()
{
  if ( !mComposition )
  {
    return;
  }

  //refresh atlas feature first, to update attributes
  if ( mComposition->atlasMode() == QgsComposition::PreviewAtlas )
  {
    //block signals from atlas, since the later call to mComposition->refreshItems() will
    //also trigger items to refresh atlas dependent properties
    mComposition->atlasComposition().blockSignals( true );
    mComposition->atlasComposition().refreshFeature();
    mComposition->atlasComposition().blockSignals( false );
  }

  mComposition->refreshItems();
  mComposition->update();
}

void QgsComposer::mActionShowGrid_triggered( bool checked )
{
  //show or hide grid
  if ( mComposition )
  {
    mComposition->setGridVisible( checked );
  }
}

void QgsComposer::mActionSnapGrid_triggered( bool checked )
{
  //enable or disable snap items to grid
  if ( mComposition )
  {
    mComposition->setSnapToGridEnabled( checked );
  }
}

void QgsComposer::mActionShowGuides_triggered( bool checked )
{
  //show or hide guide lines
  if ( mComposition )
  {
    mComposition->setSnapLinesVisible( checked );
  }
}

void QgsComposer::mActionSnapGuides_triggered( bool checked )
{
  //enable or disable snap items to guides
  if ( mComposition )
  {
    mComposition->setAlignmentSnap( checked );
  }
}

void QgsComposer::mActionSmartGuides_triggered( bool checked )
{
  //enable or disable smart snapping guides
  if ( mComposition )
  {
    mComposition->setSmartGuidesEnabled( checked );
  }
}

void QgsComposer::mActionShowBoxes_triggered( bool checked )
{
  //show or hide bounding boxes
  if ( mComposition )
  {
    mComposition->setBoundingBoxesVisible( checked );
  }
}

void QgsComposer::mActionShowPage_triggered( bool checked )
{
  //toggle page display
  if ( mComposition )
  {
    mComposition->setPagesVisible( checked );
  }
}

void QgsComposer::mActionClearGuides_triggered()
{
  //clear guide lines
  if ( mComposition )
  {
    mComposition->clearSnapLines();
  }
}

void QgsComposer::toggleRulers( bool checked )
{
  //show or hide rulers
  mHorizontalRuler->setVisible( checked );
  mVerticalRuler->setVisible( checked );
  mRulerLayoutFix->setVisible( checked );

  QgsSettings myQSettings;
  myQSettings.setValue( QStringLiteral( "Composer/showRulers" ), checked );
}

void QgsComposer::mActionAtlasSettings_triggered()
{
  if ( !mAtlasDock->isVisible() )
  {
    mAtlasDock->show();
  }

  mAtlasDock->raise();
}

void QgsComposer::mActionToggleFullScreen_triggered()
{
  if ( mActionToggleFullScreen->isChecked() )
  {
    showFullScreen();
  }
  else
  {
    showNormal();
  }
}

void QgsComposer::mActionHidePanels_triggered()
{
  /*
  workaround the limited Qt dock widget API
  see http://qt-project.org/forums/viewthread/1141/
  and http://qt-project.org/faq/answer/how_can_i_check_which_tab_is_the_current_one_in_a_tabbed_qdockwidget
  */

  bool showPanels = !mActionHidePanels->isChecked();
  QList<QDockWidget *> docks = findChildren<QDockWidget *>();
  QList<QTabBar *> tabBars = findChildren<QTabBar *>();

  if ( !showPanels )
  {
    mPanelStatus.clear();
    //record status of all docks

    Q_FOREACH ( QDockWidget *dock, docks )
    {
      mPanelStatus.insert( dock->windowTitle(), PanelStatus( dock->isVisible(), false ) );
      dock->setVisible( false );
    }

    //record active dock tabs
    Q_FOREACH ( QTabBar *tabBar, tabBars )
    {
      QString currentTabTitle = tabBar->tabText( tabBar->currentIndex() );
      mPanelStatus[ currentTabTitle ].isActive = true;
    }
  }
  else
  {
    //restore visibility of all docks
    Q_FOREACH ( QDockWidget *dock, docks )
    {
      if ( ! mPanelStatus.contains( dock->windowTitle() ) )
      {
        dock->setVisible( true );
        continue;
      }
      dock->setVisible( mPanelStatus.value( dock->windowTitle() ).isVisible );
    }

    //restore previously active dock tabs
    Q_FOREACH ( QTabBar *tabBar, tabBars )
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

void QgsComposer::disablePreviewMode()
{
  if ( !mView )
  {
    return;
  }

  mView->setPreviewModeEnabled( false );
}

void QgsComposer::activateGrayscalePreview()
{
  if ( !mView )
  {
    return;
  }

  mView->setPreviewMode( QgsPreviewEffect::PreviewGrayscale );
  mView->setPreviewModeEnabled( true );
}

void QgsComposer::activateMonoPreview()
{
  if ( !mView )
  {
    return;
  }

  mView->setPreviewMode( QgsPreviewEffect::PreviewMono );
  mView->setPreviewModeEnabled( true );
}

void QgsComposer::activateProtanopePreview()
{
  if ( !mView )
  {
    return;
  }

  mView->setPreviewMode( QgsPreviewEffect::PreviewProtanope );
  mView->setPreviewModeEnabled( true );
}

void QgsComposer::activateDeuteranopePreview()
{
  if ( !mView )
  {
    return;
  }

  mView->setPreviewMode( QgsPreviewEffect::PreviewDeuteranope );
  mView->setPreviewModeEnabled( true );
}

void QgsComposer::dockVisibilityChanged( bool visible )
{
  if ( visible )
  {
    whileBlocking( mActionHidePanels )->setChecked( false );
  }
}

void QgsComposer::mActionExportAtlasAsPDF_triggered()
{
  QgsComposition::AtlasMode previousMode = mComposition->atlasMode();
  mComposition->setAtlasMode( QgsComposition::ExportAtlas );
  exportCompositionAsPDF( QgsComposer::Atlas );
  mComposition->setAtlasMode( previousMode );

  if ( mComposition->atlasMode() == QgsComposition::PreviewAtlas )
  {
    //after atlas output, jump back to preview first feature
    QgsAtlasComposition *atlasMap = &mComposition->atlasComposition();
    atlasMap->firstFeature();
  }
}

void QgsComposer::mActionExportAsPDF_triggered()
{
  exportCompositionAsPDF( QgsComposer::Single );
}

void QgsComposer::exportCompositionAsPDF( QgsComposer::OutputMode mode )
{
  if ( !mComposition || !mView )
  {
    return;
  }

  if ( containsWmsLayer() )
  {
    showWmsPrintingWarning();
  }

  if ( containsAdvancedEffects() )
  {
    showAdvancedEffectsWarning();
  }

  // If we are not printing as raster, temporarily disable advanced effects
  // as QPrinter does not support composition modes and can result
  // in items missing from the output
  if ( mComposition->printAsRaster() )
  {
    mComposition->setUseAdvancedEffects( true );
  }
  else
  {
    mComposition->setUseAdvancedEffects( false );
  }

  bool hasAnAtlas = mComposition->atlasComposition().enabled();
  bool atlasOnASingleFile = hasAnAtlas && mComposition->atlasComposition().singleFile();
  QgsAtlasComposition *atlasMap = &mComposition->atlasComposition();

  QString outputFileName;
  QString outputDir;

  if ( mode == QgsComposer::Single || ( mode == QgsComposer::Atlas && atlasOnASingleFile ) )
  {
    QgsSettings myQSettings;  // where we keep last used filter in persistent state
    QString lastUsedFile = myQSettings.value( QStringLiteral( "UI/lastSaveAsPdfFile" ), "qgis.pdf" ).toString();
    QFileInfo file( lastUsedFile );

    if ( hasAnAtlas && !atlasOnASingleFile &&
         ( mode == QgsComposer::Atlas || mComposition->atlasMode() == QgsComposition::PreviewAtlas ) )
    {
      outputFileName = QDir( file.path() ).filePath( atlasMap->currentFilename() ) + ".pdf";
    }
    else
    {
      outputFileName = file.path();
    }
#ifdef Q_OS_MAC
    mQgis->activateWindow();
    this->raise();
#endif
    outputFileName = QFileDialog::getSaveFileName(
                       this,
                       tr( "Save composition as" ),
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

    myQSettings.setValue( QStringLiteral( "UI/lastSaveAsPdfFile" ), outputFileName );
  }
  // else, we need to choose a directory
  else
  {
    if ( atlasMap->filenamePattern().isEmpty() )
    {
      int res = QMessageBox::warning( nullptr, tr( "Empty filename pattern" ),
                                      tr( "The filename pattern is empty. A default one will be used." ),
                                      QMessageBox::Ok | QMessageBox::Cancel,
                                      QMessageBox::Ok );
      if ( res == QMessageBox::Cancel )
      {
        return;
      }
      atlasMap->setFilenamePattern( QStringLiteral( "'output_'||@atlas_featurenumber" ) );
    }

    QgsSettings myQSettings;
    QString lastUsedDir = myQSettings.value( QStringLiteral( "UI/lastSaveAtlasAsPdfDir" ), QDir::homePath() ).toString();
    outputDir = QFileDialog::getExistingDirectory( this,
                tr( "Export atlas to directory" ),
                lastUsedDir,
                QFileDialog::ShowDirsOnly );
    if ( outputDir.isEmpty() )
    {
      return;
    }
    // test directory (if it exists and is writable)
    if ( !QDir( outputDir ).exists() || !QFileInfo( outputDir ).isWritable() )
    {
      QMessageBox::warning( nullptr, tr( "Unable to write into the directory" ),
                            tr( "The given output directory is not writable. Canceling." ),
                            QMessageBox::Ok,
                            QMessageBox::Ok );
      return;
    }

    myQSettings.setValue( QStringLiteral( "UI/lastSaveAtlasAsPdfDir" ), outputDir );
  }

  mView->setPaintingEnabled( false );

  if ( mode == QgsComposer::Atlas )
  {
    QPrinter printer;

    QPainter painter;

    loadAtlasPredefinedScalesFromProject();
    if ( ! atlasMap->beginRender() && !atlasMap->featureFilterErrorString().isEmpty() )
    {
      QMessageBox::warning( this, tr( "Atlas processing error" ),
                            tr( "Feature filter parser error: %1" ).arg( atlasMap->featureFilterErrorString() ),
                            QMessageBox::Ok,
                            QMessageBox::Ok );
      mView->setPaintingEnabled( true );
      return;
    }
    if ( atlasOnASingleFile )
    {
      //prepare for first feature, so that we know paper size to begin with
      atlasMap->prepareForFeature( 0 );
      mComposition->beginPrintAsPDF( printer, outputFileName );
      // set the correct resolution
      mComposition->beginPrint( printer );
      bool printReady = painter.begin( &printer );
      if ( !printReady )
      {
        QMessageBox::warning( this, tr( "Atlas processing error" ),
                              QString( tr( "Cannot write to %1.\n\nThis file may be open in another application." ) ).arg( outputFileName ),
                              QMessageBox::Ok,
                              QMessageBox::Ok );
        mView->setPaintingEnabled( true );
        return;
      }
    }

    QProgressDialog progress( tr( "Rendering maps..." ), tr( "Abort" ), 0, atlasMap->numFeatures(), this );
    progress.setWindowTitle( tr( "Exporting Atlas" ) );
    QApplication::setOverrideCursor( Qt::BusyCursor );

    for ( int featureI = 0; featureI < atlasMap->numFeatures(); ++featureI )
    {
      progress.setValue( featureI );
      // process input events in order to allow aborting
      QCoreApplication::processEvents();
      if ( progress.wasCanceled() )
      {
        atlasMap->endRender();
        break;
      }
      if ( !atlasMap->prepareForFeature( featureI ) )
      {
        QMessageBox::warning( this, tr( "Atlas processing error" ),
                              tr( "Atlas processing error" ),
                              QMessageBox::Ok,
                              QMessageBox::Ok );
        mView->setPaintingEnabled( true );
        QApplication::restoreOverrideCursor();
        return;
      }
      if ( !atlasOnASingleFile )
      {
        // bugs #7263 and #6856
        // QPrinter does not seem to be reset correctly and may cause generated PDFs (all except the first) corrupted
        // when transparent objects are rendered. We thus use a new QPrinter object here
        QPrinter multiFilePrinter;
        outputFileName = QDir( outputDir ).filePath( atlasMap->currentFilename() ) + ".pdf";
        mComposition->beginPrintAsPDF( multiFilePrinter, outputFileName );
        // set the correct resolution
        mComposition->beginPrint( multiFilePrinter );
        bool printReady = painter.begin( &multiFilePrinter );
        if ( !printReady )
        {
          QMessageBox::warning( this, tr( "Atlas processing error" ),
                                QString( tr( "Cannot write to %1.\n\nThis file may be open in another application." ) ).arg( outputFileName ),
                                QMessageBox::Ok,
                                QMessageBox::Ok );
          mView->setPaintingEnabled( true );
          QApplication::restoreOverrideCursor();
          return;
        }
        mComposition->doPrint( multiFilePrinter, painter );
        painter.end();
        mComposition->georeferenceOutput( outputFileName );
      }
      else
      {
        //start print on a new page if we're not on the first feature
        mComposition->doPrint( printer, painter, featureI > 0 );
      }
    }
    atlasMap->endRender();
    if ( atlasOnASingleFile )
    {
      painter.end();
    }
  }
  else
  {
    bool exportOk = mComposition->exportAsPDF( outputFileName );
    mComposition->georeferenceOutput( outputFileName );

    if ( !exportOk )
    {
      QMessageBox::warning( this, tr( "Atlas processing error" ),
                            QString( tr( "Cannot write to %1.\n\nThis file may be open in another application." ) ).arg( outputFileName ),
                            QMessageBox::Ok,
                            QMessageBox::Ok );
      mView->setPaintingEnabled( true );
      QApplication::restoreOverrideCursor();
      return;
    }
  }

  if ( ! mComposition->useAdvancedEffects() )
  {
    //Switch advanced effects back on
    mComposition->setUseAdvancedEffects( true );
  }
  mView->setPaintingEnabled( true );
  QApplication::restoreOverrideCursor();
}

void QgsComposer::mActionPrint_triggered()
{
  //print only current feature
  printComposition( QgsComposer::Single );
}

void QgsComposer::mActionPrintAtlas_triggered()
{
  //print whole atlas
  QgsComposition::AtlasMode previousMode = mComposition->atlasMode();
  mComposition->setAtlasMode( QgsComposition::ExportAtlas );
  printComposition( QgsComposer::Atlas );
  mComposition->setAtlasMode( previousMode );
}

void QgsComposer::printComposition( QgsComposer::OutputMode mode )
{
  if ( !mComposition || !mView )
  {
    return;
  }

  if ( containsWmsLayer() )
  {
    showWmsPrintingWarning();
  }

  if ( containsAdvancedEffects() )
  {
    showAdvancedEffectsWarning();
  }

  // If we are not printing as raster, temporarily disable advanced effects
  // as QPrinter does not support composition modes and can result
  // in items missing from the output
  if ( mComposition->printAsRaster() )
  {
    mComposition->setUseAdvancedEffects( true );
  }
  else
  {
    mComposition->setUseAdvancedEffects( false );
  }

  //set printer page orientation
  setPrinterPageOrientation();

  QPrintDialog printDialog( printer(), nullptr );
  if ( printDialog.exec() != QDialog::Accepted )
  {
    return;
  }

  QApplication::setOverrideCursor( Qt::BusyCursor );
  mView->setPaintingEnabled( false );

  QgsAtlasComposition *atlasMap = &mComposition->atlasComposition();
  if ( mode == QgsComposer::Single )
  {
    mComposition->print( *printer(), true );
  }
  else
  {
    //prepare for first feature, so that we know paper size to begin with
    atlasMap->prepareForFeature( 0 );

    mComposition->beginPrint( *printer(), true );
    QPainter painter( printer() );

    loadAtlasPredefinedScalesFromProject();
    if ( ! atlasMap->beginRender() && !atlasMap->featureFilterErrorString().isEmpty() )
    {
      QMessageBox::warning( this, tr( "Atlas processing error" ),
                            tr( "Feature filter parser error: %1" ).arg( atlasMap->featureFilterErrorString() ),
                            QMessageBox::Ok,
                            QMessageBox::Ok );
      mView->setPaintingEnabled( true );
      QApplication::restoreOverrideCursor();
      return;
    }
    QProgressDialog progress( tr( "Rendering maps..." ), tr( "Abort" ), 0, atlasMap->numFeatures(), this );
    progress.setWindowTitle( tr( "Exporting Atlas" ) );

    for ( int i = 0; i < atlasMap->numFeatures(); ++i )
    {
      progress.setValue( i );
      // process input events in order to allow canceling
      QCoreApplication::processEvents();

      if ( progress.wasCanceled() )
      {
        atlasMap->endRender();
        break;
      }
      if ( !atlasMap->prepareForFeature( i ) )
      {
        QMessageBox::warning( this, tr( "Atlas processing error" ),
                              tr( "Atlas processing error" ),
                              QMessageBox::Ok,
                              QMessageBox::Ok );
        mView->setPaintingEnabled( true );
        QApplication::restoreOverrideCursor();
        return;
      }

      //start print on a new page if we're not on the first feature
      mComposition->doPrint( *printer(), painter, i > 0 );
    }
    atlasMap->endRender();
    painter.end();
  }

  if ( ! mComposition->useAdvancedEffects() )
  {
    //Switch advanced effects back on
    mComposition->setUseAdvancedEffects( true );
  }
  mView->setPaintingEnabled( true );
  QApplication::restoreOverrideCursor();
}

void QgsComposer::mActionExportAtlasAsImage_triggered()
{
  //print whole atlas
  QgsComposition::AtlasMode previousMode = mComposition->atlasMode();
  mComposition->setAtlasMode( QgsComposition::ExportAtlas );
  exportCompositionAsImage( QgsComposer::Atlas );
  mComposition->setAtlasMode( previousMode );

  if ( mComposition->atlasMode() == QgsComposition::PreviewAtlas )
  {
    //after atlas output, jump back to preview first feature
    QgsAtlasComposition *atlasMap = &mComposition->atlasComposition();
    atlasMap->firstFeature();
  }
}

void QgsComposer::mActionExportAsImage_triggered()
{
  exportCompositionAsImage( QgsComposer::Single );
}

void QgsComposer::exportCompositionAsImage( QgsComposer::OutputMode mode )
{
  if ( !mComposition || !mView )
  {
    return;
  }

  if ( containsWmsLayer() )
  {
    showWmsPrintingWarning();
  }

  QgsSettings settings;

  // Image size
  int width = ( int )( mComposition->printResolution() * mComposition->paperWidth() / 25.4 );
  int height = ( int )( mComposition-> printResolution() * mComposition->paperHeight() / 25.4 );
  int dpi = mComposition->printResolution();

  int memuse = width * height * 3 / 1000000;  // pixmap + image
  QgsDebugMsg( QString( "Image %1x%2" ).arg( width ).arg( height ) );
  QgsDebugMsg( QString( "memuse = %1" ).arg( memuse ) );

  if ( memuse > 200 )   // about 4500x4500
  {
    int answer = QMessageBox::warning( nullptr, tr( "Big image" ),
                                       tr( "To create image %1x%2 requires about %3 MB of memory. Proceed?" )
                                       .arg( width ).arg( height ).arg( memuse ),
                                       QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok );

    raise();
    if ( answer == QMessageBox::Cancel )
      return;
  }

  //get some defaults from the composition
  bool cropToContents = mComposition->customProperty( QStringLiteral( "imageCropToContents" ), false ).toBool();
  int marginTop = mComposition->customProperty( QStringLiteral( "imageCropMarginTop" ), 0 ).toInt();
  int marginRight = mComposition->customProperty( QStringLiteral( "imageCropMarginRight" ), 0 ).toInt();
  int marginBottom = mComposition->customProperty( QStringLiteral( "imageCropMarginBottom" ), 0 ).toInt();
  int marginLeft = mComposition->customProperty( QStringLiteral( "imageCropMarginLeft" ), 0 ).toInt();

  QgsComposerImageExportOptionsDialog imageDlg( this );
  imageDlg.setImageSize( QSizeF( mComposition->paperWidth(), mComposition->paperHeight() ) );
  imageDlg.setResolution( mComposition->printResolution() );
  imageDlg.setCropToContents( cropToContents );
  imageDlg.setCropMargins( marginTop, marginRight, marginBottom, marginLeft );

  QgsAtlasComposition *atlasMap = &mComposition->atlasComposition();
  if ( mode == QgsComposer::Single )
  {
    QString outputFileName = QString();

    if ( atlasMap->enabled() && mComposition->atlasMode() == QgsComposition::PreviewAtlas )
    {
      QString lastUsedDir = settings.value( QStringLiteral( "UI/lastSaveAsImageDir" ), QDir::homePath() ).toString();
      outputFileName = QDir( lastUsedDir ).filePath( atlasMap->currentFilename() );
    }

#ifdef Q_OS_MAC
    mQgis->activateWindow();
    this->raise();
#endif
    QPair<QString, QString> fileNExt = QgsGuiUtils::getSaveAsImageName( this, tr( "Save composition as" ), outputFileName );
    this->activateWindow();

    if ( fileNExt.first.isEmpty() )
    {
      return;
    }

    if ( !imageDlg.exec() )
      return;

    cropToContents = imageDlg.cropToContents();
    imageDlg.getCropMargins( marginTop, marginRight, marginBottom, marginLeft );
    mComposition->setCustomProperty( QStringLiteral( "imageCropToContents" ), cropToContents );
    mComposition->setCustomProperty( QStringLiteral( "imageCropMarginTop" ), marginTop );
    mComposition->setCustomProperty( QStringLiteral( "imageCropMarginRight" ), marginRight );
    mComposition->setCustomProperty( QStringLiteral( "imageCropMarginBottom" ), marginBottom );
    mComposition->setCustomProperty( QStringLiteral( "imageCropMarginLeft" ), marginLeft );

    mView->setPaintingEnabled( false );

    int worldFilePageNo = -1;
    if ( mComposition->referenceMap() )
    {
      worldFilePageNo = mComposition->referenceMap()->page() - 1;
    }

    for ( int i = 0; i < mComposition->numPages(); ++i )
    {
      if ( !mComposition->shouldExportPage( i + 1 ) )
      {
        continue;
      }

      QImage image;
      QRectF bounds;
      if ( cropToContents )
      {
        if ( mComposition->numPages() == 1 )
        {
          // single page, so include everything
          bounds = mComposition->compositionBounds( true );
        }
        else
        {
          // multi page, so just clip to items on current page
          bounds = mComposition->pageItemBounds( i, true );
        }
        if ( bounds.width() <= 0 || bounds.height() <= 0 )
        {
          //invalid size, skip page
          continue;
        }
        double pixelToMm = 25.4 / mComposition->printResolution();
        bounds = bounds.adjusted( -marginLeft * pixelToMm,
                                  -marginTop * pixelToMm,
                                  marginRight * pixelToMm,
                                  marginBottom * pixelToMm );
        image = mComposition->renderRectAsRaster( bounds, QSize(), imageDlg.resolution() );
      }
      else
      {
        image = mComposition->printPageAsRaster( i, QSize( imageDlg.imageWidth(), imageDlg.imageHeight() ) );
      }

      if ( image.isNull() )
      {
        QMessageBox::warning( nullptr, tr( "Memory Allocation Error" ),
                              tr( "Trying to create image #%1( %2x%3 @ %4dpi ) "
                                  "may result in a memory overflow.\n"
                                  "Please try a lower resolution or a smaller papersize" )
                              .arg( i + 1 ).arg( width ).arg( height ).arg( dpi ),
                              QMessageBox::Ok, QMessageBox::Ok );
        mView->setPaintingEnabled( true );
        return;
      }
      bool saveOk;
      QString outputFilePath;
      if ( i == 0 )
      {
        outputFilePath = fileNExt.first;
      }
      else
      {
        QFileInfo fi( fileNExt.first );
        outputFilePath = fi.absolutePath() + '/' + fi.baseName() + '_' + QString::number( i + 1 ) + '.' + fi.suffix();
      }

      saveOk = saveImage( image, outputFilePath, fileNExt.second );

      if ( !saveOk )
      {
        QMessageBox::warning( this, tr( "Image export error" ),
                              QString( tr( "Cannot write to %1.\n\nThis file may be open in another application." ) ).arg( fileNExt.first ),
                              QMessageBox::Ok,
                              QMessageBox::Ok );
        mView->setPaintingEnabled( true );
        return;
      }

      if ( i == worldFilePageNo )
      {
        mComposition->georeferenceOutput( outputFilePath, nullptr, bounds, imageDlg.resolution() );

        if ( mComposition->generateWorldFile() )
        {
          // should generate world file for this page
          double a, b, c, d, e, f;
          if ( bounds.isValid() )
            mComposition->computeWorldFileParameters( bounds, a, b, c, d, e, f );
          else
            mComposition->computeWorldFileParameters( a, b, c, d, e, f );

          QFileInfo fi( outputFilePath );
          // build the world file name
          QString outputSuffix = fi.suffix();
          QString worldFileName = fi.absolutePath() + '/' + fi.baseName() + '.'
                                  + outputSuffix.at( 0 ) + outputSuffix.at( fi.suffix().size() - 1 ) + 'w';

          writeWorldFile( worldFileName, a, b, c, d, e, f );
        }
      }
    }

    mView->setPaintingEnabled( true );
  }
  else
  {
    // else, it has an atlas to render, so a directory must first be selected
    if ( atlasMap->filenamePattern().isEmpty() )
    {
      int res = QMessageBox::warning( nullptr, tr( "Empty filename pattern" ),
                                      tr( "The filename pattern is empty. A default one will be used." ),
                                      QMessageBox::Ok | QMessageBox::Cancel,
                                      QMessageBox::Ok );
      if ( res == QMessageBox::Cancel )
      {
        return;
      }
      atlasMap->setFilenamePattern( QStringLiteral( "'output_'||@atlas_featurenumber" ) );
    }

    QgsSettings myQSettings;
    QString lastUsedDir = myQSettings.value( QStringLiteral( "UI/lastSaveAtlasAsImagesDir" ), QDir::homePath() ).toString();

    QFileDialog dlg( this, tr( "Export atlas to directory" ) );
    dlg.setFileMode( QFileDialog::Directory );
    dlg.setOption( QFileDialog::ShowDirsOnly, true );
    dlg.setDirectory( lastUsedDir );

    if ( !dlg.exec() )
    {
      return;
    }
    QStringList s = dlg.selectedFiles();
    if ( s.empty() || s.at( 0 ).isEmpty() )
    {
      return;
    }
    QString dir = s.at( 0 );
    QString format = atlasMap->fileFormat();
    QString fileExt = '.' + format;

    if ( dir.isEmpty() )
    {
      return;
    }
    // test directory (if it exists and is writable)
    if ( !QDir( dir ).exists() || !QFileInfo( dir ).isWritable() )
    {
      QMessageBox::warning( nullptr, tr( "Unable to write into the directory" ),
                            tr( "The given output directory is not writable. Canceling." ),
                            QMessageBox::Ok,
                            QMessageBox::Ok );
      return;
    }

    if ( !imageDlg.exec() )
      return;

    cropToContents = imageDlg.cropToContents();
    imageDlg.getCropMargins( marginTop, marginRight, marginBottom, marginLeft );
    mComposition->setCustomProperty( QStringLiteral( "imageCropToContents" ), cropToContents );
    mComposition->setCustomProperty( QStringLiteral( "imageCropMarginTop" ), marginTop );
    mComposition->setCustomProperty( QStringLiteral( "imageCropMarginRight" ), marginRight );
    mComposition->setCustomProperty( QStringLiteral( "imageCropMarginBottom" ), marginBottom );
    mComposition->setCustomProperty( QStringLiteral( "imageCropMarginLeft" ), marginLeft );

    myQSettings.setValue( QStringLiteral( "UI/lastSaveAtlasAsImagesDir" ), dir );

    // So, now we can render the atlas
    mView->setPaintingEnabled( false );
    QApplication::setOverrideCursor( Qt::BusyCursor );

    loadAtlasPredefinedScalesFromProject();
    if ( ! atlasMap->beginRender() && !atlasMap->featureFilterErrorString().isEmpty() )
    {
      QMessageBox::warning( this, tr( "Atlas processing error" ),
                            tr( "Feature filter parser error: %1" ).arg( atlasMap->featureFilterErrorString() ),
                            QMessageBox::Ok,
                            QMessageBox::Ok );
      mView->setPaintingEnabled( true );
      QApplication::restoreOverrideCursor();
      return;
    }

    QProgressDialog progress( tr( "Rendering maps..." ), tr( "Abort" ), 0, atlasMap->numFeatures(), this );
    progress.setWindowTitle( tr( "Exporting Atlas" ) );

    for ( int feature = 0; feature < atlasMap->numFeatures(); ++feature )
    {
      progress.setValue( feature );
      // process input events in order to allow canceling
      QCoreApplication::processEvents();

      if ( progress.wasCanceled() )
      {
        atlasMap->endRender();
        break;
      }
      if ( ! atlasMap->prepareForFeature( feature ) )
      {
        QMessageBox::warning( this, tr( "Atlas processing error" ),
                              tr( "Atlas processing error" ),
                              QMessageBox::Ok,
                              QMessageBox::Ok );
        mView->setPaintingEnabled( true );
        QApplication::restoreOverrideCursor();
        return;
      }

      QString filename = QDir( dir ).filePath( atlasMap->currentFilename() ) + fileExt;

      int worldFilePageNo = -1;
      if ( mComposition->referenceMap() )
      {
        worldFilePageNo = mComposition->referenceMap()->page() - 1;
      }

      for ( int i = 0; i < mComposition->numPages(); ++i )
      {
        if ( !mComposition->shouldExportPage( i + 1 ) )
        {
          continue;
        }

        QImage image;
        QRectF bounds;
        if ( cropToContents )
        {
          if ( mComposition->numPages() == 1 )
          {
            // single page, so include everything
            bounds = mComposition->compositionBounds( true );
          }
          else
          {
            // multi page, so just clip to items on current page
            bounds = mComposition->pageItemBounds( i, true );
          }
          if ( bounds.width() <= 0 || bounds.height() <= 0 )
          {
            //invalid size, skip page
            continue;
          }
          double pixelToMm = 25.4 / mComposition->printResolution();
          bounds = bounds.adjusted( -marginLeft * pixelToMm,
                                    -marginTop * pixelToMm,
                                    marginRight * pixelToMm,
                                    marginBottom * pixelToMm );
          image = mComposition->renderRectAsRaster( bounds, QSize(), imageDlg.resolution() );
        }
        else
        {
          //note - we can't safely use the preset width/height set in imageDlg here,
          //as the atlas may have differing page size. So use resolution instead.
          image = mComposition->printPageAsRaster( i, QSize(), imageDlg.resolution() );
        }

        QString imageFilename = filename;

        if ( i != 0 )
        {
          //append page number
          QFileInfo fi( filename );
          imageFilename = fi.absolutePath() + '/' + fi.baseName() + '_' + QString::number( i + 1 ) + '.' + fi.suffix();
        }

        bool saveOk = saveImage( image, imageFilename, format );
        if ( !saveOk )
        {
          QMessageBox::warning( this, tr( "Atlas processing error" ),
                                QString( tr( "Cannot write to %1.\n\nThis file may be open in another application." ) ).arg( imageFilename ),
                                QMessageBox::Ok,
                                QMessageBox::Ok );
          mView->setPaintingEnabled( true );
          QApplication::restoreOverrideCursor();
          return;
        }

        if ( i == worldFilePageNo )
        {
          mComposition->georeferenceOutput( imageFilename, nullptr, bounds, imageDlg.resolution() );

          if ( mComposition->generateWorldFile() )
          {
            // should generate world file for this page
            double a, b, c, d, e, f;
            if ( bounds.isValid() )
              mComposition->computeWorldFileParameters( bounds, a, b, c, d, e, f );
            else
              mComposition->computeWorldFileParameters( a, b, c, d, e, f );

            QFileInfo fi( imageFilename );
            // build the world file name
            QString outputSuffix = fi.suffix();
            QString worldFileName = fi.absolutePath() + '/' + fi.baseName() + '.'
                                    + outputSuffix.at( 0 ) + outputSuffix.at( fi.suffix().size() - 1 ) + 'w';

            writeWorldFile( worldFileName, a, b, c, d, e, f );
          }
        }
      }
    }
    atlasMap->endRender();
    mView->setPaintingEnabled( true );
    QApplication::restoreOverrideCursor();
  }
}

bool QgsComposer::saveImage( const QImage &img, const QString &imageFilename, const QString &imageFormat )
{
  QImageWriter w( imageFilename, imageFormat.toLocal8Bit().constData() );
  if ( imageFormat.compare( QLatin1String( "tiff" ), Qt::CaseInsensitive ) == 0 || imageFormat.compare( QLatin1String( "tif" ), Qt::CaseInsensitive ) == 0 )
  {
    w.setCompression( 1 ); //use LZW compression
  }
  return w.write( img );
}

void QgsComposer::mActionExportAtlasAsSVG_triggered()
{
  QgsComposition::AtlasMode previousMode = mComposition->atlasMode();
  mComposition->setAtlasMode( QgsComposition::ExportAtlas );
  exportCompositionAsSVG( QgsComposer::Atlas );
  mComposition->setAtlasMode( previousMode );

  if ( mComposition->atlasMode() == QgsComposition::PreviewAtlas )
  {
    //after atlas output, jump back to preview first feature
    QgsAtlasComposition *atlasMap = &mComposition->atlasComposition();
    atlasMap->firstFeature();
  }
}

void QgsComposer::mActionExportAsSVG_triggered()
{
  exportCompositionAsSVG( QgsComposer::Single );
}

// utility class that will hide all items until it's destroyed
struct QgsItemTempHider
{
    explicit QgsItemTempHider( const QList<QGraphicsItem *> &items )
    {
      QList<QGraphicsItem *>::const_iterator it = items.begin();
      for ( ; it != items.end(); ++it )
      {
        mItemVisibility[*it] = ( *it )->isVisible();
        ( *it )->hide();
      }
    }
    void hideAll()
    {
      QgsItemVisibilityHash::const_iterator it = mItemVisibility.constBegin();
      for ( ; it != mItemVisibility.constEnd(); ++it ) it.key()->hide();
    }
    ~QgsItemTempHider()
    {
      QgsItemVisibilityHash::const_iterator it = mItemVisibility.constBegin();
      for ( ; it != mItemVisibility.constEnd(); ++it )
      {
        it.key()->setVisible( it.value() );
      }
    }
  private:
    Q_DISABLE_COPY( QgsItemTempHider )
    typedef QHash<QGraphicsItem *, bool> QgsItemVisibilityHash;
    QgsItemVisibilityHash mItemVisibility;
};

void QgsComposer::exportCompositionAsSVG( QgsComposer::OutputMode mode )
{
  if ( containsWmsLayer() )
  {
    showWmsPrintingWarning();
  }

  QString settingsLabel = QStringLiteral( "/UI/displaySVGWarning" );
  QgsSettings settings;

  bool displaySVGWarning = settings.value( settingsLabel, true ).toBool();

  if ( displaySVGWarning )
  {
    QgsMessageViewer *m = new QgsMessageViewer( this );
    m->setWindowTitle( tr( "SVG Warning" ) );
    m->setCheckBoxText( tr( "Don't show this message again" ) );
    m->setCheckBoxState( Qt::Unchecked );
    m->setCheckBoxVisible( true );
    m->setCheckBoxQgsSettingsLabel( settingsLabel );
    m->setMessageAsHtml( tr( "<p>The SVG export function in QGIS has several "
                             "problems due to bugs and deficiencies in the " )
                         + tr( "Qt4 svg code. In particular, there are problems "
                               "with layers not being clipped to the map "
                               "bounding box.</p>" )
                         + tr( "If you require a vector-based output file from "
                               "QGIS it is suggested that you try printing "
                               "to PostScript if the SVG output is not "
                               "satisfactory."
                               "</p>" ) );
    m->exec();
  }

  QgsAtlasComposition *atlasMap = &mComposition->atlasComposition();

  QString outputFileName;
  QString outputDir;
  bool groupLayers = false;
  bool prevSettingLabelsAsOutlines = QgsProject::instance()->readBoolEntry( QStringLiteral( "PAL" ), QStringLiteral( "/DrawOutlineLabels" ), true );
  bool clipToContent = false;
  double marginTop = 0.0;
  double marginRight = 0.0;
  double marginBottom = 0.0;
  double marginLeft = 0.0;

  if ( mode == QgsComposer::Single )
  {
    QString lastUsedFile = settings.value( QStringLiteral( "UI/lastSaveAsSvgFile" ), "qgis.svg" ).toString();
    QFileInfo file( lastUsedFile );

    if ( atlasMap->enabled() && mComposition->atlasMode() == QgsComposition::PreviewAtlas )
    {
      outputFileName = QDir( file.path() ).filePath( atlasMap->currentFilename() ) + ".svg";
    }
    else
    {
      outputFileName = file.path();
    }

    // open file dialog
#ifdef Q_OS_MAC
    mQgis->activateWindow();
    this->raise();
#endif
    outputFileName = QFileDialog::getSaveFileName(
                       this,
                       tr( "Save composition as" ),
                       outputFileName,
                       tr( "SVG Format" ) + " (*.svg *.SVG)" );
    this->activateWindow();

    if ( outputFileName.isEmpty() )
      return;

    if ( !outputFileName.endsWith( QLatin1String( ".svg" ), Qt::CaseInsensitive ) )
    {
      outputFileName += QLatin1String( ".svg" );
    }

    settings.setValue( QStringLiteral( "UI/lastSaveAsSvgFile" ), outputFileName );
  }
  else
  {
    // If we have an Atlas
    if ( atlasMap->filenamePattern().isEmpty() )
    {
      int res = QMessageBox::warning( nullptr, tr( "Empty filename pattern" ),
                                      tr( "The filename pattern is empty. A default one will be used." ),
                                      QMessageBox::Ok | QMessageBox::Cancel,
                                      QMessageBox::Ok );
      if ( res == QMessageBox::Cancel )
      {
        return;
      }
      atlasMap->setFilenamePattern( QStringLiteral( "'output_'||@atlas_featurenumber" ) );
    }

    QgsSettings myQSettings;
    QString lastUsedDir = myQSettings.value( QStringLiteral( "UI/lastSaveAtlasAsSvgDir" ), QDir::homePath() ).toString();

    // open file dialog
    outputDir = QFileDialog::getExistingDirectory( this,
                tr( "Export atlas to directory" ),
                lastUsedDir,
                QFileDialog::ShowDirsOnly );

    if ( outputDir.isEmpty() )
    {
      return;
    }
    // test directory (if it exists and is writable)
    if ( !QDir( outputDir ).exists() || !QFileInfo( outputDir ).isWritable() )
    {
      QMessageBox::warning( nullptr, tr( "Unable to write into the directory" ),
                            tr( "The given output directory is not writable. Canceling." ),
                            QMessageBox::Ok,
                            QMessageBox::Ok );
      return;
    }
    myQSettings.setValue( QStringLiteral( "UI/lastSaveAtlasAsSvgDir" ), outputDir );
  }

  // open options dialog
  QDialog dialog;
  Ui::QgsSvgExportOptionsDialog options;
  options.setupUi( &dialog );
  options.chkTextAsOutline->setChecked( prevSettingLabelsAsOutlines );
  options.chkMapLayersAsGroup->setChecked( mComposition->customProperty( QStringLiteral( "svgGroupLayers" ), false ).toBool() );
  options.mClipToContentGroupBox->setChecked( mComposition->customProperty( QStringLiteral( "svgCropToContents" ), false ).toBool() );
  options.mTopMarginSpinBox->setValue( mComposition->customProperty( QStringLiteral( "svgCropMarginTop" ), 0 ).toInt() );
  options.mRightMarginSpinBox->setValue( mComposition->customProperty( QStringLiteral( "svgCropMarginRight" ), 0 ).toInt() );
  options.mBottomMarginSpinBox->setValue( mComposition->customProperty( QStringLiteral( "svgCropMarginBottom" ), 0 ).toInt() );
  options.mLeftMarginSpinBox->setValue( mComposition->customProperty( QStringLiteral( "svgCropMarginLeft" ), 0 ).toInt() );

  if ( dialog.exec() != QDialog::Accepted )
    return;

  groupLayers = options.chkMapLayersAsGroup->isChecked();
  clipToContent = options.mClipToContentGroupBox->isChecked();
  marginTop = options.mTopMarginSpinBox->value();
  marginRight = options.mRightMarginSpinBox->value();
  marginBottom = options.mBottomMarginSpinBox->value();
  marginLeft = options.mLeftMarginSpinBox->value();

  //save dialog settings
  mComposition->setCustomProperty( QStringLiteral( "svgGroupLayers" ), groupLayers );
  mComposition->setCustomProperty( QStringLiteral( "svgCropToContents" ), clipToContent );
  mComposition->setCustomProperty( QStringLiteral( "svgCropMarginTop" ), marginTop );
  mComposition->setCustomProperty( QStringLiteral( "svgCropMarginRight" ), marginRight );
  mComposition->setCustomProperty( QStringLiteral( "svgCropMarginBottom" ), marginBottom );
  mComposition->setCustomProperty( QStringLiteral( "svgCropMarginLeft" ), marginLeft );

  //temporarily override label draw outlines setting
  QgsProject::instance()->writeEntry( QStringLiteral( "PAL" ), QStringLiteral( "/DrawOutlineLabels" ), options.chkTextAsOutline->isChecked() );

  mView->setPaintingEnabled( false );

  int featureI = 0;
  if ( mode == QgsComposer::Atlas )
  {
    loadAtlasPredefinedScalesFromProject();
    if ( ! atlasMap->beginRender() && !atlasMap->featureFilterErrorString().isEmpty() )
    {
      QMessageBox::warning( this, tr( "Atlas processing error" ),
                            tr( "Feature filter parser error: %1" ).arg( atlasMap->featureFilterErrorString() ),
                            QMessageBox::Ok,
                            QMessageBox::Ok );
      mView->setPaintingEnabled( true );
      QgsProject::instance()->writeEntry( QStringLiteral( "PAL" ), QStringLiteral( "/DrawOutlineLabels" ), prevSettingLabelsAsOutlines );
      return;
    }
  }
  QProgressDialog progress( tr( "Rendering maps..." ), tr( "Abort" ), 0, atlasMap->numFeatures(), this );
  progress.setWindowTitle( tr( "Exporting Atlas" ) );

  do
  {
    if ( mode == QgsComposer::Atlas )
    {
      if ( atlasMap->numFeatures() == 0 )
        break;

      progress.setValue( featureI );
      // process input events in order to allow aborting
      QCoreApplication::processEvents();
      if ( progress.wasCanceled() )
      {
        atlasMap->endRender();
        break;
      }
      if ( !atlasMap->prepareForFeature( featureI ) )
      {
        QMessageBox::warning( this, tr( "Atlas processing error" ),
                              tr( "Atlas processing error" ),
                              QMessageBox::Ok,
                              QMessageBox::Ok );
        mView->setPaintingEnabled( true );
        QgsProject::instance()->writeEntry( QStringLiteral( "PAL" ), QStringLiteral( "/DrawOutlineLabels" ), prevSettingLabelsAsOutlines );
        return;
      }
      outputFileName = QDir( outputDir ).filePath( atlasMap->currentFilename() ) + ".svg";
    }

    if ( !groupLayers )
    {
      for ( int i = 0; i < mComposition->numPages(); ++i )
      {
        if ( !mComposition->shouldExportPage( i + 1 ) )
        {
          continue;
        }
        QSvgGenerator generator;
        generator.setTitle( QgsProject::instance()->title() );
        QString currentFileName = outputFileName;
        if ( i == 0 )
        {
          generator.setFileName( outputFileName );
        }
        else
        {
          QFileInfo fi( outputFileName );
          currentFileName = fi.absolutePath() + '/' + fi.baseName() + '_' + QString::number( i + 1 ) + '.' + fi.suffix();
          generator.setFileName( currentFileName );
        }

        QRectF bounds;
        if ( clipToContent )
        {
          if ( mComposition->numPages() == 1 )
          {
            // single page, so include everything
            bounds = mComposition->compositionBounds( true );
          }
          else
          {
            // multi page, so just clip to items on current page
            bounds = mComposition->pageItemBounds( i, true );
          }
          bounds = bounds.adjusted( -marginLeft, -marginTop, marginRight, marginBottom );
        }
        else
          bounds = QRectF( 0, 0, mComposition->paperWidth(), mComposition->paperHeight() );

        //width in pixel
        int width = ( int )( bounds.width() * mComposition->printResolution() / 25.4 );
        //height in pixel
        int height = ( int )( bounds.height() * mComposition->printResolution() / 25.4 );
        if ( width == 0 || height == 0 )
        {
          //invalid size, skip this page
          continue;
        }
        generator.setSize( QSize( width, height ) );
        generator.setViewBox( QRect( 0, 0, width, height ) );
        generator.setResolution( mComposition->printResolution() ); //because the rendering is done in mm, convert the dpi

        QPainter p;
        bool createOk = p.begin( &generator );
        if ( !createOk )
        {
          QMessageBox::warning( this, tr( "SVG export error" ),
                                QString( tr( "Cannot write to %1.\n\nThis file may be open in another application." ) ).arg( currentFileName ),
                                QMessageBox::Ok,
                                QMessageBox::Ok );
          mView->setPaintingEnabled( true );
          QgsProject::instance()->writeEntry( QStringLiteral( "PAL" ), QStringLiteral( "/DrawOutlineLabels" ), prevSettingLabelsAsOutlines );
          return;
        }

        if ( clipToContent )
          mComposition->renderRect( &p, bounds );
        else
          mComposition->renderPage( &p, i );
        p.end();
      }
    }
    else
    {
      //width and height in pixel
      const int pageWidth = ( int )( mComposition->paperWidth() * mComposition->printResolution() / 25.4 );
      const int pageHeight = ( int )( mComposition->paperHeight() * mComposition->printResolution() / 25.4 );
      QList< QgsPaperItem * > paperItems( mComposition->pages() );

      for ( int i = 0; i < mComposition->numPages(); ++i )
      {
        if ( !mComposition->shouldExportPage( i + 1 ) )
        {
          continue;
        }

        int width = pageWidth;
        int height = pageHeight;

        QRectF bounds;
        if ( clipToContent )
        {
          if ( mComposition->numPages() == 1 )
          {
            // single page, so include everything
            bounds = mComposition->compositionBounds( true );
          }
          else
          {
            // multi page, so just clip to items on current page
            bounds = mComposition->pageItemBounds( i, true );
          }
          bounds = bounds.adjusted( -marginLeft, -marginTop, marginRight, marginBottom );
          width = bounds.width() * mComposition->printResolution() / 25.4;
          height = bounds.height() * mComposition->printResolution() / 25.4;
        }

        if ( width == 0 || height == 0 )
        {
          //invalid size, skip this page
          continue;
        }

        QDomDocument svg;
        QDomNode svgDocRoot;
        QgsPaperItem *paperItem = paperItems[i];
        const QRectF paperRect = QRectF( paperItem->pos().x(),
                                         paperItem->pos().y(),
                                         paperItem->rect().width(),
                                         paperItem->rect().height() );

        QList<QGraphicsItem *> items = mComposition->items( paperRect,
                                       Qt::IntersectsItemBoundingRect,
                                       Qt::AscendingOrder );
        if ( ! items.isEmpty()
             && dynamic_cast<QgsPaperGrid *>( items.last() )
             && !mComposition->gridVisible() ) items.pop_back();
        QgsItemTempHider itemsHider( items );
        int composerItemLayerIdx = 0;
        QList<QGraphicsItem *>::const_iterator it = items.constBegin();
        for ( unsigned svgLayerId = 1; it != items.constEnd(); ++svgLayerId )
        {
          itemsHider.hideAll();
          QgsComposerItem *composerItem = dynamic_cast<QgsComposerItem *>( *it );
          QString layerName( "Layer " + QString::number( svgLayerId ) );
          if ( composerItem && composerItem->numberExportLayers() )
          {
            composerItem->show();
            composerItem->setCurrentExportLayer( composerItemLayerIdx );
            ++composerItemLayerIdx;
          }
          else
          {
            // show all items until the next item that renders on a separate layer
            for ( ; it != items.constEnd(); ++it )
            {
              composerItem = dynamic_cast<QgsComposerMap *>( *it );
              if ( composerItem && composerItem->numberExportLayers() )
              {
                break;
              }
              else
              {
                ( *it )->show();
              }
            }
          }

          QBuffer svgBuffer;
          {
            QSvgGenerator generator;
            generator.setTitle( QgsProject::instance()->title() );
            generator.setOutputDevice( &svgBuffer );
            generator.setSize( QSize( width, height ) );
            generator.setViewBox( QRect( 0, 0, width, height ) );
            generator.setResolution( mComposition->printResolution() ); //because the rendering is done in mm, convert the dpi

            QPainter p( &generator );
            if ( clipToContent )
              mComposition->renderRect( &p, bounds );
            else
              mComposition->renderPage( &p, i );
          }
          // post-process svg output to create groups in a single svg file
          // we create inkscape layers since it's nice and clean and free
          // and fully svg compatible
          {
            svgBuffer.close();
            svgBuffer.open( QIODevice::ReadOnly );
            QDomDocument doc;
            QString errorMsg;
            int errorLine;
            if ( ! doc.setContent( &svgBuffer, false, &errorMsg, &errorLine ) )
              QMessageBox::warning( nullptr, tr( "SVG error" ), tr( "There was an error in SVG output for SVG layer " ) + layerName + tr( " on page " ) + QString::number( i + 1 ) + '(' + errorMsg + ')' );
            if ( 1 == svgLayerId )
            {
              svg = QDomDocument( doc.doctype() );
              svg.appendChild( svg.importNode( doc.firstChild(), false ) );
              svgDocRoot = svg.importNode( doc.elementsByTagName( QStringLiteral( "svg" ) ).at( 0 ), false );
              svgDocRoot.toElement().setAttribute( QStringLiteral( "xmlns:inkscape" ), QStringLiteral( "http://www.inkscape.org/namespaces/inkscape" ) );
              svg.appendChild( svgDocRoot );
            }
            QDomNode mainGroup = svg.importNode( doc.elementsByTagName( QStringLiteral( "g" ) ).at( 0 ), true );
            mainGroup.toElement().setAttribute( QStringLiteral( "id" ), layerName );
            mainGroup.toElement().setAttribute( QStringLiteral( "inkscape:label" ), layerName );
            mainGroup.toElement().setAttribute( QStringLiteral( "inkscape:groupmode" ), QStringLiteral( "layer" ) );
            QDomNode defs = svg.importNode( doc.elementsByTagName( QStringLiteral( "defs" ) ).at( 0 ), true );
            svgDocRoot.appendChild( defs );
            svgDocRoot.appendChild( mainGroup );
          }

          if ( composerItem && composerItem->numberExportLayers() && composerItem->numberExportLayers() == composerItemLayerIdx ) // restore and pass to next item
          {
            composerItem->setCurrentExportLayer();
            composerItemLayerIdx = 0;
            ++it;
          }
        }
        QFileInfo fi( outputFileName );
        QString currentFileName = i == 0 ? outputFileName : fi.absolutePath() + '/' + fi.baseName() + '_' + QString::number( i + 1 ) + '.' + fi.suffix();
        QFile out( currentFileName );
        bool openOk = out.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate );
        if ( !openOk )
        {
          QMessageBox::warning( this, tr( "SVG export error" ),
                                QString( tr( "Cannot write to %1.\n\nThis file may be open in another application." ) ).arg( currentFileName ),
                                QMessageBox::Ok,
                                QMessageBox::Ok );
          mView->setPaintingEnabled( true );
          QgsProject::instance()->writeEntry( QStringLiteral( "PAL" ), QStringLiteral( "/DrawOutlineLabels" ), prevSettingLabelsAsOutlines );
          return;
        }

        out.write( svg.toByteArray() );
      }
    }
    featureI++;
  }
  while ( mode == QgsComposer::Atlas && featureI < atlasMap->numFeatures() );

  if ( mode == QgsComposer::Atlas )
    atlasMap->endRender();

  mView->setPaintingEnabled( true );
  QgsProject::instance()->writeEntry( QStringLiteral( "PAL" ), QStringLiteral( "/DrawOutlineLabels" ), prevSettingLabelsAsOutlines );
}

void QgsComposer::mActionSelectMoveItem_triggered()
{
  if ( mView )
  {
    mView->setCurrentTool( QgsComposerView::Select );
  }
}

void QgsComposer::mActionAddNewMap_triggered()
{
  if ( mView )
  {
    mView->setCurrentTool( QgsComposerView::AddMap );
  }
}

void QgsComposer::mActionAddNewLegend_triggered()
{
  if ( mView )
  {
    mView->setCurrentTool( QgsComposerView::AddLegend );
  }
}

void QgsComposer::mActionAddNewLabel_triggered()
{
  if ( mView )
  {
    mView->setCurrentTool( QgsComposerView::AddLabel );
  }
}

void QgsComposer::mActionAddNewScalebar_triggered()
{
  if ( mView )
  {
    mView->setCurrentTool( QgsComposerView::AddScalebar );
  }
}

void QgsComposer::mActionAddImage_triggered()
{
  if ( mView )
  {
    mView->setCurrentTool( QgsComposerView::AddPicture );
  }
}

void QgsComposer::mActionAddRectangle_triggered()
{
  if ( mView )
  {
    mView->setCurrentTool( QgsComposerView::AddRectangle );
  }
}

void QgsComposer::mActionAddTriangle_triggered()
{
  if ( mView )
  {
    mView->setCurrentTool( QgsComposerView::AddTriangle );
  }
}

void QgsComposer::mActionAddEllipse_triggered()
{
  if ( mView )
  {
    mView->setCurrentTool( QgsComposerView::AddEllipse );
  }
}

void QgsComposer::mActionAddPolygon_triggered()
{
  if ( mView )
  {
    mView->setCurrentTool( QgsComposerView::AddPolygon );
  }
}

void QgsComposer::mActionAddPolyline_triggered()
{
  if ( mView )
  {
    mView->setCurrentTool( QgsComposerView::AddPolyline );
  }
}

void QgsComposer::mActionAddTable_triggered()
{
  if ( mView )
  {
    mView->setCurrentTool( QgsComposerView::AddTable );
  }
}

void QgsComposer::mActionAddAttributeTable_triggered()
{
  if ( mView )
  {
    mView->setCurrentTool( QgsComposerView::AddAttributeTable );
  }
}

void QgsComposer::mActionAddHtml_triggered()
{
  if ( mView )
  {
    mView->setCurrentTool( QgsComposerView::AddHtml );
  }
}

void QgsComposer::mActionAddArrow_triggered()
{
  if ( mView )
  {
    mView->setCurrentTool( QgsComposerView::AddArrow );
  }
}

void QgsComposer::mActionSaveProject_triggered()
{
  mQgis->actionSaveProject()->trigger();
}

void QgsComposer::mActionNewComposer_triggered()
{
  QString title;
  if ( !mQgis->uniqueComposerTitle( this, title, true ) )
  {
    return;
  }
  mQgis->createNewComposer( title );
}

void QgsComposer::mActionDuplicateComposer_triggered()
{
  QString newTitle;
  if ( !mQgis->uniqueComposerTitle( this, newTitle, false, mComposition->name() + tr( " copy" ) ) )
  {
    return;
  }

  // provide feedback, since loading of template into duplicate composer will be hidden
  QDialog *dlg = new QgsBusyIndicatorDialog( tr( "Duplicating composer..." ) );
  dlg->setStyleSheet( mQgis->styleSheet() );
  dlg->show();

  QgsComposer *newComposer = mQgis->duplicateComposer( this, newTitle );

  dlg->close();
  delete dlg;
  dlg = nullptr;

  if ( !newComposer )
  {
    QMessageBox::warning( this, tr( "Duplicate Composer" ),
                          tr( "Composer duplication failed." ) );
  }
}

void QgsComposer::mActionComposerManager_triggered()
{
  // NOTE: Avoid crash where composer that spawned modal manager from toolbar ends up
  // being deleted by user, but event loop tries to return to composer on manager close
  // (does not seem to be an issue for menu action)
  QTimer::singleShot( 0, mQgis->actionShowComposerManager(), SLOT( trigger() ) );
}

void QgsComposer::mActionSaveAsTemplate_triggered()
{
  //show file dialog
  QgsSettings settings;
  QString lastSaveDir = settings.value( QStringLiteral( "UI/lastComposerTemplateDir" ), QDir::homePath() ).toString();
#ifdef Q_OS_MAC
  mQgis->activateWindow();
  this->raise();
#endif
  QString saveFileName = QFileDialog::getSaveFileName(
                           this,
                           tr( "Save template" ),
                           lastSaveDir,
                           tr( "Composer templates" ) + " (*.qpt *.QPT)" );
  if ( saveFileName.isEmpty() )
    return;

  QFileInfo saveFileInfo( saveFileName );
  //check if suffix has been added
  if ( saveFileInfo.suffix().isEmpty() )
  {
    QString saveFileNameWithSuffix = saveFileName.append( ".qpt" );
    saveFileInfo = QFileInfo( saveFileNameWithSuffix );
  }
  settings.setValue( QStringLiteral( "UI/lastComposerTemplateDir" ), saveFileInfo.absolutePath() );

  QFile templateFile( saveFileName );
  if ( !templateFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
  {
    return;
  }

  QDomDocument saveDocument;
  QgsProject::instance()->layoutManager()->saveAsTemplate( mComposition->name(), saveDocument );

  if ( templateFile.write( saveDocument.toByteArray() ) == -1 )
  {
    QMessageBox::warning( nullptr, tr( "Save error" ), tr( "Error, could not save file" ) );
  }
}

void QgsComposer::mActionLoadFromTemplate_triggered()
{
  if ( !mComposition )
    return;

  QgsSettings settings;
  QString openFileDir = settings.value( QStringLiteral( "UI/lastComposerTemplateDir" ), QDir::homePath() ).toString();
  QString openFileString = QFileDialog::getOpenFileName( nullptr, tr( "Load template" ), openFileDir, QStringLiteral( "*.qpt" ) );

  if ( openFileString.isEmpty() )
  {
    return; //canceled by the user
  }

  QFileInfo openFileInfo( openFileString );
  settings.setValue( QStringLiteral( "UI/LastComposerTemplateDir" ), openFileInfo.absolutePath() );

  QFile templateFile( openFileString );
  if ( !templateFile.open( QIODevice::ReadOnly ) )
  {
    QMessageBox::warning( this, tr( "Read error" ), tr( "Error, could not read file" ) );
    return;
  }

  QDomDocument templateDoc;
  if ( templateDoc.setContent( &templateFile ) )
  {
    loadFromTemplate( templateDoc, false );
  }
}

void QgsComposer::mActionMoveItemContent_triggered()
{
  if ( mView )
  {
    mView->setCurrentTool( QgsComposerView::MoveItemContent );
  }
}

void QgsComposer::mActionEditNodesItem_triggered()
{
  if ( mView )
  {
    mView->setCurrentTool( QgsComposerView::EditNodesItem );
  }
}

void QgsComposer::mActionPan_triggered()
{
  if ( mView )
  {
    mView->setCurrentTool( QgsComposerView::Pan );
  }
}

void QgsComposer::mActionGroupItems_triggered()
{
  if ( mView )
  {
    mView->groupItems();
  }
}

void QgsComposer::mActionUngroupItems_triggered()
{
  if ( mView )
  {
    mView->ungroupItems();
  }
}

void QgsComposer::mActionLockItems_triggered()
{
  if ( mComposition )
  {
    mComposition->lockSelectedItems();
  }
}

void QgsComposer::mActionUnlockAll_triggered()
{
  if ( mComposition )
  {
    mComposition->unlockAllItems();
  }
}

void QgsComposer::actionCutTriggered()
{
  if ( mView )
  {
    mView->copyItems( QgsComposerView::ClipboardModeCut );
  }
}

void QgsComposer::actionCopyTriggered()
{
  if ( mView )
  {
    mView->copyItems( QgsComposerView::ClipboardModeCopy );
  }
}

void QgsComposer::actionPasteTriggered()
{
  if ( mView )
  {
    QPointF pt = mView->mapToScene( mView->mapFromGlobal( QCursor::pos() ) );
    //TODO - use a better way of determining whether paste was triggered by keystroke
    //or menu item
    if ( ( pt.x() < 0 ) || ( pt.y() < 0 ) )
    {
      //action likely triggered by menu, paste items in center of screen
      mView->pasteItems( QgsComposerView::PasteModeCenter );
    }
    else
    {
      //action likely triggered by keystroke, paste items at cursor position
      mView->pasteItems( QgsComposerView::PasteModeCursor );
    }
  }
}

void QgsComposer::mActionPasteInPlace_triggered()
{
  if ( mView )
  {
    mView->pasteItems( QgsComposerView::PasteModeInPlace );
  }
}

void QgsComposer::mActionDeleteSelection_triggered()
{
  if ( mView )
  {
    mView->deleteSelectedItems();
  }
}

void QgsComposer::mActionSelectAll_triggered()
{
  if ( mView )
  {
    mView->selectAll();
  }
}

void QgsComposer::mActionDeselectAll_triggered()
{
  if ( mView )
  {
    mView->selectNone();
  }
}

void QgsComposer::mActionInvertSelection_triggered()
{
  if ( mView )
  {
    mView->selectInvert();
  }
}

void QgsComposer::mActionSelectNextAbove_triggered()
{
  if ( mComposition )
  {
    mComposition->selectNextByZOrder( QgsComposition::ZValueAbove );
  }
}

void QgsComposer::mActionSelectNextBelow_triggered()
{
  if ( mComposition )
  {
    mComposition->selectNextByZOrder( QgsComposition::ZValueBelow );
  }
}

void QgsComposer::mActionRaiseItems_triggered()
{
  if ( mComposition )
  {
    mComposition->raiseSelectedItems();
  }
}

void QgsComposer::mActionLowerItems_triggered()
{
  if ( mComposition )
  {
    mComposition->lowerSelectedItems();
  }
}

void QgsComposer::mActionMoveItemsToTop_triggered()
{
  if ( mComposition )
  {
    mComposition->moveSelectedItemsToTop();
  }
}

void QgsComposer::mActionMoveItemsToBottom_triggered()
{
  if ( mComposition )
  {
    mComposition->moveSelectedItemsToBottom();
  }
}

void QgsComposer::mActionAlignLeft_triggered()
{
  if ( mComposition )
  {
    mComposition->alignSelectedItemsLeft();
  }
}

void QgsComposer::mActionAlignHCenter_triggered()
{
  if ( mComposition )
  {
    mComposition->alignSelectedItemsHCenter();
  }
}

void QgsComposer::mActionAlignRight_triggered()
{
  if ( mComposition )
  {
    mComposition->alignSelectedItemsRight();
  }
}

void QgsComposer::mActionAlignTop_triggered()
{
  if ( mComposition )
  {
    mComposition->alignSelectedItemsTop();
  }
}

void QgsComposer::mActionAlignVCenter_triggered()
{
  if ( mComposition )
  {
    mComposition->alignSelectedItemsVCenter();
  }
}

void QgsComposer::mActionAlignBottom_triggered()
{
  if ( mComposition )
  {
    mComposition->alignSelectedItemsBottom();
  }
}

void QgsComposer::mActionUndo_triggered()
{
  if ( mComposition && mComposition->undoStack() )
  {
    mComposition->undoStack()->undo();
  }
}

void QgsComposer::mActionRedo_triggered()
{
  if ( mComposition && mComposition->undoStack() )
  {
    mComposition->undoStack()->redo();
  }
}

void QgsComposer::closeEvent( QCloseEvent *e )
{
  Q_UNUSED( e );
  emit aboutToClose();
  saveWindowState();
}

void QgsComposer::moveEvent( QMoveEvent *e )
{
  Q_UNUSED( e );
  saveWindowState();
}

void QgsComposer::resizeEvent( QResizeEvent *e )
{
  Q_UNUSED( e );

  // Move size grip when window is resized
#if 0
  mSizeGrip->move( rect().bottomRight() - mSizeGrip->rect().bottomRight() );
#endif

  saveWindowState();
}

void QgsComposer::saveWindowState()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Composer/geometry" ), saveGeometry() );
  // store the toolbar/dock widget settings using Qt4 settings API
  settings.setValue( QStringLiteral( "ComposerUI/state" ), saveState() );
}

#include "ui_defaults.h"

void QgsComposer::restoreWindowState()
{
  // restore the toolbar and dock widgets positions using Qt4 settings API
  QgsSettings settings;

  if ( !restoreState( settings.value( QStringLiteral( "ComposerUI/state" ), QByteArray::fromRawData( ( char * )defaultComposerUIstate, sizeof defaultComposerUIstate ) ).toByteArray() ) )
  {
    QgsDebugMsg( "restore of composer UI state failed" );
  }
  // restore window geometry
  if ( !restoreGeometry( settings.value( QStringLiteral( "Composer/geometry" ), QByteArray::fromRawData( ( char * )defaultComposerUIgeometry, sizeof defaultComposerUIgeometry ) ).toByteArray() ) )
  {
    QgsDebugMsg( "restore of composer UI geometry failed" );
  }
}

void QgsComposer::createCompositionWidget()
{
  if ( !mComposition )
  {
    return;
  }

  QgsCompositionWidget *compositionWidget = new QgsCompositionWidget( mGeneralDock, mComposition );
  compositionWidget->setDockMode( true );
  connect( mComposition, &QgsComposition::paperSizeChanged, compositionWidget, &QgsCompositionWidget::displayCompositionWidthHeight );
  connect( this, &QgsComposer::printAsRasterChanged, compositionWidget, &QgsCompositionWidget::setPrintAsRasterCheckBox );
  connect( compositionWidget, &QgsCompositionWidget::pageOrientationChanged, this, &QgsComposer::pageOrientationChanged );
  mGeneralPropertiesStack->setMainPanel( compositionWidget );
}

void QgsComposer::restoreGridSettings()
{
  //restore grid settings
  mActionSnapGrid->setChecked( mComposition->snapToGridEnabled() );
  mActionShowGrid->setChecked( mComposition->gridVisible() );
  //restore guide settings
  mActionShowGuides->setChecked( mComposition->snapLinesVisible() );
  mActionSnapGuides->setChecked( mComposition->alignmentSnap() );
  mActionSmartGuides->setChecked( mComposition->smartGuidesEnabled() );
  //general view settings
  mActionShowBoxes->setChecked( mComposition->boundingBoxesVisible() );
}

void QgsComposer::deleteItem( QgsComposerItem * )
{
  showItemOptions( nullptr );
}

void QgsComposer::setSelectionTool()
{
  mActionSelectMoveItem->setChecked( true );
  mActionSelectMoveItem_triggered();
}

bool QgsComposer::containsWmsLayer() const
{
  QList< QgsComposerMap *> maps;
  mComposition->composerItems( maps );

  Q_FOREACH ( QgsComposerMap *map, maps )
  {
    if ( map->containsWmsLayer() )
      return true;
  }
  return false;
}

bool QgsComposer::containsAdvancedEffects() const
{
  QList< QgsComposerItem *> items;
  mComposition->composerItems( items );

  Q_FOREACH ( QgsComposerItem *currentItem, items )
  {
    // Check composer item's blend mode
    if ( currentItem->blendMode() != QPainter::CompositionMode_SourceOver )
    {
      return true;
    }

    // If item is a composer map, check if it contains any advanced effects
    if ( QgsComposerMap *currentMap = dynamic_cast<QgsComposerMap *>( currentItem ) )
    {
      if ( currentMap->containsAdvancedEffects() )
        return true;
    }
  }
  return false;
}

void QgsComposer::showWmsPrintingWarning()
{
  QString myQSettingsLabel = QStringLiteral( "/UI/displayComposerWMSWarning" );
  QgsSettings myQSettings;

  bool displayWMSWarning = myQSettings.value( myQSettingsLabel, true ).toBool();
  if ( displayWMSWarning )
  {
    QgsMessageViewer *m = new QgsMessageViewer( this );
    m->setWindowTitle( tr( "Project Contains WMS Layers" ) );
    m->setMessage( tr( "Some WMS servers (e.g. UMN mapserver) have a limit for the WIDTH and HEIGHT parameter. Printing layers from such servers may exceed this limit. If this is the case, the WMS layer will not be printed" ), QgsMessageOutput::MessageText );
    m->setCheckBoxText( tr( "Don't show this message again" ) );
    m->setCheckBoxState( Qt::Unchecked );
    m->setCheckBoxVisible( true );
    m->setCheckBoxQgsSettingsLabel( myQSettingsLabel );
    m->exec();
  }
}

void QgsComposer::showAdvancedEffectsWarning()
{
  if ( ! mComposition->printAsRaster() )
  {
    QgsMessageViewer *m = new QgsMessageViewer( this, QgsGuiUtils::ModalDialogFlags, false );
    m->setWindowTitle( tr( "Project Contains Composition Effects" ) );
    m->setMessage( tr( "Advanced composition effects such as blend modes or vector layer transparency are enabled in this project, which cannot be printed as vectors. Printing as a raster is recommended." ), QgsMessageOutput::MessageText );
    m->setCheckBoxText( tr( "Print as raster" ) );
    m->setCheckBoxState( Qt::Checked );
    m->setCheckBoxVisible( true );
    m->showMessage( true );

    if ( m->checkBoxState() == Qt::Checked )
    {
      mComposition->setPrintAsRaster( true );
      //make sure print as raster checkbox is updated
      emit printAsRasterChanged( true );
    }
    else
    {
      mComposition->setPrintAsRaster( false );
      emit printAsRasterChanged( false );
    }

    delete m;
  }
}

void QgsComposer::cleanupAfterTemplateRead()
{
  Q_FOREACH ( QGraphicsItem *item, mComposition->items() )
  {
    //update all legends completely
    QgsComposerLegend *legendItem = dynamic_cast<QgsComposerLegend *>( item );
    if ( legendItem )
    {
      legendItem->updateLegend();
      continue;
    }

    //update composer map extent if it does not intersect the full extent of all layers
    QgsComposerMap *mapItem = dynamic_cast<QgsComposerMap *>( item );
    if ( mapItem )
    {
      //test if composer map extent intersects extent of all layers
      bool intersects = false;
      QgsMapCanvas *canvas = mQgis && mQgis->mapCanvas() ? mQgis->mapCanvas() : nullptr;

      QgsRectangle composerMapExtent = mapItem->extent();
      if ( canvas )
      {
        QgsRectangle mapCanvasExtent = mQgis->mapCanvas()->fullExtent();
        if ( composerMapExtent.intersects( mapCanvasExtent ) )
        {
          intersects = true;
        }
      }

      //if not: apply current canvas extent
      if ( canvas && !intersects )
      {
        double currentWidth = mapItem->rect().width();
        double currentHeight = mapItem->rect().height();
        if ( currentWidth - 0 > 0.0 ) //don't divide through zero
        {
          QgsRectangle canvasExtent = canvas->mapSettings().visibleExtent();
          //adapt min y of extent such that the size of the map item stays the same
          double newCanvasExtentHeight = currentHeight / currentWidth * canvasExtent.width();
          canvasExtent.setYMinimum( canvasExtent.yMaximum() - newCanvasExtentHeight );
          mapItem->setNewExtent( canvasExtent );
        }
      }
    }
  }
}

void QgsComposer::mActionPageSetup_triggered()
{
  if ( !mComposition )
  {
    return;
  }

  //set printer page orientation
  setPrinterPageOrientation();
  QPageSetupDialog pageSetupDialog( printer(), this );
  pageSetupDialog.exec();
}

void QgsComposer::populatePrintComposersMenu()
{
  mQgis->populateComposerMenu( mPrintComposersMenu );
}

void QgsComposer::populateWindowMenu()
{
  populateWithOtherMenu( mWindowMenu, mQgis->windowMenu() );
}

void QgsComposer::populateHelpMenu()
{
  populateWithOtherMenu( mHelpMenu, mQgis->helpMenu() );
}

void QgsComposer::populateWithOtherMenu( QMenu *thisMenu, QMenu *otherMenu )
{
  thisMenu->clear();
  Q_FOREACH ( QAction *act, otherMenu->actions() )
  {
    if ( act->menu() )
    {
      thisMenu->addMenu( mirrorOtherMenu( act->menu() ) );
    }
    else
    {
      thisMenu->addAction( act );
    }
  }
}

QMenu *QgsComposer::mirrorOtherMenu( QMenu *otherMenu )
{
  QMenu *newMenu = new QMenu( otherMenu->title(), this );
  Q_FOREACH ( QAction *act, otherMenu->actions() )
  {
    if ( act->menu() )
    {
      newMenu->addMenu( mirrorOtherMenu( act->menu() ) );
    }
    else
    {
      newMenu->addAction( act );
    }
  }
  return newMenu;
}

void QgsComposer::createComposerView()
{
  if ( !mViewLayout )
  {
    return;
  }

  delete mView;
  mView = new QgsComposerView();
  mView->setMapCanvas( mQgis->mapCanvas() );
  mView->setContentsMargins( 0, 0, 0, 0 );
  mView->setHorizontalRuler( mHorizontalRuler );
  mView->setVerticalRuler( mVerticalRuler );
  mViewLayout->addWidget( mView, 1, 1 );

  //view does not accept focus via tab
  mView->setFocusPolicy( Qt::ClickFocus );
}

void QgsComposer::writeWorldFile( const QString &worldFileName, double a, double b, double c, double d, double e, double f ) const
{
  QFile worldFile( worldFileName );
  if ( !worldFile.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) )
  {
    return;
  }
  QTextStream fout( &worldFile );

  // QString::number does not use locale settings (for the decimal point)
  // which is what we want here
  fout << QString::number( a, 'f', 12 ) << "\r\n";
  fout << QString::number( d, 'f', 12 ) << "\r\n";
  fout << QString::number( b, 'f', 12 ) << "\r\n";
  fout << QString::number( e, 'f', 12 ) << "\r\n";
  fout << QString::number( c, 'f', 12 ) << "\r\n";
  fout << QString::number( f, 'f', 12 ) << "\r\n";
}


void QgsComposer::setAtlasFeature( QgsMapLayer *layer, const QgsFeature &feat )
{
  //check if composition atlas settings match
  QgsAtlasComposition &atlas = mComposition->atlasComposition();
  if ( ! atlas.enabled() || atlas.coverageLayer() != layer )
  {
    //either atlas isn't enabled, or layer doesn't match
    return;
  }

  if ( mComposition->atlasMode() != QgsComposition::PreviewAtlas )
  {
    mComposition->setAtlasMode( QgsComposition::PreviewAtlas );
    //update gui controls
    whileBlocking( mActionAtlasPreview )->setChecked( true );
    mActionAtlasFirst->setEnabled( true );
    mActionAtlasLast->setEnabled( true );
    mActionAtlasNext->setEnabled( true );
    mActionAtlasPrev->setEnabled( true );
    mAtlasPageComboBox->setEnabled( true );
  }

  //bring composer window to foreground
  activate();

  mapCanvas()->stopRendering();

  //set current preview feature id
  atlas.prepareForFeature( &feat );
  emit atlasPreviewFeatureChanged();
}

void QgsComposer::pageOrientationChanged( const QString & )
{
  mSetPageOrientation = false;
}

void QgsComposer::setPrinterPageOrientation()
{
  if ( !mSetPageOrientation )
  {
    double paperWidth = mComposition->paperWidth();
    double paperHeight = mComposition->paperHeight();

    //set printer page orientation
    if ( paperWidth > paperHeight )
    {
      printer()->setOrientation( QPrinter::Landscape );
    }
    else
    {
      printer()->setOrientation( QPrinter::Portrait );
    }

    mSetPageOrientation = true;
  }
}

void QgsComposer::loadAtlasPredefinedScalesFromProject()
{
  if ( !mComposition )
  {
    return;
  }
  QgsAtlasComposition &atlasMap = mComposition->atlasComposition();
  QVector<qreal> pScales;
  // first look at project's scales
  QStringList scales( QgsProject::instance()->readListEntry( QStringLiteral( "Scales" ), QStringLiteral( "/ScalesList" ) ) );
  bool hasProjectScales( QgsProject::instance()->readBoolEntry( QStringLiteral( "Scales" ), QStringLiteral( "/useProjectScales" ) ) );
  if ( !hasProjectScales || scales.isEmpty() )
  {
    // default to global map tool scales
    QgsSettings settings;
    QString scalesStr( settings.value( QStringLiteral( "Map/scales" ), PROJECT_SCALES ).toString() );
    scales = scalesStr.split( ',' );
  }

  for ( QStringList::const_iterator scaleIt = scales.constBegin(); scaleIt != scales.constEnd(); ++scaleIt )
  {
    QStringList parts( scaleIt->split( ':' ) );
    if ( parts.size() == 2 )
    {
      pScales.push_back( parts[1].toDouble() );
    }
  }
  atlasMap.setPredefinedScales( pScales );
}

QPrinter *QgsComposer::printer()
{
  //only create the printer on demand - creating a printer object can be very slow
  //due to QTBUG-3033
  if ( !mPrinter )
    mPrinter = new QPrinter();

  return mPrinter;
}


//
// QgsAppComposerInterface
//

QgsAppComposerInterface::QgsAppComposerInterface( QgsComposer *composer )
  : QgsComposerInterface( composer )
  , mComposer( composer )
{}

QgsComposerView *QgsAppComposerInterface::view()
{
  return mComposer->view();
}

QgsComposition *QgsAppComposerInterface::composition()
{
  return mComposer->composition();
}

void QgsAppComposerInterface::close()
{
  mComposer->close();
}

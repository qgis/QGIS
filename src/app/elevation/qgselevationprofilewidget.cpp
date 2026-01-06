/***************************************************************************
                          qgselevationprofilewidget.cpp
                          ---------------
    begin                : March 2022
    copyright            : (C) 2022 by Nyall Dawson
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
#include "qgselevationprofilewidget.h"

#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsdockablewidgethelper.h"
#include "qgselevationprofile.h"
#include "qgselevationprofilecanvas.h"
#include "qgselevationprofileimageexportdialog.h"
#include "qgselevationprofilelayertreeview.h"
#include "qgselevationprofilemanager.h"
#include "qgselevationprofilepdfexportdialog.h"
#include "qgselevationprofiletoolidentify.h"
#include "qgselevationprofiletoolmeasure.h"
#include "qgselevationutils.h"
#include "qgsexpressioncontextutils.h"
#include "qgsfileutils.h"
#include "qgsfillsymbol.h"
#include "qgsfillsymbollayer.h"
#include "qgsgui.h"
#include "qgslayertree.h"
#include "qgslayertreeregistrybridge.h"
#include "qgslayertreeviewdefaultactions.h"
#include "qgslinesymbol.h"
#include "qgslinesymbollayer.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayerelevationproperties.h"
#include "qgsmaplayermodel.h"
#include "qgsmaplayerproxymodel.h"
#include "qgsmaptoolprofilecurve.h"
#include "qgsmaptoolprofilecurvefromfeature.h"
#include "qgsmarkersymbol.h"
#include "qgsmarkersymbollayer.h"
#include "qgsmessagebar.h"
#include "qgsmulticurve.h"
#include "qgsnewnamedialog.h"
#include "qgsplot.h"
#include "qgsplottoolpan.h"
#include "qgsplottoolxaxiszoom.h"
#include "qgsplottoolzoom.h"
#include "qgsprofileexporter.h"
#include "qgsprofilerenderer.h"
#include "qgsprofilesourceregistry.h"
#include "qgsprojectelevationproperties.h"
#include "qgsrubberband.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingstree.h"
#include "qgsshortcutsmanager.h"
#include "qgsstyle.h"
#include "qgssymbolselectordialog.h"
#include "qgsterrainprovider.h"
#include "qgsvectorlayerelevationproperties.h"

#include <QActionGroup>
#include <QPdfWriter>
#include <QProgressBar>
#include <QShortcut>
#include <QSplitter>
#include <QTimer>
#include <QToolBar>

#include "moc_qgselevationprofilewidget.cpp"

const QgsSettingsEntryDouble *QgsElevationProfileWidget::settingTolerance = new QgsSettingsEntryDouble( u"tolerance"_s, QgsSettingsTree::sTreeElevationProfile, 0.1, u"Tolerance distance for elevation profile plots"_s, Qgis::SettingsOptions(), 0 );

const QgsSettingsEntryBool *QgsElevationProfileWidget::settingShowLayerTree = new QgsSettingsEntryBool( u"show-layer-tree"_s, QgsSettingsTree::sTreeElevationProfile, true, u"Whether the layer tree should be shown for elevation profile plots"_s );
const QgsSettingsEntryBool *QgsElevationProfileWidget::settingLockAxis = new QgsSettingsEntryBool( u"lock-axis-ratio"_s, QgsSettingsTree::sTreeElevationProfile, false, u"Whether the the distance and elevation axis scales are locked to each other"_s );
const QgsSettingsEntryString *QgsElevationProfileWidget::settingLastExportDir = new QgsSettingsEntryString( u"last-export-dir"_s, QgsSettingsTree::sTreeElevationProfile, QString(), u"Last elevation profile export directory"_s );
const QgsSettingsEntryColor *QgsElevationProfileWidget::settingBackgroundColor = new QgsSettingsEntryColor( u"background-color"_s, QgsSettingsTree::sTreeElevationProfile, QColor(), u"Elevation profile chart background color"_s );
const QgsSettingsEntryBool *QgsElevationProfileWidget::settingShowSubsections = new QgsSettingsEntryBool( u"show-sub-sections"_s, QgsSettingsTree::sTreeElevationProfile, false, u"Whether to display subsections"_s );
const QgsSettingsEntryBool *QgsElevationProfileWidget::settingShowScaleRatioInToolbar = new QgsSettingsEntryBool( u"show-scale-ratio-in-toolbar"_s, QgsSettingsTree::sTreeElevationProfile, false, u"If true, the scale ratio widget will be moved to the toolbar instead of the options menu"_s );
//
// QgsElevationProfileLayersDialog
//

QgsElevationProfileLayersDialog::QgsElevationProfileLayersDialog( QWidget *parent )
  : QDialog( parent )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  mFilterLineEdit->setShowClearButton( true );
  mFilterLineEdit->setShowSearchIcon( true );

  mModel = new QgsMapLayerProxyModel( listMapLayers );
  listMapLayers->setModel( mModel );
  const QModelIndex firstLayer = mModel->index( 0, 0 );
  listMapLayers->selectionModel()->select( firstLayer, QItemSelectionModel::Select );

  connect( listMapLayers, &QListView::doubleClicked, this, &QgsElevationProfileLayersDialog::accept );

  connect( mFilterLineEdit, &QLineEdit::textChanged, mModel, &QgsMapLayerProxyModel::setFilterString );
  connect( mCheckBoxVisibleLayers, &QCheckBox::toggled, this, &QgsElevationProfileLayersDialog::filterVisible );

  mFilterLineEdit->setFocus();
}

void QgsElevationProfileLayersDialog::setVisibleLayers( const QList<QgsMapLayer *> &layers )
{
  mVisibleLayers = layers;
}

void QgsElevationProfileLayersDialog::setHiddenLayers( const QList<QgsMapLayer *> &layers )
{
  mModel->setExceptedLayerList( layers );
}

QList<QgsMapLayer *> QgsElevationProfileLayersDialog::selectedLayers() const
{
  QList<QgsMapLayer *> layers;

  const QModelIndexList selection = listMapLayers->selectionModel()->selectedIndexes();
  for ( const QModelIndex &index : selection )
  {
    const QModelIndex sourceIndex = mModel->mapToSource( index );
    if ( !sourceIndex.isValid() )
    {
      continue;
    }

    QgsMapLayer *layer = mModel->sourceLayerModel()->layerFromIndex( sourceIndex );
    if ( layer )
      layers << layer;
  }
  return layers;
}

void QgsElevationProfileLayersDialog::filterVisible( bool enabled )
{
  if ( enabled )
    mModel->setLayerAllowlist( mVisibleLayers );
  else
    mModel->setLayerAllowlist( QList<QgsMapLayer *>() );
}


QgsElevationProfileWidget::QgsElevationProfileWidget( QgsElevationProfile *profile, QgsMapCanvas *canvas )
  : QWidget( nullptr )
  , mProfile( profile )
{
  setObjectName( u"ElevationProfile"_s );

  connect( mProfile, &QObject::destroyed, this, &QgsElevationProfileWidget::close );
  connect( mProfile, &QgsElevationProfile::nameChanged, this, [this]( const QString &newName ) {
    mDockableWidgetHelper->setWindowTitle( newName );
  } );
  connect( mProfile, &QgsElevationProfile::useProjectLayerTreeChanged, this, [this] { setupLayerTreeView( true ); } );

  setAttribute( Qt::WA_DeleteOnClose );
  const QgsSettings setting;

  QToolBar *toolBar = new QToolBar( this );
  toolBar->setIconSize( QgisApp::instance()->iconSize( true ) );

  mCanvas = new QgsElevationProfileCanvas( this );
  mCanvas->setProject( QgsProject::instance() );
  connect( mCanvas, &QgsElevationProfileCanvas::activeJobCountChanged, this, &QgsElevationProfileWidget::onTotalPendingJobsCountChanged );
  connect( mCanvas, &QgsElevationProfileCanvas::canvasPointHovered, this, &QgsElevationProfileWidget::onCanvasPointHovered );

  mCanvas->setLockAxisScales( mProfile->lockAxisScales() );

  mCanvas->setBackgroundColor( settingBackgroundColor->value() );
  connect( QgsGui::instance(), &QgsGui::optionsChanged, this, [this] {
    mCanvas->setBackgroundColor( settingBackgroundColor->value() );
  } );

  mPanTool = new QgsPlotToolPan( mCanvas );

  mLayerTreeView = new QgsAppElevationProfileLayerTreeView( mProfile->layerTree() ? mProfile->layerTree() : QgsProject::instance()->layerTreeRoot() );
  connect( mLayerTreeView, &QgsAppElevationProfileLayerTreeView::addLayers, this, &QgsElevationProfileWidget::addLayersInternal );

  connect( mLayerTreeView, &QAbstractItemView::doubleClicked, this, [this]( const QModelIndex &index ) {
    if ( QgsMapLayer *layer = mLayerTreeView->layerForIndex( index ) )
    {
      QgisApp::instance()->showLayerProperties( layer, u"mOptsPage_Elevation"_s );
    }
  } );

  // These 2 connections should be made after mCanvas is created, since they will
  // override canvas sources, set by a connection made in canvas constructor
  connect( QgsApplication::profileSourceRegistry(), &QgsProfileSourceRegistry::profileSourceRegistered, mLayerTreeView, &QgsElevationProfileLayerTreeView::addNodeForRegisteredSource );
  connect( QgsApplication::profileSourceRegistry(), &QgsProfileSourceRegistry::profileSourceUnregistered, mLayerTreeView, &QgsElevationProfileLayerTreeView::removeNodeForUnregisteredSource );

  mZoomTool = new QgsPlotToolZoom( mCanvas );
  mXAxisZoomTool = new QgsPlotToolXAxisZoom( mCanvas );
  mIdentifyTool = new QgsElevationProfileToolIdentify( mCanvas );

  mCanvas->setTool( mIdentifyTool );

  QAction *addLayerAction = new QAction( tr( "Add Layers" ), this );
  addLayerAction->setIcon( QgsApplication::getThemeIcon( u"mActionAddLayer.svg"_s ) );
  connect( addLayerAction, &QAction::triggered, this, &QgsElevationProfileWidget::addLayers );
  toolBar->addAction( addLayerAction );

  mActionAddGroup = mLayerTreeView->defaultActions()->actionAddGroup( this );
  toolBar->addAction( mActionAddGroup );

  QAction *showLayerTree = new QAction( tr( "Show Layer Tree" ), this );
  showLayerTree->setIcon( QgsApplication::getThemeIcon( u"mIconLayerTree.svg"_s ) );
  showLayerTree->setCheckable( true );
  connect( showLayerTree, &QAction::toggled, this, [this]( bool checked ) {
    settingShowLayerTree->setValue( checked );
    mLayerTreeView->setVisible( checked );
  } );
  showLayerTree->setChecked( settingShowLayerTree->value() );
  mLayerTreeView->setVisible( settingShowLayerTree->value() );
  toolBar->addAction( showLayerTree );
  toolBar->addSeparator();

  mCaptureCurveAction = new QAction( tr( "Capture Curve" ), this );
  mCaptureCurveAction->setIcon( QgsApplication::getThemeIcon( u"mActionCaptureLine.svg"_s ) );
  mCaptureCurveAction->setCheckable( true );
  connect( mCaptureCurveAction, &QAction::triggered, this, [this] {
    if ( mCaptureCurveMapTool && mMainCanvas )
    {
      mMainCanvas->setMapTool( mCaptureCurveMapTool.get() );
    }
  } );
  toolBar->addAction( mCaptureCurveAction );

  mCaptureCurveFromFeatureAction = new QAction( tr( "Capture Curve From Feature" ), this );
  mCaptureCurveFromFeatureAction->setIcon( QgsApplication::getThemeIcon( u"mActionCaptureCurveFromFeature.svg"_s ) );
  mCaptureCurveFromFeatureAction->setCheckable( true );
  connect( mCaptureCurveFromFeatureAction, &QAction::triggered, this, [this] {
    if ( mCaptureCurveFromFeatureMapTool && mMainCanvas )
    {
      mMainCanvas->setMapTool( mCaptureCurveFromFeatureMapTool.get() );
    }
  } );
  toolBar->addAction( mCaptureCurveFromFeatureAction );

  mNudgeLeftAction = new QAction( tr( "Nudge Left" ), this );
  mNudgeLeftAction->setIcon( QgsApplication::getThemeIcon( u"mActionArrowLeft.svg"_s ) );
  connect( mNudgeLeftAction, &QAction::triggered, this, &QgsElevationProfileWidget::nudgeLeft );
  mNudgeLeftAction->setEnabled( false );
  toolBar->addAction( mNudgeLeftAction );

  mNudgeRightAction = new QAction( tr( "Nudge Right" ), this );
  mNudgeRightAction->setIcon( QgsApplication::getThemeIcon( u"mActionArrowRight.svg"_s ) );
  connect( mNudgeRightAction, &QAction::triggered, this, &QgsElevationProfileWidget::nudgeRight );
  mNudgeRightAction->setEnabled( false );
  toolBar->addAction( mNudgeRightAction );

  auto createShortcuts = [this]( const QString &objectName, void ( QgsElevationProfileWidget::*slot )() ) {
    if ( QShortcut *sc = QgsGui::shortcutsManager()->shortcutByName( objectName ) )
      connect( sc, &QShortcut::activated, this, slot );
  };
  createShortcuts( u"mProfileToolNudgeLeft"_s, &QgsElevationProfileWidget::nudgeLeft );
  createShortcuts( u"mProfileToolNudgeRight"_s, &QgsElevationProfileWidget::nudgeRight );

  QAction *clearAction = new QAction( tr( "Clear" ), this );
  clearAction->setIcon( QgsApplication::getThemeIcon( u"console/iconClearConsole.svg"_s ) );
  connect( clearAction, &QAction::triggered, this, &QgsElevationProfileWidget::clear );
  toolBar->addAction( clearAction );

  toolBar->addSeparator();

  QAction *identifyToolAction = new QAction( tr( "Identify Features" ), this );
  identifyToolAction->setIcon( QgsApplication::getThemeIcon( u"/mActionIdentify.svg"_s ) );
  identifyToolAction->setCheckable( true );
  identifyToolAction->setChecked( true );
  mIdentifyTool->setAction( identifyToolAction );
  connect( identifyToolAction, &QAction::triggered, mPanTool, [this] { mCanvas->setTool( mIdentifyTool ); } );
  toolBar->addAction( identifyToolAction );

  QAction *panToolAction = new QAction( tr( "Pan" ), this );
  panToolAction->setIcon( QgsApplication::getThemeIcon( u"/mActionPan.svg"_s ) );
  panToolAction->setCheckable( true );
  panToolAction->setChecked( false );
  mPanTool->setAction( panToolAction );
  connect( panToolAction, &QAction::triggered, mPanTool, [this] { mCanvas->setTool( mPanTool ); } );
  toolBar->addAction( panToolAction );

  QAction *zoomXAxisToolAction = new QAction( tr( "Zoom X Axis" ), this );
  zoomXAxisToolAction->setCheckable( true );
  zoomXAxisToolAction->setIcon( QgsApplication::getThemeIcon( u"/mActionZoomInXAxis.svg"_s ) );
  mXAxisZoomTool->setAction( zoomXAxisToolAction );
  connect( zoomXAxisToolAction, &QAction::triggered, mXAxisZoomTool, [this] { mCanvas->setTool( mXAxisZoomTool ); } );
  toolBar->addAction( zoomXAxisToolAction );

  QAction *zoomToolAction = new QAction( tr( "Zoom" ), this );
  zoomToolAction->setCheckable( true );
  zoomToolAction->setIcon( QgsApplication::getThemeIcon( u"/mActionZoomIn.svg"_s ) );
  mZoomTool->setAction( zoomToolAction );
  connect( zoomToolAction, &QAction::triggered, mZoomTool, [this] { mCanvas->setTool( mZoomTool ); } );
  toolBar->addAction( zoomToolAction );

  QAction *resetViewAction = new QAction( tr( "Zoom Full" ), this );
  resetViewAction->setIcon( QgsApplication::getThemeIcon( u"/mActionZoomFullExtent.svg"_s ) );
  connect( resetViewAction, &QAction::triggered, mCanvas, &QgsElevationProfileCanvas::zoomFull );
  toolBar->addAction( resetViewAction );

  QAction *enabledSnappingAction = new QAction( tr( "Enable Snapping" ), this );
  enabledSnappingAction->setIcon( QgsApplication::getThemeIcon( u"/mIconSnapping.svg"_s ) );
  enabledSnappingAction->setCheckable( true );
  enabledSnappingAction->setChecked( true );
  connect( enabledSnappingAction, &QAction::toggled, mCanvas, &QgsElevationProfileCanvas::setSnappingEnabled );
  toolBar->addAction( enabledSnappingAction );

  mMeasureTool = std::make_unique<QgsElevationProfileToolMeasure>( mCanvas );

  QAction *measureToolAction = new QAction( tr( "Measure Distances" ), this );
  measureToolAction->setIcon( QgsApplication::getThemeIcon( u"/mActionMeasure.svg"_s ) );
  measureToolAction->setCheckable( true );
  mMeasureTool->setAction( measureToolAction );
  connect( measureToolAction, &QAction::triggered, this, [this] {
    mCanvas->setTool( mMeasureTool.get() );
  } );
  toolBar->addAction( measureToolAction );

  toolBar->addSeparator();

  QAction *exportAsPdfAction = new QAction( tr( "Export as PDF" ), this );
  exportAsPdfAction->setIcon( QgsApplication::getThemeIcon( u"/mActionSaveAsPDF.svg"_s ) );
  connect( exportAsPdfAction, &QAction::triggered, this, &QgsElevationProfileWidget::exportAsPdf );
  toolBar->addAction( exportAsPdfAction );

  QAction *exportAsImageAction = new QAction( tr( "Export as Image" ), this );
  exportAsImageAction->setIcon( QgsApplication::getThemeIcon( u"/mActionSaveMapAsImage.svg"_s ) );
  connect( exportAsImageAction, &QAction::triggered, this, &QgsElevationProfileWidget::exportAsImage );
  toolBar->addAction( exportAsImageAction );

  QMenu *exportMenu = new QMenu( this );

  QAction *exportAs3DFeaturesAction = new QAction( tr( "Export 3D Features…" ), this );
  connect( exportAs3DFeaturesAction, &QAction::triggered, this, [this] { exportResults( Qgis::ProfileExportType::Features3D ); } );
  exportMenu->addAction( exportAs3DFeaturesAction );

  QAction *exportAs2DProfileAction = new QAction( tr( "Export 2D Profile…" ), this );
  connect( exportAs2DProfileAction, &QAction::triggered, this, [this] { exportResults( Qgis::ProfileExportType::Profile2D ); } );
  exportMenu->addAction( exportAs2DProfileAction );

  QAction *exportAsTableAction = new QAction( tr( "Export Distance/Elevation Table…" ), this );
  connect( exportAsTableAction, &QAction::triggered, this, [this] { exportResults( Qgis::ProfileExportType::DistanceVsElevationTable ); } );
  exportMenu->addAction( exportAsTableAction );

  QToolButton *exportResultsButton = new QToolButton();
  exportResultsButton->setAutoRaise( true );
  exportResultsButton->setToolTip( tr( "Export Results" ) );
  exportResultsButton->setIcon( QgsApplication::getThemeIcon( u"mActionFileSaveAs.svg"_s ) );
  exportResultsButton->setPopupMode( QToolButton::InstantPopup );
  exportResultsButton->setMenu( exportMenu );

  toolBar->addWidget( exportResultsButton );

  toolBar->addSeparator();

  // Options Menu
  mOptionsMenu = new QMenu( this );

  mLockRatioAction = new QAction( tr( "Lock Distance/Elevation Scales" ), this );
  mLockRatioAction->setCheckable( true );
  mLockRatioAction->setChecked( mProfile->lockAxisScales() );
  connect( mLockRatioAction, &QAction::toggled, this, &QgsElevationProfileWidget::axisScaleLockToggled );
  mOptionsMenu->addAction( mLockRatioAction );

  mScaleRatioSettingsAction = new QgsElevationProfileScaleRatioWidgetSettingsAction( mOptionsMenu );

  if ( !settingShowScaleRatioInToolbar->value() )
  {
    mScaleRatioSettingsAction->setDefaultWidget( mScaleRatioSettingsAction->newWidget() );
    mOptionsMenu->addAction( mScaleRatioSettingsAction );
  }

  mSyncLayerTreeAction = new QAction( tr( "Synchronize Layers to Project" ), this );
  mSyncLayerTreeAction->setCheckable( true );
  mSyncLayerTreeAction->setChecked( profile->useProjectLayerTree() );
  connect( mSyncLayerTreeAction, &QAction::toggled, this, &QgsElevationProfileWidget::syncProjectToggled );
  connect( mProfile, &QgsElevationProfile::useProjectLayerTreeChanged, mSyncLayerTreeAction, &QAction::setChecked );
  mOptionsMenu->addAction( mSyncLayerTreeAction );

  mOptionsMenu->addSeparator();

  mToleranceSettingsAction = new QgsElevationProfileToleranceWidgetSettingsAction( mOptionsMenu );

  mToleranceSettingsAction->toleranceSpinBox()->setValue( mProfile->tolerance() );
  connect( mToleranceSettingsAction->toleranceSpinBox(), qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double value ) {
    if ( mProfile )
    {
      mProfile->setTolerance( value );
    }
    settingTolerance->setValue( value );
    createOrUpdateRubberBands();
    scheduleUpdate();
  } );

  mOptionsMenu->addAction( mToleranceSettingsAction );

  mDistanceUnitMenu = new QMenu( tr( "Distance Units" ), this );
  QActionGroup *unitGroup = new QActionGroup( this );
  for ( Qgis::DistanceUnit unit :
        {
          Qgis::DistanceUnit::Kilometers,
          Qgis::DistanceUnit::Meters,
          Qgis::DistanceUnit::Centimeters,
          Qgis::DistanceUnit::Millimeters,
          Qgis::DistanceUnit::Miles,
          Qgis::DistanceUnit::NauticalMiles,
          Qgis::DistanceUnit::Yards,
          Qgis::DistanceUnit::Feet,
          Qgis::DistanceUnit::Inches,
          Qgis::DistanceUnit::Degrees,
        } )
  {
    QString title;
    if ( ( QgsGui::higFlags() & QgsGui::HigDialogTitleIsTitleCase ) )
    {
      title = QgsStringUtils::capitalize( QgsUnitTypes::toString( unit ), Qgis::Capitalization::TitleCase );
    }
    else
    {
      title = QgsUnitTypes::toString( unit );
    }
    QAction *action = new QAction( title );
    action->setData( QVariant::fromValue( unit ) );
    action->setCheckable( true );
    action->setActionGroup( unitGroup );
    connect( action, &QAction::toggled, this, [this, unit]( bool active ) {
      if ( active )
      {
        if ( mProfile )
        {
          mProfile->setDistanceUnit( unit );
        }
        mCanvas->setDistanceUnit( unit );
      }
    } );
    mDistanceUnitMenu->addAction( action );
  }
  connect( mDistanceUnitMenu, &QMenu::aboutToShow, this, [this] {
    for ( QAction *action : mDistanceUnitMenu->actions() )
    {
      if ( action->data().value<Qgis::DistanceUnit>() == mCanvas->distanceUnit() && !action->isChecked() )
        action->setChecked( true );
    }
  } );

  mOptionsMenu->addMenu( mDistanceUnitMenu );

  // show Subsections Indicator Action
  // create a default simple symbology
  mSubsectionsSymbol = mProfile->subsectionsSymbol() ? std::unique_ptr<QgsLineSymbol>( mProfile->subsectionsSymbol()->clone() ) : QgsProfilePlotRenderer::defaultSubSectionsSymbol();
  mShowSubsectionsAction = new QAction( tr( "Show Subsections Indicator" ), this );
  mShowSubsectionsAction->setCheckable( true );
  mShowSubsectionsAction->setChecked( settingShowSubsections->value() );
  connect( mShowSubsectionsAction, &QAction::triggered, this, &QgsElevationProfileWidget::showSubsectionsTriggered );
  mOptionsMenu->addAction( mShowSubsectionsAction );

  // Edit Subsections Symbology action
  mSubsectionsSymbologyAction = new QAction( tr( "Subsections Symbology…" ), this );
  mSubsectionsSymbologyAction->setEnabled( settingShowSubsections->value() );
  connect( mSubsectionsSymbologyAction, &QAction::triggered, this, &QgsElevationProfileWidget::editSubsectionsSymbology );
  mOptionsMenu->addAction( mSubsectionsSymbologyAction );

  mOptionsMenu->addSeparator();

  mRenameProfileAction = new QAction( tr( "Rename Profile…" ), this );
  connect( mRenameProfileAction, &QAction::triggered, this, &QgsElevationProfileWidget::renameProfileTriggered );
  mOptionsMenu->addAction( mRenameProfileAction );

  mBtnOptions = new QToolButton();
  mBtnOptions->setAutoRaise( true );
  mBtnOptions->setToolTip( tr( "Options" ) );
  mBtnOptions->setIcon( QgsApplication::getThemeIcon( u"mActionOptions.svg"_s ) );
  mBtnOptions->setPopupMode( QToolButton::InstantPopup );
  mBtnOptions->setMenu( mOptionsMenu );

  toolBar->addWidget( mBtnOptions );

  mProgressPendingJobs = new QProgressBar( this );
  mProgressPendingJobs->setRange( 0, 0 );
  mProgressPendingJobs->hide();

  QHBoxLayout *topLayout = new QHBoxLayout;
  topLayout->setContentsMargins( 0, 0, 0, 0 );
  topLayout->setSpacing( style()->pixelMetric( QStyle::PM_LayoutHorizontalSpacing ) );
  topLayout->addWidget( toolBar );
  topLayout->addStretch( 1 );

  topLayout->addWidget( mProgressPendingJobs );

  QVBoxLayout *layout = new QVBoxLayout;
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->setSpacing( 0 );
  layout->addLayout( topLayout );

  QSplitter *splitter = new QSplitter( Qt::Horizontal );
  splitter->addWidget( mLayerTreeView );
  splitter->addWidget( mCanvas );
  layout->addWidget( splitter );
  splitter->setCollapsible( 0, false );
  splitter->setCollapsible( 1, false );
  splitter->setSizes( { QFontMetrics( font() ).horizontalAdvance( '0' ) * 10, splitter->width() } );
  QgsSettings settings;
  splitter->restoreState( settings.value( u"Windows/ElevationProfile/SplitState"_s ).toByteArray() );

  connect( splitter, &QSplitter::splitterMoved, this, [splitter] {
    QgsSettings settings;
    settings.setValue( u"Windows/ElevationProfile/SplitState"_s, splitter->saveState() );
  } );
  setLayout( layout );

  mDockableWidgetHelper = new QgsDockableWidgetHelper( mProfile->name(), this, QgisApp::instance(), mProfile->name(), QStringList(), QgsDockableWidgetHelper::OpeningMode::RespectSetting, true, Qt::DockWidgetArea::BottomDockWidgetArea, QgsDockableWidgetHelper::Option::RaiseTab );

  QToolButton *toggleButton = mDockableWidgetHelper->createDockUndockToolButton();
  toggleButton->setToolTip( tr( "Dock Elevation Profile View" ) );
  toolBar->addWidget( toggleButton );
  connect( mDockableWidgetHelper, &QgsDockableWidgetHelper::closed, this, [this]() {
    close();
  } );

  if ( settingShowScaleRatioInToolbar->value() )
  {
    toolBar->addWidget( mScaleRatioSettingsAction->newWidget() );
  }

  // updating the profile plot is deferred on a timer, so that we don't trigger it too often
  mSetCurveTimer = new QTimer( this );
  mSetCurveTimer->setSingleShot( true );
  mSetCurveTimer->stop();
  connect( mSetCurveTimer, &QTimer::timeout, this, &QgsElevationProfileWidget::updatePlot );

  setupLayerTreeView( false );

  // the layer tree registry bridge we rely on to sync project layers with the layer tree ignores layers added to the project
  // but not the legend -- this ignores eg loading layers from QLR files or pasted layers, which have special logic which directly
  // manipulate the PROJECT's layer tree ONLY, and consequently do not trigger the QgsProject::legendLayersAdded signal which the
  // bridge relies on. So we also need to explicitly listen out for QgsProject::layersAddedWithoutLegend here, to ensure that
  // layers added via QLR or copy/paste are also added to the elevation profile's layer tree
  connect( QgsProject::instance(), &QgsProject::layersAddedWithoutLegend, mLayerTreeView, [this] {
    if ( mProfile && !mProfile->useProjectLayerTree() )
    {
      mLayerTreeView->populateMissingLayers( QgsProject::instance() );
    }
  } );

  connect( QgsProject::instance()->elevationProperties(), &QgsProjectElevationProperties::changed, this, &QgsElevationProfileWidget::onProjectElevationPropertiesChanged );
  connect( QgsProject::instance(), &QgsProject::crs3DChanged, this, &QgsElevationProfileWidget::onProjectElevationPropertiesChanged );
  mCanvas->setCrs( QgsProject::instance()->crs3D() );

  connect( mCanvas, &QgsElevationProfileCanvas::scaleChanged, this, [this] {
    mBlockScaleRatioChanges++;
    const double distanceToElevationScaleRatio = mCanvas->axisScaleRatio();
    mScaleRatioSettingsAction->scaleRatioWidget()->setScale( 1.0 / distanceToElevationScaleRatio );
    mBlockScaleRatioChanges--;
  } );

  connect( mScaleRatioSettingsAction->scaleRatioWidget(), &QgsScaleComboBox::scaleChanged, this, [this]( double scale ) {
    const double distanceToElevationRatio = 1.0 / scale;
    if ( mBlockScaleRatioChanges )
      return;

    mCanvas->setAxisScaleRatio( distanceToElevationRatio );
    createOrUpdateRubberBands();
    scheduleUpdate();
  } );

  updateCanvasSources();
  setMainCanvas( canvas );

  if ( mProfile->distanceUnit() != Qgis::DistanceUnit::Unknown )
  {
    mCanvas->setDistanceUnit( mProfile->distanceUnit() );
  }
  if ( const QgsCurve *existingCurve = mProfile->profileCurve() )
  {
    // restore profile curve from stored version
    const QgsCoordinateTransform storedCrsToProjectCrsTransform( mProfile->crs(), QgsProject::instance()->crs3D(), QgsProject::instance()->transformContext() );
    QgsGeometry storedCurveGeometry( existingCurve->clone() );
    try
    {
      storedCurveGeometry.transform( storedCrsToProjectCrsTransform );
    }
    catch ( QgsCsException &e )
    {
      QgsDebugError( u"Could not transform stored elevation profile curve: %1"_s.arg( e.what() ) );
    }

    mCanvas->setCrs( QgsProject::instance()->crs3D() );
    setProfileCurve( storedCurveGeometry, true, false );
  }
}

QgsElevationProfileWidget::~QgsElevationProfileWidget()
{
  if ( mRubberBand )
    mRubberBand.reset();
  if ( mToleranceRubberBand )
    mToleranceRubberBand.reset();

  if ( mMapPointRubberBand )
    mMapPointRubberBand.reset();

  // must be deleted BEFORE the dockable widget helper, or we get a crash
  // when a queued event triggered by the dockable widget helper cleanup
  // tries to repaint the layer tree view, which is then in an undefined state... :o
  delete mLayerTreeView;
  mLayerTreeView = nullptr;

  delete mDockableWidgetHelper;
}

void QgsElevationProfileWidget::applyDefaultSettingsToProfile( QgsElevationProfile *profile )
{
  profile->setLockAxisScales( QgsElevationProfileWidget::settingLockAxis->value() );
  profile->setTolerance( QgsElevationProfileWidget::settingTolerance->value() );
  profile->setDistanceUnit( QgisApp::instance()->mapCanvas()->mapSettings().destinationCrs().mapUnits() );

  // new elevation profiles should start with same layer tree structure as project, i.e.
  // respecting the initial set of groups
  copyProjectTree( profile->layerTree() );
}

void QgsElevationProfileWidget::copyProjectTree( QgsLayerTree *destination )
{
  destination->clear();

  std::function< void( const QgsLayerTreeGroup *sourceGroup, QgsLayerTreeGroup *destinationGroup ) > addGroup;
  addGroup = [&addGroup]( const QgsLayerTreeGroup *sourceGroup, QgsLayerTreeGroup *destinationGroup ) {
    const QList< QgsLayerTreeNode * > sourceGroupChildren = sourceGroup->children();
    for ( const QgsLayerTreeNode *sourceChild : sourceGroupChildren )
    {
      if ( const QgsLayerTreeGroup *sourceChildGroup = qobject_cast< const QgsLayerTreeGroup * >( sourceChild ) )
      {
        QgsLayerTreeGroup *newGroupNode = destinationGroup->addGroup( sourceChildGroup->name() );
        newGroupNode->setExpanded( sourceChildGroup->isExpanded() );
        addGroup( sourceChildGroup, newGroupNode );
      }
      else if ( const QgsLayerTreeLayer *sourceChildLayer = qobject_cast< const QgsLayerTreeLayer * >( sourceChild ) )
      {
        if ( QgsMapLayer *layer = sourceChildLayer->layer() )
        {
          QgsLayerTreeLayer *newLayerNode = destinationGroup->addLayer( layer );

          if ( layer->customProperty( u"_include_in_elevation_profiles"_s ).isValid() )
          {
            newLayerNode->setItemVisibilityChecked( layer->customProperty( u"_include_in_elevation_profiles"_s ).toBool() );
          }
          else
          {
            newLayerNode->setItemVisibilityChecked( layer->elevationProperties() && layer->elevationProperties()->showByDefaultInElevationProfilePlots() );
          }
        }
      }
    }
  };

  addGroup( QgsProject::instance()->layerTreeRoot(), destination );
}

QgsElevationProfile *QgsElevationProfileWidget::profile()
{
  return mProfile;
}

void QgsElevationProfileWidget::setMainCanvas( QgsMapCanvas *canvas )
{
  mMainCanvas = canvas;

  mCaptureCurveMapTool = std::make_unique<QgsMapToolProfileCurve>( canvas, QgisApp::instance()->cadDockWidget() );
  mCaptureCurveMapTool->setAction( mCaptureCurveAction );
  connect( mCaptureCurveMapTool.get(), &QgsMapToolProfileCurve::curveCaptured, this, [this]( const QgsGeometry &curve ) { setProfileCurve( curve, true ); } );
  connect( mCaptureCurveMapTool.get(), &QgsMapToolProfileCurve::captureStarted, this, [this] {
    // if capturing a new curve, we just hide the existing rubber band -- if the user cancels the new curve digitizing then we'll
    // re-show the old curve rubber band
    if ( mRubberBand )
      mRubberBand->hide();
    if ( mToleranceRubberBand )
      mToleranceRubberBand->hide();
    if ( mMapPointRubberBand )
      mMapPointRubberBand->hide();
  } );
  connect( mCaptureCurveMapTool.get(), &QgsMapToolProfileCurve::captureCanceled, this, [this] {
    if ( mRubberBand )
      mRubberBand->show();
    if ( mToleranceRubberBand )
      mToleranceRubberBand->show();
    if ( mMapPointRubberBand )
      mMapPointRubberBand->show();
  } );

  mCaptureCurveFromFeatureMapTool = std::make_unique<QgsMapToolProfileCurveFromFeature>( canvas );
  mCaptureCurveFromFeatureMapTool->setAction( mCaptureCurveFromFeatureAction );
  connect( mCaptureCurveFromFeatureMapTool.get(), &QgsMapToolProfileCurveFromFeature::curveCaptured, this, [this]( const QgsGeometry &curve ) { setProfileCurve( curve, true ); } );

  mMapPointRubberBand.reset( new QgsRubberBand( canvas, Qgis::GeometryType::Point ) );
  mMapPointRubberBand->setZValue( 1000 );
  mMapPointRubberBand->setIcon( QgsRubberBand::ICON_FULL_DIAMOND );
  mMapPointRubberBand->setWidth( QgsGuiUtils::scaleIconSize( 8 ) );
  mMapPointRubberBand->setIconSize( QgsGuiUtils::scaleIconSize( 4 ) );
  mMapPointRubberBand->setSecondaryStrokeColor( QColor( 255, 255, 255, 100 ) );
  mMapPointRubberBand->setColor( QColor( 0, 0, 0 ) );
  mMapPointRubberBand->hide();

  mCanvas->setDistanceUnit( mMainCanvas->mapSettings().destinationCrs().mapUnits() );

  connect( mMainCanvas, &QgsMapCanvas::destinationCrsChanged, this, [this] {
    if ( mProfile )
    {
      mProfile->setDistanceUnit( mMainCanvas->mapSettings().destinationCrs().mapUnits() );
    }
    mCanvas->setDistanceUnit( mMainCanvas->mapSettings().destinationCrs().mapUnits() );
  } );
}

void QgsElevationProfileWidget::setupLayerTreeView( bool resetTree )
{
  if ( !mProfile )
    return;

  if ( mLayerTree )
  {
    disconnect( mLayerTree, &QgsLayerTree::layerOrderChanged, this, &QgsElevationProfileWidget::updateCanvasSources );
    disconnect( mLayerTree, &QgsLayerTreeGroup::visibilityChanged, this, &QgsElevationProfileWidget::updateCanvasSources );
  }

  if ( mProfile->useProjectLayerTree() )
  {
    if ( mLayerTreeBridge )
    {
      delete mLayerTreeBridge;
      mLayerTreeBridge = nullptr;
    }

    mLayerTree = QgsProject::instance()->layerTreeRoot();
    mLayerTreeView->setLayerTree( mLayerTree );
    qobject_cast<QgsElevationProfileLayerTreeModel * >( mLayerTreeView->layerTreeModel() )->setAllowModifications( false );

    mActionAddGroup->setEnabled( false );
  }
  else
  {
    mLayerTree = mProfile->layerTree();
    mLayerTreeView->setLayerTree( mLayerTree );

    // when switching from sync to project tree to custom tree, we initially copy the project tree over
    if ( resetTree )
    {
      copyProjectTree( mLayerTree );
    }
    qobject_cast<QgsElevationProfileLayerTreeModel * >( mLayerTreeView->layerTreeModel() )->setAllowModifications( true );

    mLayerTreeBridge = new QgsLayerTreeRegistryBridge( mProfile->layerTree(), QgsProject::instance(), this );
    mActionAddGroup->setEnabled( true );
  }

  connect( mLayerTree, &QgsLayerTree::layerOrderChanged, this, &QgsElevationProfileWidget::updateCanvasSources );
  connect( mLayerTree, &QgsLayerTreeGroup::visibilityChanged, this, &QgsElevationProfileWidget::updateCanvasSources );
}

void QgsElevationProfileWidget::cancelJobs()
{
  mCanvas->cancelJobs();
}

void QgsElevationProfileWidget::addLayers()
{
  QgsElevationProfileLayersDialog addDialog( this );
  const QMap<QString, QgsMapLayer *> allMapLayers = QgsProject::instance()->mapLayers();

  // The add layers dialog should only show layers which CAN have elevation, yet currently don't
  // have it enabled. So collect layers which don't match this criteria now for filtering out.
  QList<QgsMapLayer *> layersWhichAlreadyHaveElevationOrCannotHaveElevation;
  for ( auto it = allMapLayers.constBegin(); it != allMapLayers.constEnd(); ++it )
  {
    if ( !QgsElevationUtils::canEnableElevationForLayer( it.value() ) || it.value()->elevationProperties()->hasElevation() )
    {
      layersWhichAlreadyHaveElevationOrCannotHaveElevation << it.value();
      continue;
    }
  }
  addDialog.setHiddenLayers( layersWhichAlreadyHaveElevationOrCannotHaveElevation );

  addDialog.setVisibleLayers( mMainCanvas->layers( true ) );

  if ( addDialog.exec() == QDialog::Accepted )
  {
    addLayersInternal( addDialog.selectedLayers() );
  }
}

void QgsElevationProfileWidget::addLayersInternal( const QList<QgsMapLayer *> &layers )
{
  if ( !mProfile || mProfile->useProjectLayerTree() )
    return;

  QList<QgsMapLayer *> updatedLayers;
  if ( !layers.empty() )
  {
    for ( QgsMapLayer *layer : layers )
    {
      if ( QgsElevationUtils::enableElevationForLayer( layer ) )
        updatedLayers << layer;
    }

    mLayerTreeView->proxyModel()->invalidate();
    for ( QgsMapLayer *layer : std::as_const( updatedLayers ) )
    {
      if ( QgsLayerTreeLayer *node = mProfile->layerTree()->findLayer( layer ) )
      {
        node->setItemVisibilityChecked( true );
      }
    }

    updateCanvasSources();
    scheduleUpdate(); // Do we need this call? updateCanvasSources() calls it anyway.
  }
}

void QgsElevationProfileWidget::updateCanvasSources()
{
  if ( !mProfile || !mLayerTree )
    return;

  QList<QgsMapLayer *> layers;
  QList<QgsAbstractProfileSource *> sources;
  const QList<QgsLayerTreeNode *> layerAndCustomNodeOrder = mLayerTree->layerAndCustomNodeOrder();

  for ( QgsLayerTreeNode *node : layerAndCustomNodeOrder )
  {
    if ( QgsLayerTree::isLayer( node ) )
    {
      QgsMapLayer *layer = QgsLayerTree::toLayer( node )->layer();
      // safety check. maybe elevation properties have been disabled externally.
      if ( !layer->elevationProperties() || !layer->elevationProperties()->hasElevation() )
        continue;

      if ( mLayerTree->findLayer( layer )->isVisible() )
      {
        layers << layer;
        sources << layer->profileSource();
      }
    }
    else if ( QgsLayerTree::isCustomNode( node ) && node->customProperty( u"source"_s ) == QgsElevationProfileLayerTreeView::CUSTOM_NODE_ELEVATION_PROFILE_SOURCE )
    {
      QgsLayerTreeCustomNode *customNode = QgsLayerTree::toCustomNode( node );
      if ( mLayerTree->findCustomNode( customNode->nodeId() )->isVisible() )
      {
        if ( QgsAbstractProfileSource *customSource = QgsApplication::profileSourceRegistry()->findSourceById( customNode->nodeId() ) )
        {
          sources << customSource;
        }
      }
    }
  }

  // Legacy: layer tree layers are in opposite direction to what canvas layers requires
  std::reverse( layers.begin(), layers.end() );
  mCanvas->setLayers( layers );

  mCanvas->setSources( sources );
  scheduleUpdate();
}

void QgsElevationProfileWidget::onTotalPendingJobsCountChanged( int count )
{
  if ( count )
  {
    mLastJobTime.restart();
    // if previous job took less than 0.5 seconds, delay the appearance of the
    // job in progress status bar by 0.5 seconds - this avoids the status bar
    // rapidly appearing and then disappearing for very fast jobs
    if ( mLastJobTimeSeconds > 0 && mLastJobTimeSeconds < 0.5 )
    {
      mJobProgressBarTimer.setSingleShot( true );
      mJobProgressBarTimer.setInterval( 500 );
      disconnect( mJobProgressBarTimerConnection );
      mJobProgressBarTimerConnection = connect( &mJobProgressBarTimer, &QTimer::timeout, this, [this]() {
        mProgressPendingJobs->setVisible( true );
      } );
      mJobProgressBarTimer.start();
    }
    else
    {
      mProgressPendingJobs->setVisible( true );
    }
  }
  else
  {
    mJobProgressBarTimer.stop();
    mLastJobTimeSeconds = mLastJobTime.elapsed() / 1000.0;
    mProgressPendingJobs->setVisible( false );
  }
}

void QgsElevationProfileWidget::setProfileCurve( const QgsGeometry &curve, bool resetView, bool storeCurve )
{
  mNudgeLeftAction->setEnabled( !curve.isEmpty() );
  mNudgeRightAction->setEnabled( !curve.isEmpty() );
  mShowSubsectionsAction->setEnabled( !curve.isEmpty() );

  mProfileCurve = curve;
  createOrUpdateRubberBands();
  if ( resetView )
  {
    mCanvas->invalidateCurrentPlotExtent();
    if ( mMeasureTool->isActive() )
    {
      mMeasureTool->clear();
    }
  }
  scheduleUpdate();
  if ( storeCurve && mProfile )
  {
    if ( const QgsCurve *profileCurve = qgsgeometry_cast< const QgsCurve * >( curve.constGet() ) )
    {
      mProfile->setProfileCurve( profileCurve->clone() );
      mProfile->setCrs( QgsProject::instance()->crs3D() );
    }
  }
}

void QgsElevationProfileWidget::onCanvasPointHovered( const QgsPointXY &, const QgsProfilePoint &profilePoint )
{
  if ( !mMapPointRubberBand )
    return;

  const QgsGeometry mapPoint = mProfileCurve.interpolate( profilePoint.distance() );
  if ( mapPoint.isEmpty() )
  {
    mMapPointRubberBand->hide();
  }
  else
  {
    mMapPointRubberBand->setToGeometry( mapPoint );
    mMapPointRubberBand->show();
  }
}

void QgsElevationProfileWidget::updatePlot()
{
  mCanvas->setTolerance( mToleranceSettingsAction->toleranceSpinBox()->value() );
  mCanvas->setCrs( QgsProject::instance()->crs3D() );
  showSubsectionsTriggered();

  if ( !mProfileCurve.isEmpty() )
  {
    if ( const QgsCurve *curve = qgsgeometry_cast<const QgsCurve *>( mProfileCurve.constGet()->simplifiedTypeRef() ) )
    {
      mCanvas->setProfileCurve( curve->clone() );
      mCanvas->refresh();
    }
    else if ( const QgsMultiCurve *multiCurve = qgsgeometry_cast<const QgsMultiCurve *>( mProfileCurve.constGet()->simplifiedTypeRef() ) )
    {
      // hm, just grab the first part!
      mCanvas->setProfileCurve( multiCurve->curveN( 0 )->clone() );
      mCanvas->refresh();
    }
  }
  mUpdateScheduled = false;
}

void QgsElevationProfileWidget::scheduleUpdate()
{
  if ( !mUpdateScheduled )
  {
    mSetCurveTimer->start( 1 );
    mUpdateScheduled = true;
  }
}

void QgsElevationProfileWidget::clear()
{
  mRubberBand.reset();
  mToleranceRubberBand.reset();
  if ( mMapPointRubberBand )
    mMapPointRubberBand->hide();
  mCanvas->clear();
  if ( mMeasureTool->isActive() )
  {
    mMeasureTool->clear();
  }
  mNudgeLeftAction->setEnabled( false );
  mNudgeRightAction->setEnabled( false );
  mShowSubsectionsAction->setEnabled( false );
  mProfileCurve = QgsGeometry();
}

void QgsElevationProfileWidget::exportAsPdf()
{
  QgsSettings s;
  QString outputFileName = QgsFileUtils::findClosestExistingPath( s.value( u"lastProfileExportDir"_s, QDir::homePath(), QgsSettings::App ).toString() );

#ifdef Q_OS_MAC
  QgisApp::instance()->activateWindow();
  this->raise();
#endif
  outputFileName = QFileDialog::getSaveFileName(
    this,
    tr( "Export to PDF" ),
    outputFileName,
    tr( "PDF Format" ) + " (*.pdf *.PDF)"
  );
  this->activateWindow();
  if ( outputFileName.isEmpty() )
  {
    return;
  }
  outputFileName = QgsFileUtils::ensureFileNameHasExtension( outputFileName, { u"pdf"_s } );
  QgsSettings().setValue( u"lastProfileExportDir"_s, outputFileName, QgsSettings::App );

  QgsElevationProfilePdfExportDialog dialog( this );
  dialog.setPlotSettings( mCanvas->plot() );

  if ( !dialog.exec() )
    return;

  QPdfWriter pdfWriter( outputFileName );

  const QgsLayoutSize pageSizeMM = dialog.pageSizeMM();
  QPageLayout pageLayout( QPageSize( pageSizeMM.toQSizeF(), QPageSize::Millimeter ), QPageLayout::Portrait, QMarginsF( 0, 0, 0, 0 ) );
  pageLayout.setMode( QPageLayout::FullPageMode );
  pdfWriter.setPageLayout( pageLayout );
  pdfWriter.setPageMargins( QMarginsF( 0, 0, 0, 0 ) );
  pdfWriter.setResolution( 1200 );

  QPainter p;
  if ( !p.begin( &pdfWriter ) )
  {
    //error beginning print
    QgisApp::instance()->messageBar()->pushWarning( tr( "Save as PDF" ), tr( "Could not create %1" ).arg( QDir::toNativeSeparators( outputFileName ) ) );
    return;
  }

  QgsRenderContext rc = QgsRenderContext::fromQPainter( &p );
  rc.setFlag( Qgis::RenderContextFlag::Antialiasing, true );
  rc.setRasterizedRenderingPolicy( Qgis::RasterizedRenderingPolicy::PreferVector );
  rc.setFlag( Qgis::RenderContextFlag::ApplyScalingWorkaroundForTextRendering, true );
  rc.setFlag( Qgis::RenderContextFlag::HighQualityImageTransforms, true );
  rc.setTextRenderFormat( Qgis::TextRenderFormat::AlwaysText );
  rc.setPainterFlagsUsingContext( &p );

  Qgs2DXyPlot plotSettings;
  dialog.updatePlotSettings( plotSettings );

  mCanvas->render( rc, rc.convertToPainterUnits( pageSizeMM.width(), Qgis::RenderUnit::Millimeters ), rc.convertToPainterUnits( pageSizeMM.height(), Qgis::RenderUnit::Millimeters ), plotSettings );
  p.end();

  QgisApp::instance()->messageBar()->pushSuccess( tr( "Save as PDF" ), tr( "Successfully saved the profile to <a href=\"%1\">%2</a>" ).arg( QUrl::fromLocalFile( outputFileName ).toString(), QDir::toNativeSeparators( outputFileName ) ) );
}

void QgsElevationProfileWidget::exportAsImage()
{
  QgsSettings s;
  QString outputFileName = QgsFileUtils::findClosestExistingPath( s.value( u"lastProfileExportDir"_s, QDir::homePath(), QgsSettings::App ).toString() );

#ifdef Q_OS_MAC
  QgisApp::instance()->activateWindow();
  this->raise();
#endif
  const QPair<QString, QString> fileWithExtension = QgsGuiUtils::getSaveAsImageName( this, tr( "Save Plot As" ), outputFileName );

  this->activateWindow();
  if ( fileWithExtension.first.isEmpty() )
  {
    return;
  }
  QgsSettings().setValue( u"lastProfileExportDir"_s, fileWithExtension.first, QgsSettings::App );

  QgsElevationProfileImageExportDialog dialog( this );
  dialog.setPlotSettings( mCanvas->plot() );
  dialog.setImageSize( mCanvas->plot().size().toSize() );

  if ( !dialog.exec() )
    return;

  QImage image( dialog.imageSize(), QImage::Format_ARGB32 );
  if ( image.isNull() )
  {
    QgisApp::instance()->messageBar()->pushWarning( tr( "Save as Image" ), tr( "Could not create image" ) );
    return;
  }
  image.fill( Qt::transparent );

  QPainter p( &image );

  QgsRenderContext rc = QgsRenderContext::fromQPainter( &p );
  rc.setFlag( Qgis::RenderContextFlag::Antialiasing, true );
  rc.setFlag( Qgis::RenderContextFlag::ApplyScalingWorkaroundForTextRendering, true );
  rc.setFlag( Qgis::RenderContextFlag::HighQualityImageTransforms, true );
  rc.setPainterFlagsUsingContext( &p );

  Qgs2DXyPlot plotSettings;
  dialog.updatePlotSettings( plotSettings );

  mCanvas->render( rc, image.width(), image.height(), plotSettings );
  p.end();

  image.save( fileWithExtension.first );

  QgisApp::instance()->messageBar()->pushSuccess( tr( "Save as Image" ), tr( "Successfully saved the profile to <a href=\"%1\">%2</a>" ).arg( QUrl::fromLocalFile( fileWithExtension.first ).toString(), QDir::toNativeSeparators( fileWithExtension.first ) ) );
}

void QgsElevationProfileWidget::exportResults( Qgis::ProfileExportType type )
{
  std::unique_ptr<QgsCurve> profileCurve;
  if ( !mProfileCurve.isEmpty() )
  {
    if ( const QgsCurve *curve = qgsgeometry_cast<const QgsCurve *>( mProfileCurve.constGet()->simplifiedTypeRef() ) )
    {
      profileCurve.reset( curve->clone() );
    }
    else if ( const QgsMultiCurve *multiCurve = qgsgeometry_cast<const QgsMultiCurve *>( mProfileCurve.constGet()->simplifiedTypeRef() ) )
    {
      // hm, just grab the first part!
      profileCurve.reset( multiCurve->curveN( 0 )->clone() );
    }
  }
  if ( !profileCurve )
    return;

  QString initialExportDirectory = settingLastExportDir->value();
  if ( initialExportDirectory.isEmpty() )
    initialExportDirectory = QDir::homePath();

  QString selectedFilter;
  QString file = QFileDialog::getSaveFileName( this, tr( "Select Output File" ), initialExportDirectory, QgsVectorFileWriter::fileFilterString(), &selectedFilter );
  if ( file.isEmpty() )
    return;

  settingLastExportDir->setValue( QFileInfo( file ).path() );
  file = QgsFileUtils::ensureFileNameHasExtension( file, QgsFileUtils::extensionsFromFilter( selectedFilter ) );

  QgsProfileRequest request( profileCurve.release() );
  request.setCrs( QgsProject::instance()->crs3D() );
  request.setTolerance( mToleranceSettingsAction->toleranceSpinBox()->value() );
  request.setTransformContext( QgsProject::instance()->transformContext() );
  request.setTerrainProvider( QgsProject::instance()->elevationProperties()->terrainProvider() ? QgsProject::instance()->elevationProperties()->terrainProvider()->clone() : nullptr );
  QgsExpressionContext context;
  context.appendScope( QgsExpressionContextUtils::globalScope() );
  context.appendScope( QgsExpressionContextUtils::projectScope( QgsProject::instance() ) );
  request.setExpressionContext( context );

  QgsProfileExporterTask *exportTask = new QgsProfileExporterTask( mCanvas->sources(), request, type, file, QgsProject::instance()->transformContext() );
  connect( exportTask, &QgsTask::taskCompleted, this, [exportTask] {
    switch ( exportTask->result() )
    {
      case QgsProfileExporterTask::ExportResult::Success:
      {
        if ( exportTask->createdFiles().size() == 1 )
        {
          const QString fileName = exportTask->createdFiles().at( 0 );
          QgisApp::instance()->messageBar()->pushSuccess( tr( "Exported Profile" ), tr( "Successfully saved the profile to <a href=\"%1\">%2</a>" ).arg( QUrl::fromLocalFile( fileName ).toString(), QDir::toNativeSeparators( fileName ) ) );
        }
        else if ( !exportTask->createdFiles().empty() )
        {
          const QString firstFile = exportTask->createdFiles().at( 0 );
          const QString firstFilePath = QFileInfo( firstFile ).path();
          QgisApp::instance()->messageBar()->pushSuccess( tr( "Exported Profile" ), tr( "Successfully saved the profile to <a href=\"%1\">%2</a>" ).arg( QUrl::fromLocalFile( firstFile ).toString(), QDir::toNativeSeparators( firstFilePath ) ) );
        }
        break;
      }

      case QgsProfileExporterTask::ExportResult::Empty:
      case QgsProfileExporterTask::ExportResult::DeviceError:
      case QgsProfileExporterTask::ExportResult::DxfExportFailed:
      case QgsProfileExporterTask::ExportResult::LayerExportFailed:
      case QgsProfileExporterTask::ExportResult::Canceled:
        QgisApp::instance()->messageBar()->pushCritical( tr( "Export Failed" ), tr( "The elevation profile could not be exported" ) );
        break;
    }
  } );
  QgsApplication::taskManager()->addTask( exportTask );
}

void QgsElevationProfileWidget::nudgeLeft()
{
  nudgeCurve( Qgis::BufferSide::Left );
}

void QgsElevationProfileWidget::nudgeRight()
{
  nudgeCurve( Qgis::BufferSide::Right );
}

void QgsElevationProfileWidget::nudgeCurve( Qgis::BufferSide side )
{
  // for now we match the nudge distance to the tolerance distance, so that nudging results in
  // a completely different set of point features in the curve. We may want to revisit and expose
  // this as a user configurable setting at some point...
  const double distance = mToleranceSettingsAction->toleranceSpinBox()->value() * 2;

  const QgsGeometry nudgedCurve = mProfileCurve.offsetCurve( side == Qgis::BufferSide::Left ? distance : -distance, 8, Qgis::JoinStyle::Miter, 2 );
  setProfileCurve( nudgedCurve, false );
}

void QgsElevationProfileWidget::axisScaleLockToggled( bool active )
{
  if ( mProfile )
  {
    mProfile->setLockAxisScales( active );
  }
  settingLockAxis->setValue( active );
  mCanvas->setLockAxisScales( active );
}

void QgsElevationProfileWidget::renameProfileTriggered()
{
  bool titleValid = false;
  QString newTitle = mProfile->name();

  const QString chooseMsg = tr( "Enter a unique elevation profile title" );
  QString titleMsg = chooseMsg;

  QStringList profileNames;
  const QList<QgsElevationProfile *> profiles = QgsProject::instance()->elevationProfileManager()->profiles();
  profileNames.reserve( profiles.size() + 1 );
  for ( QgsElevationProfile *l : profiles )
  {
    profileNames << l->name();
  }

  const QString windowTitle = tr( "Rename Elevation Profile" );

  while ( !titleValid )
  {
    QgsNewNameDialog dlg( tr( "elevation profile" ), newTitle, QStringList(), profileNames, Qt::CaseSensitive, this );
    dlg.setWindowTitle( windowTitle );
    dlg.setHintString( titleMsg );
    dlg.setOverwriteEnabled( false );
    dlg.setAllowEmptyName( true );
    dlg.setConflictingNameWarning( tr( "Title already exists!" ) );

    if ( dlg.exec() != QDialog::Accepted )
    {
      return;
    }

    newTitle = dlg.name();
    if ( newTitle.isEmpty() )
    {
      titleMsg = chooseMsg + "\n\n" + tr( "Title can not be empty!" );
    }
    else
    {
      titleValid = true;
    }
  }

  mProfile->setName( newTitle );
  mDockableWidgetHelper->setWindowTitle( newTitle );
}

void QgsElevationProfileWidget::showSubsectionsTriggered()
{
  const bool showSubSections = mShowSubsectionsAction->isChecked();

  settingShowSubsections->setValue( showSubSections );
  mSubsectionsSymbologyAction->setEnabled( showSubSections );

  if ( showSubSections )
  {
    mCanvas->setSubsectionsSymbol( mSubsectionsSymbol ? mSubsectionsSymbol->clone() : nullptr );
  }
  else
  {
    mCanvas->setSubsectionsSymbol( nullptr );
  }
}

void QgsElevationProfileWidget::editSubsectionsSymbology()
{
  QgsSymbolSelectorDialog symbolDialog( mSubsectionsSymbol.get(), QgsStyle::defaultStyle(), nullptr, this );
  symbolDialog.setWindowTitle( tr( "Subsections Symbol Selector" ) );
  if ( symbolDialog.exec() )
  {
    if ( mProfile && mSubsectionsSymbol )
    {
      mProfile->setSubsectionsSymbol( mSubsectionsSymbol->clone() );
    }
    showSubsectionsTriggered();
  }
}

void QgsElevationProfileWidget::syncProjectToggled( bool active )
{
  if ( !mProfile )
    return;

  mProfile->setUseProjectLayerTree( active );
}

void QgsElevationProfileWidget::createOrUpdateRubberBands()
{
  if ( !mRubberBand )
  {
    mRubberBand.reset( new QgsRubberBand( mMainCanvas, Qgis::GeometryType::Line ) );
    mRubberBand->setZValue( 1000 );
    mRubberBand->setWidth( QgsGuiUtils::scaleIconSize( 2 ) );

    QgsSymbolLayerList layers;

    auto bottomLayer = std::make_unique<QgsSimpleLineSymbolLayer>();
    bottomLayer->setWidth( 0.8 );
    bottomLayer->setWidthUnit( Qgis::RenderUnit::Millimeters );
    bottomLayer->setColor( QColor( 40, 40, 40, 100 ) );
    bottomLayer->setPenCapStyle( Qt::PenCapStyle::FlatCap );
    layers.append( bottomLayer.release() );

    auto arrowLayer = std::make_unique<QgsMarkerLineSymbolLayer>();
    arrowLayer->setPlacements( Qgis::MarkerLinePlacement::CentralPoint );

    QgsSymbolLayerList markerLayers;
    auto arrowSymbolLayer = std::make_unique<QgsSimpleMarkerSymbolLayer>( Qgis::MarkerShape::EquilateralTriangle );
    arrowSymbolLayer->setSize( 4 );
    arrowSymbolLayer->setAngle( 90 );
    arrowSymbolLayer->setSizeUnit( Qgis::RenderUnit::Millimeters );
    arrowSymbolLayer->setColor( QColor( 40, 40, 40, 100 ) );
    arrowSymbolLayer->setStrokeColor( QColor( 255, 255, 255, 255 ) );
    arrowSymbolLayer->setStrokeWidth( 0.2 );
    markerLayers.append( arrowSymbolLayer.release() );

    auto markerSymbol = std::make_unique<QgsMarkerSymbol>( markerLayers );
    arrowLayer->setSubSymbol( markerSymbol.release() );

    layers.append( arrowLayer.release() );

    auto topLayer = std::make_unique<QgsSimpleLineSymbolLayer>();
    topLayer->setWidth( 0.4 );
    topLayer->setWidthUnit( Qgis::RenderUnit::Millimeters );
    topLayer->setColor( QColor( 255, 255, 255, 255 ) );
    topLayer->setPenStyle( Qt::DashLine );
    topLayer->setPenCapStyle( Qt::PenCapStyle::FlatCap );
    layers.append( topLayer.release() );

    auto symbol = std::make_unique<QgsLineSymbol>( layers );

    mRubberBand->setSymbol( symbol.release() );
    mRubberBand->updatePosition();
    mRubberBand->show();
  }

  mRubberBand->setToGeometry( mProfileCurve );

  const double tolerance = mToleranceSettingsAction->toleranceSpinBox()->value();
  if ( !qgsDoubleNear( tolerance, 0, 0.000001 ) )
  {
    if ( !mToleranceRubberBand )
    {
      mToleranceRubberBand.reset( new QgsRubberBand( mMainCanvas, Qgis::GeometryType::Polygon ) );
      mToleranceRubberBand->setZValue( 999 );

      QgsSymbolLayerList layers;

      auto bottomLayer = std::make_unique<QgsSimpleFillSymbolLayer>();
      bottomLayer->setColor( QColor( 40, 40, 40, 50 ) );
      bottomLayer->setStrokeColor( QColor( 255, 255, 255, 150 ) );
      layers.append( bottomLayer.release() );

      auto symbol = std::make_unique<QgsFillSymbol>( layers );
      mToleranceRubberBand->setSymbol( symbol.release() );
    }

    const QgsGeometry buffered = mProfileCurve.buffer( tolerance, 8, Qgis::EndCapStyle::Flat, Qgis::JoinStyle::Round, 2 );
    mToleranceRubberBand->setToGeometry( buffered );
    mToleranceRubberBand->show();
  }
  else
  {
    if ( mToleranceRubberBand )
      mToleranceRubberBand->hide();
  }
}

void QgsElevationProfileWidget::onProjectElevationPropertiesChanged()
{
  // Only the vector layers whose clamping depend on the terrain need to be updated
  // if the project elevation properties have changed.
  // Therefore, update the plot if this criteria is met.
  for ( QgsMapLayer *layer : mCanvas->layers() )
  {
    if ( layer->type() == Qgis::LayerType::Vector )
    {
      QgsVectorLayerElevationProperties *elevationProperties = qgis::down_cast<QgsVectorLayerElevationProperties *>( layer->elevationProperties() );
      switch ( elevationProperties->clamping() )
      {
        case Qgis::AltitudeClamping::Relative:
        case Qgis::AltitudeClamping::Terrain:
        {
          scheduleUpdate();
          break;
        }

        case Qgis::AltitudeClamping::Absolute:
          break;
      }
    }
  }
}

//
// QgsElevationProfileToleranceWidgetSettingsAction
//

QgsElevationProfileToleranceWidgetSettingsAction::QgsElevationProfileToleranceWidgetSettingsAction( QWidget *parent )
  : QWidgetAction( parent )
{
  QGridLayout *gLayout = new QGridLayout();
  gLayout->setContentsMargins( 3, 2, 3, 2 );

  mToleranceWidget = new QgsDoubleSpinBox();
  mToleranceWidget->setClearValue( QgsElevationProfileWidget::settingTolerance->defaultValue() );
  mToleranceWidget->setValue( QgsElevationProfileWidget::settingTolerance->defaultValue() );
  mToleranceWidget->setKeyboardTracking( false );
  mToleranceWidget->setMaximumWidth( QFontMetrics( mToleranceWidget->font() ).horizontalAdvance( '0' ) * 50 );
  mToleranceWidget->setDecimals( 2 );
  mToleranceWidget->setRange( 0, 9999999999 );
  mToleranceWidget->setSingleStep( 1.0 );

  QLabel *label = new QLabel( tr( "Tolerance" ) );
  gLayout->addWidget( label, 0, 0 );
  gLayout->addWidget( mToleranceWidget, 0, 1 );

  QWidget *w = new QWidget();
  w->setLayout( gLayout );
  setDefaultWidget( w );
}


//
// QgsElevationProfileScaleRatioWidgetSettingsAction
//

QgsElevationProfileScaleRatioWidgetSettingsAction::QgsElevationProfileScaleRatioWidgetSettingsAction( QWidget *parent )
  : QWidgetAction( parent )
{
}

QWidget *QgsElevationProfileScaleRatioWidgetSettingsAction::newWidget()
{
  QGridLayout *gLayout = new QGridLayout();
  gLayout->setContentsMargins( 3, 2, 3, 2 );

  mScaleRatioWidget = new QgsScaleComboBox();
  mScaleRatioWidget->setRatioMode( QgsScaleComboBox::RatioMode::Flexible );
  mScaleRatioWidget->setScale( 1.0 );
  mScaleRatioWidget->setMaximumWidth( QFontMetrics( mScaleRatioWidget->font() ).horizontalAdvance( '0' ) * 50 );
  mScaleRatioWidget->setPredefinedScales( { 0.01, 0.1, 0.5, 1, 2.0, 10, 100 } );

  QLabel *label = new QLabel( tr( "Scale Ratio" ) );
  gLayout->addWidget( label, 0, 0 );
  gLayout->addWidget( mScaleRatioWidget, 0, 1 );

  QWidget *w = new QWidget();
  w->setLayout( gLayout );

  w->setToolTip( tr( "Specifies the ratio of distance to elevation units used for the profile's scale" ) );
  return w;
}

QgsAppElevationProfileLayerTreeView::QgsAppElevationProfileLayerTreeView( QgsLayerTree *rootNode, QWidget *parent )
  : QgsElevationProfileLayerTreeView( rootNode, parent )
{
}

void QgsAppElevationProfileLayerTreeView::contextMenuEvent( QContextMenuEvent *event )
{
  const QModelIndex index = indexAt( event->pos() );
  if ( !index.isValid() )
    setCurrentIndex( QModelIndex() );

  const bool allowModifications = qobject_cast<QgsElevationProfileLayerTreeModel * >( layerTreeModel() )->allowModifications();
  QMenu *menu = new QMenu();
  if ( QgsMapLayer *layer = layerForIndex( index ) )
  {
    QAction *propertiesAction = new QAction( tr( "Properties…" ), menu );
    connect( propertiesAction, &QAction::triggered, this, [layer] {
      QgisApp::instance()->showLayerProperties( layer, u"mOptsPage_Elevation"_s );
    } );
    menu->addAction( propertiesAction );
  }
  else if ( QgsLayerTreeNode *node = index2node( index ) )
  {
    if ( allowModifications && QgsLayerTree::isGroup( node ) )
    {
      menu->addAction( defaultActions()->actionRenameGroupOrLayer( menu ) );
      menu->addAction( defaultActions()->actionRemoveGroupPromoteLayers( menu ) );
    }
  }

  if ( allowModifications && selectedNodes( true ).count() >= 2 )
    menu->addAction( defaultActions()->actionGroupSelected( menu ) );

  if ( !menu->isEmpty() )
  {
    menu->exec( mapToGlobal( event->pos() ) );
  }
  delete menu;
}

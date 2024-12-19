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
#include "moc_qgselevationprofilewidget.cpp"
#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgselevationprofilecanvas.h"
#include "qgsdockablewidgethelper.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayerelevationproperties.h"
#include "qgsmaplayermodel.h"
#include "qgsmaptoolprofilecurve.h"
#include "qgsmaptoolprofilecurvefromfeature.h"
#include "qgsprojectelevationproperties.h"
#include "qgsvectorlayerelevationproperties.h"
#include "qgsrubberband.h"
#include "qgsplottoolpan.h"
#include "qgsplottoolxaxiszoom.h"
#include "qgsplottoolzoom.h"
#include "qgselevationprofilepdfexportdialog.h"
#include "qgselevationprofileimageexportdialog.h"
#include "qgsfileutils.h"
#include "qgsmessagebar.h"
#include "qgsplot.h"
#include "qgsmulticurve.h"
#include "qgslinesymbol.h"
#include "qgslinesymbollayer.h"
#include "qgsfillsymbol.h"
#include "qgsfillsymbollayer.h"
#include "qgsmarkersymbol.h"
#include "qgsmarkersymbollayer.h"
#include "qgslayertree.h"
#include "qgslayertreeregistrybridge.h"
#include "qgselevationprofilelayertreeview.h"
#include "qgsgui.h"
#include "qgsshortcutsmanager.h"
#include "qgselevationprofiletoolidentify.h"
#include "qgselevationprofiletoolmeasure.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingstree.h"
#include "qgsmaplayerproxymodel.h"
#include "qgselevationutils.h"
#include "qgsprofileexporter.h"
#include "qgsexpressioncontextutils.h"
#include "qgsterrainprovider.h"
#include "qgsprofilesourceregistry.h"
#include "qgsnewnamedialog.h"

#include <QToolBar>
#include <QProgressBar>
#include <QTimer>
#include <QPdfWriter>
#include <QSplitter>
#include <QShortcut>
#include <QActionGroup>

const QgsSettingsEntryDouble *QgsElevationProfileWidget::settingTolerance = new QgsSettingsEntryDouble( QStringLiteral( "tolerance" ), QgsSettingsTree::sTreeElevationProfile, 0.1, QStringLiteral( "Tolerance distance for elevation profile plots" ), Qgis::SettingsOptions(), 0 );

const QgsSettingsEntryBool *QgsElevationProfileWidget::settingShowLayerTree = new QgsSettingsEntryBool( QStringLiteral( "show-layer-tree" ), QgsSettingsTree::sTreeElevationProfile, true, QStringLiteral( "Whether the layer tree should be shown for elevation profile plots" ) );
const QgsSettingsEntryBool *QgsElevationProfileWidget::settingLockAxis = new QgsSettingsEntryBool( QStringLiteral( "lock-axis-ratio" ), QgsSettingsTree::sTreeElevationProfile, false, QStringLiteral( "Whether the the distance and elevation axis scales are locked to each other" ) );
const QgsSettingsEntryString *QgsElevationProfileWidget::settingLastExportDir = new QgsSettingsEntryString( QStringLiteral( "last-export-dir" ), QgsSettingsTree::sTreeElevationProfile, QString(), QStringLiteral( "Last elevation profile export directory" ) );
const QgsSettingsEntryColor *QgsElevationProfileWidget::settingBackgroundColor = new QgsSettingsEntryColor( QStringLiteral( "background-color" ), QgsSettingsTree::sTreeElevationProfile, QColor(), QStringLiteral( "Elevation profile chart background color" ) );
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


QgsElevationProfileWidget::QgsElevationProfileWidget( const QString &name )
  : QWidget( nullptr )
  , mCanvasName( name )
  , mLayerTree( new QgsLayerTree() )
  , mLayerTreeBridge( new QgsLayerTreeRegistryBridge( mLayerTree.get(), QgsProject::instance(), this ) )
{
  setObjectName( QStringLiteral( "ElevationProfile" ) );

  setAttribute( Qt::WA_DeleteOnClose );
  const QgsSettings setting;

  QToolBar *toolBar = new QToolBar( this );
  toolBar->setIconSize( QgisApp::instance()->iconSize( true ) );

  connect( mLayerTree.get(), &QgsLayerTree::layerOrderChanged, this, &QgsElevationProfileWidget::updateCanvasLayers );
  connect( mLayerTree.get(), &QgsLayerTreeGroup::visibilityChanged, this, &QgsElevationProfileWidget::updateCanvasLayers );

  mCanvas = new QgsElevationProfileCanvas( this );
  mCanvas->setProject( QgsProject::instance() );
  connect( mCanvas, &QgsElevationProfileCanvas::activeJobCountChanged, this, &QgsElevationProfileWidget::onTotalPendingJobsCountChanged );
  connect( mCanvas, &QgsElevationProfileCanvas::canvasPointHovered, this, &QgsElevationProfileWidget::onCanvasPointHovered );

  mCanvas->setLockAxisScales( settingLockAxis->value() );

  mCanvas->setBackgroundColor( settingBackgroundColor->value() );
  connect( QgsGui::instance(), &QgsGui::optionsChanged, this, [=] {
    mCanvas->setBackgroundColor( settingBackgroundColor->value() );
  } );

  mPanTool = new QgsPlotToolPan( mCanvas );

  mLayerTreeView = new QgsAppElevationProfileLayerTreeView( mLayerTree.get() );
  connect( mLayerTreeView, &QgsAppElevationProfileLayerTreeView::addLayers, this, &QgsElevationProfileWidget::addLayersInternal );

  connect( mLayerTreeView, &QAbstractItemView::doubleClicked, this, [=]( const QModelIndex &index ) {
    if ( QgsMapLayer *layer = mLayerTreeView->indexToLayer( index ) )
    {
      QgisApp::instance()->showLayerProperties( layer, QStringLiteral( "mOptsPage_Elevation" ) );
    }
  } );

  mZoomTool = new QgsPlotToolZoom( mCanvas );
  mXAxisZoomTool = new QgsPlotToolXAxisZoom( mCanvas );
  mIdentifyTool = new QgsElevationProfileToolIdentify( mCanvas );

  mCanvas->setTool( mIdentifyTool );

  QAction *addLayerAction = new QAction( tr( "Add Layers" ), this );
  addLayerAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionAddLayer.svg" ) ) );
  connect( addLayerAction, &QAction::triggered, this, &QgsElevationProfileWidget::addLayers );
  toolBar->addAction( addLayerAction );

  QAction *showLayerTree = new QAction( tr( "Show Layer Tree" ), this );
  showLayerTree->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mIconLayerTree.svg" ) ) );
  showLayerTree->setCheckable( true );
  connect( showLayerTree, &QAction::toggled, this, [=]( bool checked ) {
    settingShowLayerTree->setValue( checked );
    mLayerTreeView->setVisible( checked );
  } );
  showLayerTree->setChecked( settingShowLayerTree->value() );
  mLayerTreeView->setVisible( settingShowLayerTree->value() );
  toolBar->addAction( showLayerTree );
  toolBar->addSeparator();

  mCaptureCurveAction = new QAction( tr( "Capture Curve" ), this );
  mCaptureCurveAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionCaptureLine.svg" ) ) );
  mCaptureCurveAction->setCheckable( true );
  connect( mCaptureCurveAction, &QAction::triggered, this, [=] {
    if ( mCaptureCurveMapTool && mMainCanvas )
    {
      mMainCanvas->setMapTool( mCaptureCurveMapTool.get() );
    }
  } );
  toolBar->addAction( mCaptureCurveAction );

  mCaptureCurveFromFeatureAction = new QAction( tr( "Capture Curve From Feature" ), this );
  mCaptureCurveFromFeatureAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionCaptureCurveFromFeature.svg" ) ) );
  mCaptureCurveFromFeatureAction->setCheckable( true );
  connect( mCaptureCurveFromFeatureAction, &QAction::triggered, this, [=] {
    if ( mCaptureCurveFromFeatureMapTool && mMainCanvas )
    {
      mMainCanvas->setMapTool( mCaptureCurveFromFeatureMapTool.get() );
    }
  } );
  toolBar->addAction( mCaptureCurveFromFeatureAction );

  mNudgeLeftAction = new QAction( tr( "Nudge Left" ), this );
  mNudgeLeftAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionArrowLeft.svg" ) ) );
  connect( mNudgeLeftAction, &QAction::triggered, this, &QgsElevationProfileWidget::nudgeLeft );
  mNudgeLeftAction->setEnabled( false );
  toolBar->addAction( mNudgeLeftAction );

  mNudgeRightAction = new QAction( tr( "Nudge Right" ), this );
  mNudgeRightAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionArrowRight.svg" ) ) );
  connect( mNudgeRightAction, &QAction::triggered, this, &QgsElevationProfileWidget::nudgeRight );
  mNudgeRightAction->setEnabled( false );
  toolBar->addAction( mNudgeRightAction );

  auto createShortcuts = [=]( const QString &objectName, void ( QgsElevationProfileWidget::*slot )() ) {
    if ( QShortcut *sc = QgsGui::shortcutsManager()->shortcutByName( objectName ) )
      connect( sc, &QShortcut::activated, this, slot );
  };
  createShortcuts( QStringLiteral( "mProfileToolNudgeLeft" ), &QgsElevationProfileWidget::nudgeLeft );
  createShortcuts( QStringLiteral( "mProfileToolNudgeRight" ), &QgsElevationProfileWidget::nudgeRight );

  QAction *clearAction = new QAction( tr( "Clear" ), this );
  clearAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "console/iconClearConsole.svg" ) ) );
  connect( clearAction, &QAction::triggered, this, &QgsElevationProfileWidget::clear );
  toolBar->addAction( clearAction );

  toolBar->addSeparator();

  QAction *identifyToolAction = new QAction( tr( "Identify Features" ), this );
  identifyToolAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionIdentify.svg" ) ) );
  identifyToolAction->setCheckable( true );
  identifyToolAction->setChecked( true );
  mIdentifyTool->setAction( identifyToolAction );
  connect( identifyToolAction, &QAction::triggered, mPanTool, [=] { mCanvas->setTool( mIdentifyTool ); } );
  toolBar->addAction( identifyToolAction );

  QAction *panToolAction = new QAction( tr( "Pan" ), this );
  panToolAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionPan.svg" ) ) );
  panToolAction->setCheckable( true );
  panToolAction->setChecked( false );
  mPanTool->setAction( panToolAction );
  connect( panToolAction, &QAction::triggered, mPanTool, [=] { mCanvas->setTool( mPanTool ); } );
  toolBar->addAction( panToolAction );

  QAction *zoomXAxisToolAction = new QAction( tr( "Zoom X Axis" ), this );
  zoomXAxisToolAction->setCheckable( true );
  zoomXAxisToolAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomInXAxis.svg" ) ) );
  mXAxisZoomTool->setAction( zoomXAxisToolAction );
  connect( zoomXAxisToolAction, &QAction::triggered, mXAxisZoomTool, [=] { mCanvas->setTool( mXAxisZoomTool ); } );
  toolBar->addAction( zoomXAxisToolAction );

  QAction *zoomToolAction = new QAction( tr( "Zoom" ), this );
  zoomToolAction->setCheckable( true );
  zoomToolAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomIn.svg" ) ) );
  mZoomTool->setAction( zoomToolAction );
  connect( zoomToolAction, &QAction::triggered, mZoomTool, [=] { mCanvas->setTool( mZoomTool ); } );
  toolBar->addAction( zoomToolAction );

  QAction *resetViewAction = new QAction( tr( "Zoom Full" ), this );
  resetViewAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomFullExtent.svg" ) ) );
  connect( resetViewAction, &QAction::triggered, mCanvas, &QgsElevationProfileCanvas::zoomFull );
  toolBar->addAction( resetViewAction );

  QAction *enabledSnappingAction = new QAction( tr( "Enable Snapping" ), this );
  enabledSnappingAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mIconSnapping.svg" ) ) );
  enabledSnappingAction->setCheckable( true );
  enabledSnappingAction->setChecked( true );
  connect( enabledSnappingAction, &QAction::toggled, mCanvas, &QgsElevationProfileCanvas::setSnappingEnabled );
  toolBar->addAction( enabledSnappingAction );

  mMeasureTool = std::make_unique<QgsElevationProfileToolMeasure>( mCanvas );

  QAction *measureToolAction = new QAction( tr( "Measure Distances" ), this );
  measureToolAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionMeasure.svg" ) ) );
  measureToolAction->setCheckable( true );
  mMeasureTool->setAction( measureToolAction );
  connect( measureToolAction, &QAction::triggered, this, [=] {
    mCanvas->setTool( mMeasureTool.get() );
  } );
  toolBar->addAction( measureToolAction );

  toolBar->addSeparator();

  QAction *exportAsPdfAction = new QAction( tr( "Export as PDF" ), this );
  exportAsPdfAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionSaveAsPDF.svg" ) ) );
  connect( exportAsPdfAction, &QAction::triggered, this, &QgsElevationProfileWidget::exportAsPdf );
  toolBar->addAction( exportAsPdfAction );

  QAction *exportAsImageAction = new QAction( tr( "Export as Image" ), this );
  exportAsImageAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionSaveMapAsImage.svg" ) ) );
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
  exportResultsButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionFileSaveAs.svg" ) ) );
  exportResultsButton->setPopupMode( QToolButton::InstantPopup );
  exportResultsButton->setMenu( exportMenu );

  toolBar->addWidget( exportResultsButton );

  toolBar->addSeparator();

  // Options Menu
  mOptionsMenu = new QMenu( this );

  mLockRatioAction = new QAction( tr( "Lock Distance/Elevation Scales" ), this );
  mLockRatioAction->setCheckable( true );
  mLockRatioAction->setChecked( settingLockAxis->value() );
  connect( mLockRatioAction, &QAction::toggled, this, &QgsElevationProfileWidget::axisScaleLockToggled );
  mOptionsMenu->addAction( mLockRatioAction );

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
    connect( action, &QAction::toggled, this, [=]( bool active ) {
      if ( active )
      {
        mCanvas->setDistanceUnit( unit );
      }
    } );
    mDistanceUnitMenu->addAction( action );
  }
  connect( mDistanceUnitMenu, &QMenu::aboutToShow, this, [=] {
    for ( QAction *action : mDistanceUnitMenu->actions() )
    {
      if ( action->data().value<Qgis::DistanceUnit>() == mCanvas->distanceUnit() && !action->isChecked() )
        action->setChecked( true );
    }
  } );

  mOptionsMenu->addMenu( mDistanceUnitMenu );
  mOptionsMenu->addSeparator();

  mSettingsAction = new QgsElevationProfileWidgetSettingsAction( mOptionsMenu );

  mSettingsAction->toleranceSpinBox()->setValue( settingTolerance->value() );
  connect( mSettingsAction->toleranceSpinBox(), qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [=]( double value ) {
    settingTolerance->setValue( value );
    createOrUpdateRubberBands();
    scheduleUpdate();
  } );

  mOptionsMenu->addAction( mSettingsAction );

  mOptionsMenu->addSeparator();

  mRenameProfileAction = new QAction( tr( "Rename Profile…" ), this );
  connect( mRenameProfileAction, &QAction::triggered, this, &QgsElevationProfileWidget::renameProfileTriggered );
  mOptionsMenu->addAction( mRenameProfileAction );

  mBtnOptions = new QToolButton();
  mBtnOptions->setAutoRaise( true );
  mBtnOptions->setToolTip( tr( "Options" ) );
  mBtnOptions->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionOptions.svg" ) ) );
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
  splitter->restoreState( settings.value( QStringLiteral( "Windows/ElevationProfile/SplitState" ) ).toByteArray() );

  connect( splitter, &QSplitter::splitterMoved, this, [splitter] {
    QgsSettings settings;
    settings.setValue( QStringLiteral( "Windows/ElevationProfile/SplitState" ), splitter->saveState() );
  } );
  setLayout( layout );

  mDockableWidgetHelper = new QgsDockableWidgetHelper( true, mCanvasName, this, QgisApp::instance(), Qt::BottomDockWidgetArea, QStringList(), true );
  QToolButton *toggleButton = mDockableWidgetHelper->createDockUndockToolButton();
  toggleButton->setToolTip( tr( "Dock Elevation Profile View" ) );
  toolBar->addWidget( toggleButton );
  connect( mDockableWidgetHelper, &QgsDockableWidgetHelper::closed, this, [=]() {
    close();
  } );

  // updating the profile plot is deferred on a timer, so that we don't trigger it too often
  mSetCurveTimer = new QTimer( this );
  mSetCurveTimer->setSingleShot( true );
  mSetCurveTimer->stop();
  connect( mSetCurveTimer, &QTimer::timeout, this, &QgsElevationProfileWidget::updatePlot );

  // initially populate layer tree with project layers
  mLayerTreeView->populateInitialLayers( QgsProject::instance() );

  connect( QgsProject::instance()->elevationProperties(), &QgsProjectElevationProperties::changed, this, &QgsElevationProfileWidget::onProjectElevationPropertiesChanged );
  connect( QgsProject::instance(), &QgsProject::crs3DChanged, this, &QgsElevationProfileWidget::onProjectElevationPropertiesChanged );

  updateCanvasLayers();
}

QgsElevationProfileWidget::~QgsElevationProfileWidget()
{
  if ( mRubberBand )
    mRubberBand.reset();
  if ( mToleranceRubberBand )
    mToleranceRubberBand.reset();

  if ( mMapPointRubberBand )
    mMapPointRubberBand.reset();

  delete mDockableWidgetHelper;
}

void QgsElevationProfileWidget::setCanvasName( const QString &name )
{
  mCanvasName = name;
  mDockableWidgetHelper->setWindowTitle( name );
}

void QgsElevationProfileWidget::setMainCanvas( QgsMapCanvas *canvas )
{
  mMainCanvas = canvas;

  mCaptureCurveMapTool = std::make_unique<QgsMapToolProfileCurve>( canvas, QgisApp::instance()->cadDockWidget() );
  mCaptureCurveMapTool->setAction( mCaptureCurveAction );
  connect( mCaptureCurveMapTool.get(), &QgsMapToolProfileCurve::curveCaptured, this, [=]( const QgsGeometry &curve ) { setProfileCurve( curve, true ); } );
  connect( mCaptureCurveMapTool.get(), &QgsMapToolProfileCurve::captureStarted, this, [=] {
    // if capturing a new curve, we just hide the existing rubber band -- if the user cancels the new curve digitizing then we'll
    // re-show the old curve rubber band
    if ( mRubberBand )
      mRubberBand->hide();
    if ( mToleranceRubberBand )
      mToleranceRubberBand->hide();
    if ( mMapPointRubberBand )
      mMapPointRubberBand->hide();
  } );
  connect( mCaptureCurveMapTool.get(), &QgsMapToolProfileCurve::captureCanceled, this, [=] {
    if ( mRubberBand )
      mRubberBand->show();
    if ( mToleranceRubberBand )
      mToleranceRubberBand->show();
    if ( mMapPointRubberBand )
      mMapPointRubberBand->show();
  } );

  mCaptureCurveFromFeatureMapTool = std::make_unique<QgsMapToolProfileCurveFromFeature>( canvas );
  mCaptureCurveFromFeatureMapTool->setAction( mCaptureCurveFromFeatureAction );
  connect( mCaptureCurveFromFeatureMapTool.get(), &QgsMapToolProfileCurveFromFeature::curveCaptured, this, [=]( const QgsGeometry &curve ) { setProfileCurve( curve, true ); } );

  mMapPointRubberBand.reset( new QgsRubberBand( canvas, Qgis::GeometryType::Point ) );
  mMapPointRubberBand->setZValue( 1000 );
  mMapPointRubberBand->setIcon( QgsRubberBand::ICON_FULL_DIAMOND );
  mMapPointRubberBand->setWidth( QgsGuiUtils::scaleIconSize( 8 ) );
  mMapPointRubberBand->setIconSize( QgsGuiUtils::scaleIconSize( 4 ) );
  mMapPointRubberBand->setSecondaryStrokeColor( QColor( 255, 255, 255, 100 ) );
  mMapPointRubberBand->setColor( QColor( 0, 0, 0 ) );
  mMapPointRubberBand->hide();

  mCanvas->setDistanceUnit( mMainCanvas->mapSettings().destinationCrs().mapUnits() );
  connect( mMainCanvas, &QgsMapCanvas::destinationCrsChanged, this, [=] {
    mCanvas->setDistanceUnit( mMainCanvas->mapSettings().destinationCrs().mapUnits() );
  } );
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
      if ( QgsLayerTreeLayer *node = mLayerTree->findLayer( layer ) )
      {
        node->setItemVisibilityChecked( true );
      }
    }

    updateCanvasLayers();
    scheduleUpdate();
  }
}

void QgsElevationProfileWidget::updateCanvasLayers()
{
  QList<QgsMapLayer *> layers;
  const QList<QgsMapLayer *> layerOrder = mLayerTree->layerOrder();
  layers.reserve( layerOrder.size() );
  for ( QgsMapLayer *layer : layerOrder )
  {
    // safety check. maybe elevation properties have been disabled externally.
    if ( !layer->elevationProperties() || !layer->elevationProperties()->hasElevation() )
      continue;

    if ( mLayerTree->findLayer( layer )->isVisible() )
      layers << layer;
  }

  std::reverse( layers.begin(), layers.end() );
  mCanvas->setLayers( layers );
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
      mJobProgressBarTimerConnection = connect( &mJobProgressBarTimer, &QTimer::timeout, this, [=]() {
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

void QgsElevationProfileWidget::setProfileCurve( const QgsGeometry &curve, bool resetView )
{
  mNudgeLeftAction->setEnabled( !curve.isEmpty() );
  mNudgeRightAction->setEnabled( !curve.isEmpty() );

  mProfileCurve = curve;
  createOrUpdateRubberBands();
  if ( resetView )
    mCanvas->invalidateCurrentPlotExtent();
  scheduleUpdate();
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
  mCanvas->setTolerance( mSettingsAction->toleranceSpinBox()->value() );
  mCanvas->setCrs( QgsProject::instance()->crs3D() );

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
  mNudgeLeftAction->setEnabled( false );
  mNudgeRightAction->setEnabled( false );
  mProfileCurve = QgsGeometry();
}

void QgsElevationProfileWidget::exportAsPdf()
{
  QgsSettings s;
  QString outputFileName = QgsFileUtils::findClosestExistingPath( s.value( QStringLiteral( "lastProfileExportDir" ), QDir::homePath(), QgsSettings::App ).toString() );

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
  outputFileName = QgsFileUtils::ensureFileNameHasExtension( outputFileName, { QStringLiteral( "pdf" ) } );
  QgsSettings().setValue( QStringLiteral( "lastProfileExportDir" ), outputFileName, QgsSettings::App );

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
  rc.setFlag( Qgis::RenderContextFlag::ForceVectorOutput, true );
  rc.setFlag( Qgis::RenderContextFlag::ApplyScalingWorkaroundForTextRendering, true );
  rc.setFlag( Qgis::RenderContextFlag::HighQualityImageTransforms, true );
  rc.setTextRenderFormat( Qgis::TextRenderFormat::AlwaysText );
  rc.setPainterFlagsUsingContext( &p );

  Qgs2DPlot plotSettings;
  dialog.updatePlotSettings( plotSettings );

  mCanvas->render( rc, rc.convertToPainterUnits( pageSizeMM.width(), Qgis::RenderUnit::Millimeters ), rc.convertToPainterUnits( pageSizeMM.height(), Qgis::RenderUnit::Millimeters ), plotSettings );
  p.end();

  QgisApp::instance()->messageBar()->pushSuccess( tr( "Save as PDF" ), tr( "Successfully saved the profile to <a href=\"%1\">%2</a>" ).arg( QUrl::fromLocalFile( outputFileName ).toString(), QDir::toNativeSeparators( outputFileName ) ) );
}

void QgsElevationProfileWidget::exportAsImage()
{
  QgsSettings s;
  QString outputFileName = QgsFileUtils::findClosestExistingPath( s.value( QStringLiteral( "lastProfileExportDir" ), QDir::homePath(), QgsSettings::App ).toString() );

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
  QgsSettings().setValue( QStringLiteral( "lastProfileExportDir" ), fileWithExtension.first, QgsSettings::App );

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

  Qgs2DPlot plotSettings;
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
  request.setTolerance( mSettingsAction->toleranceSpinBox()->value() );
  request.setTransformContext( QgsProject::instance()->transformContext() );
  request.setTerrainProvider( QgsProject::instance()->elevationProperties()->terrainProvider() ? QgsProject::instance()->elevationProperties()->terrainProvider()->clone() : nullptr );
  QgsExpressionContext context;
  context.appendScope( QgsExpressionContextUtils::globalScope() );
  context.appendScope( QgsExpressionContextUtils::projectScope( QgsProject::instance() ) );
  request.setExpressionContext( context );

  const QList<QgsMapLayer *> layersToGenerate = mCanvas->layers();
  QList<QgsAbstractProfileSource *> sources;
  const QList<QgsAbstractProfileSource *> registrySources = QgsApplication::profileSourceRegistry()->profileSources();
  sources.reserve( layersToGenerate.size() + registrySources.size() );

  sources << registrySources;
  for ( QgsMapLayer *layer : layersToGenerate )
  {
    if ( QgsAbstractProfileSource *source = dynamic_cast<QgsAbstractProfileSource *>( layer ) )
      sources.append( source );
  }

  QgsProfileExporterTask *exportTask = new QgsProfileExporterTask( sources, request, type, file, QgsProject::instance()->transformContext() );
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
  const double distance = mSettingsAction->toleranceSpinBox()->value() * 2;

  const QgsGeometry nudgedCurve = mProfileCurve.offsetCurve( side == Qgis::BufferSide::Left ? distance : -distance, 8, Qgis::JoinStyle::Miter, 2 );
  setProfileCurve( nudgedCurve, false );
}

void QgsElevationProfileWidget::axisScaleLockToggled( bool active )
{
  settingLockAxis->setValue( active );
  mCanvas->setLockAxisScales( active );
}

void QgsElevationProfileWidget::renameProfileTriggered()
{
  QgsNewNameDialog dlg( tr( "elevation profile" ), canvasName(), {}, {}, Qt::CaseSensitive, this );
  dlg.setWindowTitle( tr( "Rename Elevation Profile" ) );
  dlg.setHintString( tr( "Enter a new elevation profile title" ) );
  dlg.setAllowEmptyName( false );
  if ( dlg.exec() == QDialog::Accepted )
  {
    setCanvasName( dlg.name() );
  }
}

void QgsElevationProfileWidget::createOrUpdateRubberBands()
{
  if ( !mRubberBand )
  {
    mRubberBand.reset( new QgsRubberBand( mMainCanvas, Qgis::GeometryType::Line ) );
    mRubberBand->setZValue( 1000 );
    mRubberBand->setWidth( QgsGuiUtils::scaleIconSize( 2 ) );

    QgsSymbolLayerList layers;

    std::unique_ptr<QgsSimpleLineSymbolLayer> bottomLayer = std::make_unique<QgsSimpleLineSymbolLayer>();
    bottomLayer->setWidth( 0.8 );
    bottomLayer->setWidthUnit( Qgis::RenderUnit::Millimeters );
    bottomLayer->setColor( QColor( 40, 40, 40, 100 ) );
    bottomLayer->setPenCapStyle( Qt::PenCapStyle::FlatCap );
    layers.append( bottomLayer.release() );

    std::unique_ptr<QgsMarkerLineSymbolLayer> arrowLayer = std::make_unique<QgsMarkerLineSymbolLayer>();
    arrowLayer->setPlacements( Qgis::MarkerLinePlacement::CentralPoint );

    QgsSymbolLayerList markerLayers;
    std::unique_ptr<QgsSimpleMarkerSymbolLayer> arrowSymbolLayer = std::make_unique<QgsSimpleMarkerSymbolLayer>( Qgis::MarkerShape::EquilateralTriangle );
    arrowSymbolLayer->setSize( 4 );
    arrowSymbolLayer->setAngle( 90 );
    arrowSymbolLayer->setSizeUnit( Qgis::RenderUnit::Millimeters );
    arrowSymbolLayer->setColor( QColor( 40, 40, 40, 100 ) );
    arrowSymbolLayer->setStrokeColor( QColor( 255, 255, 255, 255 ) );
    arrowSymbolLayer->setStrokeWidth( 0.2 );
    markerLayers.append( arrowSymbolLayer.release() );

    std::unique_ptr<QgsMarkerSymbol> markerSymbol = std::make_unique<QgsMarkerSymbol>( markerLayers );
    arrowLayer->setSubSymbol( markerSymbol.release() );

    layers.append( arrowLayer.release() );

    std::unique_ptr<QgsSimpleLineSymbolLayer> topLayer = std::make_unique<QgsSimpleLineSymbolLayer>();
    topLayer->setWidth( 0.4 );
    topLayer->setWidthUnit( Qgis::RenderUnit::Millimeters );
    topLayer->setColor( QColor( 255, 255, 255, 255 ) );
    topLayer->setPenStyle( Qt::DashLine );
    topLayer->setPenCapStyle( Qt::PenCapStyle::FlatCap );
    layers.append( topLayer.release() );

    std::unique_ptr<QgsLineSymbol> symbol = std::make_unique<QgsLineSymbol>( layers );

    mRubberBand->setSymbol( symbol.release() );
    mRubberBand->updatePosition();
    mRubberBand->show();
  }

  mRubberBand->setToGeometry( mProfileCurve );

  const double tolerance = mSettingsAction->toleranceSpinBox()->value();
  if ( !qgsDoubleNear( tolerance, 0, 0.000001 ) )
  {
    if ( !mToleranceRubberBand )
    {
      mToleranceRubberBand.reset( new QgsRubberBand( mMainCanvas, Qgis::GeometryType::Polygon ) );
      mToleranceRubberBand->setZValue( 999 );

      QgsSymbolLayerList layers;

      std::unique_ptr<QgsSimpleFillSymbolLayer> bottomLayer = std::make_unique<QgsSimpleFillSymbolLayer>();
      bottomLayer->setColor( QColor( 40, 40, 40, 50 ) );
      bottomLayer->setStrokeColor( QColor( 255, 255, 255, 150 ) );
      layers.append( bottomLayer.release() );

      std::unique_ptr<QgsFillSymbol> symbol = std::make_unique<QgsFillSymbol>( layers );
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

QgsElevationProfileWidgetSettingsAction::QgsElevationProfileWidgetSettingsAction( QWidget *parent )
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

QgsAppElevationProfileLayerTreeView::QgsAppElevationProfileLayerTreeView( QgsLayerTree *rootNode, QWidget *parent )
  : QgsElevationProfileLayerTreeView( rootNode, parent )
{
}

void QgsAppElevationProfileLayerTreeView::contextMenuEvent( QContextMenuEvent *event )
{
  const QModelIndex index = indexAt( event->pos() );
  if ( !index.isValid() )
    setCurrentIndex( QModelIndex() );

  if ( QgsMapLayer *layer = indexToLayer( index ) )
  {
    QMenu *menu = new QMenu();

    QAction *propertiesAction = new QAction( tr( "Properties…" ), menu );
    connect( propertiesAction, &QAction::triggered, this, [layer] {
      QgisApp::instance()->showLayerProperties( layer, QStringLiteral( "mOptsPage_Elevation" ) );
    } );
    menu->addAction( propertiesAction );

    menu->exec( mapToGlobal( event->pos() ) );
    delete menu;
  }
}

/***************************************************************************
  qgs3dmapcanvaswidget.cpp
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

#include "qgs3dmapcanvaswidget.h"

#include "qgisapp.h"
#include "qgs3danimationsettings.h"
#include "qgs3danimationwidget.h"
#include "qgs3dcameracontrolswidget.h"
#include "qgs3ddebugwidget.h"
#include "qgs3deditingtoolbar.h"
#include "qgs3dmapconfigwidget.h"
#include "qgs3dmapexportsettings.h"
#include "qgs3dmapexportwidget.h"
#include "qgs3dmapscene.h"
#include "qgs3dmapsettings.h"
#include "qgs3dmaptoolidentify.h"
#include "qgs3dmaptoolmeasureline.h"
#include "qgs3dmaptoolpointcloudchangeattribute.h"
#include "qgs3dnavigationwidget.h"
#include "qgs3dpointcloudeditingtoolbar.h"
#include "qgs3dutils.h"
#include "qgsannotationlayer.h"
#include "qgsapplication.h"
#include "qgsbloomsettings.h"
#include "qgscameracontroller.h"
#include "qgscrosssection.h"
#include "qgscurve.h"
#include "qgsdockablewidgethelper.h"
#include "qgselevationprofile.h"
#include "qgsflatterrainsettings.h"
#include "qgsframegraph.h"
#include "qgsgui.h"
#include "qgshelp.h"
#include "qgsidentifyresultsdialog.h"
#include "qgslateralpanelwidget.h"
#include "qgslinestring.h"
#include "qgsmapcanvas.h"
#include "qgsmapthemecollection.h"
#include "qgsmaptoolclippingplanes.h"
#include "qgsmaptoolextent.h"
#include "qgsmaptoolidentifyaction.h"
#include "qgsmessagebar.h"
#include "qgspointcloudlayer.h"
#include "qgspointcloudlayer3drenderer.h"
#include "qgspointcloudquerybuilder.h"
#include "qgspolygon.h"
#include "qgsprofilepoint.h"
#include "qgsrubberband.h"
#include "qgsrubberband3d.h"
#include "qgssettings.h"
#include "qgssettingstree.h"
#include "qgsshortcutsmanager.h"
#include "qgswindow3dengine.h"

#include <QAction>
#include <QActionGroup>
#include <QProgressBar>
#include <QShortcut>
#include <QString>
#include <QTabWidget>
#include <QToolBar>
#include <QToolBox>
#include <QWidget>

#include "moc_qgs3dmapcanvaswidget.cpp"

using namespace Qt::StringLiterals;

const QgsSettingsEntryDouble *Qgs3DMapCanvasWidget::settingClippingTolerance
  = new QgsSettingsEntryDouble( u"tolerance"_s, QgsSettingsTree::sTree3DMap, 100, u"Tolerance distance for 3D Map cross section"_s, Qgis::SettingsOptions(), 0 );
const QgsSettingsEntryBool *Qgs3DMapCanvasWidget::settingCrossSectionToleranceLocked
  = new QgsSettingsEntryBool( u"cross-section-tolerance-locked"_s, QgsSettingsTree::sTree3DMap, true, u"Whether cross section tolerance is locked"_s );


Qgs3DMapCanvasWidget::Qgs3DMapCanvasWidget( const QString &name, bool isDocked )
  : QWidget( nullptr )
  , mCanvasName( name )
{
  const QgsSettings setting;

  mToolbarMenu = new QMenu( tr( "Toolbars" ), this );
  mToolbarMenu->setObjectName( u"mToolbarMenu"_s );

  QToolBar *toolBar = new QToolBar( this );
  toolBar->setObjectName( u"m3DMapToolBar"_s );
  toolBar->setIconSize( QgsGui::iconSize( isDocked ? Qgis::UserInterfaceIconType::DockedToolbar : Qgis::UserInterfaceIconType::MainWindowToolbar ) );

  QAction *actionCameraControl = toolBar->addAction( QIcon( QgsApplication::iconPath( "mActionPan.svg" ) ), tr( "Camera Control" ), this, &Qgs3DMapCanvasWidget::cameraControl );
  actionCameraControl->setObjectName( u"m3DActionCameraControl"_s );
  actionCameraControl->setCheckable( true );

  QAction *zoomFullAction = toolBar->addAction( QgsApplication::getThemeIcon( u"mActionZoomFullExtent.svg"_s ), tr( "Zoom Full" ), this, &Qgs3DMapCanvasWidget::resetView );
  zoomFullAction->setObjectName( u"m3DActionZoomFull"_s );
  zoomFullAction->setShortcut( QKeySequence( tr( "Ctrl+0" ) ) );

  // Editing toolbar
  mEditingToolBar = new QToolBar( this );
  mEditingToolBar->setObjectName( u"m3DEditingToolBar"_s );
  mEditingToolBar->setWindowTitle( tr( "Editing Toolbar" ) );

  mActionUndo = new QAction( QgsApplication::getThemeIcon( u"/mActionUndo.svg"_s ), tr( "Undo" ), this );
  mActionRedo = new QAction( QgsApplication::getThemeIcon( u"/mActionRedo.svg"_s ), tr( "Redo" ), this );

  mActionUndo->setObjectName( u"m3DActionUndo"_s );
  mActionRedo->setObjectName( u"m3DActionRedo"_s );

  mEditingToolBar->addAction( mActionUndo );
  mEditingToolBar->addAction( mActionRedo );
  mEditingToolBar->addSeparator();

  mPointCloudEditingToolbar = new Qgs3DPointCloudEditingToolBar( this );
  mEditingToolBar->addWidget( mPointCloudEditingToolbar )->setObjectName( mPointCloudEditingToolbar->objectName() + u"Action"_s );

  mActionEditingToolbar = toolBar->addAction( QIcon( QgsApplication::iconPath( "mActionToggleEditing.svg" ) ), tr( "Show Editing Toolbar" ) );
  mActionEditingToolbar->setObjectName( u"m3DActionShowEditingToolbar"_s );
  mActionEditingToolbar->setEnabled( false );
  mActionEditingToolbar->setCheckable( true );
  mActionEditingToolbar->setChecked( setting.value( u"/3D/editingToolbar/visibility"_s, false, QgsSettings::Gui ).toBool() );
  connect( mActionEditingToolbar, &QAction::toggled, this, &Qgs3DMapCanvasWidget::toggleEditingToolbar );

  QAction *toggleOnScreenNavigation = toolBar->addAction( QgsApplication::getThemeIcon( u"mAction3DNavigation.svg"_s ), tr( "Toggle On-Screen Navigation" ) );
  toggleOnScreenNavigation->setObjectName( u"m3DActionToggleOnScreenNavigation"_s );
  // this is no more a toggle but a button to show the widget
  toggleOnScreenNavigation->setCheckable( false );
  QObject::connect( toggleOnScreenNavigation, &QAction::triggered, this, [this]() { toggleNavigationWidget( true ); } );

  toolBar->addSeparator();

  QAction *actionIdentify = toolBar->addAction( QIcon( QgsApplication::iconPath( "mActionIdentify.svg" ) ), tr( "Identify" ), this, &Qgs3DMapCanvasWidget::identify );
  actionIdentify->setObjectName( u"m3DActionIdentify"_s );
  actionIdentify->setCheckable( true );

  QAction *actionMeasurementTool = toolBar->addAction( QIcon( QgsApplication::iconPath( "mActionMeasure.svg" ) ), tr( "Measurement Line" ), this, &Qgs3DMapCanvasWidget::measureLine );
  actionMeasurementTool->setObjectName( u"m3DActionMeasurementLine"_s );
  actionMeasurementTool->setCheckable( true );

  // Create action group to make the action exclusive
  mToolActionGroup = new QActionGroup( this );
  mToolActionGroup->addAction( actionCameraControl );
  mToolActionGroup->addAction( actionIdentify );
  mToolActionGroup->addAction( actionMeasurementTool );

  for ( auto toolbar : mEditingToolBar->findChildren<Qgs3DEditingToolBar *>() )
  {
    for ( auto action : toolbar->groupActions() )
      mToolActionGroup->addAction( action );
  }
  mToolActionGroup->setExclusive( true );

  mActionAnim = toolBar->addAction( QIcon( QgsApplication::iconPath( "mTaskRunning.svg" ) ), tr( "Animations" ), this, &Qgs3DMapCanvasWidget::toggleAnimations );
  mActionAnim->setObjectName( u"m3DActionAnimations"_s );
  mActionAnim->setCheckable( true );

  // Export Menu
  mExportMenu = new QMenu( this );
  mExportMenu->setObjectName( u"m3DExportMenu"_s );

  mActionExport = new QAction( QgsApplication::getThemeIcon( u"mActionSharingExport.svg"_s ), tr( "Export" ), this );
  mActionExport->setObjectName( u"m3DActionExport"_s );
  mActionExport->setMenu( mExportMenu );
  toolBar->addAction( mActionExport );
  QToolButton *exportButton = qobject_cast<QToolButton *>( toolBar->widgetForAction( mActionExport ) );
  exportButton->setPopupMode( QToolButton::ToolButtonPopupMode::InstantPopup );

  mExportMenu->addAction( QgsApplication::getThemeIcon( u"mActionSaveMapAsImage.svg"_s ), tr( "Save as Image…" ), this, &Qgs3DMapCanvasWidget::saveAsImage )->setObjectName( u"m3DActionSaveAsImage"_s );

  mExportMenu->addAction( QgsApplication::getThemeIcon( u"3d.svg"_s ), tr( "Export 3D Scene" ), this, &Qgs3DMapCanvasWidget::exportScene )->setObjectName( u"m3DActionExportScene"_s );

  toolBar->addSeparator();

  // Map Theme Menu
  mMapThemeMenu = new QMenu( this );
  mMapThemeMenu->setObjectName( u"m3DMapThemeMenu"_s );
  connect( mMapThemeMenu, &QMenu::aboutToShow, this, &Qgs3DMapCanvasWidget::mapThemeMenuAboutToShow );
  connect( QgsProject::instance()->mapThemeCollection(), &QgsMapThemeCollection::mapThemeRenamed, this, &Qgs3DMapCanvasWidget::currentMapThemeRenamed );

  mActionMapThemes = new QAction( tr( "Set View Theme" ), this );
  mActionMapThemes->setObjectName( u"m3DActionSetViewTheme"_s );
  mActionMapThemes->setMenu( mMapThemeMenu );
  mActionMapThemes->setIcon( QgsApplication::getThemeIcon( u"/mActionShowAllLayers.svg"_s ) );
  toolBar->addAction( mActionMapThemes );
  QToolButton *mapThemesButton = qobject_cast<QToolButton *>( toolBar->widgetForAction( mActionMapThemes ) );
  mapThemesButton->setPopupMode( QToolButton::ToolButtonPopupMode::InstantPopup );


  toolBar->addSeparator();

  // Camera Menu
  mCameraMenu = new QMenu( this );
  mCameraMenu->setObjectName( u"m3DCameraMenu"_s );

  mActionCamera = new QAction( QgsApplication::getThemeIcon( u"mIconCamera.svg"_s ), tr( "Camera" ), this );
  mActionCamera->setObjectName( u"m3DActionCamera"_s );
  mActionCamera->setMenu( mCameraMenu );
  toolBar->addAction( mActionCamera );
  QToolButton *cameraButton = qobject_cast<QToolButton *>( toolBar->widgetForAction( mActionCamera ) );
  cameraButton->setPopupMode( QToolButton::ToolButtonPopupMode::InstantPopup );

  mActionSync2DNavTo3D = new QAction( tr( "2D Map View Follows 3D Camera" ), this );
  mActionSync2DNavTo3D->setObjectName( u"m3DActionSync2DNavTo3D"_s );
  mActionSync2DNavTo3D->setCheckable( true );
  connect( mActionSync2DNavTo3D, &QAction::triggered, this, [this]( bool enabled ) {
    Qgis::ViewSyncModeFlags syncMode = mCanvas->mapSettings()->viewSyncMode();
    syncMode.setFlag( Qgis::ViewSyncModeFlag::Sync2DTo3D, enabled );
    mCanvas->mapSettings()->setViewSyncMode( syncMode );
  } );
  mCameraMenu->addAction( mActionSync2DNavTo3D );

  mActionSync3DNavTo2D = new QAction( tr( "3D Camera Follows 2D Map View" ), this );
  mActionSync3DNavTo2D->setObjectName( u"m3DActionSync3DNavTo2D"_s );
  mActionSync3DNavTo2D->setCheckable( true );
  connect( mActionSync3DNavTo2D, &QAction::triggered, this, [this]( bool enabled ) {
    Qgis::ViewSyncModeFlags syncMode = mCanvas->mapSettings()->viewSyncMode();
    syncMode.setFlag( Qgis::ViewSyncModeFlag::Sync3DTo2D, enabled );
    mCanvas->mapSettings()->setViewSyncMode( syncMode );
  } );
  mCameraMenu->addAction( mActionSync3DNavTo2D );

  mShowFrustumPolygon = new QAction( tr( "Show Visible Camera Area in 2D Map View" ), this );
  mShowFrustumPolygon->setObjectName( u"m3DActionShowFrustumPolygon"_s );
  mShowFrustumPolygon->setCheckable( true );
  connect( mShowFrustumPolygon, &QAction::triggered, this, [this]( bool enabled ) { mCanvas->mapSettings()->setViewFrustumVisualizationEnabled( enabled ); } );
  mCameraMenu->addAction( mShowFrustumPolygon );

  mActionShow2DMapOverlay = new QAction( tr( "Show 2D Map Overlay" ), this );
  mActionShow2DMapOverlay->setObjectName( u"m3DActionShow2DMapOverlay"_s );
  mActionShow2DMapOverlay->setCheckable( true );
  connect( mActionShow2DMapOverlay, &QAction::triggered, this, [this]( bool enabled ) { mCanvas->mapSettings()->setIs2DMapOverlayEnabled( enabled ); } );
  mCameraMenu->addAction( mActionShow2DMapOverlay );

  mActionSetSceneExtent = mCameraMenu->addAction( QgsApplication::getThemeIcon( u"extents.svg"_s ), tr( "Set 3D Scene Extent on 2D Map View" ), this, &Qgs3DMapCanvasWidget::setSceneExtentOn2DCanvas );
  mActionSetSceneExtent->setObjectName( u"m3DActionSetSceneExtent"_s );
  mActionSetSceneExtent->setCheckable( true );

  auto createShortcuts = [this]( const QString &objectName, void ( Qgs3DMapCanvasWidget::*slot )() ) {
    if ( QShortcut *sc = QgsGui::shortcutsManager()->shortcutByName( objectName ) )
      connect( sc, &QShortcut::activated, this, slot );
  };
  createShortcuts( u"m3DSetSceneExtent"_s, &Qgs3DMapCanvasWidget::setSceneExtentOn2DCanvas );

  mActionOpenCameraControlsWidget = new QAction( QgsApplication::getThemeIcon( u"/mIconCamera.svg"_s ), tr( "Camera Controls" ), this );
  mActionOpenCameraControlsWidget->setObjectName( u"m3DActionOpenCameraControls"_s );
  connect( mActionOpenCameraControlsWidget, &QAction::triggered, this, &Qgs3DMapCanvasWidget::configureCamera );
  mCameraMenu->addAction( mActionOpenCameraControlsWidget );

  mCrossSectionMenu = new QMenu( this );
  mCrossSectionMenu->setObjectName( u"m3DCrossSectionMenu"_s );
  mActionCrossSection = new QAction( QgsApplication::getThemeIcon( u"mActionEditCut.svg"_s ), tr( "Cross Section" ), this );
  mActionCrossSection->setObjectName( u"m3DActionCrossSection"_s );
  mActionCrossSection->setMenu( mCrossSectionMenu );
  toolBar->addAction( mActionCrossSection );

  QToolButton *crossSectionButton = qobject_cast<QToolButton *>( toolBar->widgetForAction( mActionCrossSection ) );
  crossSectionButton->setPopupMode( QToolButton::ToolButtonPopupMode::InstantPopup );

  mActionSetClippingPlanes = mCrossSectionMenu->addAction( QgsApplication::getThemeIcon( u"mActionEditCut.svg"_s ), tr( "Cross Section Tool" ), this, &Qgs3DMapCanvasWidget::setClippingPlanesOn2DCanvas );
  mActionSetClippingPlanes->setObjectName( u"m3DActionSetClippingPlanes"_s );
  mActionSetClippingPlanes->setCheckable( true );

  mClippingToleranceAction = new Qgs3DMapClippingToleranceWidgetSettingsAction( mCrossSectionMenu );
  mClippingToleranceAction->setObjectName( u"m3DActionClippingTolerance"_s );
  connect( mClippingToleranceAction->toleranceSpinBox(), qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double value ) {
    settingClippingTolerance->setValue( value );
    updateClippingRubberBand();
  } );
  mCrossSectionMenu->addAction( mClippingToleranceAction );

  connect( mClippingToleranceAction, &Qgs3DMapClippingToleranceWidgetSettingsAction::lockStateChanged, this, [this]( bool locked ) {
    lockCrossSectionTolerance( !locked );
    settingCrossSectionToleranceLocked->setValue( !locked );
  } );

  mActionNudgeLeft = new QAction( QgsApplication::getThemeIcon( u"/mActionArrowLeft.svg"_s ), tr( "Nudge Left" ), this );
  mActionNudgeRight = new QAction( QgsApplication::getThemeIcon( u"/mActionArrowRight.svg"_s ), tr( "Nudge Right" ), this );

  mActionNudgeLeft->setObjectName( u"m3DActionNudgeLeft"_s );
  mActionNudgeRight->setObjectName( u"m3DActionNudgeRight"_s );

  mActionNudgeLeft->setDisabled( true );
  mActionNudgeRight->setDisabled( true );

  connect( mActionNudgeLeft, &QAction::triggered, this, &Qgs3DMapCanvasWidget::nudgeLeft );
  connect( mActionNudgeRight, &QAction::triggered, this, &Qgs3DMapCanvasWidget::nudgeRight );

  createShortcuts( u"m3DCrossSectionNudgeLeft"_s, &Qgs3DMapCanvasWidget::nudgeLeft );
  createShortcuts( u"m3DCrossSectionNudgeRight"_s, &Qgs3DMapCanvasWidget::nudgeRight );

  mCrossSectionMenu->addAction( mActionNudgeLeft );
  mCrossSectionMenu->addAction( mActionNudgeRight );

  mCrossSectionMenu->addSeparator();
  mActionDisableClippingPlanes
    = mCrossSectionMenu->addAction( QgsApplication::getThemeIcon( u"mActionEditCutDisabled.svg"_s ), tr( "Disable Cross Section" ), this, &Qgs3DMapCanvasWidget::disableCrossSection );
  mActionDisableClippingPlanes->setObjectName( u"m3DActionDisableCrossSection"_s );
  mActionDisableClippingPlanes->setDisabled( true );

  // Effects Menu
  mEffectsMenu = new QMenu( this );
  mEffectsMenu->setObjectName( u"m3DEffectsMenu"_s );

  mActionEffects = new QAction( QgsApplication::getThemeIcon( u"mIconShadow.svg"_s ), tr( "Effects" ), this );
  mActionEffects->setObjectName( u"m3DActionEffects"_s );
  mActionEffects->setMenu( mEffectsMenu );
  toolBar->addAction( mActionEffects );
  QToolButton *effectsButton = qobject_cast<QToolButton *>( toolBar->widgetForAction( mActionEffects ) );
  effectsButton->setPopupMode( QToolButton::ToolButtonPopupMode::InstantPopup );

  mActionEnableShadows = new QAction( tr( "Show Shadows" ), this );
  mActionEnableShadows->setObjectName( u"m3DActionEnableShadows"_s );
  mActionEnableShadows->setCheckable( true );
  connect( mActionEnableShadows, &QAction::toggled, this, [this]( bool enabled ) {
    QgsShadowSettings settings = mCanvas->mapSettings()->shadowSettings();
    settings.setRenderShadows( enabled );
    mCanvas->mapSettings()->setShadowSettings( settings );
  } );
  mEffectsMenu->addAction( mActionEnableShadows );

  mActionEnableEyeDome = new QAction( tr( "Show Eye Dome Lighting" ), this );
  mActionEnableEyeDome->setObjectName( u"m3DActionEnableEyeDome"_s );
  mActionEnableEyeDome->setCheckable( true );
  connect( mActionEnableEyeDome, &QAction::triggered, this, [this]( bool enabled ) { mCanvas->mapSettings()->setEyeDomeLightingEnabled( enabled ); } );
  mEffectsMenu->addAction( mActionEnableEyeDome );

  mActionEnableAmbientOcclusion = new QAction( tr( "Show Ambient Occlusion" ), this );
  mActionEnableAmbientOcclusion->setObjectName( u"m3DActionEnableAmbientOcclusion"_s );
  mActionEnableAmbientOcclusion->setCheckable( true );
  connect( mActionEnableAmbientOcclusion, &QAction::triggered, this, [this]( bool enabled ) {
    QgsAmbientOcclusionSettings ambientOcclusionSettings = mCanvas->mapSettings()->ambientOcclusionSettings();
    ambientOcclusionSettings.setEnabled( enabled );
    mCanvas->mapSettings()->setAmbientOcclusionSettings( ambientOcclusionSettings );
  } );
  mEffectsMenu->addAction( mActionEnableAmbientOcclusion );

  mActionEnableBloom = new QAction( tr( "Show Bloom Lighting Effect" ), this );
  mActionEnableBloom->setObjectName( u"m3DActionEnableBloom"_s );
  mActionEnableBloom->setCheckable( true );
  connect( mActionEnableBloom, &QAction::triggered, this, [this]( bool enabled ) {
    QgsBloomSettings bloomSettings = mCanvas->mapSettings()->bloomSettings();
    bloomSettings.setEnabled( enabled );
    mCanvas->mapSettings()->setBloomSettings( bloomSettings );
  } );
  mEffectsMenu->addAction( mActionEnableBloom );

  // Options Menu
  QAction *configureAction = new QAction( QgsApplication::getThemeIcon( u"mActionOptions.svg"_s ), tr( "Configure…" ), this );
  configureAction->setObjectName( u"m3DActionConfigure"_s );
  connect( configureAction, &QAction::triggered, this, &Qgs3DMapCanvasWidget::configure );
  toolBar->addAction( configureAction );

  mCanvas = new Qgs3DMapCanvas( this );
  mCanvas->setMinimumSize( QSize( 200, 200 ) );

  connect( mCanvas, &Qgs3DMapCanvas::savedAsImage, this, []( const QString &fileName ) {
    QgisApp::instance()
      ->messageBar()
      ->pushSuccess( tr( "Save as Image" ), tr( "Successfully saved the 3D map to <a href=\"%1\">%2</a>" ).arg( QUrl::fromLocalFile( fileName ).toString(), QDir::toNativeSeparators( fileName ) ) );
  } );

  connect( mCanvas, &Qgs3DMapCanvas::fpsCountChanged, this, &Qgs3DMapCanvasWidget::updateFpsCount );
  connect( mCanvas, &Qgs3DMapCanvas::fpsCounterEnabledChanged, this, &Qgs3DMapCanvasWidget::toggleFpsCounter );
  connect( mCanvas, &Qgs3DMapCanvas::cameraNavigationSpeedChanged, this, &Qgs3DMapCanvasWidget::cameraNavigationSpeedChanged );
  connect( mCanvas, &Qgs3DMapCanvas::viewed2DExtentFrom3DChanged, this, &Qgs3DMapCanvasWidget::onViewed2DExtentFrom3DChanged );
  connect( mCanvas, &Qgs3DMapCanvas::crossSectionEnabledChanged, mActionDisableClippingPlanes, &QAction::setEnabled );

  QgsMapToolIdentifyAction *identifyTool2D = QgisApp::instance()->identifyMapTool();
  QgsIdentifyResultsDialog *resultDialog = identifyTool2D->resultsDialog();
  connect( resultDialog, &QgsIdentifyResultsDialog::featureHighlighted, mCanvas, &Qgs3DMapCanvas::highlightFeature );
  connect( resultDialog, &QgsIdentifyResultsDialog::highlightsCleared, mCanvas, &Qgs3DMapCanvas::clearHighlights );

  mMapToolIdentify = new Qgs3DMapToolIdentify( mCanvas );

  mMapToolMeasureLine = new Qgs3DMapToolMeasureLine( this );

  mLabelPendingJobs = new QLabel( this );
  mProgressPendingJobs = new QProgressBar( this );
  mProgressPendingJobs->setRange( 0, 0 );
  mLabelFpsCounter = new QLabel( this );
  mLabelNavigationSpeed = new QLabel( this );

  mAnimationWidget = new Qgs3DAnimationWidget( this );
  mAnimationWidget->setVisible( false );

  mMessageBar = new QgsMessageBar( this );
  mMessageBar->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );

  QHBoxLayout *topLayout = new QHBoxLayout;
  topLayout->setContentsMargins( 0, 0, 0, 0 );
  topLayout->setSpacing( style()->pixelMetric( QStyle::PM_LayoutHorizontalSpacing ) );
  topLayout->addWidget( toolBar );
  topLayout->addStretch( 1 );
  topLayout->addWidget( mLabelPendingJobs );
  topLayout->addWidget( mProgressPendingJobs );
  topLayout->addWidget( mLabelNavigationSpeed );
  mLabelNavigationSpeed->hide();
  topLayout->addWidget( mLabelFpsCounter );

  mLabelNavSpeedHideTimeout = new QTimer( this );
  mLabelNavSpeedHideTimeout->setInterval( 1000 );
  connect( mLabelNavSpeedHideTimeout, &QTimer::timeout, this, [this] {
    mLabelNavigationSpeed->hide();
    mLabelNavSpeedHideTimeout->stop();
  } );

  // create main vertical layout
  QVBoxLayout *layout = new QVBoxLayout;
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->setSpacing( 0 );
  layout->addLayout( topLayout );
  layout->addWidget( mEditingToolBar );
  layout->addWidget( mMessageBar );

  // mContainer takes ownership of Qgs3DMapCanvas
  mContainer = QWidget::createWindowContainer( mCanvas );
  mContainer->setMinimumSize( QSize( 200, 200 ) );
  mContainer->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

  // create the lateral panel
  QAction *showHideLateralPanel = new QAction( QgsApplication::getThemeIcon( u"mActionResizeWidest.svg"_s ), tr( "Show/hide right panel" ), this );
  showHideLateralPanel->setObjectName( u"showHideLateralPanel"_s );
  mLateralPanel = new QgsLateralPanelWidget( showHideLateralPanel );
  mLateralPanel->hide();

  mNavigationWidget = new Qgs3DNavigationWidget( mCanvas );
  mNavigationWidget->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding );
  mDebugWidget = new Qgs3DDebugWidget( mCanvas );

  mLateralPanel->addWidget( mNavigationWidget, u"Navigation"_s );
  mLateralPanel->addWidget( mDebugWidget, u"Debug"_s );

  // create sub horizontal layout
  QHBoxLayout *hLayout = new QHBoxLayout;
  hLayout->setContentsMargins( 0, 0, 0, 0 );
  hLayout->addWidget( mContainer );
  hLayout->addWidget( mLateralPanel );

  QShortcut *debugPanelShortCut = new QShortcut( QKeySequence( tr( "Ctrl+Shift+d" ) ), this );
  connect( debugPanelShortCut, &QShortcut::activated, this, qOverload<>( &Qgs3DMapCanvasWidget::toggleDebugWidget ) );
  debugPanelShortCut->setObjectName( u"DebugPanel"_s );
  debugPanelShortCut->setWhatsThis( tr( "Debug panel visibility" ) );

  toggleNavigationWidget( setting.value( u"/3D/navigationWidget/visibility"_s, false, QgsSettings::Gui ).toBool() );

  layout->addLayout( hLayout );
  layout->addWidget( mAnimationWidget );

  setLayout( layout );

  onTotalPendingJobsCountChanged();

  mDockableWidgetHelper
    = new QgsDockableWidgetHelper( mCanvasName, this, QgisApp::instance(), mCanvasName, QStringList(), isDocked ? Qgis::DockableWidgetInitialState::ForceDocked : Qgis::DockableWidgetInitialState::RestorePreviousState );

  if ( QDialog *dialog = mDockableWidgetHelper->dialog() )
  {
    QFontMetrics fm( font() );
    const int initialSize = fm.horizontalAdvance( '0' ) * 75;
    dialog->resize( initialSize, initialSize );
  }
  QAction *dockAction = mDockableWidgetHelper->createDockUndockAction( tr( "Dock 3D Map View" ), this );
  dockAction->setObjectName( u"m3DActionDock"_s );
  toolBar->addAction( dockAction );
  connect( mDockableWidgetHelper, &QgsDockableWidgetHelper::closed, this, [this]() { QgisApp::instance()->close3DMapView( canvasName() ); } );
  connect( dockAction, &QAction::toggled, this, [toolBar]( const bool isSmallSize ) {
    toolBar->setIconSize( QgsGui::iconSize( isSmallSize ? Qgis::UserInterfaceIconType::DockedToolbar : Qgis::UserInterfaceIconType::MainWindowToolbar ) );
  } );

  // add action to show/hide lateral panel
  toolBar->addSeparator();
  toolBar->addAction( mLateralPanel->toggleAction() );

  updateLayerRelatedActions( QgisApp::instance()->activeLayer() );
  mEditingToolBar->setVisible( setting.value( u"/3D/editingToolbar/visibility"_s, false, QgsSettings::Gui ).toBool() );

  QList<QAction *> toolbarMenuActions;
  // Set action names so that they can be used in customization
  for ( QToolBar *toolBar : { mEditingToolBar } )
  {
    toolBar->toggleViewAction()->setObjectName( "mActionToggle" + toolBar->objectName().mid( 1 ) );
    toolbarMenuActions << toolBar->toggleViewAction();
  }

  // sort actions in toolbar menu
  std::sort( toolbarMenuActions.begin(), toolbarMenuActions.end(), []( QAction *a, QAction *b ) { return QString::localeAwareCompare( a->text(), b->text() ) < 0; } );

  mToolbarMenu->addActions( toolbarMenuActions );

  toolBar->installEventFilter( this );
  mEditingToolBar->installEventFilter( this );
}

Qgs3DMapCanvasWidget::~Qgs3DMapCanvasWidget()
{
  if ( mCrossSectionRubberBand )
    mCrossSectionRubberBand.reset();

  delete mDockableWidgetHelper;
}

QList<Qgs3DEditingToolBar *> Qgs3DMapCanvasWidget::editingToolBars() const
{
  QList<Qgs3DEditingToolBar *> out;
  if ( mEditingToolBar )
    for ( Qgs3DEditingToolBar *tb : mEditingToolBar->findChildren<Qgs3DEditingToolBar *>() )
    {
      out.append( tb );
    }

  return out;
}

void Qgs3DMapCanvasWidget::addEditingToolBar( Qgs3DEditingToolBar *newToolBar )
{
  if ( mEditingToolBar && newToolBar )
  {
    mEditingToolBar->addWidget( newToolBar );
    mToolbarMenu->addAction( newToolBar->toggleViewAction() );
    for ( auto action : newToolBar->groupActions() )
      mToolActionGroup->addAction( action );

    // disable toolbar by default
    newToolBar->deactivate();
  }
}

QgsLateralPanelWidget *Qgs3DMapCanvasWidget::lateralPanel() const
{
  return mLateralPanel;
}

void Qgs3DMapCanvasWidget::saveAsImage()
{
  const QPair<QString, QString> fileNameAndFilter = QgsGuiUtils::getSaveAsImageName( this, tr( "Choose a file name to save the 3D map canvas to an image" ) );
  if ( !fileNameAndFilter.first.isEmpty() )
  {
    mCanvas->saveAsImage( fileNameAndFilter.first, fileNameAndFilter.second );
  }
}

void Qgs3DMapCanvasWidget::toggleAnimations()
{
  if ( mAnimationWidget->isVisible() )
  {
    mAnimationWidget->setVisible( false );
    return;
  }

  mAnimationWidget->setVisible( true );

  // create a dummy animation when first started - better to have something than nothing...
  if ( mAnimationWidget->animation().duration() == 0 )
  {
    mAnimationWidget->setDefaultAnimation();
  }
}

void Qgs3DMapCanvasWidget::cameraControl()
{
  QAction *action = qobject_cast<QAction *>( sender() );
  if ( !action )
    return;

  mCanvas->setMapTool( nullptr );
}

void Qgs3DMapCanvasWidget::identify()
{
  QAction *action = qobject_cast<QAction *>( sender() );
  if ( !action )
    return;

  mCanvas->setMapTool( action->isChecked() ? mMapToolIdentify : nullptr );
}

void Qgs3DMapCanvasWidget::measureLine()
{
  QAction *action = qobject_cast<QAction *>( sender() );
  if ( !action )
    return;

  mCanvas->setMapTool( action->isChecked() ? mMapToolMeasureLine : nullptr );
}

void Qgs3DMapCanvasWidget::setCanvasName( const QString &name )
{
  mCanvasName = name;
  mDockableWidgetHelper->setWindowTitle( name );
}

void Qgs3DMapCanvasWidget::updateLayerRelatedActions( QgsMapLayer *layer )
{
  if ( !layer || layer == mLayer )
    return;

  qDebug() << __FUNCTION__ << __LINE__ << "for layer:" << layer;
  // toggle previous layer if editable and not modified
  if ( mLayer && mLayer->isEditable() && !mLayer->isModified() )
    QgisApp::instance()->toggleEditing( mLayer );

  // set new working layer
  mLayer = layer;

  updateEditingToolBar();
}

void Qgs3DMapCanvasWidget::updateEditingToolBar()
{
  if ( !mEditingToolBar )
    return;

  mActionUndo->disconnect();
  mActionRedo->disconnect();
  disconnect( mUndoConnection );
  disconnect( mRedoConnection );

  bool toolbarFound = false;
  for ( auto toolbar : mEditingToolBar->findChildren<Qgs3DEditingToolBar *>() )
  {
    if ( mLayer && toolbar->accept( mLayer ) && mLayer->supportsEditing() )
    {
      toolbar->activate( mLayer );
      toolbarFound = true;
    }
    else
      toolbar->deactivate();
  }

  if ( toolbarFound && mLayer && mLayer->supportsEditing() )
  {
    // enable mEditingToolBar
    mActionEditingToolbar->setEnabled( true );
    QgsSettings setting;
    mEditingToolBar->setVisible( setting.value( u"/3D/editingToolbar/visibility"_s, false, QgsSettings::Gui ).toBool() );

    // toggle editing if not already editable
    if ( !mLayer->isEditable() )
      QgisApp::instance()->toggleEditing( mLayer );

    mDockableWidgetHelper->setWindowTitle( u"%1 - %2"_s.arg( mCanvasName ).arg( mLayer->name() ) );

    connect( mActionUndo, &QAction::triggered, mLayer->undoStack(), &QUndoStack::undo );
    connect( mActionRedo, &QAction::triggered, mLayer->undoStack(), &QUndoStack::redo );
    mActionUndo->setEnabled( mLayer->undoStack()->canUndo() );
    mActionRedo->setEnabled( mLayer->undoStack()->canRedo() );
    mUndoConnection = connect( mLayer->undoStack(), &QUndoStack::canUndoChanged, mActionUndo, &QAction::setEnabled );
    mRedoConnection = connect( mLayer->undoStack(), &QUndoStack::canRedoChanged, mActionRedo, &QAction::setEnabled );
  }
  else
  {
    // disable mEditingToolBar
    mActionEditingToolbar->setEnabled( false );
    mEditingToolBar->setVisible( false );

    mDockableWidgetHelper->setWindowTitle( mCanvasName );
  }
}

bool Qgs3DMapCanvasWidget::eventFilter( QObject *watched, QEvent *event )
{
  if ( qobject_cast< QToolBar * >( watched ) )
  {
    if ( event->type() != QEvent::MouseButtonPress )
      return QObject::eventFilter( watched, event );

    QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent *>( event );
    if ( !mouseEvent )
      return QObject::eventFilter( watched, event );

    if ( mouseEvent->button() != Qt::RightButton )
      return QObject::eventFilter( watched, event );

    mToolbarMenu->exec( mouseEvent->globalPos() );
    return false;
  }
  return QObject::eventFilter( watched, event );
}

void Qgs3DMapCanvasWidget::toggleNavigationWidget( const bool visibility )
{
  if ( visibility )
  {
    mLateralPanel->showWidget( u"Navigation"_s );
  }
  else
  {
    mLateralPanel->hide();
  }
  QgsSettings setting;
  setting.setValue( u"/3D/navigationWidget/visibility"_s, visibility, QgsSettings::Gui );
}

void Qgs3DMapCanvasWidget::toggleEditingToolbar( const bool visibility )
{
  mEditingToolBar->setVisible( visibility );
  QgsSettings setting;
  setting.setValue( u"/3D/editingToolbar/visibility"_s, visibility, QgsSettings::Gui );

  // toggle editing if not already editable
  if ( !visibility && mLayer && mLayer->isEditable() && !mLayer->isModified() )
    QgisApp::instance()->toggleEditing( mLayer );

  updateEditingToolBar();
}

void Qgs3DMapCanvasWidget::toggleFpsCounter( const bool visibility )
{
  mLabelFpsCounter->setVisible( visibility );
}

void Qgs3DMapCanvasWidget::toggleDebugWidget( const bool visibility ) const
{
  if ( visibility )
  {
    mLateralPanel->showWidget( u"Debug"_s );
  }
  else
  {
    mLateralPanel->hideWidget( u"Debug"_s );
  }
}

// this is used only for keyboard shortcut, you should supply the visibility value
void Qgs3DMapCanvasWidget::toggleDebugWidget() const
{
  Qgis::Map3DDebugFlags debugFlags = mCanvas->mapSettings()->debugFlags();
  debugFlags.setFlag( Qgis::Map3DDebugFlag::ShowDebugPanel, !debugFlags.testFlag( Qgis::Map3DDebugFlag::ShowDebugPanel ) );
  mCanvas->mapSettings()->setDebugFlags( debugFlags );
  toggleDebugWidget( debugFlags.testFlag( Qgis::Map3DDebugFlag::ShowDebugPanel ) );
}

void Qgs3DMapCanvasWidget::setMapSettings( Qgs3DMapSettings *map )
{
  updateCheckedActionsFromMapSettings( map );

  mCanvas->setMapSettings( map );
  connect( map, &Qgs3DMapSettings::showDebugPanelChanged, this, qOverload<bool>( &Qgs3DMapCanvasWidget::toggleDebugWidget ) );
  toggleDebugWidget( mCanvas->mapSettings()->debugFlags().testFlag( Qgis::Map3DDebugFlag::ShowDebugPanel ) );
  mDebugWidget->setMapSettings( map );

  connect( mCanvas->scene(), &Qgs3DMapScene::totalPendingJobsCountChanged, this, &Qgs3DMapCanvasWidget::onTotalPendingJobsCountChanged );
  connect( mCanvas->scene(), &Qgs3DMapScene::gpuMemoryLimitReached, this, &Qgs3DMapCanvasWidget::onGpuMemoryLimitReached );

  // Connect the camera to the debug widget.
  connect( mCanvas->cameraController(), &QgsCameraController::cameraChanged, mDebugWidget, &Qgs3DDebugWidget::updateFromCamera );
  // update the debug widget when the near/far planes have been updated by the map scene
  connect( mCanvas->cameraController()->camera(), &Qt3DRender::QCamera::nearPlaneChanged, mDebugWidget, &Qgs3DDebugWidget::updateFromCamera );
  connect( mCanvas->cameraController()->camera(), &Qt3DRender::QCamera::farPlaneChanged, mDebugWidget, &Qgs3DDebugWidget::updateFromCamera );

  mAnimationWidget->setCameraController( mCanvas->cameraController() );
  mAnimationWidget->setMap( map );

  // Disable button for switching the map theme if the terrain generator is a mesh, or if there is no terrain
  mActionMapThemes->setDisabled(
    !mCanvas->mapSettings()->terrainRenderingEnabled() || !mCanvas->mapSettings()->terrainGenerator() || mCanvas->mapSettings()->terrainGenerator()->type() == QgsTerrainGenerator::Mesh
  );
  mLabelFpsCounter->setVisible( mCanvas->mapSettings()->debugFlags().testFlag( Qgis::Map3DDebugFlag::ShowFPS ) );

  mMapToolClippingPlanes = std::make_unique<QgsMapToolClippingPlanes>( mMainCanvas, this );
  mMapToolClippingPlanes->setAction( mActionSetClippingPlanes );
  connect( mMapToolClippingPlanes.get(), &QgsMapToolClippingPlanes::finishedSuccessfully, this, &Qgs3DMapCanvasWidget::onCrossSectionToolFinished );
  lockCrossSectionTolerance( settingCrossSectionToleranceLocked->value() );

  // none of the actions in the Camera menu are supported by globe yet, so just hide it completely
  mActionCamera->setVisible( map->sceneMode() == Qgis::SceneMode::Local );

  connect( map, &Qgs3DMapSettings::viewFrustumVisualizationEnabledChanged, this, &Qgs3DMapCanvasWidget::onViewFrustumVisualizationEnabledChanged );
  connect( map, &Qgs3DMapSettings::extentChanged, this, &Qgs3DMapCanvasWidget::onExtentChanged );
  connect( map, &Qgs3DMapSettings::showExtentIn2DViewChanged, this, &Qgs3DMapCanvasWidget::onExtentChanged );
  onExtentChanged();
}

void Qgs3DMapCanvasWidget::setMainCanvas( QgsMapCanvas *canvas )
{
  mMainCanvas = canvas;

  mMapToolExtent = std::make_unique<QgsMapToolExtent>( canvas );
  mMapToolExtent->setAction( mActionSetSceneExtent );
  connect( mMapToolExtent.get(), &QgsMapToolExtent::extentChanged, this, &Qgs3DMapCanvasWidget::setSceneExtent );

  connect( mMainCanvas, &QgsMapCanvas::layersChanged, this, &Qgs3DMapCanvasWidget::onMainCanvasLayersChanged );
  connect( mMainCanvas, &QgsMapCanvas::canvasColorChanged, this, &Qgs3DMapCanvasWidget::onMainCanvasColorChanged );
  connect( mMainCanvas, &QgsMapCanvas::extentsChanged, this, &Qgs3DMapCanvasWidget::onMainMapCanvasExtentChanged );

  mCrossSectionRubberBand = make_qobject_unique<QgsRubberBand>( mMainCanvas, Qgis::GeometryType::Polygon );
  QColor polygonColor = QColorConstants::Red.lighter();
  polygonColor.setAlphaF( 0.5 );
  mCrossSectionRubberBand->setColor( polygonColor );

  if ( !mViewFrustumHighlight )
  {
    mViewFrustumHighlight = make_qobject_unique<QgsRubberBand>( canvas, Qgis::GeometryType::Polygon );
    mViewFrustumHighlight->setColor( QColor::fromRgba( qRgba( 0, 0, 255, 50 ) ) );
  }

  if ( !mViewExtentHighlight )
  {
    mViewExtentHighlight = make_qobject_unique<QgsRubberBand>( canvas, Qgis::GeometryType::Polygon );
    mViewExtentHighlight->setColor( QColor::fromRgba( qRgba( 255, 0, 0, 50 ) ) );
  }
}

void Qgs3DMapCanvasWidget::resetView()
{
  mCanvas->resetView();
}

void Qgs3DMapCanvasWidget::configureCamera()
{
  if ( mCameraControlsDialog )
  {
    mCameraControlsDialog->raise();
    return;
  }

  mCameraControlsDialog = new QDialog( this );
  mCameraControlsDialog->setAttribute( Qt::WA_DeleteOnClose );
  mCameraControlsDialog->setWindowTitle( tr( "Camera Controls" ) );
  mCameraControlsDialog->setObjectName( u"3DCameraControlsDialog"_s );
  mCameraControlsDialog->setMinimumSize( 300, 200 );
  QgsGui::enableAutoGeometryRestore( mCameraControlsDialog );

  Qgs3DCameraControlsWidget *w = new Qgs3DCameraControlsWidget( mCanvas, mCameraControlsDialog );
  connect( mCanvas->cameraController(), &QgsCameraController::cameraChanged, w, &Qgs3DCameraControlsWidget::updateFromCamera );

  QVBoxLayout *layout = new QVBoxLayout( mCameraControlsDialog );
  layout->addWidget( w );

  mCameraControlsDialog->show();
}

void Qgs3DMapCanvasWidget::configure()
{
  if ( mConfigureDialog )
  {
    mConfigureDialog->raise();
    return;
  }

  mConfigureDialog = new QDialog( this );
  mConfigureDialog->setAttribute( Qt::WA_DeleteOnClose );
  mConfigureDialog->setWindowTitle( tr( "3D Configuration" ) );
  mConfigureDialog->setObjectName( u"3DConfigurationDialog"_s );
  mConfigureDialog->setMinimumSize( 600, 460 );
  QgsGui::enableAutoGeometryRestore( mConfigureDialog );

  Qgs3DMapSettings *map = mCanvas->mapSettings();
  Qgs3DMapConfigWidget *w = new Qgs3DMapConfigWidget( map, mMainCanvas, mCanvas, mConfigureDialog );
  QDialogButtonBox *buttons = new QDialogButtonBox( QDialogButtonBox::Apply | QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, mConfigureDialog );

  auto applyConfig = [this, map, w]() {
    const QgsVector3D oldOrigin = map->origin();
    const QgsCoordinateReferenceSystem oldCrs = map->crs();
    const QgsCameraPose oldCameraPose = mCanvas->cameraController()->cameraPose();
    const QgsVector3D oldLookingAt = oldCameraPose.centerPoint();

    // update map
    w->apply();

    const QgsVector3D p = Qgs3DUtils::transformWorldCoordinates( oldLookingAt, oldOrigin, oldCrs, map->origin(), map->crs(), QgsProject::instance()->transformContext() );

    if ( p != oldLookingAt )
    {
      // apply() call has moved origin of the world so let's move camera so we look still at the same place
      QgsCameraPose newCameraPose = oldCameraPose;
      newCameraPose.setCenterPoint( p );
      mCanvas->cameraController()->setCameraPose( newCameraPose );
    }

    // Disable map theme button if the terrain generator is a mesh, or if there is no terrain
    mActionMapThemes->setDisabled( !mCanvas->mapSettings()->terrainRenderingEnabled() || !mCanvas->mapSettings()->terrainGenerator() || map->terrainGenerator()->type() == QgsTerrainGenerator::Mesh );
  };

  connect( buttons, &QDialogButtonBox::rejected, mConfigureDialog, &QDialog::reject );
  connect( buttons, &QDialogButtonBox::clicked, mConfigureDialog, [this, buttons, applyConfig, map]( QAbstractButton *button ) {
    if ( button == buttons->button( QDialogButtonBox::Apply ) || button == buttons->button( QDialogButtonBox::Ok ) )
      applyConfig();
    if ( button == buttons->button( QDialogButtonBox::Ok ) )
    {
      mConfigureDialog->accept();
      updateCheckedActionsFromMapSettings( map );
    }
  } );
  connect( buttons, &QDialogButtonBox::helpRequested, w, []() { QgsHelp::openHelp( u"map_views/3d_map_view.html#scene-configuration"_s ); } );

  connect( w, &Qgs3DMapConfigWidget::isValidChanged, this, [buttons]( bool valid ) {
    buttons->button( QDialogButtonBox::Apply )->setEnabled( valid );
    buttons->button( QDialogButtonBox::Ok )->setEnabled( valid );
  } );

  QVBoxLayout *layout = new QVBoxLayout( mConfigureDialog );
  layout->addWidget( w, 1 );
  layout->addWidget( buttons );

  mConfigureDialog->show();
}

void Qgs3DMapCanvasWidget::exportScene()
{
  QDialog dlg( this );
  dlg.setWindowTitle( tr( "Export 3D Scene" ) );
  dlg.setObjectName( u"3DSceneExportDialog"_s );
  QgsGui::enableAutoGeometryRestore( &dlg );

  Qgs3DMapExportSettings exportSettings;
  Qgs3DMapExportWidget exportWidget( mCanvas->scene(), &exportSettings );

  QDialogButtonBox *buttons = new QDialogButtonBox( QDialogButtonBox::Cancel | QDialogButtonBox::Help | QDialogButtonBox::Ok, &dlg );

  connect( buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept );
  connect( buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject );
  connect( buttons, &QDialogButtonBox::helpRequested, &dlg, [] { QgsHelp::openHelp( u"map_views/3d_map_view.html"_s ); } );

  QVBoxLayout *layout = new QVBoxLayout( &dlg );
  layout->addWidget( &exportWidget, 1 );
  layout->addWidget( buttons );
  if ( dlg.exec() )
  {
    const bool success = exportWidget.exportScene();
    const QString exportFileUri = exportSettings.exportFileUri();
    if ( success )
    {
      mMessageBar->pushMessage( tr( "Export 3D scene" ), tr( "Successfully exported scene to <a href=\"%1\">%2</a>" ).arg( exportFileUri, QDir::toNativeSeparators( exportFileUri ) ), Qgis::MessageLevel::Success, 0 );
    }
    else
    {
      mMessageBar->pushMessage( tr( "Export 3D scene" ), tr( "Unable to export scene to <a href=\"%1\">%2</a>" ).arg( exportFileUri, QDir::toNativeSeparators( exportFileUri ) ), Qgis::MessageLevel::Warning, 0 );
    }
  }
}

void Qgs3DMapCanvasWidget::onMainCanvasLayersChanged()
{
  QList<QgsMapLayer *> layers = mMainCanvas->layers( true );
  layers.insert( 0, QgsProject::instance()->mainAnnotationLayer() );

  mCanvas->mapSettings()->setLayers( layers );
}

void Qgs3DMapCanvasWidget::onMainCanvasColorChanged()
{
  mCanvas->mapSettings()->setBackgroundColor( mMainCanvas->canvasColor() );
}

void Qgs3DMapCanvasWidget::onTotalPendingJobsCountChanged()
{
  const int count = mCanvas->scene() ? mCanvas->scene()->totalPendingJobsCount() : 0;
  mProgressPendingJobs->setVisible( count );
  mLabelPendingJobs->setVisible( count );
  if ( count )
    mLabelPendingJobs->setText( tr( "Loading %n tile(s)", nullptr, count ) );
}

void Qgs3DMapCanvasWidget::updateFpsCount( float fpsCount )
{
  mLabelFpsCounter->setText( u"%1 fps"_s.arg( fpsCount, 10, 'f', 2, ' '_L1 ) );
}

void Qgs3DMapCanvasWidget::cameraNavigationSpeedChanged( double speed )
{
  mLabelNavigationSpeed->setText( u"Speed: %1 ×"_s.arg( QString::number( speed, 'f', 2 ) ) );
  mLabelNavigationSpeed->show();
  mLabelNavSpeedHideTimeout->start();
}

void Qgs3DMapCanvasWidget::mapThemeMenuAboutToShow()
{
  qDeleteAll( mMapThemeMenuPresetActions );
  mMapThemeMenuPresetActions.clear();

  const QString currentTheme = mCanvas->mapSettings()->terrainMapTheme();

  QAction *actionFollowMain = new QAction( tr( "(none)" ), mMapThemeMenu );
  actionFollowMain->setObjectName( u"m3DActionMapThemeNone"_s );
  actionFollowMain->setCheckable( true );
  if ( currentTheme.isEmpty() || !QgsProject::instance()->mapThemeCollection()->hasMapTheme( currentTheme ) )
  {
    actionFollowMain->setChecked( true );
  }
  connect( actionFollowMain, &QAction::triggered, this, [this] { mCanvas->mapSettings()->setTerrainMapTheme( QString() ); } );
  mMapThemeMenuPresetActions.append( actionFollowMain );

  const auto constMapThemes = QgsProject::instance()->mapThemeCollection()->mapThemes();
  for ( const QString &grpName : constMapThemes )
  {
    QAction *a = new QAction( grpName, mMapThemeMenu );
    a->setObjectName( u"m3DActionMapTheme_"_s + grpName );
    a->setCheckable( true );
    if ( grpName == currentTheme )
    {
      a->setChecked( true );
    }
    connect( a, &QAction::triggered, this, [a, this] { mCanvas->mapSettings()->setTerrainMapTheme( a->text() ); } );
    mMapThemeMenuPresetActions.append( a );
  }
  mMapThemeMenu->addActions( mMapThemeMenuPresetActions );
}

void Qgs3DMapCanvasWidget::currentMapThemeRenamed( const QString &theme, const QString &newTheme )
{
  if ( theme == mCanvas->mapSettings()->terrainMapTheme() )
  {
    mCanvas->mapSettings()->setTerrainMapTheme( newTheme );
  }
}

void Qgs3DMapCanvasWidget::onMainMapCanvasExtentChanged()
{
  if ( mCanvas->mapSettings()->viewSyncMode().testFlag( Qgis::ViewSyncModeFlag::Sync3DTo2D ) )
  {
    mCanvas->setViewFrom2DExtent( mMainCanvas->extent() );
  }
}

void Qgs3DMapCanvasWidget::onViewed2DExtentFrom3DChanged( QVector<QgsPointXY> extent )
{
  if ( mCanvas->mapSettings()->viewSyncMode().testFlag( Qgis::ViewSyncModeFlag::Sync2DTo3D ) )
  {
    QgsRectangle extentRect;
    extentRect.setNull();
    for ( QgsPointXY &pt : extent )
    {
      extentRect.include( pt );
    }
    if ( !extentRect.isEmpty() && extentRect.isFinite() && !extentRect.isNull() )
    {
      if ( mCanvas->mapSettings()->viewSyncMode().testFlag( Qgis::ViewSyncModeFlag::Sync3DTo2D ) )
      {
        whileBlocking( mMainCanvas )->setExtent( extentRect );
      }
      else
      {
        mMainCanvas->setExtent( extentRect );
      }
      mMainCanvas->refresh();
    }
  }

  onViewFrustumVisualizationEnabledChanged();
}

void Qgs3DMapCanvasWidget::onViewFrustumVisualizationEnabledChanged()
{
  mViewFrustumHighlight->reset( Qgis::GeometryType::Polygon );
  if ( mCanvas->mapSettings()->viewFrustumVisualizationEnabled() )
  {
    for ( QgsPointXY &pt : mCanvas->viewFrustum2DExtent() )
    {
      mViewFrustumHighlight->addPoint( pt, false );
    }
    mViewFrustumHighlight->closePoints();
  }
}

void Qgs3DMapCanvasWidget::onExtentChanged()
{
  Qgs3DMapSettings *mapSettings = mCanvas->mapSettings();
  mViewExtentHighlight->reset( Qgis::GeometryType::Polygon );
  if ( mapSettings->showExtentIn2DView() )
  {
    QgsRectangle extent = mapSettings->extent();
    mViewExtentHighlight->addPoint( QgsPointXY( extent.xMinimum(), extent.yMinimum() ), false );
    mViewExtentHighlight->addPoint( QgsPointXY( extent.xMinimum(), extent.yMaximum() ), false );
    mViewExtentHighlight->addPoint( QgsPointXY( extent.xMaximum(), extent.yMaximum() ), false );
    mViewExtentHighlight->addPoint( QgsPointXY( extent.xMaximum(), extent.yMinimum() ), false );
    mViewExtentHighlight->closePoints();
  }
}

void Qgs3DMapCanvasWidget::onGpuMemoryLimitReached()
{
  // let's report this issue just once, rather than spamming user if this happens repeatedly
  if ( mGpuMemoryLimitReachedReported )
    return;

  const QgsSettings settings;
  double memLimit = settings.value( u"map3d/gpuMemoryLimit"_s, 500.0, QgsSettings::App ).toDouble();
  mMessageBar->pushMessage(
    tr(
      "A map layer has used all graphics memory allowed (%1 MB). "
      "You may want to lower the amount of detail in the scene, or increase the limit in the options."
    )
      .arg( memLimit ),
    Qgis::MessageLevel::Warning
  );
  mGpuMemoryLimitReachedReported = true;
}

void Qgs3DMapCanvasWidget::setSceneExtentOn2DCanvas()
{
  if ( !qobject_cast<QgsMapToolExtent *>( mMainCanvas->mapTool() ) )
    mMapToolPrevious = mMainCanvas->mapTool();

  mMainCanvas->setMapTool( mMapToolExtent.get() );
  QgisApp::instance()->activateWindow();
  QgisApp::instance()->raise();
  mMessageBar->pushInfo( QString(), tr( "Drag a rectangle on the main 2D map view to define this 3D scene's extent" ) );
}

void Qgs3DMapCanvasWidget::setSceneExtent( const QgsRectangle &extent )
{
  this->activateWindow();
  this->raise();
  mMessageBar->clearWidgets();
  if ( !extent.isEmpty() )
    mCanvas->mapSettings()->setExtent( extent );

  if ( !mapCanvas3D()->scene()->clipPlaneEquations().isEmpty() )
  {
    if ( !mMapToolClippingPlanes->clippedPolygon().intersects( extent ) )
    {
      disableCrossSection();
      mMessageBar->pushInfo( QString(), tr( "Cross-section has been disabled, because it is outside the current extent" ) );
    }
  }

  if ( mMapToolPrevious )
    mMainCanvas->setMapTool( mMapToolPrevious );
  else
    mMainCanvas->unsetMapTool( mMapToolExtent.get() );
}

void Qgs3DMapCanvasWidget::setClippingPlanesOn2DCanvas()
{
  if ( !qobject_cast<QgsMapToolClippingPlanes *>( mMainCanvas->mapTool() ) )
    mMapToolPrevious = mMainCanvas->mapTool();

  mMainCanvas->setMapTool( mMapToolClippingPlanes.get() );
  QgisApp::instance()->activateWindow();
  QgisApp::instance()->raise();

  if ( !mClippingToleranceAction->isLocked() )
    mMessageBar->pushInfo( QString(), tr( "Select a rectangle using 3 points on the main 2D map view to define the cross-section of this 3D scene" ) );
}

void Qgs3DMapCanvasWidget::onCrossSectionToolFinished()
{
  this->activateWindow();
  this->raise();
  mMessageBar->clearWidgets();

  if ( mMapToolPrevious )
    mMainCanvas->setMapTool( mMapToolPrevious );
  else
    mMainCanvas->unsetMapTool( mMapToolClippingPlanes.get() );

  mMapToolClippingPlanes->clear();

  mActionNudgeLeft->setEnabled( true );
  mActionNudgeRight->setEnabled( true );

  QgsCrossSection cs = mCanvas->crossSection();
  if ( cs.startPoint() != cs.endPoint() )
  {
    QgsCoordinateTransform ct( mCanvas->mapSettings()->crs(), mMainCanvas->mapSettings().destinationCrs(), mMainCanvas->mapSettings().transformContext() );

    if ( cs.halfWidth() <= 0.0 )
    {
      cs.setHalfWidth( mClippingToleranceAction->toleranceSpinBox()->value() );
    }

    mCanvas->setCrossSection( cs );
    mCrossSectionRubberBand->setToGeometry( cs.asGeometry( &ct ) );
    mCrossSectionRubberBand->show();
    mCanvas->cameraController()->setCrossSectionSideView( cs );
  }
}

void Qgs3DMapCanvasWidget::disableCrossSection()
{
  mMapToolClippingPlanes->clear();
  mCrossSectionRubberBand->reset( Qgis::GeometryType::Polygon );
  mCanvas->setCrossSection( QgsCrossSection() );

  mActionNudgeLeft->setEnabled( false );
  mActionNudgeRight->setEnabled( false );
}

void Qgs3DMapCanvasWidget::nudgeLeft()
{
  nudgeCurve( Qgis::BufferSide::Left );
}

void Qgs3DMapCanvasWidget::nudgeRight()
{
  nudgeCurve( Qgis::BufferSide::Right );
}

void Qgs3DMapCanvasWidget::nudgeCurve( Qgis::BufferSide side )
{
  QgsCrossSection crossSection = mCanvas->crossSection();
  double distance = crossSection.halfWidth() * 2;

  const QgsPoint previousStartPoint = crossSection.startPoint();

  if ( side == Qgis::BufferSide::Left )
    crossSection.nudgeLeft( distance );
  else
    crossSection.nudgeRight( distance );

  mCanvas->setCrossSection( crossSection );

  const QgsVector cameraOffset = crossSection.startPoint() - previousStartPoint;
  QgsCoordinateTransform ct( mCanvas->mapSettings()->crs(), mMainCanvas->mapSettings().destinationCrs(), mMainCanvas->mapSettings().transformContext() );

  mCrossSectionRubberBand->setToGeometry( crossSection.asGeometry( &ct ) );

  mCanvas->scene()->cameraController()->moveCenterPoint( QVector3D( static_cast<float>( cameraOffset.x() ), static_cast<float>( cameraOffset.y() ), 0 ) );
}

void Qgs3DMapCanvasWidget::updateClippingRubberBand()
{
  QgsCrossSection crossSection = mCanvas->crossSection();

  const double distance = mClippingToleranceAction->toleranceSpinBox()->value();
  crossSection.setHalfWidth( distance );

  QgsCoordinateTransform ct( mCanvas->mapSettings()->crs(), mMainCanvas->mapSettings().destinationCrs(), mMainCanvas->mapSettings().transformContext() );
  mCrossSectionRubberBand->setToGeometry( crossSection.asGeometry( &ct ) );

  mCanvas->setCrossSection( crossSection );
}

void Qgs3DMapCanvasWidget::lockCrossSectionTolerance( bool enabled )
{
  mMapToolClippingPlanes->setToleranceLocked( enabled );
}

void Qgs3DMapCanvasWidget::updateCheckedActionsFromMapSettings( const Qgs3DMapSettings *mapSettings ) const
{
  whileBlocking( mActionEnableShadows )->setChecked( mapSettings->shadowSettings().renderShadows() );
  whileBlocking( mActionEnableEyeDome )->setChecked( mapSettings->eyeDomeLightingEnabled() );
  whileBlocking( mActionEnableAmbientOcclusion )->setChecked( mapSettings->ambientOcclusionSettings().isEnabled() );
  whileBlocking( mActionEnableBloom )->setChecked( mapSettings->bloomSettings().isEnabled() );
  whileBlocking( mActionSync2DNavTo3D )->setChecked( mapSettings->viewSyncMode().testFlag( Qgis::ViewSyncModeFlag::Sync2DTo3D ) );
  whileBlocking( mActionSync3DNavTo2D )->setChecked( mapSettings->viewSyncMode().testFlag( Qgis::ViewSyncModeFlag::Sync3DTo2D ) );
  whileBlocking( mShowFrustumPolygon )->setChecked( mapSettings->viewFrustumVisualizationEnabled() );
  whileBlocking( mActionShow2DMapOverlay )->setChecked( mapSettings->is2DMapOverlayEnabled() );
}

void Qgs3DMapCanvasWidget::updateProfileCursorPosition( QgsElevationProfile *profile, const QgsPointXY &mapPoint, const QgsProfilePoint &profilePoint )
{
  if ( !mCanvas || !mElevationProfileData.contains( profile ) )
    return;

  ElevationProfileData &data = mElevationProfileData[profile];

  QgsCurve *curve = profile->profileCurve();
  if ( mapPoint.isEmpty() || profilePoint.isEmpty() || !curve )
  {
    data.cursorLineRubberBand.reset();
    data.cursorPolygonRubberBand.reset();
    return;
  }

  if ( !data.cursorLineRubberBand )
  {
    data.cursorLineRubberBand = std::make_unique<QgsRubberBand3D>( mCanvas->scene(), Qgis::GeometryType::Line );
    data.cursorLineRubberBand->setColor( QColor( 0, 0, 0, 200 ) );
    data.cursorLineRubberBand->setWidth( 3 );
    data.cursorLineRubberBand->setMarkersEnabled( false );
  }

  QgsGeometry cursorGeom( new QgsLineString(
    QVector<QgsPoint> {
      QgsPoint( mapPoint.x(), mapPoint.y(), data.zMin ),
      QgsPoint( mapPoint.x(), mapPoint.y(), data.zMax ),
    }
  ) );
  data.cursorLineRubberBand->setGeometry( cursorGeom );

  // we need to properly rotate the curve depending on where it is
  const double curveLength = curve->length();
  const double offset = std::min( 0.1, curveLength * 0.001 );
  const double profilePointDistance = profilePoint.distance();

  const double d0 = std::max( 0.0, profilePointDistance - offset );
  const double d1 = std::min( curveLength, profilePointDistance + offset );

  std::unique_ptr<QgsPoint> p0( curve->interpolatePoint( d0 ) );
  std::unique_ptr<QgsPoint> p1( curve->interpolatePoint( d1 ) );

  if ( !p0 || !p1 )
  {
    data.cursorLineRubberBand.reset();
    data.cursorPolygonRubberBand.reset();
    return;
  }

  double dx = p1->x() - p0->x();
  double dy = p1->y() - p0->y();

  const double length = std::sqrt( dx * dx + dy * dy );

  dx /= length;
  dy /= length;

  const double tolerance = profile->tolerance();
  const double nX = -dy * tolerance;
  const double nY = dx * tolerance;

  const double x = mapPoint.x();
  const double y = mapPoint.y();

  QgsGeometry polyGeom( new QgsPolygon( new QgsLineString(
    QVector<QgsPoint> {
      QgsPoint( x + nX, y + nY, data.zMin ),
      QgsPoint( x - nX, y - nY, data.zMin ),
      QgsPoint( x - nX, y - nY, data.zMax ),
      QgsPoint( x + nX, y + nY, data.zMax ),
      QgsPoint( x + nX, y + nY, data.zMin ),
    }
  ) ) );

  if ( !data.cursorPolygonRubberBand )
  {
    data.cursorPolygonRubberBand = std::make_unique<QgsRubberBand3D>( mCanvas->scene(), Qgis::GeometryType::Polygon );
    data.cursorPolygonRubberBand->setColor( QColor( 50, 50, 50, 100 ) );
    data.cursorPolygonRubberBand->setWidth( 0 );
    data.cursorPolygonRubberBand->setMarkersEnabled( false );
  }

  data.cursorPolygonRubberBand->setGeometry( polyGeom );
}

void Qgs3DMapCanvasWidget::setProfileData( QgsElevationProfile *profile, double zMin, double zMax )
{
  // only when the generation of the profile finishes, we get zMin and zMax
  // however, rasters and point clouds trigger profile generation on every elevation profile canvas move/zoom
  // so Z values of the profile curve in 3D can fluctuate when dealing with those formats and zooming/moving in elevation canvas
  if ( !profile )
    return;

  if ( mElevationProfileData.contains( profile ) )
  {
    ElevationProfileData &data = mElevationProfileData[profile];
    data.zMin = zMin;
    data.zMax = zMax;
    updateProfileRubberBands( profile );
    return;
  }

  ElevationProfileData profileData;
  profileData.zMin = zMin;
  profileData.zMax = zMax;
  mElevationProfileData[profile] = std::move( profileData );

  connect( profile, &QObject::destroyed, this, [this, profile] { mElevationProfileData.erase( profile ); } );

  updateProfileRubberBands( profile );
}

void Qgs3DMapCanvasWidget::removeProfileData( QgsElevationProfile *profile )
{
  hideProfileRubberBands( profile );
  mElevationProfileData.erase( profile );
}

void Qgs3DMapCanvasWidget::hideProfileRubberBands( QgsElevationProfile *profile )
{
  if ( mElevationProfileData.contains( profile ) )
  {
    ElevationProfileData &data = mElevationProfileData[profile];
    data.rubberBandZMin.reset();
    data.rubberBandZMax.reset();
    data.rubberBandSideLines.clear();
  }
}

void Qgs3DMapCanvasWidget::updateProfileRubberBands( QgsElevationProfile *profile )
{
  if ( !mCanvas || !mElevationProfileData.contains( profile ) )
    return;

  ElevationProfileData &data = mElevationProfileData[profile];

  QgsCurve *curve = profile->profileCurve();
  if ( !curve )
  {
    data.rubberBandZMin.reset();
    data.rubberBandZMax.reset();
    data.rubberBandSideLines.clear();
    return;
  }

  const double tolerance = profile->tolerance();
  QgsGeometry curveGeom( curve->clone() );

  QgsGeometry rubberBandGeom;
  Qgis::GeometryType geomType;
  if ( tolerance > 0 )
  {
    rubberBandGeom = curveGeom.buffer( tolerance, 8, Qgis::EndCapStyle::Flat, Qgis::JoinStyle::Round, 2 );
    geomType = Qgis::GeometryType::Polygon;
  }
  else
  {
    rubberBandGeom = QgsGeometry( curve->curveToLine() );
    geomType = Qgis::GeometryType::Line;
  }

  if ( data.geomType != geomType )
  {
    data.rubberBandZMin.reset();
    data.rubberBandZMax.reset();
    data.rubberBandSideLines.clear();
    data.geomType = geomType;
  }

  if ( !data.rubberBandZMin )
  {
    data.rubberBandZMin = std::make_unique<QgsRubberBand3D>( mCanvas->scene(), geomType );
    data.rubberBandZMin->setColor( QColor( 200, 200, 200, 200 ) );
    data.rubberBandZMin->setOutlineColor( QColor( 200, 200, 200, 200 ) );
    data.rubberBandZMin->setWidth( 3 );
    data.rubberBandZMin->setMarkersEnabled( false );
    data.rubberBandZMin->setFillEnabled( false );
  }

  if ( !data.rubberBandZMax )
  {
    data.rubberBandZMax = std::make_unique<QgsRubberBand3D>( mCanvas->scene(), geomType );
    data.rubberBandZMax->setColor( QColor( 200, 200, 200, 200 ) );
    data.rubberBandZMax->setOutlineColor( QColor( 200, 200, 200, 200 ) );
    data.rubberBandZMax->setWidth( 3 );
    data.rubberBandZMax->setMarkersEnabled( false );
    data.rubberBandZMax->setFillEnabled( false );
  }

  QgsGeometry geomZMin = rubberBandGeom;
  QgsGeometry geomZMax = rubberBandGeom;

  QgsAbstractGeometry *gZMin = geomZMin.get();
  gZMin->addZValue( data.zMin );

  QgsAbstractGeometry *gZMax = geomZMax.get();
  gZMax->addZValue( data.zMax );

  data.rubberBandZMin->setGeometry( geomZMin );
  data.rubberBandZMax->setGeometry( geomZMax );

  QVector<QgsPointXY> sidePoints; // used to construct vertical lines at the sides of the bottom/top polygons
  if ( tolerance > 0 )
  {
    const int numCurvePoints = curve->numPoints();

    // vertical lines at the beginning
    QgsPoint pt1 = curve->vertexAt( QgsVertexId( 0, 0, 0 ) );
    QgsPoint pt2 = curve->vertexAt( QgsVertexId( 0, 0, 1 ) );

    double x, y;
    QgsGeometryUtilsBase::perpendicularOffsetPointAlongSegment( pt1.x(), pt1.y(), pt2.x(), pt2.y(), 0.0, tolerance, &x, &y );
    sidePoints.append( QgsPointXY( x, y ) );

    QgsGeometryUtilsBase::perpendicularOffsetPointAlongSegment( pt1.x(), pt1.y(), pt2.x(), pt2.y(), 0.0, -tolerance, &x, &y );
    sidePoints.append( QgsPointXY( x, y ) );

    // we want only one vertical line at the arc middle point (where the curve bends, if the curve is made out of multiple segments)
    for ( int i = 1; i < numCurvePoints - 1; i++ )
    {
      const QgsPoint vertexPrevious = curve->vertexAt( QgsVertexId( 0, 0, i - 1 ) );
      const QgsPoint vertexMiddle = curve->vertexAt( QgsVertexId( 0, 0, i ) );
      const QgsPoint vertexNext = curve->vertexAt( QgsVertexId( 0, 0, i + 1 ) );

      if ( vertexMiddle == vertexPrevious || vertexMiddle == vertexNext )
        continue;

      const double averageAngle = QgsGeometryUtilsBase::averageAngle( vertexPrevious.x(), vertexPrevious.y(), vertexMiddle.x(), vertexMiddle.y(), vertexNext.x(), vertexNext.y() ) - M_PI_2;
      const double dx = std::sin( averageAngle );
      const double dy = std::cos( averageAngle );

      const double angleBetweenPoints = QgsGeometryUtilsBase::angleBetweenThreePoints( vertexPrevious.x(), vertexPrevious.y(), vertexMiddle.x(), vertexMiddle.y(), vertexNext.x(), vertexNext.y() );
      const double turnAngle = QgsGeometryUtilsBase::normalizedAngle( angleBetweenPoints - M_PI );
      const double angleHalf = turnAngle < M_PI ? turnAngle * 0.5 : ( 2.0 * M_PI - turnAngle ) * 0.5;

      const double cosAngleHalf = std::cos( angleHalf );
      const double miter = std::abs( cosAngleHalf ) > 1e-10 ? tolerance / cosAngleHalf : tolerance;

      const double left = turnAngle < M_PI ? tolerance : miter;
      const double right = turnAngle < M_PI ? miter : tolerance;

      sidePoints.append( QgsPointXY( vertexMiddle.x() + dx * left, vertexMiddle.y() + dy * left ) );
      sidePoints.append( QgsPointXY( vertexMiddle.x() - dx * right, vertexMiddle.y() - dy * right ) );
    }

    // vertical lines at the end
    pt1 = curve->vertexAt( QgsVertexId( 0, 0, numCurvePoints - 2 ) );
    pt2 = curve->vertexAt( QgsVertexId( 0, 0, numCurvePoints - 1 ) );

    QgsGeometryUtilsBase::perpendicularOffsetPointAlongSegment( pt1.x(), pt1.y(), pt2.x(), pt2.y(), 1.0, tolerance, &x, &y );
    sidePoints.append( QgsPointXY( x, y ) );

    QgsGeometryUtilsBase::perpendicularOffsetPointAlongSegment( pt1.x(), pt1.y(), pt2.x(), pt2.y(), 1.0, -tolerance, &x, &y );
    sidePoints.append( QgsPointXY( x, y ) );
  }
  else // not a polygon, so we just construct vertical lines at each vertex of the curve
  {
    for ( auto vertex = rubberBandGeom.vertices_begin(); vertex != rubberBandGeom.vertices_end(); vertex++ )
    {
      sidePoints.append( QgsPointXY( ( *vertex ).x(), ( *vertex ).y() ) );
    }
  }

  data.rubberBandSideLines.resize( sidePoints.size() );

  // construct vertical lines from zMin to zMax
  for ( qsizetype i = 0; i < sidePoints.size(); i++ )
  {
    const QgsPointXY &point = sidePoints[i];

    QgsGeometry lineGeom( new QgsLineString(
      QVector<QgsPoint> {
        QgsPoint( point.x(), point.y(), data.zMin ),
        QgsPoint( point.x(), point.y(), data.zMax ),
      }
    ) );

    if ( !data.rubberBandSideLines[i] )
    {
      data.rubberBandSideLines[i] = std::make_unique<QgsRubberBand3D>( mCanvas->scene(), Qgis::GeometryType::Line );
      data.rubberBandSideLines[i]->setColor( QColor( 200, 200, 200, 150 ) );
      data.rubberBandSideLines[i]->setWidth( 3 );
      data.rubberBandSideLines[i]->setMarkersEnabled( false );
    }

    data.rubberBandSideLines[i]->setGeometry( lineGeom );
  }
}

//
// Qgs3DMapClippingToleranceWidgetSettingsAction
//

Qgs3DMapClippingToleranceWidgetSettingsAction::Qgs3DMapClippingToleranceWidgetSettingsAction( QWidget *parent )
  : QWidgetAction( parent )
{
  QGridLayout *gLayout = new QGridLayout();
  gLayout->setContentsMargins( 3, 2, 3, 2 );

  mToleranceWidget = new QgsDoubleSpinBox();
  mToleranceWidget->setClearValue( Qgs3DMapCanvasWidget::settingClippingTolerance->defaultValue() );
  mToleranceWidget->setValue( Qgs3DMapCanvasWidget::settingClippingTolerance->value() );
  mToleranceWidget->setKeyboardTracking( false );
  mToleranceWidget->setMaximumWidth( QFontMetrics( mToleranceWidget->font() ).horizontalAdvance( '0' ) * 50 );
  mToleranceWidget->setDecimals( 2 );
  mToleranceWidget->setRange( 0, 9999999999 );
  mToleranceWidget->setSingleStep( 1.0 );

  QLabel *label = new QLabel( tr( "Tolerance" ) );

  mLockButton = new QToolButton();
  mLockButton->setEnabled( true );
  mLockButton->setCheckable( true );
  mLockButton->setAutoRaise( true );
  mLockButton->setToolButtonStyle( Qt::ToolButtonIconOnly );

  auto refreshLockButton = [this]( bool locked ) {
    mLockButton->setIcon( QIcon( QgsApplication::iconPath( locked ? u"locked.svg"_s : u"unlocked.svg"_s ) ) );
    mToleranceWidget->setEnabled( locked );
    mLockButton->setToolTip(
      locked ? tr( "Locked: spinbox enabled, cross section width from tolerance.\nClick to unlock for width from 3rd point." )
             : tr( "Unlocked: spinbox disabled, cross section width from 3rd point.\nClick to lock for tolerance width." )
    );
  };

  const bool isLocked = !Qgs3DMapCanvasWidget::settingCrossSectionToleranceLocked->value();

  mLockButton->setChecked( isLocked );
  refreshLockButton( isLocked );

  QObject::connect( mLockButton, &QToolButton::toggled, this, [this, refreshLockButton]( bool locked ) {
    refreshLockButton( locked );
    emit lockStateChanged( locked );
  } );

  gLayout->addWidget( label, 0, 0 );
  gLayout->addWidget( mToleranceWidget, 0, 1 );
  gLayout->addWidget( mLockButton, 0, 2 );

  QWidget *w = new QWidget();
  w->setLayout( gLayout );
  setDefaultWidget( w );
}

ClassValidator::ClassValidator( QWidget *parent )
  : QValidator( parent )
{
  mRx = QRegularExpression( u"([0-9]{1,3})"_s );
}

QValidator::State ClassValidator::validate( QString &input, int &pos ) const
{
  QRegularExpressionMatch match = mRx.match( input );
  const QString number = match.captured();
  bool ok;
  const int n = number.toInt( &ok );

  if ( !ok && pos == 0 )
  {
    input.clear();
    return QValidator::State::Intermediate;
  }

  if ( !ok )
    return QValidator::State::Invalid;
  if ( n < 0 || n > 255 )
    return QValidator::State::Invalid;
  if ( mClasses.contains( n ) )
  {
    input = u"%1 (%2)"_s.arg( n ).arg( mClasses[n] );
    if ( pos > number.size() )
      pos = number.size();
    return QValidator::State::Acceptable;
  }
  return QValidator::State::Intermediate;
}

void ClassValidator::fixup( QString &input ) const
{
  QRegularExpressionMatch match = mRx.match( input );
  const QString number = match.captured();
  bool ok;
  const int n = number.toInt( &ok );
  input = u"%1 (%2)"_s.arg( n ).arg( mClasses[n] );
}

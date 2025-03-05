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
#include "moc_qgs3dmapcanvaswidget.cpp"

#include <QProgressBar>
#include <QToolBar>
#include <QAction>
#include <QShortcut>
#include <QWidget>
#include <QActionGroup>

#include "qgisapp.h"
#include "qgs3dmapcanvas.h"
#include "qgs3dmapconfigwidget.h"
#include "qgs3dmapscene.h"
#include "qgscameracontroller.h"
#include "qgshelp.h"
#include "qgsidentifyresultsdialog.h"
#include "qgsmapcanvas.h"
#include "qgsmaptoolextent.h"
#include "qgsmaptoolidentifyaction.h"
#include "qgsmessagebar.h"
#include "qgsapplication.h"
#include "qgssettings.h"
#include "qgsgui.h"
#include "qgsmapthemecollection.h"
#include "qgsshortcutsmanager.h"

#include "qgs3danimationsettings.h"
#include "qgs3danimationwidget.h"
#include "qgs3dmapsettings.h"
#include "qgs3dmaptoolidentify.h"
#include "qgs3dmaptoolmeasureline.h"
#include "qgs3dmaptoolpointcloudchangeattribute.h"
#include "qgs3dnavigationwidget.h"
#include "qgs3ddebugwidget.h"
#include "qgs3dutils.h"

#include "qgsmap3dexportwidget.h"
#include "qgs3dmapexportsettings.h"
#include "qgs3dmaptoolpointcloudchangeattributepaintbrush.h"
#include "qgs3dmaptoolpointcloudchangeattributepolygon.h"

#include "qgsdockablewidgethelper.h"
#include "qgsrubberband.h"
#include "qgspointcloudlayer.h"
#include "qgspointcloudlayer3drenderer.h"

Qgs3DMapCanvasWidget::Qgs3DMapCanvasWidget( const QString &name, bool isDocked )
  : QWidget( nullptr )
  , mCanvasName( name )
{
  const QgsSettings setting;

  mToolbarMenu = new QMenu( tr( "Toolbars" ), this );
  mToolbarMenu->setObjectName( QStringLiteral( "mToolbarMenu" ) );

  QToolBar *toolBar = new QToolBar( this );
  toolBar->setIconSize( QgisApp::instance()->iconSize( isDocked ) );

  QAction *actionCameraControl = toolBar->addAction( QIcon( QgsApplication::iconPath( "mActionPan.svg" ) ), tr( "Camera Control" ), this, &Qgs3DMapCanvasWidget::cameraControl );
  actionCameraControl->setCheckable( true );

  toolBar->addAction( QgsApplication::getThemeIcon( QStringLiteral( "mActionZoomFullExtent.svg" ) ), tr( "Zoom Full" ), this, &Qgs3DMapCanvasWidget::resetView );

  // Editing toolbar
  mEditingToolBar = new QToolBar( this );
  mEditingToolBar->setWindowTitle( tr( "Editing Toolbar" ) );
  mEditingToolBar->setVisible( false );
  mEditingToolsMenu = new QMenu( this );

  mPointCloudEditingToolbar = new QToolBar( this );

  mActionToggleEditing = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionToggleEditing.svg" ) ), tr( "Toggle editing" ), this );
  mActionToggleEditing->setCheckable( true );
  connect( mActionToggleEditing, &QAction::triggered, this, [] { QgisApp::instance()->toggleEditing( QgisApp::instance()->activeLayer() ); } );
  mActionUndo = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionUndo.svg" ) ), tr( "Undo" ), this );
  mActionRedo = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionRedo.svg" ) ), tr( "Redo" ), this );

  mEditingToolBar->addAction( mActionToggleEditing );
  mEditingToolBar->addAction( mActionUndo );
  mEditingToolBar->addAction( mActionRedo );
  mEditingToolBar->addSeparator();

  mEditingToolsAction = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "mActionSelectPolygon.svg" ) ), tr( "Select Editing Tool" ), this );
  mEditingToolsAction->setMenu( mEditingToolsMenu );
  mEditingToolBar->addAction( mEditingToolsAction );
  QToolButton *editingToolsButton = qobject_cast<QToolButton *>( mEditingToolBar->widgetForAction( mEditingToolsAction ) );
  editingToolsButton->setPopupMode( QToolButton::ToolButtonPopupMode::InstantPopup );
  QAction *actionPointCloudChangeAttributeTool = mEditingToolsMenu->addAction( QIcon( QgsApplication::iconPath( QStringLiteral( "mActionSelectPolygon.svg" ) ) ), tr( "Select by Polygon" ), this, &Qgs3DMapCanvasWidget::changePointCloudAttributeByPolygon );
  QAction *actionPaintbrush = mEditingToolsMenu->addAction( QIcon( QgsApplication::iconPath( QStringLiteral( "propertyicons/rendering.svg" ) ) ), tr( "Select by Paintbrush" ), this, &Qgs3DMapCanvasWidget::changePointCloudAttributeByPaintbrush );

  mEditingToolBar->addWidget( mPointCloudEditingToolbar );
  mPointCloudEditingToolbar->addWidget( new QLabel( tr( "Attribute" ) ) );
  mCboChangeAttribute = new QComboBox();
  mPointCloudEditingToolbar->addWidget( mCboChangeAttribute );
  mSpinChangeAttributeValue = new QgsDoubleSpinBox();
  mSpinChangeAttributeValue->setShowClearButton( false );
  mPointCloudEditingToolbar->addWidget( new QLabel( tr( "Value" ) ) );
  mSpinChangeAttributeValueAction = mPointCloudEditingToolbar->addWidget( mSpinChangeAttributeValue );
  mSpinChangeAttributeValueAction->setVisible( false );
  mCboChangeAttributeValue = new QComboBox();
  mCboChangeAttributeValue->setEditable( true );
  mClassValidator = new ClassValidator( this );
  mCboChangeAttributeValueAction = mPointCloudEditingToolbar->addWidget( mCboChangeAttributeValue );

  QAction *actionEditingToolbar = toolBar->addAction( QIcon( QgsApplication::iconPath( "mIconPointCloudLayer.svg" ) ), tr( "Show Editing Toolbar" ), this, [this] { mEditingToolBar->setVisible( !mEditingToolBar->isVisible() ); } );
  actionEditingToolbar->setCheckable( true );
  connect( mCboChangeAttribute, qOverload<int>( &QComboBox::currentIndexChanged ), this, [this]( int ) { onPointCloudChangeAttributeSettingsChanged(); } );
  connect( mCboChangeAttributeValue, qOverload<const QString &>( &QComboBox::currentTextChanged ), this, [this]( const QString &text ) {
    double newValue = 0;
    if ( mCboChangeAttributeValue->isEditable() )
    {
      const QStringList split = text.split( ' ' );
      if ( !split.isEmpty() )
      {
        newValue = split.constFirst().toDouble();
      }
    }
    else
    {
      newValue = mCboChangeAttributeValue->currentData().toDouble();
    }
    mMapToolChangeAttribute->setNewValue( newValue );
  } );
  connect( mSpinChangeAttributeValue, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, [this]( double ) { mMapToolChangeAttribute->setNewValue( mSpinChangeAttributeValue->value() ); } );

  QAction *toggleOnScreenNavigation = toolBar->addAction(
    QgsApplication::getThemeIcon( QStringLiteral( "mAction3DNavigation.svg" ) ),
    tr( "Toggle On-Screen Navigation" )
  );

  toggleOnScreenNavigation->setCheckable( true );
  toggleOnScreenNavigation->setChecked(
    setting.value( QStringLiteral( "/3D/navigationWidget/visibility" ), true, QgsSettings::Gui ).toBool()
  );
  QObject::connect( toggleOnScreenNavigation, &QAction::toggled, this, &Qgs3DMapCanvasWidget::toggleNavigationWidget );

  toolBar->addSeparator();

  QAction *actionIdentify = toolBar->addAction( QIcon( QgsApplication::iconPath( "mActionIdentify.svg" ) ), tr( "Identify" ), this, &Qgs3DMapCanvasWidget::identify );
  actionIdentify->setCheckable( true );

  QAction *actionMeasurementTool = toolBar->addAction( QIcon( QgsApplication::iconPath( "mActionMeasure.svg" ) ), tr( "Measurement Line" ), this, &Qgs3DMapCanvasWidget::measureLine );
  actionMeasurementTool->setCheckable( true );

  // Create action group to make the action exclusive
  QActionGroup *actionGroup = new QActionGroup( this );
  actionGroup->addAction( actionCameraControl );
  actionGroup->addAction( actionIdentify );
  actionGroup->addAction( actionMeasurementTool );
  actionGroup->addAction( actionPaintbrush );
  actionGroup->addAction( actionPointCloudChangeAttributeTool );
  actionGroup->setExclusive( true );

  mActionAnim = toolBar->addAction( QIcon( QgsApplication::iconPath( "mTaskRunning.svg" ) ), tr( "Animations" ), this, &Qgs3DMapCanvasWidget::toggleAnimations );
  mActionAnim->setCheckable( true );

  // Export Menu
  mExportMenu = new QMenu( this );

  mActionExport = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "mActionSharingExport.svg" ) ), tr( "Export" ), this );
  mActionExport->setMenu( mExportMenu );
  toolBar->addAction( mActionExport );
  QToolButton *exportButton = qobject_cast<QToolButton *>( toolBar->widgetForAction( mActionExport ) );
  exportButton->setPopupMode( QToolButton::ToolButtonPopupMode::InstantPopup );

  mExportMenu->addAction( QgsApplication::getThemeIcon( QStringLiteral( "mActionSaveMapAsImage.svg" ) ), tr( "Save as Image…" ), this, &Qgs3DMapCanvasWidget::saveAsImage );

  mExportMenu->addAction( QgsApplication::getThemeIcon( QStringLiteral( "3d.svg" ) ), tr( "Export 3D Scene" ), this, &Qgs3DMapCanvasWidget::exportScene );

  toolBar->addSeparator();

  // Map Theme Menu
  mMapThemeMenu = new QMenu( this );
  connect( mMapThemeMenu, &QMenu::aboutToShow, this, &Qgs3DMapCanvasWidget::mapThemeMenuAboutToShow );
  connect( QgsProject::instance()->mapThemeCollection(), &QgsMapThemeCollection::mapThemeRenamed, this, &Qgs3DMapCanvasWidget::currentMapThemeRenamed );

  mActionMapThemes = new QAction( tr( "Set View Theme" ), this );
  mActionMapThemes->setMenu( mMapThemeMenu );
  mActionMapThemes->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionShowAllLayers.svg" ) ) );
  toolBar->addAction( mActionMapThemes );
  QToolButton *mapThemesButton = qobject_cast<QToolButton *>( toolBar->widgetForAction( mActionMapThemes ) );
  mapThemesButton->setPopupMode( QToolButton::ToolButtonPopupMode::InstantPopup );


  toolBar->addSeparator();

  // Camera Menu
  mCameraMenu = new QMenu( this );

  mActionCamera = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "mIconCamera.svg" ) ), tr( "Camera" ), this );
  mActionCamera->setMenu( mCameraMenu );
  toolBar->addAction( mActionCamera );
  QToolButton *cameraButton = qobject_cast<QToolButton *>( toolBar->widgetForAction( mActionCamera ) );
  cameraButton->setPopupMode( QToolButton::ToolButtonPopupMode::InstantPopup );

  mActionSync2DNavTo3D = new QAction( tr( "2D Map View Follows 3D Camera" ), this );
  mActionSync2DNavTo3D->setCheckable( true );
  connect( mActionSync2DNavTo3D, &QAction::triggered, this, [=]( bool enabled ) {
    Qgis::ViewSyncModeFlags syncMode = mCanvas->mapSettings()->viewSyncMode();
    syncMode.setFlag( Qgis::ViewSyncModeFlag::Sync2DTo3D, enabled );
    mCanvas->mapSettings()->setViewSyncMode( syncMode );
  } );
  mCameraMenu->addAction( mActionSync2DNavTo3D );

  mActionSync3DNavTo2D = new QAction( tr( "3D Camera Follows 2D Map View" ), this );
  mActionSync3DNavTo2D->setCheckable( true );
  connect( mActionSync3DNavTo2D, &QAction::triggered, this, [=]( bool enabled ) {
    Qgis::ViewSyncModeFlags syncMode = mCanvas->mapSettings()->viewSyncMode();
    syncMode.setFlag( Qgis::ViewSyncModeFlag::Sync3DTo2D, enabled );
    mCanvas->mapSettings()->setViewSyncMode( syncMode );
  } );
  mCameraMenu->addAction( mActionSync3DNavTo2D );

  mShowFrustumPolygon = new QAction( tr( "Show Visible Camera Area in 2D Map View" ), this );
  mShowFrustumPolygon->setCheckable( true );
  connect( mShowFrustumPolygon, &QAction::triggered, this, [=]( bool enabled ) {
    mCanvas->mapSettings()->setViewFrustumVisualizationEnabled( enabled );
  } );
  mCameraMenu->addAction( mShowFrustumPolygon );

  mActionSetSceneExtent = mCameraMenu->addAction( QgsApplication::getThemeIcon( QStringLiteral( "extents.svg" ) ), tr( "Set 3D Scene Extent on 2D Map View" ), this, &Qgs3DMapCanvasWidget::setSceneExtentOn2DCanvas );
  mActionSetSceneExtent->setCheckable( true );
  auto createShortcuts = [=]( const QString &objectName, void ( Qgs3DMapCanvasWidget::*slot )() ) {
    if ( QShortcut *sc = QgsGui::shortcutsManager()->shortcutByName( objectName ) )
      connect( sc, &QShortcut::activated, this, slot );
  };
  createShortcuts( QStringLiteral( "m3DSetSceneExtent" ), &Qgs3DMapCanvasWidget::setSceneExtentOn2DCanvas );

  // Effects Menu
  mEffectsMenu = new QMenu( this );

  mActionEffects = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "mIconShadow.svg" ) ), tr( "Effects" ), this );
  mActionEffects->setMenu( mEffectsMenu );
  toolBar->addAction( mActionEffects );
  QToolButton *effectsButton = qobject_cast<QToolButton *>( toolBar->widgetForAction( mActionEffects ) );
  effectsButton->setPopupMode( QToolButton::ToolButtonPopupMode::InstantPopup );

  mActionEnableShadows = new QAction( tr( "Show Shadows" ), this );
  mActionEnableShadows->setCheckable( true );
  connect( mActionEnableShadows, &QAction::toggled, this, [=]( bool enabled ) {
    QgsShadowSettings settings = mCanvas->mapSettings()->shadowSettings();
    settings.setRenderShadows( enabled );
    mCanvas->mapSettings()->setShadowSettings( settings );
  } );
  mEffectsMenu->addAction( mActionEnableShadows );

  mActionEnableEyeDome = new QAction( tr( "Show Eye Dome Lighting" ), this );
  mActionEnableEyeDome->setCheckable( true );
  connect( mActionEnableEyeDome, &QAction::triggered, this, [=]( bool enabled ) {
    mCanvas->mapSettings()->setEyeDomeLightingEnabled( enabled );
  } );
  mEffectsMenu->addAction( mActionEnableEyeDome );

  mActionEnableAmbientOcclusion = new QAction( tr( "Show Ambient Occlusion" ), this );
  mActionEnableAmbientOcclusion->setCheckable( true );
  connect( mActionEnableAmbientOcclusion, &QAction::triggered, this, [=]( bool enabled ) {
    QgsAmbientOcclusionSettings ambientOcclusionSettings = mCanvas->mapSettings()->ambientOcclusionSettings();
    ambientOcclusionSettings.setEnabled( enabled );
    mCanvas->mapSettings()->setAmbientOcclusionSettings( ambientOcclusionSettings );
  } );
  mEffectsMenu->addAction( mActionEnableAmbientOcclusion );

  // Options Menu
  QAction *configureAction = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "mActionOptions.svg" ) ), tr( "Configure…" ), this );
  connect( configureAction, &QAction::triggered, this, &Qgs3DMapCanvasWidget::configure );
  toolBar->addAction( configureAction );

  mCanvas = new Qgs3DMapCanvas;
  mCanvas->setMinimumSize( QSize( 200, 200 ) );

  connect( mCanvas, &Qgs3DMapCanvas::savedAsImage, this, [=]( const QString &fileName ) {
    QgisApp::instance()->messageBar()->pushSuccess( tr( "Save as Image" ), tr( "Successfully saved the 3D map to <a href=\"%1\">%2</a>" ).arg( QUrl::fromLocalFile( fileName ).toString(), QDir::toNativeSeparators( fileName ) ) );
  } );

  connect( mCanvas, &Qgs3DMapCanvas::fpsCountChanged, this, &Qgs3DMapCanvasWidget::updateFpsCount );
  connect( mCanvas, &Qgs3DMapCanvas::fpsCounterEnabledChanged, this, &Qgs3DMapCanvasWidget::toggleFpsCounter );
  connect( mCanvas, &Qgs3DMapCanvas::cameraNavigationSpeedChanged, this, &Qgs3DMapCanvasWidget::cameraNavigationSpeedChanged );
  connect( mCanvas, &Qgs3DMapCanvas::viewed2DExtentFrom3DChanged, this, &Qgs3DMapCanvasWidget::onViewed2DExtentFrom3DChanged );

  QgsMapToolIdentifyAction *identifyTool2D = QgisApp::instance()->identifyMapTool();
  QgsIdentifyResultsDialog *resultDialog = identifyTool2D->resultsDialog();
  connect( resultDialog, &QgsIdentifyResultsDialog::featureHighlighted, mCanvas, &Qgs3DMapCanvas::highlightFeature );
  connect( resultDialog, &QgsIdentifyResultsDialog::highlightsCleared, mCanvas, &Qgs3DMapCanvas::clearHighlights );

  mMapToolIdentify = new Qgs3DMapToolIdentify( mCanvas );

  mMapToolMeasureLine = new Qgs3DMapToolMeasureLine( mCanvas );

  mMapToolChangeAttribute.reset( new Qgs3DMapToolPointCloudChangeAttribute( mCanvas ) );

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
  connect( mLabelNavSpeedHideTimeout, &QTimer::timeout, this, [=] {
    mLabelNavigationSpeed->hide();
    mLabelNavSpeedHideTimeout->stop();
  } );

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
  mNavigationWidget = new Qgs3DNavigationWidget( mCanvas );
  mNavigationWidget->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding );
  mDebugWidget = new Qgs3DDebugWidget( mCanvas );

  QHBoxLayout *hLayout = new QHBoxLayout;
  hLayout->setContentsMargins( 0, 0, 0, 0 );
  hLayout->addWidget( mContainer );
  hLayout->addWidget( mNavigationWidget );
  hLayout->addWidget( mDebugWidget );

  QShortcut *debugPanelShortCut = new QShortcut( QKeySequence( tr( "Ctrl+Shift+d" ) ), this );
  connect( debugPanelShortCut, &QShortcut::activated, this, qOverload<>( &Qgs3DMapCanvasWidget::toggleDebugWidget ) );
  debugPanelShortCut->setObjectName( QStringLiteral( "DebugPanel" ) );
  debugPanelShortCut->setWhatsThis( tr( "Debug panel visibility" ) );
  toggleNavigationWidget(
    setting.value( QStringLiteral( "/3D/navigationWidget/visibility" ), false, QgsSettings::Gui ).toBool()
  );

  layout->addLayout( hLayout );
  layout->addWidget( mAnimationWidget );

  setLayout( layout );

  onTotalPendingJobsCountChanged();

  mDockableWidgetHelper = new QgsDockableWidgetHelper( mCanvasName, this, QgisApp::instance(), mCanvasName, QStringList(), isDocked ? QgsDockableWidgetHelper::OpeningMode::ForceDocked : QgsDockableWidgetHelper::OpeningMode::RespectSetting );

  if ( QDialog *dialog = mDockableWidgetHelper->dialog() )
  {
    QFontMetrics fm( font() );
    const int initialSize = fm.horizontalAdvance( '0' ) * 75;
    dialog->resize( initialSize, initialSize );
  }
  QAction *dockAction = mDockableWidgetHelper->createDockUndockAction( tr( "Dock 3D Map View" ), this );
  toolBar->addAction( dockAction );
  connect( mDockableWidgetHelper, &QgsDockableWidgetHelper::closed, this, [=]() {
    QgisApp::instance()->close3DMapView( canvasName() );
  } );
  connect( dockAction, &QAction::toggled, this, [=]( const bool isSmallSize ) {
    toolBar->setIconSize( QgisApp::instance()->iconSize( isSmallSize ) );
  } );

  updateLayerRelatedActions( QgisApp::instance()->activeLayer() );

  QList<QAction *> toolbarMenuActions;
  // Set action names so that they can be used in customization
  for ( QToolBar *toolBar : { mEditingToolBar } )
  {
    toolBar->toggleViewAction()->setObjectName( "mActionToggle" + toolBar->objectName().mid( 1 ) );
    toolbarMenuActions << toolBar->toggleViewAction();
  }

  // sort actions in toolbar menu
  std::sort( toolbarMenuActions.begin(), toolbarMenuActions.end(), []( QAction *a, QAction *b ) {
    return QString::localeAwareCompare( a->text(), b->text() ) < 0;
  } );

  mToolbarMenu->addActions( toolbarMenuActions );

  toolBar->installEventFilter( this );
  mEditingToolBar->installEventFilter( this );
}

Qgs3DMapCanvasWidget::~Qgs3DMapCanvasWidget()
{
  delete mDockableWidgetHelper;
  // we don't want canvas to reset the map tool as it is managed by unique_ptr
  if ( mMapToolChangeAttribute.get() == mCanvas->mapTool() )
  {
    mCanvas->setMapTool( nullptr );
  }
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

void Qgs3DMapCanvasWidget::changePointCloudAttributeByPaintbrush()
{
  const QAction *action = qobject_cast<QAction *>( sender() );
  if ( !action )
    return;

  mCanvas->setMapTool( nullptr );
  mMapToolChangeAttribute.reset( new Qgs3DMapToolPointCloudChangeAttributePaintbrush( mCanvas ) );
  onPointCloudChangeAttributeSettingsChanged();
  mCanvas->setMapTool( mMapToolChangeAttribute.get() );
  mEditingToolsAction->setIcon( action->icon() );
}

void Qgs3DMapCanvasWidget::changePointCloudAttributeByPolygon()
{
  const QAction *action = qobject_cast<QAction *>( sender() );
  if ( !action )
    return;

  mCanvas->setMapTool( nullptr );
  mMapToolChangeAttribute.reset( new Qgs3DMapToolPointCloudChangeAttributePolygon( mCanvas ) );
  onPointCloudChangeAttributeSettingsChanged();
  mCanvas->setMapTool( mMapToolChangeAttribute.get() );
  mEditingToolsAction->setIcon( action->icon() );
}

void Qgs3DMapCanvasWidget::setCanvasName( const QString &name )
{
  mCanvasName = name;
  mDockableWidgetHelper->setWindowTitle( name );
}

void Qgs3DMapCanvasWidget::updateLayerRelatedActions( QgsMapLayer *layer )
{
  mActionUndo->disconnect();
  mActionRedo->disconnect();

  if ( !layer || layer->type() != Qgis::LayerType::PointCloud )
  {
    mPointCloudEditingToolbar->setEnabled( false );
    mActionToggleEditing->setEnabled( false );
    mActionToggleEditing->setChecked( false );
    mEditingToolsAction->setEnabled( false );
    mActionUndo->setEnabled( false );
    mActionRedo->setEnabled( false );

    if ( mCanvas->mapTool() )
      mCanvas->setMapTool( nullptr );

    return;
  }


  QgsPointCloudLayer *pcLayer = qobject_cast<QgsPointCloudLayer *>( layer );
  const QVector<QgsPointCloudAttribute> attributes = pcLayer->attributes().attributes();
  const QString previousAttribute = mCboChangeAttribute->currentText();
  whileBlocking( mCboChangeAttribute )->clear();
  for ( const QgsPointCloudAttribute &attribute : attributes )
  {
    if ( attribute.name() == QLatin1String( "X" ) || attribute.name() == QLatin1String( "Y" ) || attribute.name() == QLatin1String( "Z" ) )
      continue;

    whileBlocking( mCboChangeAttribute )->addItem( attribute.name() );
  }

  int index = mCboChangeAttribute->findText( previousAttribute );
  if ( index < 0 )
    index = mCboChangeAttribute->findText( QStringLiteral( "Classification" ) );
  mCboChangeAttribute->setCurrentIndex( std::max( index, 0 ) );

  mActionToggleEditing->setEnabled( pcLayer->supportsEditing() );
  mActionToggleEditing->setChecked( pcLayer->isEditable() );
  connect( mActionUndo, &QAction::triggered, pcLayer->undoStack(), &QUndoStack::undo );
  connect( mActionRedo, &QAction::triggered, pcLayer->undoStack(), &QUndoStack::redo );
  mActionUndo->setEnabled( pcLayer->undoStack()->canUndo() );
  mActionRedo->setEnabled( pcLayer->undoStack()->canRedo() );
  connect( pcLayer->undoStack(), &QUndoStack::canUndoChanged, mActionUndo, &QAction::setEnabled );
  connect( pcLayer->undoStack(), &QUndoStack::canRedoChanged, mActionRedo, &QAction::setEnabled );
  mPointCloudEditingToolbar->setEnabled( pcLayer->isEditable() );
  mEditingToolsAction->setEnabled( pcLayer->isEditable() );
  // Re-parse the class values when the renderer changes - renderer3DChanged() is not fired when only the renderer symbol is changed
  connect( pcLayer, &QgsMapLayer::request3DUpdate, this, &Qgs3DMapCanvasWidget::onPointCloudChangeAttributeSettingsChanged );
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
  mNavigationWidget->setVisible( visibility );
  QgsSettings setting;
  setting.setValue( QStringLiteral( "/3D/navigationWidget/visibility" ), visibility, QgsSettings::Gui );
}

void Qgs3DMapCanvasWidget::toggleFpsCounter( const bool visibility )
{
  mLabelFpsCounter->setVisible( visibility );
}

void Qgs3DMapCanvasWidget::toggleDebugWidget( const bool visibility ) const
{
  mDebugWidget->setVisible( visibility );
}

// this is used only for keyboard shortcut, you should supply the visibility value
void Qgs3DMapCanvasWidget::toggleDebugWidget() const
{
  const bool newVisibility = !mCanvas->mapSettings()->showDebugPanel();
  mDebugWidget->setVisible( newVisibility );
  mCanvas->mapSettings()->setShowDebugPanel( newVisibility );
}

void Qgs3DMapCanvasWidget::setMapSettings( Qgs3DMapSettings *map )
{
  whileBlocking( mActionEnableShadows )->setChecked( map->shadowSettings().renderShadows() );
  whileBlocking( mActionEnableEyeDome )->setChecked( map->eyeDomeLightingEnabled() );
  whileBlocking( mActionEnableAmbientOcclusion )->setChecked( map->ambientOcclusionSettings().isEnabled() );
  whileBlocking( mActionSync2DNavTo3D )->setChecked( map->viewSyncMode().testFlag( Qgis::ViewSyncModeFlag::Sync2DTo3D ) );
  whileBlocking( mActionSync3DNavTo2D )->setChecked( map->viewSyncMode().testFlag( Qgis::ViewSyncModeFlag::Sync3DTo2D ) );
  whileBlocking( mShowFrustumPolygon )->setChecked( map->viewFrustumVisualizationEnabled() );

  mCanvas->setMapSettings( map );
  connect( map, &Qgs3DMapSettings::showDebugPanelChanged, this, qOverload<bool>( &Qgs3DMapCanvasWidget::toggleDebugWidget ) );
  toggleDebugWidget( map->showDebugPanel() );
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
  mActionMapThemes->setDisabled( !mCanvas->mapSettings()->terrainRenderingEnabled() || !mCanvas->mapSettings()->terrainGenerator() || mCanvas->mapSettings()->terrainGenerator()->type() == QgsTerrainGenerator::Mesh );
  mLabelFpsCounter->setVisible( map->isFpsCounterEnabled() );

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

  if ( !mViewFrustumHighlight )
  {
    mViewFrustumHighlight.reset( new QgsRubberBand( canvas, Qgis::GeometryType::Polygon ) );
    mViewFrustumHighlight->setColor( QColor::fromRgba( qRgba( 0, 0, 255, 50 ) ) );
  }

  if ( !mViewExtentHighlight )
  {
    mViewExtentHighlight.reset( new QgsRubberBand( canvas, Qgis::GeometryType::Polygon ) );
    mViewExtentHighlight->setColor( QColor::fromRgba( qRgba( 255, 0, 0, 50 ) ) );
  }
}

void Qgs3DMapCanvasWidget::resetView()
{
  mCanvas->resetView();
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
  mConfigureDialog->setObjectName( QStringLiteral( "3DConfigurationDialog" ) );
  mConfigureDialog->setMinimumSize( 600, 460 );
  QgsGui::enableAutoGeometryRestore( mConfigureDialog );

  Qgs3DMapSettings *map = mCanvas->mapSettings();
  Qgs3DMapConfigWidget *w = new Qgs3DMapConfigWidget( map, mMainCanvas, mCanvas, mConfigureDialog );
  QDialogButtonBox *buttons = new QDialogButtonBox( QDialogButtonBox::Apply | QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, mConfigureDialog );

  auto applyConfig = [=]() {
    const QgsVector3D oldOrigin = map->origin();
    const QgsCoordinateReferenceSystem oldCrs = map->crs();
    const QgsCameraPose oldCameraPose = mCanvas->cameraController()->cameraPose();
    const QgsVector3D oldLookingAt = oldCameraPose.centerPoint();

    // update map
    w->apply();

    const QgsVector3D p = Qgs3DUtils::transformWorldCoordinates(
      oldLookingAt,
      oldOrigin, oldCrs,
      map->origin(), map->crs(), QgsProject::instance()->transformContext()
    );

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
  connect( buttons, &QDialogButtonBox::clicked, mConfigureDialog, [=]( QAbstractButton *button ) {
    if ( button == buttons->button( QDialogButtonBox::Apply ) || button == buttons->button( QDialogButtonBox::Ok ) )
      applyConfig();
    if ( button == buttons->button( QDialogButtonBox::Ok ) )
      mConfigureDialog->accept();
  } );
  connect( buttons, &QDialogButtonBox::helpRequested, w, []() { QgsHelp::openHelp( QStringLiteral( "map_views/3d_map_view.html#scene-configuration" ) ); } );

  connect( w, &Qgs3DMapConfigWidget::isValidChanged, this, [=]( bool valid ) {
    buttons->button( QDialogButtonBox::Apply )->setEnabled( valid );
    buttons->button( QDialogButtonBox::Ok )->setEnabled( valid );
  } );

  QVBoxLayout *layout = new QVBoxLayout( mConfigureDialog );
  layout->addWidget( w, 1 );
  layout->addWidget( buttons );

  mConfigureDialog->show();

  whileBlocking( mActionEnableShadows )->setChecked( map->shadowSettings().renderShadows() );
  whileBlocking( mActionEnableEyeDome )->setChecked( map->eyeDomeLightingEnabled() );
  whileBlocking( mActionEnableAmbientOcclusion )->setChecked( map->ambientOcclusionSettings().isEnabled() );
  whileBlocking( mActionSync2DNavTo3D )->setChecked( map->viewSyncMode().testFlag( Qgis::ViewSyncModeFlag::Sync2DTo3D ) );
  whileBlocking( mActionSync3DNavTo2D )->setChecked( map->viewSyncMode().testFlag( Qgis::ViewSyncModeFlag::Sync3DTo2D ) );
  whileBlocking( mShowFrustumPolygon )->setChecked( map->viewFrustumVisualizationEnabled() );
}

void Qgs3DMapCanvasWidget::exportScene()
{
  QDialog dlg;
  dlg.setWindowTitle( tr( "Export 3D Scene" ) );
  dlg.setObjectName( QStringLiteral( "3DSceneExportDialog" ) );
  QgsGui::enableAutoGeometryRestore( &dlg );

  Qgs3DMapExportSettings exportSettings;
  QgsMap3DExportWidget exportWidget( mCanvas->scene(), &exportSettings );

  QDialogButtonBox *buttons = new QDialogButtonBox( QDialogButtonBox::Cancel | QDialogButtonBox::Help | QDialogButtonBox::Ok, &dlg );

  connect( buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept );
  connect( buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject );
  connect( buttons, &QDialogButtonBox::helpRequested, &dlg, [=] { QgsHelp::openHelp( QStringLiteral( "map_views/3d_map_view.html" ) ); } );

  QVBoxLayout *layout = new QVBoxLayout( &dlg );
  layout->addWidget( &exportWidget, 1 );
  layout->addWidget( buttons );
  if ( dlg.exec() )
  {
    const bool success = exportWidget.exportScene();
    const QString exportFilePath = QDir( exportSettings.sceneFolderPath() ).filePath( exportSettings.sceneName() + QStringLiteral( ".obj" ) );
    if ( success )
    {
      mMessageBar->pushMessage( tr( "Export 3D scene" ), tr( "Successfully exported scene to <a href=\"%1\">%2</a>" ).arg( QUrl::fromLocalFile( exportFilePath ).toString(), QDir::toNativeSeparators( exportFilePath ) ), Qgis::MessageLevel::Success, 0 );
    }
    else
    {
      mMessageBar->pushMessage( tr( "Export 3D scene" ), tr( "Unable to export scene to <a href=\"%1\">%2</a>" ).arg( QUrl::fromLocalFile( exportFilePath ).toString(), QDir::toNativeSeparators( exportFilePath ) ), Qgis::MessageLevel::Warning, 0 );
    }
  }
}

void Qgs3DMapCanvasWidget::onMainCanvasLayersChanged()
{
  mCanvas->mapSettings()->setLayers( mMainCanvas->layers( true ) );
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
  mLabelFpsCounter->setText( QStringLiteral( "%1 fps" ).arg( fpsCount, 10, 'f', 2, QLatin1Char( ' ' ) ) );
}

void Qgs3DMapCanvasWidget::cameraNavigationSpeedChanged( double speed )
{
  mLabelNavigationSpeed->setText( QStringLiteral( "Speed: %1 ×" ).arg( QString::number( speed, 'f', 2 ) ) );
  mLabelNavigationSpeed->show();
  mLabelNavSpeedHideTimeout->start();
}

void Qgs3DMapCanvasWidget::mapThemeMenuAboutToShow()
{
  qDeleteAll( mMapThemeMenuPresetActions );
  mMapThemeMenuPresetActions.clear();

  const QString currentTheme = mCanvas->mapSettings()->terrainMapTheme();

  QAction *actionFollowMain = new QAction( tr( "(none)" ), mMapThemeMenu );
  actionFollowMain->setCheckable( true );
  if ( currentTheme.isEmpty() || !QgsProject::instance()->mapThemeCollection()->hasMapTheme( currentTheme ) )
  {
    actionFollowMain->setChecked( true );
  }
  connect( actionFollowMain, &QAction::triggered, this, [=] {
    mCanvas->mapSettings()->setTerrainMapTheme( QString() );
  } );
  mMapThemeMenuPresetActions.append( actionFollowMain );

  const auto constMapThemes = QgsProject::instance()->mapThemeCollection()->mapThemes();
  for ( const QString &grpName : constMapThemes )
  {
    QAction *a = new QAction( grpName, mMapThemeMenu );
    a->setCheckable( true );
    if ( grpName == currentTheme )
    {
      a->setChecked( true );
    }
    connect( a, &QAction::triggered, this, [a, this] {
      mCanvas->mapSettings()->setTerrainMapTheme( a->text() );
    } );
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
  double memLimit = settings.value( QStringLiteral( "map3d/gpuMemoryLimit" ), 500.0, QgsSettings::App ).toDouble();
  mMessageBar->pushMessage( tr( "A map layer has used all graphics memory allowed (%1 MB). "
                                "You may want to lower the amount of detail in the scene, or increase the limit in the options." )
                              .arg( memLimit ),
                            Qgis::MessageLevel::Warning );
  mGpuMemoryLimitReachedReported = true;
}

void Qgs3DMapCanvasWidget::onPointCloudChangeAttributeSettingsChanged()
{
  const QString attributeName = mCboChangeAttribute->currentText();

  mSpinChangeAttributeValue->setSuffix( QString() );
  bool useComboBox = false;

  if ( attributeName == QLatin1String( "Intensity" ) || attributeName == QLatin1String( "PointSourceId" ) || attributeName == QLatin1String( "Red" ) || attributeName == QLatin1String( "Green" ) || attributeName == QLatin1String( "Blue" ) || attributeName == QLatin1String( "Infrared" ) )
  {
    mSpinChangeAttributeValue->setMinimum( 0 );
    mSpinChangeAttributeValue->setMaximum( 65535 );
    mSpinChangeAttributeValue->setDecimals( 0 );
  }
  else if ( attributeName == QLatin1String( "ReturnNumber" ) || attributeName == QLatin1String( "NumberOfReturns" ) )
  {
    mSpinChangeAttributeValue->setMinimum( 0 );
    mSpinChangeAttributeValue->setMaximum( 15 );
    mSpinChangeAttributeValue->setDecimals( 0 );
  }
  else if ( attributeName == QLatin1String( "Synthetic" ) || attributeName == QLatin1String( "KeyPoint" ) || attributeName == QLatin1String( "Withheld" ) || attributeName == QLatin1String( "Overlap" ) || attributeName == QLatin1String( "ScanDirectionFlag" ) || attributeName == QLatin1String( "EdgeOfFlightLine" ) )
  {
    useComboBox = true;
    const int oldIndex = mCboChangeAttributeValue->currentIndex();
    QgsSignalBlocker< QComboBox > blocker( mCboChangeAttributeValue );
    mCboChangeAttributeValue->clear();
    mCboChangeAttributeValue->addItem( tr( "False" ), 0 );
    mCboChangeAttributeValue->addItem( tr( "True" ), 1 );
    mCboChangeAttributeValue->setEditable( false );
    mCboChangeAttributeValue->setCurrentIndex( std::min( oldIndex, 1 ) );
  }
  else if ( attributeName == QLatin1String( "ScannerChannel" ) )
  {
    mSpinChangeAttributeValue->setMinimum( 0 );
    mSpinChangeAttributeValue->setMaximum( 3 );
    mSpinChangeAttributeValue->setDecimals( 0 );
  }
  else if ( attributeName == QLatin1String( "Classification" ) )
  {
    useComboBox = true;
    const QStringList split = mCboChangeAttributeValue->currentText().split( ' ' );
    const int oldValue = split.isEmpty() ? 0 : split.constFirst().toInt();

    whileBlocking( mCboChangeAttributeValue )->clear();
    // We will fill the combobox with all available classes from the Classification renderer (may have changed names) and the layer statistics
    // Users will be able to manually type in any other class number too.
    QMap<int, QString> lasCodes = QgsPointCloudDataProvider::translatedLasClassificationCodes();
    QMap<int, QString> classes;

    QgsPointCloudLayer *layer = qobject_cast<QgsPointCloudLayer *>( QgisApp::instance()->activeLayer() );
    if ( layer )
    {
      QgsAbstract3DRenderer *r = layer->renderer3D();
      // if there's a clsasification renderer, let's use the classes' labels
      if ( QgsPointCloudLayer3DRenderer *cr = dynamic_cast<QgsPointCloudLayer3DRenderer *>( r ) )
      {
        const QgsPointCloud3DSymbol *s = cr->symbol();
        if ( const QgsClassificationPointCloud3DSymbol *cs = dynamic_cast<const QgsClassificationPointCloud3DSymbol *>( s ) )
        {
          if ( cs->attribute() == QLatin1String( "Classification" ) )
          {
            for ( const QgsPointCloudCategory &c : cs->categoriesList() )
            {
              classes[c.value()] = c.label();
            }
          }
        }
      }

      // then add missing classes from the layer stats too
      const QMap<int, int> statisticsClasses = layer->statistics().availableClasses( QStringLiteral( "Classification" ) );
      for ( auto it = statisticsClasses.constBegin(); it != statisticsClasses.constEnd(); ++it )
      {
        if ( !classes.contains( it.key() ) )
          classes[it.key()] = lasCodes[it.key()];
      }
      for ( auto it = classes.constBegin(); it != classes.constEnd(); ++it )
      {
        // populate the combobox
        whileBlocking( mCboChangeAttributeValue )->addItem( QStringLiteral( "%1 (%2)" ).arg( it.key() ).arg( it.value() ), it.key() );
        // and also update the labels in the full list of classes, which will be used in the editable combobox validator.
        lasCodes[it.key()] = it.value();
      }
    }
    // new values (manually edited) will be added after a separator
    mCboChangeAttributeValue->insertSeparator( mCboChangeAttributeValue->count() );
    mClassValidator->setClasses( lasCodes );
    mCboChangeAttributeValue->setEditable( true );
    mCboChangeAttributeValue->setValidator( mClassValidator );
    mCboChangeAttributeValue->setCompleter( nullptr );

    // Try to reselect last selected value
    for ( int i = 0; i < mCboChangeAttributeValue->count(); ++i )
    {
      if ( mCboChangeAttributeValue->itemText( i ).startsWith( QStringLiteral( "%1 " ).arg( oldValue ) ) )
      {
        mCboChangeAttributeValue->setCurrentIndex( i );
        break;
      }
    }
  }
  else if ( attributeName == QLatin1String( "UserData" ) )
  {
    mSpinChangeAttributeValue->setMinimum( 0 );
    mSpinChangeAttributeValue->setMaximum( 255 );
    mSpinChangeAttributeValue->setDecimals( 0 );
  }
  else if ( attributeName == QLatin1String( "ScanAngleRank" ) )
  {
    mSpinChangeAttributeValue->setMinimum( -180 );
    mSpinChangeAttributeValue->setMaximum( 180 );
    mSpinChangeAttributeValue->setDecimals( 3 );
    mSpinChangeAttributeValue->setSuffix( QStringLiteral( " %1" ).arg( tr( "degrees" ) ) );
  }
  else if ( attributeName == QLatin1String( "GpsTime" ) )
  {
    mSpinChangeAttributeValue->setMinimum( 0 );
    mSpinChangeAttributeValue->setMaximum( std::numeric_limits<double>::max() );
    mSpinChangeAttributeValue->setDecimals( 42 );
  }

  mMapToolChangeAttribute->setAttribute( attributeName );
  double newValue = 0;
  if ( useComboBox && mCboChangeAttributeValue->isEditable() )
  {
    // read class integer
    const QStringList split = mCboChangeAttributeValue->currentText().split( ' ' );
    if ( !split.isEmpty() )
      newValue = split.constFirst().toDouble();
  }
  else if ( useComboBox )
  {
    // read true/false combo box
    newValue = mCboChangeAttributeValue->currentData().toDouble();
  }
  else
  {
    // read the spinbox value
    newValue = mSpinChangeAttributeValue->value();
  }
  mMapToolChangeAttribute->setNewValue( newValue );

  mCboChangeAttributeValueAction->setVisible( useComboBox );
  mSpinChangeAttributeValueAction->setVisible( !useComboBox );
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

  if ( mMapToolPrevious )
    mMainCanvas->setMapTool( mMapToolPrevious );
  else
    mMainCanvas->unsetMapTool( mMapToolExtent.get() );
}

ClassValidator::ClassValidator( QWidget *parent )
  : QValidator( parent )
{
  mRx = QRegularExpression( QStringLiteral( "([0-9]{1,3})" ) );
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
    input = QStringLiteral( "%1 (%2)" ).arg( n ).arg( mClasses[n] );
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
  input = QStringLiteral( "%1 (%2)" ).arg( n ).arg( mClasses[n] );
}

/***************************************************************************
  qgs3dmapcanvasdockwidget.cpp
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgs3dmapcanvasdockwidget.h"

#include <QBoxLayout>
#include <QDialog>
#include <QDialogButtonBox>
#include <QProgressBar>
#include <QToolBar>
#include <QUrl>
#include <QAction>

#include "qgisapp.h"
#include "qgs3dmapcanvas.h"
#include "qgs3dmapconfigwidget.h"
#include "qgs3dmapscene.h"
#include "qgscameracontroller.h"
#include "qgshelp.h"
#include "qgsmapcanvas.h"
#include "qgsmessagebar.h"
#include "qgsapplication.h"
#include "qgssettings.h"
#include "qgsgui.h"
#include "qgsmapthemecollection.h"
#include "qgstemporalcontroller.h"

#include "qgs3danimationsettings.h"
#include "qgs3danimationwidget.h"
#include "qgs3dmapsettings.h"
#include "qgs3dmaptoolidentify.h"
#include "qgs3dmaptoolmeasureline.h"
#include "qgs3dutils.h"



Qgs3DMapCanvasDockWidget::Qgs3DMapCanvasDockWidget( QWidget *parent )
  : QgsDockWidget( parent )
{
  QgsSettings setting;
  setAttribute( Qt::WA_DeleteOnClose );  // removes the dock widget from main window when

  QWidget *contentsWidget = new QWidget( this );

  QToolBar *toolBar = new QToolBar( contentsWidget );
  toolBar->setIconSize( QgisApp::instance()->iconSize( true ) );

  QAction *actionCameraControl = toolBar->addAction( QIcon( QgsApplication::iconPath( "mActionPan.svg" ) ),
                                 tr( "Camera Control" ), this, &Qgs3DMapCanvasDockWidget::cameraControl );
  actionCameraControl->setCheckable( true );

  toolBar->addAction( QgsApplication::getThemeIcon( QStringLiteral( "mActionZoomFullExtent.svg" ) ),
                      tr( "Zoom Full" ), this, &Qgs3DMapCanvasDockWidget::resetView );

  QAction *toggleOnScreenNavigation = toolBar->addAction(
                                        QgsApplication::getThemeIcon( QStringLiteral( "mAction3DNavigation.svg" ) ),
                                        tr( "Toggle On-Screen Navigation" ) );

  toggleOnScreenNavigation->setCheckable( true );
  toggleOnScreenNavigation->setChecked(
    setting.value( QStringLiteral( "/3D/navigationWidget/visibility" ), true, QgsSettings::Gui ).toBool()
  );
  QObject::connect( toggleOnScreenNavigation, &QAction::toggled, this, &Qgs3DMapCanvasDockWidget::toggleNavigationWidget );

  toolBar->addSeparator();

  QAction *actionIdentify = toolBar->addAction( QIcon( QgsApplication::iconPath( "mActionIdentify.svg" ) ),
                            tr( "Identify" ), this, &Qgs3DMapCanvasDockWidget::identify );
  actionIdentify->setCheckable( true );

  QAction *actionMeasurementTool = toolBar->addAction( QIcon( QgsApplication::iconPath( "mActionMeasure.svg" ) ),
                                   tr( "Measurement Line" ), this, &Qgs3DMapCanvasDockWidget::measureLine );
  actionMeasurementTool->setCheckable( true );

  // Create action group to make the action exclusive
  QActionGroup *actionGroup = new QActionGroup( this );
  actionGroup->addAction( actionCameraControl );
  actionGroup->addAction( actionIdentify );
  actionGroup->addAction( actionMeasurementTool );
  actionGroup->setExclusive( true );
  actionCameraControl->setChecked( true );

  QAction *actionAnim = toolBar->addAction( QIcon( QgsApplication::iconPath( "mTaskRunning.svg" ) ),
                        tr( "Animations" ), this, &Qgs3DMapCanvasDockWidget::toggleAnimations );
  actionAnim->setCheckable( true );

  toolBar->addAction( QgsApplication::getThemeIcon( QStringLiteral( "mActionSaveMapAsImage.svg" ) ),
                      tr( "Save as Image…" ), this, &Qgs3DMapCanvasDockWidget::saveAsImage );

  toolBar->addSeparator();

  // Map Theme Menu
  mMapThemeMenu = new QMenu();
  connect( mMapThemeMenu, &QMenu::aboutToShow, this, &Qgs3DMapCanvasDockWidget::mapThemeMenuAboutToShow );
  connect( QgsProject::instance()->mapThemeCollection(), &QgsMapThemeCollection::mapThemeRenamed, this, &Qgs3DMapCanvasDockWidget::currentMapThemeRenamed );

  mBtnMapThemes = new QToolButton();
  mBtnMapThemes->setAutoRaise( true );
  mBtnMapThemes->setToolTip( tr( "Set View Theme" ) );
  mBtnMapThemes->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionShowAllLayers.svg" ) ) );
  mBtnMapThemes->setPopupMode( QToolButton::InstantPopup );
  mBtnMapThemes->setMenu( mMapThemeMenu );

  toolBar->addWidget( mBtnMapThemes );

  toolBar->addAction( QgsApplication::getThemeIcon( QStringLiteral( "mActionOptions.svg" ) ),
                      tr( "Configure…" ), this, &Qgs3DMapCanvasDockWidget::configure );


  mCanvas = new Qgs3DMapCanvas( contentsWidget );
  mCanvas->setMinimumSize( QSize( 200, 200 ) );
  mCanvas->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

  connect( mCanvas, &Qgs3DMapCanvas::savedAsImage, this, [ = ]( const QString fileName )
  {
    QgisApp::instance()->messageBar()->pushSuccess( tr( "Save as Image" ), tr( "Successfully saved the 3D map to <a href=\"%1\">%2</a>" ).arg( QUrl::fromLocalFile( fileName ).toString(), QDir::toNativeSeparators( fileName ) ) );
  } );

  mMapToolIdentify = new Qgs3DMapToolIdentify( mCanvas );

  mMapToolMeasureLine = new Qgs3DMapToolMeasureLine( mCanvas );

  mLabelPendingJobs = new QLabel( this );
  mProgressPendingJobs = new QProgressBar( this );
  mProgressPendingJobs->setRange( 0, 0 );

  mAnimationWidget = new Qgs3DAnimationWidget( this );
  mAnimationWidget->setVisible( false );

  QHBoxLayout *topLayout = new QHBoxLayout;
  topLayout->setContentsMargins( 0, 0, 0, 0 );
  topLayout->setSpacing( style()->pixelMetric( QStyle::PM_LayoutHorizontalSpacing ) );
  topLayout->addWidget( toolBar );
  topLayout->addStretch( 1 );
  topLayout->addWidget( mLabelPendingJobs );
  topLayout->addWidget( mProgressPendingJobs );

  QVBoxLayout *layout = new QVBoxLayout;
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->setSpacing( 0 );
  layout->addLayout( topLayout );
  layout->addWidget( mCanvas );
  layout->addWidget( mAnimationWidget );

  contentsWidget->setLayout( layout );

  setWidget( contentsWidget );

  onTotalPendingJobsCountChanged();
}

void Qgs3DMapCanvasDockWidget::saveAsImage()
{
  QPair< QString, QString> fileNameAndFilter = QgsGuiUtils::getSaveAsImageName( this, tr( "Choose a file name to save the 3D map canvas to an image" ) );
  if ( !fileNameAndFilter.first.isEmpty() )
  {
    mCanvas->saveAsImage( fileNameAndFilter.first, fileNameAndFilter.second );
  }
}

void Qgs3DMapCanvasDockWidget::toggleAnimations()
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

void Qgs3DMapCanvasDockWidget::cameraControl()
{
  QAction *action = qobject_cast<QAction *>( sender() );
  if ( !action )
    return;

  mCanvas->setMapTool( nullptr );
}

void Qgs3DMapCanvasDockWidget::identify()
{
  QAction *action = qobject_cast<QAction *>( sender() );
  if ( !action )
    return;

  mCanvas->setMapTool( action->isChecked() ? mMapToolIdentify : nullptr );
}

void Qgs3DMapCanvasDockWidget::measureLine()
{
  QAction *action = qobject_cast<QAction *>( sender() );
  if ( !action )
    return;

  mCanvas->setMapTool( action->isChecked() ? mMapToolMeasureLine : nullptr );
}

void Qgs3DMapCanvasDockWidget::toggleNavigationWidget( bool visibility )
{
  mCanvas->setOnScreenNavigationVisibility( visibility );
}

void Qgs3DMapCanvasDockWidget::setMapSettings( Qgs3DMapSettings *map )
{
  mCanvas->setMap( map );

  connect( mCanvas->scene(), &Qgs3DMapScene::totalPendingJobsCountChanged, this, &Qgs3DMapCanvasDockWidget::onTotalPendingJobsCountChanged );

  mAnimationWidget->setCameraController( mCanvas->scene()->cameraController() );
  mAnimationWidget->setMap( map );

  // Disable button for switching the map theme if the terrain generator is a mesh
  mBtnMapThemes->setDisabled( mCanvas->map()->terrainGenerator()->type() == QgsTerrainGenerator::Mesh );
}

void Qgs3DMapCanvasDockWidget::setMainCanvas( QgsMapCanvas *canvas )
{
  mMainCanvas = canvas;

  connect( mMainCanvas, &QgsMapCanvas::layersChanged, this, &Qgs3DMapCanvasDockWidget::onMainCanvasLayersChanged );
  connect( mMainCanvas, &QgsMapCanvas::canvasColorChanged, this, &Qgs3DMapCanvasDockWidget::onMainCanvasColorChanged );
}

void Qgs3DMapCanvasDockWidget::resetView()
{
  mCanvas->resetView();
}

void Qgs3DMapCanvasDockWidget::configure()
{
  QDialog dlg;
  dlg.setWindowTitle( tr( "3D Configuration" ) );
  dlg.setObjectName( QStringLiteral( "3DConfigurationDialog" ) );
  dlg.setMinimumSize( 380, 460 );
  QgsGui::instance()->enableAutoGeometryRestore( &dlg );

  Qgs3DMapSettings *map = mCanvas->map();
  Qgs3DMapConfigWidget *w = new Qgs3DMapConfigWidget( map, mMainCanvas, &dlg );
  QDialogButtonBox *buttons = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, &dlg );
  connect( buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept );
  connect( buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject );
  connect( buttons, &QDialogButtonBox::helpRequested, w, []() { QgsHelp::openHelp( QStringLiteral( "introduction/qgis_gui.html#d-map-view" ) ); } );

  QVBoxLayout *layout = new QVBoxLayout( &dlg );
  layout->addWidget( w, 1 );
  layout->addWidget( buttons );
  if ( !dlg.exec() )
    return;

  QgsVector3D oldOrigin = map->origin();
  QgsCoordinateReferenceSystem oldCrs = map->crs();
  QgsCameraPose oldCameraPose = mCanvas->cameraController()->cameraPose();
  QgsVector3D oldLookingAt = oldCameraPose.centerPoint();

  // update map
  w->apply();

  QgsVector3D p = Qgs3DUtils::transformWorldCoordinates(
                    oldLookingAt,
                    oldOrigin, oldCrs,
                    map->origin(), map->crs(), QgsProject::instance()->transformContext() );

  if ( p != oldLookingAt )
  {
    // apply() call has moved origin of the world so let's move camera so we look still at the same place
    QgsCameraPose newCameraPose = oldCameraPose;
    newCameraPose.setCenterPoint( p );
    mCanvas->cameraController()->setCameraPose( newCameraPose );
  }

  // Disable map theme button if the terrain generator is a mesh
  mBtnMapThemes->setDisabled( map->terrainGenerator()->type() == QgsTerrainGenerator::Mesh );
}

void Qgs3DMapCanvasDockWidget::onMainCanvasLayersChanged()
{
  mCanvas->map()->setLayers( mMainCanvas->layers() );
}

void Qgs3DMapCanvasDockWidget::onMainCanvasColorChanged()
{
  mCanvas->map()->setBackgroundColor( mMainCanvas->canvasColor() );
}

void Qgs3DMapCanvasDockWidget::onTotalPendingJobsCountChanged()
{
  int count = mCanvas->scene() ? mCanvas->scene()->totalPendingJobsCount() : 0;
  mProgressPendingJobs->setVisible( count );
  mLabelPendingJobs->setVisible( count );
  if ( count )
    mLabelPendingJobs->setText( tr( "Loading %1 tiles" ).arg( count ) );
}

void Qgs3DMapCanvasDockWidget::mapThemeMenuAboutToShow()
{
  qDeleteAll( mMapThemeMenuPresetActions );
  mMapThemeMenuPresetActions.clear();

  QString currentTheme = mCanvas->map()->terrainMapTheme();

  QAction *actionFollowMain = new QAction( tr( "(none)" ), mMapThemeMenu );
  actionFollowMain->setCheckable( true );
  if ( currentTheme.isEmpty() || !QgsProject::instance()->mapThemeCollection()->hasMapTheme( currentTheme ) )
  {
    actionFollowMain->setChecked( true );
  }
  connect( actionFollowMain, &QAction::triggered, this, [ = ]
  {
    mCanvas->map()->setTerrainMapTheme( QString() );
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
    connect( a, &QAction::triggered, this, [a, this]
    {
      mCanvas->map()->setTerrainMapTheme( a->text() );
    } );
    mMapThemeMenuPresetActions.append( a );
  }
  mMapThemeMenu->addActions( mMapThemeMenuPresetActions );
}

void Qgs3DMapCanvasDockWidget::currentMapThemeRenamed( const QString &theme, const QString &newTheme )
{
  if ( theme == mCanvas->map()->terrainMapTheme() )
  {
    mCanvas->map()->setTerrainMapTheme( newTheme );
  }
}

/***************************************************************************
                         qgis_3d_sandbox.cpp
                         --------------------
    begin                : October 2020
    copyright            : (C) 2020 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QApplication>

#include "qgs3d.h"
#include "qgs3ddebugwidget.h"
#include "qgs3dmapcanvas.h"
#include "qgs3dmapconfigwidget.h"
#include "qgs3dmapscene.h"
#include "qgs3dmapsettings.h"
#include "qgs3dutils.h"
#include "qgsapplication.h"
#include "qgsflatterrainsettings.h"
#include "qgsgui.h"
#include "qgshelp.h"
#include "qgslayertree.h"
#include "qgsmapcanvas.h"
#include "qgsmapsettings.h"
#include "qgspointlightsettings.h"
#include "qgsproject.h"
#include "qgsprojectelevationproperties.h"
#include "qgsprojectviewsettings.h"
#include "qgsterrainprovider.h"

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QScreen>
#include <QToolBar>

void initCanvas3D( Qgs3DMapCanvas *canvas )
{
  QgsLayerTree *root = QgsProject::instance()->layerTreeRoot();
  const QList<QgsMapLayer *> visibleLayers = root->checkedLayers();

  QgsCoordinateReferenceSystem crs = QgsProject::instance()->crs();
  if ( crs.isGeographic() )
  {
    // we can't deal with non-projected CRS, so let's just pick something
    QgsProject::instance()->setCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) ) );
  }

  QgsMapSettings ms;
  ms.setDestinationCrs( QgsProject::instance()->crs() );
  ms.setLayers( visibleLayers );
  QgsRectangle fullExtent = QgsProject::instance()->viewSettings()->fullExtent();

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( QgsProject::instance()->crs() );
  map->setOrigin( QgsVector3D( fullExtent.center().x(), fullExtent.center().y(), 0 ) );
  map->setLayers( visibleLayers );

  map->setExtent( fullExtent );

  Qgs3DAxisSettings axis;
  axis.setMode( Qgs3DAxisSettings::Mode::Off );
  map->set3DAxisSettings( axis );

  map->setTransformContext( QgsProject::instance()->transformContext() );
  map->setPathResolver( QgsProject::instance()->pathResolver() );
  map->setMapThemeCollection( QgsProject::instance()->mapThemeCollection() );
  QObject::connect( QgsProject::instance(), &QgsProject::transformContextChanged, map, [map] {
    map->setTransformContext( QgsProject::instance()->transformContext() );
  } );

  QgsFlatTerrainSettings *flatTerrain = new QgsFlatTerrainSettings();
  flatTerrain->setElevationOffset( QgsProject::instance()->elevationProperties()->terrainProvider()->offset() );
  map->setTerrainSettings( flatTerrain );

  QgsPointLightSettings defaultPointLight;
  defaultPointLight.setPosition( QgsVector3D( 0, 0, 1000 ) );
  defaultPointLight.setConstantAttenuation( 0 );
  map->setLightSources( { defaultPointLight.clone() } );
  if ( QScreen *screen = QGuiApplication::primaryScreen() )
  {
    map->setOutputDpi( screen->physicalDotsPerInch() );
  }
  else
  {
    map->setOutputDpi( 96 );
  }

  canvas->setMapSettings( map );

  QgsRectangle extent = fullExtent;
  extent.scale( 1.3 );
  const float dist = static_cast<float>( std::max( extent.width(), extent.height() ) );
  canvas->setViewFromTop( extent.center(), dist * 2, 0 );

  QObject::connect( canvas->scene(), &Qgs3DMapScene::totalPendingJobsCountChanged, canvas, [canvas] {
    qDebug() << "pending jobs:" << canvas->scene()->totalPendingJobsCount();
  } );

  qDebug() << "pending jobs:" << canvas->scene()->totalPendingJobsCount();
}

QDialog *createConfigDialog( Qgs3DMapCanvas *canvas )
{
  const QPointer configDialog = new QDialog;
  configDialog->setWindowTitle( QStringLiteral( "3D Configuration" ) );
  configDialog->setObjectName( QStringLiteral( "3DConfigurationDialog" ) );
  configDialog->setMinimumSize( 600, 460 );
  QgsGui::enableAutoGeometryRestore( configDialog );

  Qgs3DMapSettings *map = canvas->mapSettings();
  Qgs3DMapConfigWidget *w = new Qgs3DMapConfigWidget( map, nullptr, canvas, configDialog );
  QDialogButtonBox *buttons = new QDialogButtonBox( QDialogButtonBox::Apply | QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, configDialog );

  auto applyConfig = [=] {
    const QgsVector3D oldOrigin = map->origin();
    const QgsCoordinateReferenceSystem oldCrs = map->crs();
    const QgsCameraPose oldCameraPose = canvas->cameraController()->cameraPose();
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
      canvas->cameraController()->setCameraPose( newCameraPose );
    }
  };

  QObject::connect( buttons, &QDialogButtonBox::rejected, configDialog, &QDialog::reject );
  QObject::connect( buttons, &QDialogButtonBox::clicked, configDialog, [=]( const QAbstractButton *button ) {
    if ( button == buttons->button( QDialogButtonBox::Apply ) || button == buttons->button( QDialogButtonBox::Ok ) )
      applyConfig();
    if ( button == buttons->button( QDialogButtonBox::Ok ) )
      configDialog->accept();
  } );
  QObject::connect( buttons, &QDialogButtonBox::helpRequested, w, []() { QgsHelp::openHelp( QStringLiteral( "map_views/3d_map_view.html#scene-configuration" ) ); } );

  QObject::connect( w, &Qgs3DMapConfigWidget::isValidChanged, configDialog, [=]( const bool valid ) {
    buttons->button( QDialogButtonBox::Apply )->setEnabled( valid );
    buttons->button( QDialogButtonBox::Ok )->setEnabled( valid );
  } );

  QVBoxLayout *layout = new QVBoxLayout( configDialog );
  layout->addWidget( w, 1 );
  layout->addWidget( buttons );
  return configDialog;
}

int main( int argc, char *argv[] )
{
  QgsApplication myApp( argc, argv, true, QString(), QStringLiteral( "desktop" ) );

  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  Qgs3D::initialize();

  if ( argc < 2 )
  {
    qDebug() << "need QGIS project file";
    return 1;
  }

  const QString projectFile = argv[1];
  const bool res = QgsProject::instance()->read( projectFile );
  if ( !res )
  {
    qDebug() << "can't open project file" << projectFile;
    return 1;
  }

  Qgs3DMapCanvas *canvas = new Qgs3DMapCanvas;
  initCanvas3D( canvas );

  // set up the UI
  QWidget *windowWidget = new QWidget;

  QToolBar *toolBar = new QToolBar( windowWidget );
  toolBar->setIconSize( QgsGuiUtils::iconSize() );
  toolBar->addAction( QIcon( QgsApplication::iconPath( "mActionZoomFullExtent.svg" ) ), QStringLiteral( "Reset camera to default position" ), windowWidget, [canvas] {
    canvas->resetView();
  } );
  QAction *toggleDebugPanel = toolBar->addAction(
    QgsApplication::getThemeIcon( QStringLiteral( "/propertyicons/general.svg" ) ),
    QStringLiteral( "Toggle on-screen Debug panel" )
  );
  toggleDebugPanel->setCheckable( true );
  QAction *configureAction = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "mActionOptions.svg" ) ), QStringLiteral( "Configure…" ), windowWidget );
  QDialog *configDialog = createConfigDialog( canvas );
  QObject::connect( configureAction, &QAction::triggered, windowWidget, [configDialog] {
    configDialog->setVisible( true );
  } );
  toolBar->addAction( configureAction );

  QWidget *container = QWidget::createWindowContainer( canvas );
  container->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
  Qgs3DDebugWidget *debugWidget = new Qgs3DDebugWidget( canvas );
  debugWidget->setMapSettings( canvas->mapSettings() );
  debugWidget->setVisible( false );
  QObject::connect( canvas->mapSettings(), &Qgs3DMapSettings::showDebugPanelChanged, windowWidget, [=]( const bool enabled ) {
    debugWidget->setVisible( enabled );
  } );

  // Connect the camera to the debug widget.
  QObject::connect( canvas->cameraController(), &QgsCameraController::cameraChanged, debugWidget, &Qgs3DDebugWidget::updateFromCamera );
  QObject::connect( canvas->cameraController()->camera(), &Qt3DRender::QCamera::nearPlaneChanged, debugWidget, &Qgs3DDebugWidget::updateFromCamera );
  QObject::connect( canvas->cameraController()->camera(), &Qt3DRender::QCamera::farPlaneChanged, debugWidget, &Qgs3DDebugWidget::updateFromCamera );
  QObject::connect( toggleDebugPanel, &QAction::toggled, windowWidget, [=]( const bool enabled ) {
    debugWidget->setVisible( enabled );
  } );

  // construct the layout of sandbox
  QVBoxLayout *vLayout = new QVBoxLayout;
  vLayout->setContentsMargins( 0, 0, 0, 0 );
  vLayout->setSpacing( 0 );
  vLayout->addWidget( toolBar );
  QHBoxLayout *hLayout = new QHBoxLayout;
  vLayout->addLayout( hLayout );
  hLayout->setContentsMargins( 0, 0, 0, 0 );
  hLayout->addWidget( container );
  hLayout->addWidget( debugWidget );


  windowWidget->resize( 800, debugWidget->height() );
  windowWidget->setLayout( vLayout );
  windowWidget->show();

  return myApp.exec();
}

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
#include "qgs3dmapcanvas.h"
#include "qgs3dmapscene.h"
#include "qgs3dmapsettings.h"
#include "qgsapplication.h"
#include "qgsflatterraingenerator.h"
#include "qgslayertree.h"
#include "qgsmapsettings.h"
#include "qgspointcloudlayer.h"
#include "qgspointcloudlayer3drenderer.h"
#include "qgspointlightsettings.h"
#include "qgsproject.h"
#include "qgsprojectelevationproperties.h"
#include "qgsprojectviewsettings.h"
#include "qgsterrainprovider.h"
#include "qgstiledscenelayer.h"
#include "qgstiledscenelayer3drenderer.h"
#include "qgs3ddebugwidget.h"

#include <QHBoxLayout>
#include <QScreen>
#include <QToolBar>
#include <qgisapp.h>

void initCanvas3D( Qgs3DMapCanvas *canvas )
{
  QgsLayerTree *root = QgsProject::instance()->layerTreeRoot();
  const QList< QgsMapLayer * > visibleLayers = root->checkedLayers();

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
  QObject::connect( QgsProject::instance(), &QgsProject::transformContextChanged, map, [map]
  {
    map->setTransformContext( QgsProject::instance()->transformContext() );
  } );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map->crs() );
  map->setTerrainGenerator( flatTerrain );
  map->setTerrainElevationOffset( QgsProject::instance()->elevationProperties()->terrainProvider()->offset() );

  QgsPointLightSettings defaultPointLight;
  defaultPointLight.setPosition( QgsVector3D( 0, 0, 1000 ) );
  defaultPointLight.setConstantAttenuation( 0 );
  map->setLightSources( {defaultPointLight.clone() } );
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
  const float dist = static_cast< float >( std::max( extent.width(), extent.height() ) );
  canvas->setViewFromTop( extent.center(), dist * 2, 0 );

  QObject::connect( canvas->scene(), &Qgs3DMapScene::totalPendingJobsCountChanged, canvas, [canvas]
  {
    qDebug() << "pending jobs:" << canvas->scene()->totalPendingJobsCount();
  } );

  qDebug() << "pending jobs:" << canvas->scene()->totalPendingJobsCount();
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

  // a hack to assign 3D renderer
  for ( QgsMapLayer *layer : QgsProject::instance()->layerTreeRoot()->checkedLayers() )
  {
    if ( QgsPointCloudLayer *pcLayer = qobject_cast<QgsPointCloudLayer *>( layer ) )
    {
      QgsPointCloudLayer3DRenderer *r = new QgsPointCloudLayer3DRenderer();
      r->setLayer( pcLayer );
      r->resolveReferences( *QgsProject::instance() );
      pcLayer->setRenderer3D( r );
    }

    if ( QgsTiledSceneLayer *tsLayer = qobject_cast<QgsTiledSceneLayer *>( layer ) )
    {
      QgsTiledSceneLayer3DRenderer *r = new QgsTiledSceneLayer3DRenderer();
      r->setLayer( tsLayer );
      r->resolveReferences( *QgsProject::instance() );
      tsLayer->setRenderer3D( r );
    }
  }

  Qgs3DMapCanvas *canvas = new Qgs3DMapCanvas;
  initCanvas3D( canvas );

  // set up the UI
  QWidget *windowWidget = new QWidget;
  QVBoxLayout *vLayout = new QVBoxLayout;
  vLayout->setContentsMargins( 0, 0, 0, 0 );
  vLayout->setSpacing( 0 );

  QToolBar *toolBar = new QToolBar( windowWidget );
  toolBar->setIconSize( QgisApp::instance()->iconSize( true ) );
  toolBar->addAction( QIcon( QgsApplication::iconPath( "mActionZoomFullExtent.svg" ) ), QStringLiteral( "Reset camera to default position" ), windowWidget, [canvas]
  {
    canvas->resetView();
  } );
  QAction *toggleDebugPanel = toolBar->addAction(
                                QgsApplication::getThemeIcon( QStringLiteral( "/propertyicons/general.svg" ) ),
                                QStringLiteral( "Toggle on-screen Debug panel" ) );
  toggleDebugPanel->setCheckable( true );
  vLayout->addWidget( toolBar );

  QWidget *container = QWidget::createWindowContainer( canvas );
  container->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
  Qgs3DDebugWidget *debugWidget = new Qgs3DDebugWidget( canvas );
  debugWidget->setMapSettings( canvas->mapSettings() );
  debugWidget->setVisible( false );

  // Connect the camera to the debug widget.
  QObject::connect( canvas->cameraController(), &QgsCameraController::cameraChanged, debugWidget, &Qgs3DDebugWidget::updateFromCamera );
  QObject::connect( canvas->cameraController()->camera(), &Qt3DRender::QCamera::nearPlaneChanged, debugWidget, &Qgs3DDebugWidget::updateFromCamera );
  QObject::connect( canvas->cameraController()->camera(), &Qt3DRender::QCamera::farPlaneChanged, debugWidget, &Qgs3DDebugWidget::updateFromCamera );
  QObject::connect( toggleDebugPanel, &QAction::toggled, windowWidget, [ = ]( const bool enabled )
  {
    debugWidget->setVisible( enabled );
  } );

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

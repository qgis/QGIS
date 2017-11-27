/***************************************************************************
  qgs3dmapscene.cpp
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

#include "qgs3dmapscene.h"

#include <Qt3DRender/QCamera>
#include <Qt3DRender/QMesh>
#include <Qt3DRender/QPickingSettings>
#include <Qt3DRender/QRenderSettings>
#include <Qt3DRender/QSceneLoader>
#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QSkyboxEntity>
#include <Qt3DLogic/QFrameAction>

#include <QTimer>

#include "qgsaabb.h"
#include "qgs3dmapsettings.h"
#include "qgs3dutils.h"
#include "qgsabstract3drenderer.h"
#include "qgscameracontroller.h"
#include "qgschunkedentity_p.h"
#include "qgschunknode_p.h"
#include "qgsterrainentity_p.h"
#include "qgsterraingenerator.h"
#include "qgsvectorlayer.h"


Qgs3DMapScene::Qgs3DMapScene( const Qgs3DMapSettings &map, Qt3DExtras::QForwardRenderer *defaultFrameGraph, Qt3DRender::QRenderSettings *renderSettings, Qt3DRender::QCamera *camera, const QRect &viewportRect, Qt3DCore::QNode *parent )
  : Qt3DCore::QEntity( parent )
  , mMap( map )
  , mForwardRenderer( defaultFrameGraph )
{

  connect( &map, &Qgs3DMapSettings::backgroundColorChanged, this, &Qgs3DMapScene::onBackgroundColorChanged );
  onBackgroundColorChanged();

  // TODO: strange - setting OnDemand render policy still keeps QGIS busy (Qt 5.9.0)
  // actually it is more busy than with the default "Always" policy although there are no changes in the scene.
  //renderSettings->setRenderPolicy( Qt3DRender::QRenderSettings::OnDemand );

#if QT_VERSION >= 0x050900
  // we want precise picking of terrain (also bounding volume picking does not seem to work - not sure why)
  renderSettings->pickingSettings()->setPickMethod( Qt3DRender::QPickingSettings::TrianglePicking );
#endif

  // Camera
  float aspectRatio = ( float )viewportRect.width() / viewportRect.height();
  camera->lens()->setPerspectiveProjection( 45.0f, aspectRatio, 10.f, 10000.0f );

  mFrameAction = new Qt3DLogic::QFrameAction();
  connect( mFrameAction, &Qt3DLogic::QFrameAction::triggered,
           this, &Qgs3DMapScene::onFrameTriggered );
  addComponent( mFrameAction ); // takes ownership

  // Camera controlling
  mCameraController = new QgsCameraController( this ); // attaches to the scene
  mCameraController->setViewport( viewportRect );
  mCameraController->setCamera( camera );
  mCameraController->resetView( 1000 );

  // create terrain entity

  createTerrain();
  connect( &map, &Qgs3DMapSettings::terrainGeneratorChanged, this, &Qgs3DMapScene::createTerrain );
  connect( &map, &Qgs3DMapSettings::terrainVerticalScaleChanged, this, &Qgs3DMapScene::createTerrain );
  connect( &map, &Qgs3DMapSettings::mapTileResolutionChanged, this, &Qgs3DMapScene::createTerrain );
  connect( &map, &Qgs3DMapSettings::maxTerrainScreenErrorChanged, this, &Qgs3DMapScene::createTerrain );
  connect( &map, &Qgs3DMapSettings::maxTerrainGroundErrorChanged, this, &Qgs3DMapScene::createTerrain );

  // create entities of renderers

  Q_FOREACH ( const QgsAbstract3DRenderer *renderer, map.renderers() )
  {
    Qt3DCore::QEntity *newEntity = renderer->createEntity( map );
    newEntity->setParent( this );
  }

  // listen to changes of layers in order to add/remove 3D renderer entities
  connect( &map, &Qgs3DMapSettings::layersChanged, this, &Qgs3DMapScene::onLayersChanged );

  Qt3DCore::QEntity *lightEntity = new Qt3DCore::QEntity;
  Qt3DCore::QTransform *lightTransform = new Qt3DCore::QTransform;
  lightTransform->setTranslation( QVector3D( 0, 1000, 0 ) );
  // defaults: white, intensity 0.5
  // attenuation: constant 1.0, linear 0.0, quadratic 0.0
  Qt3DRender::QPointLight *light = new Qt3DRender::QPointLight;
  light->setConstantAttenuation( 0 );
  //light->setColor(Qt::white);
  //light->setIntensity(0.5);
  lightEntity->addComponent( light );
  lightEntity->addComponent( lightTransform );
  lightEntity->setParent( this );


#if 0
  ChunkedEntity *testChunkEntity = new ChunkedEntity( AABB( -500, 0, -500, 500, 100, 500 ), 2.f, 3.f, 7, new TestChunkLoaderFactory );
  testChunkEntity->setEnabled( false );
  testChunkEntity->setParent( this );
  chunkEntities << testChunkEntity;
#endif

  connect( mCameraController, &QgsCameraController::cameraChanged, this, &Qgs3DMapScene::onCameraChanged );
  connect( mCameraController, &QgsCameraController::viewportChanged, this, &Qgs3DMapScene::onCameraChanged );

#if 0
  // experiments with loading of existing 3D models.

  // scene loader only gets loaded only when added to a scene...
  // it loads everything: geometries, materials, transforms, lights, cameras (if any)
  Qt3DCore::QEntity *loaderEntity = new Qt3DCore::QEntity;
  Qt3DRender::QSceneLoader *loader = new Qt3DRender::QSceneLoader;
  loader->setSource( QUrl( "file:///home/martin/Downloads/LowPolyModels/tree.dae" ) );
  loaderEntity->addComponent( loader );
  loaderEntity->setParent( this );

  // mesh loads just geometry as one geometry...
  // so if there are different materials (e.g. colors) used in the model, this information is lost
  Qt3DCore::QEntity *meshEntity = new Qt3DCore::QEntity;
  Qt3DRender::QMesh *mesh = new Qt3DRender::QMesh;
  mesh->setSource( QUrl( "file:///home/martin/Downloads/LowPolyModels/tree.obj" ) );
  meshEntity->addComponent( mesh );
  Qt3DExtras::QPhongMaterial *material = new Qt3DExtras::QPhongMaterial;
  material->setAmbient( Qt::red );
  meshEntity->addComponent( material );
  Qt3DCore::QTransform *meshTransform = new Qt3DCore::QTransform;
  meshTransform->setScale( 1 );
  meshEntity->addComponent( meshTransform );
  meshEntity->setParent( this );
#endif

  if ( map.hasSkyboxEnabled() )
  {
    Qt3DExtras::QSkyboxEntity *skybox = new Qt3DExtras::QSkyboxEntity;
    skybox->setBaseName( map.skyboxFileBase() );
    skybox->setExtension( map.skyboxFileExtension() );
    skybox->setParent( this );

    // docs say frustum culling must be disabled for skybox.
    // it _somehow_ works even when frustum culling is enabled with some camera positions,
    // but then when zoomed in more it would disappear - so let's keep frustum culling disabled
    defaultFrameGraph->setFrustumCullingEnabled( false );
  }

  // force initial update of chunked entities
  onCameraChanged();
}

void Qgs3DMapScene::viewZoomFull()
{
  QgsRectangle extent = mMap.terrainGenerator()->extent();
  float side = qMax( extent.width(), extent.height() );
  mCameraController->resetView( side );  // assuming FOV being 45 degrees
}

QgsChunkedEntity::SceneState _sceneState( QgsCameraController *cameraController )
{
  Qt3DRender::QCamera *camera = cameraController->camera();
  QgsChunkedEntity::SceneState state;
  state.cameraFov = camera->fieldOfView();
  state.cameraPos = camera->position();
  QRect rect = cameraController->viewport();
  state.screenSizePx = qMax( rect.width(), rect.height() ); // TODO: is this correct?
  state.viewProjectionMatrix = camera->projectionMatrix() * camera->viewMatrix();
  return state;
}

void Qgs3DMapScene::onCameraChanged()
{
  Q_FOREACH ( QgsChunkedEntity *entity, mChunkEntities )
  {
    if ( entity->isEnabled() )
      entity->update( _sceneState( mCameraController ) );
  }

  // Update near and far plane from the terrain.
  // this needs to be done with great care as we have kind of circular dependency here:
  // active nodes are culled based on the current frustum (which involves near + far plane)
  // and then based on active nodes we set near and far plane.
  //
  // All of this is just heuristics assuming that all other stuff is being rendered somewhere
  // around the area where the terrain is.
  //
  // Near/far plane is setup in order to make best use of the depth buffer to avoid:
  // 1. precision errors - if the range is too great
  // 2. unwanted clipping of scene - if the range is too small

  if ( mTerrain )
  {
    Qt3DRender::QCamera *camera = cameraController()->camera();
    QMatrix4x4 viewMatrix = camera->viewMatrix();
    float fnear = 1e9;
    float ffar = 0;

    QList<QgsChunkNode *> activeNodes = mTerrain->activeNodes();

    // it could be that there are no active nodes - they could be all culled or because root node
    // is not yet loaded - we still need at least something to understand bounds of our scene
    // so lets use the root node
    if ( activeNodes.isEmpty() )
      activeNodes << mTerrain->rootNode();

    Q_FOREACH ( QgsChunkNode *node, activeNodes )
    {
      // project each corner of bbox to camera coordinates
      // and determine closest and farthest point.
      QgsAABB bbox = node->bbox();
      for ( int i = 0; i < 8; ++i )
      {
        QVector4D p( ( ( i >> 0 ) & 1 ) ? bbox.xMin : bbox.xMax,
                     ( ( i >> 1 ) & 1 ) ? bbox.yMin : bbox.yMax,
                     ( ( i >> 2 ) & 1 ) ? bbox.zMin : bbox.zMax, 1 );
        QVector4D pc = viewMatrix * p;

        float dst = -pc.z();  // in camera coordinates, x grows right, y grows down, z grows to the back
        if ( dst < fnear )
          fnear = dst;
        if ( dst > ffar )
          ffar = dst;
      }
    }
    if ( fnear < 1 )
      fnear = 1;  // does not really make sense to use negative far plane (behind camera)

    if ( fnear == 1e9 && ffar == 0 )
    {
      // the update didn't work out... this should not happen
      // well at least temprarily use some conservative starting values
      qDebug() << "oops... this should not happen! couldn't determine near/far plane. defaulting to 1...1e9";
      fnear = 1;
      ffar = 1e9;
    }

    // set near/far plane - with some tolerance in front/behind expected near/far planes
    camera->setFarPlane( ffar * 2 );
    camera->setNearPlane( fnear / 2 );
  }
  else
    qDebug() << "no terrain - not setting near/far plane";

  //qDebug() << "camera near/far" << mCameraController->camera()->nearPlane() << mCameraController->camera()->farPlane();
}

void Qgs3DMapScene::onFrameTriggered( float dt )
{
  mCameraController->frameTriggered( dt );

  Q_FOREACH ( QgsChunkedEntity *entity, mChunkEntities )
  {
    if ( entity->isEnabled() && entity->needsUpdate() )
    {
      qDebug() << "need for update";
      entity->update( _sceneState( mCameraController ) );
    }
  }
}

void Qgs3DMapScene::createTerrain()
{
  if ( mTerrain )
  {
    mChunkEntities.removeOne( mTerrain );

    mTerrain->deleteLater();
    mTerrain = nullptr;
  }

  if ( !mTerrainUpdateScheduled )
  {
    // defer re-creation of terrain: there may be multiple invocations of this slot, so create the new entity just once
    QTimer::singleShot( 0, this, &Qgs3DMapScene::createTerrainDeferred );
    mTerrainUpdateScheduled = true;
  }
}

void Qgs3DMapScene::createTerrainDeferred()
{
  double tile0width = mMap.terrainGenerator()->extent().width();
  int maxZoomLevel = Qgs3DUtils::maxZoomLevel( tile0width, mMap.mapTileResolution(), mMap.maxTerrainGroundError() );

  mTerrain = new QgsTerrainEntity( maxZoomLevel, mMap );
  //mTerrain->setEnabled(false);
  mTerrain->setParent( this );

  if ( mMap.showTerrainBoundingBoxes() )
    mTerrain->setShowBoundingBoxes( true );

  mCameraController->addTerrainPicker( mTerrain->terrainPicker() );

  mChunkEntities << mTerrain;

  onCameraChanged();  // force update of the new terrain

  // make sure that renderers for layers are re-created as well
  Q_FOREACH ( QgsMapLayer *layer, mMap.layers() )
  {
    // remove old entity - if any
    removeLayerEntity( layer );

    // add new entity - if any 3D renderer
    addLayerEntity( layer );
  }

  mTerrainUpdateScheduled = false;
}

void Qgs3DMapScene::onBackgroundColorChanged()
{
  mForwardRenderer->setClearColor( mMap.backgroundColor() );
}

void Qgs3DMapScene::onLayerRenderer3DChanged()
{
  QgsMapLayer *layer = qobject_cast<QgsMapLayer *>( sender() );
  Q_ASSERT( layer );

  // remove old entity - if any
  removeLayerEntity( layer );

  // add new entity - if any 3D renderer
  addLayerEntity( layer );
}

void Qgs3DMapScene::onLayersChanged()
{
  QSet<QgsMapLayer *> layersBefore = QSet<QgsMapLayer *>::fromList( mLayerEntities.keys() );
  QList<QgsMapLayer *> layersAdded;
  Q_FOREACH ( QgsMapLayer *layer, mMap.layers() )
  {
    if ( !layersBefore.contains( layer ) )
    {
      layersAdded << layer;
    }
    else
    {
      layersBefore.remove( layer );
    }
  }

  // what is left in layersBefore are layers that have been removed
  Q_FOREACH ( QgsMapLayer *layer, layersBefore )
  {
    removeLayerEntity( layer );
  }

  Q_FOREACH ( QgsMapLayer *layer, layersAdded )
  {
    addLayerEntity( layer );
  }
}

void Qgs3DMapScene::addLayerEntity( QgsMapLayer *layer )
{
  QgsAbstract3DRenderer *renderer = layer->renderer3D();
  if ( renderer )
  {
    Qt3DCore::QEntity *newEntity = renderer->createEntity( mMap );
    newEntity->setParent( this );
    mLayerEntities.insert( layer, newEntity );
  }

  connect( layer, &QgsMapLayer::renderer3DChanged, this, &Qgs3DMapScene::onLayerRenderer3DChanged );

  if ( layer->type() == QgsMapLayer::VectorLayer )
  {
    QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
    connect( vlayer, &QgsVectorLayer::selectionChanged, this, &Qgs3DMapScene::onLayerRenderer3DChanged );
  }
}

void Qgs3DMapScene::removeLayerEntity( QgsMapLayer *layer )
{
  Qt3DCore::QEntity *entity = mLayerEntities.take( layer );
  if ( entity )
    entity->deleteLater();

  disconnect( layer, &QgsMapLayer::renderer3DChanged, this, &Qgs3DMapScene::onLayerRenderer3DChanged );

  if ( layer->type() == QgsMapLayer::VectorLayer )
  {
    QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
    disconnect( vlayer, &QgsVectorLayer::selectionChanged, this, &Qgs3DMapScene::onLayerRenderer3DChanged );
  }
}

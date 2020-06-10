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
#include <Qt3DRender/QObjectPicker>
#include <Qt3DRender/QPickEvent>
#include <Qt3DRender/QPickingSettings>
#include <Qt3DRender/QPickTriangleEvent>
#include <Qt3DRender/QPointLight>
#include <Qt3DRender/QDirectionalLight>
#include <Qt3DRender/QRenderSettings>
#include <Qt3DRender/QSceneLoader>
#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QSkyboxEntity>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DLogic/QFrameAction>

#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QTimer>

#include "qgsaabb.h"
#include "qgsabstract3dengine.h"
#include "qgs3dmapscenepickhandler.h"
#include "qgs3dmapsettings.h"
#include "qgs3dutils.h"
#include "qgsabstract3drenderer.h"
#include "qgscameracontroller.h"
#include "qgschunkedentity_p.h"
#include "qgschunknode_p.h"
#include "qgseventtracing.h"
#include "qgsmeshlayer.h"
#include "qgsmeshlayer3drenderer.h"
#include "qgsrulebased3drenderer.h"
#include "qgsterrainentity_p.h"
#include "qgsterraingenerator.h"
#include "qgstessellatedpolygongeometry.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayer3drenderer.h"
#include "qgspoint3dbillboardmaterial.h"
#include "qgsmaplayertemporalproperties.h"

#include "qgslinematerial_p.h"

Qgs3DMapScene::Qgs3DMapScene( const Qgs3DMapSettings &map, QgsAbstract3DEngine *engine )
  : mMap( map )
  , mEngine( engine )
{

  connect( &map, &Qgs3DMapSettings::backgroundColorChanged, this, &Qgs3DMapScene::onBackgroundColorChanged );
  onBackgroundColorChanged();

  // TODO: strange - setting OnDemand render policy still keeps QGIS busy (Qt 5.9.0)
  // actually it is more busy than with the default "Always" policy although there are no changes in the scene.
  //mRenderer->renderSettings()->setRenderPolicy( Qt3DRender::QRenderSettings::OnDemand );

#if QT_VERSION >= 0x050900
  // we want precise picking of terrain (also bounding volume picking does not seem to work - not sure why)
  mEngine->renderSettings()->pickingSettings()->setPickMethod( Qt3DRender::QPickingSettings::TrianglePicking );
#endif

  QRect viewportRect( QPoint( 0, 0 ), mEngine->size() );

  // Camera
  float aspectRatio = ( float )viewportRect.width() / viewportRect.height();
  mEngine->camera()->lens()->setPerspectiveProjection( mMap.fieldOfView(), aspectRatio, 10.f, 10000.0f );

  mFrameAction = new Qt3DLogic::QFrameAction();
  connect( mFrameAction, &Qt3DLogic::QFrameAction::triggered,
           this, &Qgs3DMapScene::onFrameTriggered );
  addComponent( mFrameAction ); // takes ownership

  // Camera controlling
  mCameraController = new QgsCameraController( this ); // attaches to the scene
  mCameraController->setViewport( viewportRect );
  mCameraController->setCamera( mEngine->camera() );
  mCameraController->resetView( 1000 );

  addCameraViewCenterEntity( mEngine->camera() );

  // create terrain entity

  createTerrainDeferred();
  connect( &map, &Qgs3DMapSettings::terrainGeneratorChanged, this, &Qgs3DMapScene::createTerrain );
  connect( &map, &Qgs3DMapSettings::terrainVerticalScaleChanged, this, &Qgs3DMapScene::createTerrain );
  connect( &map, &Qgs3DMapSettings::mapTileResolutionChanged, this, &Qgs3DMapScene::createTerrain );
  connect( &map, &Qgs3DMapSettings::maxTerrainScreenErrorChanged, this, &Qgs3DMapScene::createTerrain );
  connect( &map, &Qgs3DMapSettings::maxTerrainGroundErrorChanged, this, &Qgs3DMapScene::createTerrain );
  connect( &map, &Qgs3DMapSettings::terrainShadingChanged, this, &Qgs3DMapScene::createTerrain );
  connect( &map, &Qgs3DMapSettings::pointLightsChanged, this, &Qgs3DMapScene::updateLights );
  connect( &map, &Qgs3DMapSettings::directionalLightsChanged, this, &Qgs3DMapScene::updateLights );
  connect( &map, &Qgs3DMapSettings::fieldOfViewChanged, this, &Qgs3DMapScene::updateCameraLens );
  connect( &map, &Qgs3DMapSettings::renderersChanged, this, &Qgs3DMapScene::onRenderersChanged );

  // create entities of renderers

  onRenderersChanged();

  // listen to changes of layers in order to add/remove 3D renderer entities
  connect( &map, &Qgs3DMapSettings::layersChanged, this, &Qgs3DMapScene::onLayersChanged );

  updateLights();

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
    mEngine->setFrustumCullingEnabled( false );

    // cppcheck wrongly believes skyBox will leak
    // cppcheck-suppress memleak
  }

  // force initial update of chunked entities
  onCameraChanged();
}

void Qgs3DMapScene::viewZoomFull()
{
  QgsRectangle extent = mMap.terrainGenerator()->extent();
  float side = std::max( extent.width(), extent.height() );
  mCameraController->resetView( side );  // assuming FOV being 45 degrees
}

int Qgs3DMapScene::terrainPendingJobsCount() const
{
  return mTerrain ? mTerrain->pendingJobsCount() : 0;
}

int Qgs3DMapScene::totalPendingJobsCount() const
{
  int count = 0;
  for ( QgsChunkedEntity *entity : qgis::as_const( mChunkEntities ) )
    count += entity->pendingJobsCount();
  return count;
}

void Qgs3DMapScene::registerPickHandler( Qgs3DMapScenePickHandler *pickHandler )
{
  if ( mPickHandlers.isEmpty() )
  {
    // we need to add object pickers
    for ( Qt3DCore::QEntity *entity : mLayerEntities.values() )
    {
      if ( QgsChunkedEntity *chunkedEntity = qobject_cast<QgsChunkedEntity *>( entity ) )
        chunkedEntity->setPickingEnabled( true );
    }
  }

  mPickHandlers.append( pickHandler );
}

void Qgs3DMapScene::unregisterPickHandler( Qgs3DMapScenePickHandler *pickHandler )
{
  mPickHandlers.removeOne( pickHandler );

  if ( mPickHandlers.isEmpty() )
  {
    // we need to remove pickers
    for ( Qt3DCore::QEntity *entity : mLayerEntities.values() )
    {
      if ( QgsChunkedEntity *chunkedEntity = qobject_cast<QgsChunkedEntity *>( entity ) )
        chunkedEntity->setPickingEnabled( false );
    }
  }
}

void Qgs3DMapScene::onLayerEntityPickedObject( Qt3DRender::QPickEvent *pickEvent, QgsFeatureId fid )
{
  QgsMapLayer *layer = mLayerEntities.key( qobject_cast<QgsChunkedEntity *>( sender() ) );
  if ( !layer )
    return;

  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
  if ( !vlayer )
    return;

  for ( Qgs3DMapScenePickHandler *pickHandler : qgis::as_const( mPickHandlers ) )
  {
    pickHandler->handlePickOnVectorLayer( vlayer, fid, pickEvent->worldIntersection(), pickEvent );
  }
}


float Qgs3DMapScene::worldSpaceError( float epsilon, float distance )
{
  Qt3DRender::QCamera *camera = mCameraController->camera();
  float fov = camera->fieldOfView();
  QRect rect = mCameraController->viewport();
  float screenSizePx = std::max( rect.width(), rect.height() ); // TODO: is this correct?

  // in qgschunkedentity_p.cpp there is inverse calculation (world space error to screen space error)
  // with explanation of the math.
  float frustumWidthAtDistance = 2 * distance * tan( fov / 2 );
  float err = frustumWidthAtDistance * epsilon / screenSizePx;
  return err;
}

QgsChunkedEntity::SceneState _sceneState( QgsCameraController *cameraController )
{
  Qt3DRender::QCamera *camera = cameraController->camera();
  QgsChunkedEntity::SceneState state;
  state.cameraFov = camera->fieldOfView();
  state.cameraPos = camera->position();
  QRect rect = cameraController->viewport();
  state.screenSizePx = std::max( rect.width(), rect.height() ); // TODO: is this correct?
  state.viewProjectionMatrix = camera->projectionMatrix() * camera->viewMatrix();
  return state;
}

void Qgs3DMapScene::onCameraChanged()
{
  updateScene();
  bool changedCameraPlanes = updateCameraNearFarPlanes();

  if ( changedCameraPlanes )
  {
    // repeat update of entities - because we have updated camera's near/far planes,
    // the active nodes may have changed as well
    updateScene();
    updateCameraNearFarPlanes();
  }
}

void Qgs3DMapScene::updateScene()
{
  QgsEventTracing::addEvent( QgsEventTracing::Instant, QStringLiteral( "3D" ), QStringLiteral( "Update Scene" ) );

  for ( QgsChunkedEntity *entity : qgis::as_const( mChunkEntities ) )
  {
    if ( entity->isEnabled() )
      entity->update( _sceneState( mCameraController ) );
  }

  updateSceneState();
}

bool Qgs3DMapScene::updateCameraNearFarPlanes()
{
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
      // well at least temporarily use some conservative starting values
      qDebug() << "oops... this should not happen! couldn't determine near/far plane. defaulting to 1...1e9";
      fnear = 1;
      ffar = 1e9;
    }

    // set near/far plane - with some tolerance in front/behind expected near/far planes
    float newFar = ffar * 2;
    float newNear = fnear / 2;
    if ( !qgsFloatNear( newFar, camera->farPlane() ) || !qgsFloatNear( newNear, camera->nearPlane() ) )
    {
      camera->setFarPlane( newFar );
      camera->setNearPlane( newNear );
      return true;
    }
  }
  else
    qDebug() << "no terrain - not setting near/far plane";

  return false;
}

void Qgs3DMapScene::onFrameTriggered( float dt )
{
  mCameraController->frameTriggered( dt );

  for ( QgsChunkedEntity *entity : qgis::as_const( mChunkEntities ) )
  {
    if ( entity->isEnabled() && entity->needsUpdate() )
    {
      qDebug() << "need for update";
      entity->update( _sceneState( mCameraController ) );
    }
  }

  updateSceneState();
}

void Qgs3DMapScene::createTerrain()
{
  if ( mTerrain )
  {
    mChunkEntities.removeOne( mTerrain );

    mTerrain->deleteLater();
    mTerrain = nullptr;

    emit terrainEntityChanged();
  }

  if ( !mTerrainUpdateScheduled )
  {
    // defer re-creation of terrain: there may be multiple invocations of this slot, so create the new entity just once
    QTimer::singleShot( 0, this, &Qgs3DMapScene::createTerrainDeferred );
    mTerrainUpdateScheduled = true;
    setSceneState( Updating );
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

  mCameraController->setTerrainEntity( mTerrain );

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

  connect( mTerrain, &QgsChunkedEntity::pendingJobsCountChanged, this, &Qgs3DMapScene::totalPendingJobsCountChanged );
  connect( mTerrain, &QgsTerrainEntity::pendingJobsCountChanged, this, &Qgs3DMapScene::terrainPendingJobsCountChanged );

  emit terrainEntityChanged();
}

void Qgs3DMapScene::onBackgroundColorChanged()
{
  mEngine->setClearColor( mMap.backgroundColor() );
}


void Qgs3DMapScene::updateLights()
{
  for ( Qt3DCore::QEntity *entity : qgis::as_const( mLightEntities ) )
    entity->deleteLater();
  mLightEntities.clear();

  const auto newPointLights = mMap.pointLights();
  for ( const QgsPointLightSettings &pointLightSettings : newPointLights )
  {
    Qt3DCore::QEntity *lightEntity = new Qt3DCore::QEntity;
    Qt3DCore::QTransform *lightTransform = new Qt3DCore::QTransform;
    lightTransform->setTranslation( QVector3D( pointLightSettings.position().x(),
                                    pointLightSettings.position().y(),
                                    pointLightSettings.position().z() ) );

    Qt3DRender::QPointLight *light = new Qt3DRender::QPointLight;
    light->setColor( pointLightSettings.color() );
    light->setIntensity( pointLightSettings.intensity() );

    light->setConstantAttenuation( pointLightSettings.constantAttenuation() );
    light->setLinearAttenuation( pointLightSettings.linearAttenuation() );
    light->setQuadraticAttenuation( pointLightSettings.quadraticAttenuation() );

    lightEntity->addComponent( light );
    lightEntity->addComponent( lightTransform );
    lightEntity->setParent( this );
    mLightEntities << lightEntity;
  }

  const auto newDirectionalLights = mMap.directionalLights();
  for ( const QgsDirectionalLightSettings &directionalLightSettings : newDirectionalLights )
  {
    Qt3DCore::QEntity *lightEntity = new Qt3DCore::QEntity;
    Qt3DCore::QTransform *lightTransform = new Qt3DCore::QTransform;

    Qt3DRender::QDirectionalLight *light = new Qt3DRender::QDirectionalLight;
    light->setColor( directionalLightSettings.color() );
    light->setIntensity( directionalLightSettings.intensity() );
    QgsVector3D direction = directionalLightSettings.direction();
    light->setWorldDirection( QVector3D( direction.x(), direction.y(), direction.z() ) );

    lightEntity->addComponent( light );
    lightEntity->addComponent( lightTransform );
    lightEntity->setParent( this );
    mLightEntities << lightEntity;
  }
}

void Qgs3DMapScene::updateCameraLens()
{
  mEngine->camera()->lens()->setFieldOfView( mMap.fieldOfView() );
  onCameraChanged();
}

void Qgs3DMapScene::onRenderersChanged()
{
  // remove entities (if any)
  qDeleteAll( mRenderersEntities );
  mRenderersEntities.clear();

  // re-add entities from new set of renderers
  const QList<QgsAbstract3DRenderer *> renderers = mMap.renderers();
  for ( const QgsAbstract3DRenderer *renderer : renderers )
  {
    Qt3DCore::QEntity *newEntity = renderer->createEntity( mMap );
    if ( newEntity )
    {
      newEntity->setParent( this );
      finalizeNewEntity( newEntity );
      mRenderersEntities[renderer] = newEntity;
    }
  }
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
  QSet<QgsMapLayer *> layersBefore = qgis::listToSet( mLayerEntities.keys() );
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

void Qgs3DMapScene::updateTemporal()
{
  for ( auto layer : mLayerEntities.keys() )
  {
    if ( layer->temporalProperties()->isActive() )
    {
      removeLayerEntity( layer );
      addLayerEntity( layer );
    }
  }
}

void Qgs3DMapScene::addLayerEntity( QgsMapLayer *layer )
{
  bool needsSceneUpdate = false;
  QgsAbstract3DRenderer *renderer = layer->renderer3D();
  if ( renderer )
  {
    // Fix vector layer's renderer to make sure the renderer is pointing to its layer.
    // It has happened before that renderer pointed to a different layer (probably after copying a style).
    // This is a bit of a hack and it should be handled in QgsMapLayer::setRenderer3D() but in qgis_core
    // the vector layer 3D renderer classes are not available.
    if ( layer->type() == QgsMapLayerType::VectorLayer &&
         ( renderer->type() == QLatin1String( "vector" ) || renderer->type() == QLatin1String( "rulebased" ) ) )
    {
      static_cast<QgsAbstractVectorLayer3DRenderer *>( renderer )->setLayer( static_cast<QgsVectorLayer *>( layer ) );
    }
    else if ( layer->type() == QgsMapLayerType::MeshLayer && renderer->type() == QLatin1String( "mesh" ) )
    {
      QgsMeshLayer3DRenderer *meshRenderer = static_cast<QgsMeshLayer3DRenderer *>( renderer );
      meshRenderer->setLayer( static_cast<QgsMeshLayer *>( layer ) );

      // Before entity creation, set the maximum texture size
      // Not very clean, but for now, only place found in the workflow to do that simple
      QgsMesh3DSymbol *sym = new QgsMesh3DSymbol( *meshRenderer->symbol() );
      sym->setMaximumTextureSize( maximumTextureSize() );
      meshRenderer->setSymbol( sym );
    }

    Qt3DCore::QEntity *newEntity = renderer->createEntity( mMap );
    if ( newEntity )
    {
      newEntity->setParent( this );
      mLayerEntities.insert( layer, newEntity );

      finalizeNewEntity( newEntity );

      if ( QgsChunkedEntity *chunkedNewEntity = qobject_cast<QgsChunkedEntity *>( newEntity ) )
      {
        mChunkEntities.append( chunkedNewEntity );
        needsSceneUpdate = true;

        chunkedNewEntity->setPickingEnabled( !mPickHandlers.isEmpty() );
        connect( chunkedNewEntity, &QgsChunkedEntity::pickedObject, this, &Qgs3DMapScene::onLayerEntityPickedObject );

        connect( chunkedNewEntity, &QgsChunkedEntity::newEntityCreated, this, [this]( Qt3DCore::QEntity * entity )
        {
          finalizeNewEntity( entity );
        } );

        connect( chunkedNewEntity, &QgsChunkedEntity::pendingJobsCountChanged, this, &Qgs3DMapScene::totalPendingJobsCountChanged );
      }
    }
  }

  if ( needsSceneUpdate )
    onCameraChanged();   // needed for chunked entities

  connect( layer, &QgsMapLayer::renderer3DChanged, this, &Qgs3DMapScene::onLayerRenderer3DChanged );

  if ( layer->type() == QgsMapLayerType::VectorLayer )
  {
    QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
    connect( vlayer, &QgsVectorLayer::selectionChanged, this, &Qgs3DMapScene::onLayerRenderer3DChanged );
  }

  if ( layer->type() == QgsMapLayerType::MeshLayer )
  {
    connect( layer, &QgsMapLayer::rendererChanged, this, &Qgs3DMapScene::onLayerRenderer3DChanged );
  }

}

void Qgs3DMapScene::removeLayerEntity( QgsMapLayer *layer )
{
  Qt3DCore::QEntity *entity = mLayerEntities.take( layer );

  if ( QgsChunkedEntity *chunkedEntity = qobject_cast<QgsChunkedEntity *>( entity ) )
  {
    mChunkEntities.removeOne( chunkedEntity );
  }

  if ( entity )
    entity->deleteLater();

  disconnect( layer, &QgsMapLayer::renderer3DChanged, this, &Qgs3DMapScene::onLayerRenderer3DChanged );

  if ( layer->type() == QgsMapLayerType::VectorLayer )
  {
    QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
    disconnect( vlayer, &QgsVectorLayer::selectionChanged, this, &Qgs3DMapScene::onLayerRenderer3DChanged );
  }

  if ( layer->type() == QgsMapLayerType::MeshLayer )
  {
    disconnect( layer, &QgsMapLayer::rendererChanged, this, &Qgs3DMapScene::onLayerRenderer3DChanged );
  }
}

void Qgs3DMapScene::finalizeNewEntity( Qt3DCore::QEntity *newEntity )
{
  // this is probably not the best place for material-specific configuration,
  // maybe this could be more generalized when other materials need some specific treatment
  for ( QgsLineMaterial *lm : newEntity->findChildren<QgsLineMaterial *>() )
  {
    connect( mCameraController, &QgsCameraController::viewportChanged, lm, [lm, this]
    {
      lm->setViewportSize( mCameraController->viewport().size() );
    } );

    lm->setViewportSize( cameraController()->viewport().size() );
  }
  // configure billboard's viewport when the viewport is changed.
  for ( QgsPoint3DBillboardMaterial *bm : newEntity->findChildren<QgsPoint3DBillboardMaterial *>() )
  {
    connect( mCameraController, &QgsCameraController::viewportChanged, bm, [bm, this]
    {
      bm->setViewportSize( mCameraController->viewport().size() );
    } );

    bm->setViewportSize( mCameraController->viewport().size() );
  }
}

int Qgs3DMapScene::maximumTextureSize() const
{
  QSurface *surface = mEngine->surface();
  QOpenGLContext context;
  context.create();
  context.makeCurrent( surface );
  QOpenGLFunctions openglFunctions( &context );
  GLint size;
  openglFunctions.glGetIntegerv( GL_MAX_TEXTURE_SIZE, &size );
  return int( size );
}

void Qgs3DMapScene::addCameraViewCenterEntity( Qt3DRender::QCamera *camera )
{
  mEntityCameraViewCenter = new Qt3DCore::QEntity;

  Qt3DCore::QTransform *trCameraViewCenter = new Qt3DCore::QTransform;
  mEntityCameraViewCenter->addComponent( trCameraViewCenter );
  connect( camera, &Qt3DRender::QCamera::viewCenterChanged, this, [trCameraViewCenter, camera]
  {
    trCameraViewCenter->setTranslation( camera->viewCenter() );
  } );

  Qt3DExtras::QPhongMaterial *materialCameraViewCenter = new Qt3DExtras::QPhongMaterial;
  materialCameraViewCenter->setAmbient( Qt::red );
  mEntityCameraViewCenter->addComponent( materialCameraViewCenter );

  Qt3DExtras::QSphereMesh *rendererCameraViewCenter = new Qt3DExtras::QSphereMesh;
  rendererCameraViewCenter->setRadius( 10 );
  mEntityCameraViewCenter->addComponent( rendererCameraViewCenter );

  mEntityCameraViewCenter->setEnabled( mMap.showCameraViewCenter() );
  mEntityCameraViewCenter->setParent( this );

  connect( &mMap, &Qgs3DMapSettings::showCameraViewCenterChanged, this, [this]
  {
    mEntityCameraViewCenter->setEnabled( mMap.showCameraViewCenter() );
  } );
}

void Qgs3DMapScene::setSceneState( Qgs3DMapScene::SceneState state )
{
  if ( mSceneState == state )
    return;
  mSceneState = state;
  emit sceneStateChanged();
}

void Qgs3DMapScene::updateSceneState()
{
  if ( mTerrainUpdateScheduled )
  {
    setSceneState( Updating );
    return;
  }

  for ( QgsChunkedEntity *entity : qgis::as_const( mChunkEntities ) )
  {
    if ( entity->isEnabled() && entity->pendingJobsCount() > 0 )
    {
      setSceneState( Updating );
      return;
    }
  }

  setSceneState( Ready );
}
